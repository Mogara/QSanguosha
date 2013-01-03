#include "miniscenarios.h"

#include <QMessageBox>
#include <QFile>

MiniSceneRule::MiniSceneRule(Scenario *scenario)
    :ScenarioRule(scenario)
{
    events << GameStart << PhaseChange;
}

void MiniSceneRule::assign(QStringList &generals, QStringList &roles) const{
    for(int i=0;i<players.length();i++)
    {
        QMap<QString,QString> sp =players.at(i);
        QString name = sp["general"];
        if(name == "select")name = "sujiang";
        generals << name;
        roles << sp["role"];
    }
}

QStringList MiniSceneRule::existedGenerals() const
{
    QStringList names;
    for(int i=0;i<players.length();i++)
    {
        QMap<QString,QString> sp =players.at(i);
        QString name = sp["general"];
        if(name == "select")name = "sujiang";
        names << name;
        name = sp["general2"];
        if(name == NULL )continue;
        if(name == "select")name = "sujiang";
        names << name;
    }
    return names;
}

bool MiniSceneRule::trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const
{
    Room* room = player->getRoom();

    if(event == PhaseChange)
    {
        if(player == room->getTag("Starter").value<PlayerStar>()){
            if(player->getPhase() == Player::Start){
                room->setTag("Round", room->getTag("Round").toInt()+1);

                if(!ex_options["beforeStartRound"].isNull()){
                    if(ex_options["beforeStartRound"].toInt() == room->getTag("Round").toInt()){
                        room->gameOver(ex_options["beforeStartRoundWinner"].toString());
                    }
                }
            }
            else if(player->getPhase() == Player::NotActive){
                if(!ex_options["afterRound"].isNull()){
                    if(ex_options["afterRound"].toInt() == room->getTag("Round").toInt()){
                        room->gameOver(ex_options["afterRoundWinner"].toString());
                    }
                }
            }
        }

        if(player->getPhase()==Player::Start && this->players.first()["beforeNext"] != NULL){
            if(player->tag["playerHasPlayed"].toBool())
                room->gameOver(this->players.first()["beforeNext"]);
            else player->tag["playerHasPlayed"] = true;
        }

        if(player->getPhase() != Player::NotActive)return false;
        if(player->getState() == "robot" || this->players.first()["singleTurn"] == NULL)
            return false;
        room->gameOver(this->players.first()["singleTurn"]);
    }

    if(room->getTag("WaitForPlayer").toBool())
        return true;

    room->broadcastInvoke("animate", "lightbox:" + objectName() + ":2000");
    room->getThread()->delay(2000);
    LogMessage log;
    log.type = "#MiniSceneChanged";
    log.arg = id;
    log.arg2 = objectName();
    room->sendLog(log);
    log.type = QString("#mini_%1").arg(id);
    room->sendLog(log);

    QList<ServerPlayer*> players = room->getAllPlayers();
    while(players.first()->getState() == "robot")
        players.append(players.takeFirst());

    QStringList cards= setup.split(",");
    foreach(QString id,cards)
    {
        room->moveCardTo(Sanguosha->getCard(id.toInt()),NULL,Player::Special,true);
        room->moveCardTo(Sanguosha->getCard(id.toInt()),NULL,Player::DrawPile,true);
        room->broadcastInvoke("addHistory","pushPile");
    }

    int i=0, j=0;
    for(i = ex_options["randomRoles"].toString() == "true" ?
        qrand() % players.length() : 0, j = 0; j < players.length(); i++, j++)
    {
        i = i < players.length() ? i : i % players.length();
        ServerPlayer *sp = players.at(ex_options["randomRoles"].toString() == "true" ? j : i);

        room->setPlayerProperty(sp,"role",this->players.at(i)["role"]);

        QString general = this->players.at(i)["general"];
        {
            QString original = sp->getGeneralName();
            if(general == "select")
            {
                QStringList available,all,existed;
                existed = existedGenerals();
                all = Sanguosha->getRandomGenerals(Sanguosha->getGeneralCount());
                qShuffle(all);
                for(int i=0;i<5;i++)
                {
                    sp->setGeneral(NULL);
                    QString choice = sp->findReasonable(all);
                    if(existed.contains(choice))
                    {
                        all.removeOne(choice);
                        i--;
                        continue;
                    }
                    available << choice;
                    all.removeOne(choice);
                }
                general = room->askForGeneral(sp,available);
            }
            QString trans = QString("%1:%2").arg(original).arg(general);
            sp->invoke("transfigure",trans);
            room->setPlayerProperty(sp,"general",general);
        }
        general = this->players.at(i)["general2"];
        if(!general.isEmpty()){
            if(general == "select")
            {
                QStringList available,all,existed;
                existed = existedGenerals();
                all = Sanguosha->getRandomGenerals(Sanguosha->getGeneralCount());
                qShuffle(all);
                for(int i=0;i<5;i++)
                {
                    room->setPlayerProperty(sp,"general2", QVariant());
                    QString choice = sp->findReasonable(all);
                    if(existed.contains(choice))
                    {
                        all.removeOne(choice);
                        i--;
                        continue;
                    }
                    available << choice;
                    all.removeOne(choice);
                }
                general = room->askForGeneral(sp,available);
            }
            if(general == sp->getGeneralName())general = this->players.at(i)["general3"];
            QString trans = QString("%1:%2").arg("sujiang").arg(general);
            sp->invoke("transfigure",trans);
            room->setPlayerProperty(sp,"general2",general);
        }

        room->setPlayerProperty(sp,"kingdom",sp->getGeneral()->getKingdom());

        QString str = this->players.at(i)["maxhp"];
        if (str == NULL) str = QString::number(sp->getGeneralMaxHp());
        room->setPlayerProperty(sp,"maxhp",str.toInt());

        str = this->players.at(i)["hpadj"];
        if(str != NULL)
            room->setPlayerProperty(sp,"maxhp",sp->getMaxHp()+str.toInt());
        str=QString::number(sp->getMaxHp());

        QString str2 = this->players.at(i)["hp"];
        if(str2 != NULL)str = str2;
        room->setPlayerProperty(sp,"hp",str.toInt());

        str = this->players.at(i)["equip"];
        QStringList equips = str.split(",");
        foreach(QString equip,equips)
        {
            bool ok;
            equip.toInt(&ok);
            if(!ok)room->installEquip(sp,equip);
            else room->moveCardTo(Sanguosha->getCard(equip.toInt()),sp,Player::Equip);
        }

        str = this->players.at(i)["judge"];
        if(str != NULL)
        {
            QStringList judges = str.split(",");
            foreach(QString judge,judges)
            {
                room->moveCardTo(Sanguosha->getCard(judge.toInt()),sp,Player::Judging);
            }
        }

        str = this->players.at(i)["hand"];
        if(str !=NULL)
        {
            QStringList hands = str.split(",");
            foreach(QString hand,hands)
            {
                room->obtainCard(sp,hand.toInt());
            }

        }

        QVariant v;
        foreach(const TriggerSkill *skill, sp->getTriggerSkills()){
            if(!skill->inherits("SPConvertSkill"))
                room->getThread()->addTriggerSkill(skill);
            else continue;

            if(skill->getTriggerEvents().contains(GameStart))
                skill->trigger(GameStart, sp, v);
        }

        QString skills = this->players.at(i)["acquireSkills"];
        if(skills != NULL){
            foreach(QString skill_name, skills.split(","))
                room->acquireSkill(sp, skill_name);
        }

        if(this->players.at(i)["chained"] != NULL){
            sp->setChained(true);
            room->broadcastProperty(sp, "chained");
            room->setEmotion(sp, "chain");
        }
        if(this->players.at(i)["turned"] == "true"){
            if(sp->faceUp())
                sp->turnOver();
        }
        if(this->players.at(i)["starter"] != NULL){
            room->setCurrent(sp);
            QVariant data = QVariant::fromValue(sp);
            room->setTag("Starter", data);
        }
        if(this->players.at(i)["nationality"] != NULL){
            room->setPlayerProperty(sp, "kingdom", this->players.at(i)["nationality"]);
        }

        str = this->players.at(i)["draw"];
        if(str == NULL)str = "4";
        room->drawCards(sp,str.toInt());
        if(this->players.at(i)["marks"] != NULL)
        {
            QStringList marks = this->players.at(i)["marks"].split(",");
            foreach(QString qs,marks)
            {
                QStringList keys = qs.split("*");
                str = keys.at(1);
                room->setPlayerMark(sp, keys.at(0), str.toInt());
            }
        }
    }

    room->setTag("WaitForPlayer",QVariant(true));
    room->updateStateItem();
    return true;
}

