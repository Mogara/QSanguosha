#include "miniscenarios.h"

#include <QMessageBox>
#include <QFile>

const char* MiniScene::S_KEY_MINISCENE = "_mini_%1";
const char* MiniSceneRule::S_EXTRA_OPTION_LOSE_ON_DRAWPILE_DRAIN = "gameOverIfExtraCardDrawn";
const char* MiniSceneRule::S_EXTRA_OPTION_RANDOM_ROLES = "randomRoles";
const QString MiniSceneRule::_S_DEFAULT_HERO = "caocao";

MiniSceneRule::MiniSceneRule(Scenario *scenario)
    :ScenarioRule(scenario)
{
    events << GameStart << EventPhaseStart << FetchDrawPileCard;
}

void MiniSceneRule::assign(QStringList &generals, QStringList &roles) const{
    for(int i = 0; i < players.length(); i++)
    {
        QMap<QString,QString> sp = players.at(i);
        QString name = sp["general"];
        if (name == "select") name = _S_DEFAULT_HERO;
        generals << name;
        roles << sp["role"];
    }
}

bool MiniSceneRule::trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const
{
    if(triggerEvent == EventPhaseStart)
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

        if(player->getPhase()==Player::Start && this->players.first()["beforeNext"] != NULL
            )
        {
            if(player->tag["playerHasPlayed"].toBool())
                room->gameOver(this->players.first()["beforeNext"]);
            else player->tag["playerHasPlayed"] = true;
        }

        if(player->getPhase() != Player::NotActive)return false;
        if(player->getState() == "robot" || this->players.first()["singleTurn"] == NULL)
            return false;
        room->gameOver(this->players.first()["singleTurn"]);
        return true;
    } else if (triggerEvent == FetchDrawPileCard) {
        if (ex_options.contains(S_EXTRA_OPTION_LOSE_ON_DRAWPILE_DRAIN))
        {
            const QList<int>& drawPile = room->getDrawPile();
            foreach (int id, m_fixedDrawCards)
            {
                if (drawPile.contains(id))
                    return false;
            }
            room->gameOver(ex_options[S_EXTRA_OPTION_LOSE_ON_DRAWPILE_DRAIN].toString());
            return true;
        }
        return false;
    } else if (triggerEvent == GameStart) {
        if(room->getTag("WaitForPlayer").toBool())
            return true;

        QList<ServerPlayer*> players = room->getAllPlayers();
        while(players.first()->getState() == "robot")
            players.append(players.takeFirst());

        QList<int>& drawPile = room->getDrawPile();

        foreach(int id, m_fixedDrawCards)
        {
            if (drawPile.contains(id))
            {
                drawPile.removeOne(id);
                drawPile.prepend(id);
            }
            else
            {
                room->moveCardTo(Sanguosha->getCard(id), NULL, Player::DrawPile, true);
            }
            room->broadcastInvoke("addHistory","pushPile");
        }

        int i, j;
        for (i = ex_options["randomRoles"].toString() == "true" ?
            qrand() % players.length() : 0, j = 0; j < players.length(); i++, j++)
        {
            i = i < players.length() ? i : i % players.length();
            ServerPlayer *sp = players.at(ex_options["randomRoles"].toString() == "true" ? j : i);

            room->setPlayerProperty(sp,"role",this->players.at(i)["role"]);

            QString general = this->players[i]["general"];
            {
                if(general == "select")
                {
                    QStringList available, all, existed;
                    all = Sanguosha->getRandomGenerals(Sanguosha->getGeneralCount());
                    qShuffle(all);
                    for(int i = 0; i < 5; i++)
                    {
                        if(sp->getGeneral() != NULL){
                            foreach(const Skill* skill, sp->getGeneral()->getSkillList())
                                sp->loseSkill(skill->objectName());
                        }
                        sp->setGeneral(NULL);
                        QString choice = sp->findReasonable(all);
                        available << choice;
                        all.removeOne(choice);
                    }
                    general = room->askForGeneral(sp,available);
                }
                room->changeHero(sp, general, false, false, false, false);
            }
            general = this->players[i]["general2"];
            if(!general.isEmpty()){
                if(general == "select")
                {
                    QStringList available,all,existed;
                    all = Sanguosha->getRandomGenerals(Sanguosha->getGeneralCount());
                    qShuffle(all);
                    for(int i = 0; i < 5; i++)
                    {
                        if(sp->getGeneral2() != NULL){
                            foreach(const Skill* skill, sp->getGeneral2()->getSkillList())
                                sp->loseSkill(skill->objectName());
                        }
                        room->setPlayerProperty(sp,"general2", QVariant());
                        QString choice = sp->findReasonable(all);
                        available << choice;
                        all.removeOne(choice);
                    }
                    general = room->askForGeneral(sp,available);
                }
                if(general == sp->getGeneralName())general = this->players.at(i)["general3"];
                room->changeHero(sp, general, false, false, true, false);
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
                if (!ok) room->installEquip(sp,equip);
                else room->moveCardTo(Sanguosha->getCard(equip.toInt()), NULL, sp, Player::PlaceEquip, CardMoveReason(CardMoveReason::S_REASON_UNKNOWN, QString()));
            }

            str = this->players.at(i)["judge"];
            if(str != NULL)
            {
                QStringList judges = str.split(",");
                foreach(QString judge,judges)
                {
                    room->moveCardTo(Sanguosha->getCard(judge.toInt()),NULL,sp,Player::PlaceDelayedTrick, CardMoveReason(CardMoveReason::S_REASON_UNKNOWN, QString()));
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
                    skill->trigger(GameStart, room, sp, v);
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
            if(this->players[i]["nationality"] != NULL){
                room->setPlayerProperty(sp, "kingdom", this->players.at(i)["nationality"]);
            }

            str = this->players[i]["draw"];
            if (str == NULL) str = "4";
            room->drawCards(sp,str.toInt());
            if(this->players[i]["marks"] != NULL)
            {
                QStringList marks = this->players[i]["marks"].split(",");
                foreach(QString qs,marks)
                {
                    QStringList keys = qs.split("*");
                    str = keys[1];
                    room->setPlayerMark(sp, keys[0], str.toInt());
                }
            }
        }

        room->setTag("WaitForPlayer",QVariant(true));
        room->updateStateItem();
        return true;
    } else return false;
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
    QStringList cards= setup.split(",", QString::SkipEmptyParts);
    foreach(QString sid, cards)
    {
        bool ok;
        int id = sid.toInt(&ok);
        Q_ASSERT(ok);
        m_fixedDrawCards.append(id);
    }
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
    if(name == NULL)name = "custom_scenario";
    name.prepend("etc/customScenes/");
    name.append(".txt");

    MiniSceneRule* arule = qobject_cast<MiniSceneRule*>(this->getRule());
    arule->loadSetting(name);

}

void MiniScene::onTagSet(Room *room, const QString &key) const
{

}