void MiniSceneRule::addNPC(QString feature)
{
    QMap<QString, QString> player;
    QStringList features;
    if(feature.contains("|"))features= feature.split("|");
    else features = feature.split(" ");
    foreach(QString str, features)
    {
        QStringList keys = str.split(":");
        if(keys.size()<2)continue;
        if(keys.first().size()<1)continue;
        player.insert(keys.at(0),keys.at(1));
    }

    players << player;
}

void MiniSceneRule::setPile(QString cardList)
{
    setup = cardList;
}

void MiniSceneRule::setOptions(QStringList option){
    ex_options[option.first()] = option.last();
}

void MiniSceneRule::loadSetting(QString path)
{
    QFile file(path);
    if(file.open(QIODevice::ReadOnly)){
        players.clear();
        setup.clear();

        QTextStream stream(&file);
        while(!stream.atEnd()){
            QString aline = stream.readLine();
            if(aline.isEmpty()) continue;

            if(aline.startsWith("setPile"))
                setPile(aline.split(":").at(1));
            else if(aline.startsWith("extraOptions")){
                aline.remove("extraOptions:");
                QStringList options = aline.split(" ");
                foreach(QString option, options){
                    if(options.isEmpty()) continue;
                    QString key = option.split(":").first(), value = option.split(":").last();
                    ex_options[key] = QVariant::fromValue(value);
                }
            }
            else
                addNPC(aline);
        }
        file.close();
    }
}

MiniScene::MiniScene(const QString &name)
    :Scenario(name){
    rule = new MiniSceneRule(this);
}

void MiniScene::setupCustom(QString name) const
{
    if(name == NULL)
        name = "custom_scenario";

    MiniSceneRule* arule = qobject_cast<MiniSceneRule*>(this->getRule());
    arule->id = name;
    name.prepend("etc/customScenes/");
    name.append(".txt");
    arule->loadSetting(name);
}

#define ADD_CUSTOM_SCENARIO(name) static ScenarioAdder MiniScene##name##ScenarioAdder(QString("MiniScene_") + #name, new LoadedScenario(#name));

ADD_CUSTOM_SCENARIO(01)
ADD_CUSTOM_SCENARIO(02)
ADD_CUSTOM_SCENARIO(03)
ADD_CUSTOM_SCENARIO(04)
ADD_CUSTOM_SCENARIO(05)
ADD_CUSTOM_SCENARIO(06)
ADD_CUSTOM_SCENARIO(07)
ADD_CUSTOM_SCENARIO(08)
ADD_CUSTOM_SCENARIO(09)
ADD_CUSTOM_SCENARIO(10)
ADD_CUSTOM_SCENARIO(11)
ADD_CUSTOM_SCENARIO(12)
ADD_CUSTOM_SCENARIO(13)
ADD_CUSTOM_SCENARIO(14)
ADD_CUSTOM_SCENARIO(15)
ADD_CUSTOM_SCENARIO(16)
ADD_CUSTOM_SCENARIO(17)
ADD_CUSTOM_SCENARIO(18)
ADD_CUSTOM_SCENARIO(19)
ADD_CUSTOM_SCENARIO(20)
ADD_CUSTOM_SCENARIO(21)
ADD_CUSTOM_SCENARIO(22)
ADD_CUSTOM_SCENARIO(23)
ADD_CUSTOM_SCENARIO(24)
ADD_CUSTOM_SCENARIO(25)
ADD_CUSTOM_SCENARIO(26)
ADD_CUSTOM_SCENARIO(27)
ADD_CUSTOM_SCENARIO(28)
ADD_CUSTOM_SCENARIO(29)
ADD_CUSTOM_SCENARIO(30)
ADD_CUSTOM_SCENARIO(31)
ADD_CUSTOM_SCENARIO(32)
ADD_CUSTOM_SCENARIO(33)

ADD_SCENARIO(Custom)
