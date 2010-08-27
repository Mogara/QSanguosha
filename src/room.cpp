#include "room.h"
#include "engine.h"
#include "settings.h"
#include "gamerule.h"

#include <QStringList>
#include <QMessageBox>
#include <QHostAddress>
#include <QTimer>
#include <QMetaEnum>
#include <QTimerEvent>

static PassiveSkillSorter Sorter;

bool PassiveSkillSorter::operator ()(const PassiveSkill *a, const PassiveSkill *b){
    int x = a->getPriority(target);
    int y = b->getPriority(target);

    return x > y;
}

void PassiveSkillSorter::sort(QList<const PassiveSkill *> &skills){
    qSort(skills.begin(), skills.end(), *this);
}

DamageStruct::DamageStruct()
    :from(NULL), to(NULL), card(NULL), damage(0), nature(Normal)
{
}

Room::Room(QObject *parent, int player_count)
    :QObject(parent), player_count(player_count), current(NULL),
    pile1(Sanguosha->getRandomCards()),
    draw_pile(&pile1), discard_pile(&pile2), left_seconds(Config.CountDownSeconds),
    chosen_generals(0), game_started(false),
    waiting_func(NULL), nullificators_count(0), signup_count(0)
{
}

void Room::enqueueRecord(ActiveRecord *record){
    queue.enqueue(record);
}

ServerPlayer *Room::getCurrent() const{
    return current;
}

int Room::alivePlayerCount() const{
    return alive_players.count();
}

void Room::askForSkillInvoke(ServerPlayer *player, const QVariant &data){
    player->invoke("askForSkillInvoke", data.toString());

    waiting_func = __func__;
}

void Room::askForNullification(ServerPlayer *, const QVariant &data){
    broadcastInvoke("askForNullification", data.toString());

    waiting_func = __func__;

    int index = alive_players.indexOf(current), i;
    for(i=index; i<alive_players.count(); i++)
        nullificators << alive_players.at(i);
    for(i=0; i<index; i++)
        nullificators << alive_players.at(i);

    Q_ASSERT(nullificators.length() == alive_players.length());

    nullificators_count = alive_players.length();
}

void Room::askForCardChosen(ServerPlayer *player, const QVariant &data){
    player->invoke("askForCardChosen", data.toString());

    waiting_func = __func__;
}

void Room::requestForCard(ServerPlayer *player, const QVariant &data){
    player->invoke("requestForCard", data.toString());

    waiting_func = __func__;
}

void Room::setPlayerFlag(ServerPlayer *player, const QVariant &flag){
    QString flag_str = flag.toString();
    player->setFlags(flag_str);
    broadcast(QString("#%1 flags %1").arg(flag_str));
}

void Room::setPlayerProperty(ServerPlayer *player, const QVariant &data){
    QVariantList list = data.toList();
    const char *property_name = list.at(0).toByteArray().data();
    QVariant value = list.at(1);
    player->setProperty(property_name, value);

    broadcast(QString("#%1 %2 %3").arg(player->objectName()).arg(property_name).arg(value.toString()));
}

void Room::useCard(ServerPlayer *player, const QVariant &data){
    invokePassiveSkills(CardUsed, player, data);
}

void Room::addSocket(QTcpSocket *socket){
    ServerPlayer *player = new ServerPlayer(this);
    player->setSocket(socket);
    players << player;

    connect(player, SIGNAL(disconnected()), this, SLOT(reportDisconnection()));
    connect(player, SIGNAL(request_got(QString)), this, SLOT(processRequest(QString)));
}

bool Room::isFull() const
{
    return signup_count == player_count;
}

void Room::broadcast(const QString &message, ServerPlayer *except){
    foreach(ServerPlayer *player, players){
        if(player != except){
            player->unicast(message);
        }
    }
}

int Room::drawCard(){
    if(draw_pile->isEmpty()){
        Q_ASSERT(!discard_pile->isEmpty());
        qSwap(draw_pile, discard_pile);
        int n = draw_pile->count(), i;
        for(i=0; i<n; i++){
            int r1 = qrand() % n;
            int r2 = qrand() % n;
            draw_pile->swap(r1, r2);
        }
    }
    return draw_pile->takeFirst();
}

void Room::timerEvent(QTimerEvent *event){
    if(left_seconds > 0){
        left_seconds --;
        broadcast("! startInXs " + QString::number(left_seconds));
    }else{
        killTimer(event->timerId());
        assignRoles();
    }
}

void Room::reportDisconnection(){
    ServerPlayer *player = qobject_cast<ServerPlayer*>(sender());
    if(player){
        emit room_message(player->reportHeader() + tr("disconnected"));

        if(!player->objectName().isEmpty()){
            if(game_started){
                player->setState("offline");
                broadcast(QString("#%1 state %2").arg(player->objectName()).arg("offline"));
                player->setSocket(NULL);
                return;
            }else
                broadcast("! removePlayer " + player->objectName(), player);
        }

        player->setSocket(NULL);
        players.removeOne(player);
        if(!player->objectName().isEmpty())
            signup_count--;

        delete player;
    }
}

void Room::processRequest(const QString &request){
    QStringList args = request.split(" ");
    QString command = args.first();
    ServerPlayer *player = qobject_cast<ServerPlayer*>(sender());
    if(player == NULL)
        return;

    if(current && current != player){
        player->invoke("focusWarn", player->objectName());
        return;
    }

    command.append("Command");
    bool invoked = QMetaObject::invokeMethod(this,
                                             command.toAscii(),
                                             Qt::DirectConnection,
                                             Q_ARG(ServerPlayer *, player),
                                             Q_ARG(QStringList, args));
    if(invoked)
        emit room_message(player->reportHeader() + request);
    else
        emit room_message(QString("%1: %2 is not invokable").arg(player->reportHeader()).arg(command));

    if(game_started && !waiting_func)
        invokeStackTop();
}

void Room::signupCommand(ServerPlayer *player, const QStringList &args){
    QString name = args[1];
    QString avatar = args[2];

    if(findChild<ServerPlayer*>(name)){
        player->invoke("duplicationError");
        return;
    }

    player->setObjectName(name);
    player->setProperty("avatar", avatar);

    // introduce the new joined player to existing players except himself
    broadcast(QString("! addPlayer %1:%2").arg(name).arg(avatar), player);

    // introduce all existing player to the new joined
    foreach(ServerPlayer *p, players){
        if(p == player)
            continue;

        QString name = p->objectName();
        QString avatar = p->property("avatar").toString();

        player->invoke("addPlayer", QString("%1:%2").arg(name).arg(avatar));
    }

    signup_count ++;
    if(isFull()){
        broadcast(QString("! startInXs %1").arg(left_seconds));        
        startTimer(1000);
    }
}

void Room::assignRoles(){
    static const char *role_assign_table[] = {
        "",
        "",

        "ZF", // 2
        "ZFN", // 3
        "ZCFN", // 4
        "ZCFFN", // 5
        "ZCFFFN", // 6
        "ZCCFFFN", // 7
        "ZCCFFFFN", // 8
    };

    int n = players.count(), i;

    char roles[100];
    qstrcpy(roles, role_assign_table[n]);

    for(i=0; i<n; i++){
        int r1 = qrand() % n;
        int r2 = qrand() % n;

        qSwap(roles[r1], roles[r2]);
    }

    int lord_index = -1;
    for(i=0; i<n; i++){
        ServerPlayer *player = players[i];

        QString role;
        switch(roles[i]){
        case 'Z': role = "lord"; break;
        case 'C': role = "loyalist"; break;
        case 'F': role = "rebel"; break;
        case 'N': role = "renegade"; break;
        }

        Q_ASSERT(!role.isEmpty());

        player->setRole(role);
        if(role == "lord"){
            lord_index = i;
            broadcastProperty(player, "role", "lord");

            QStringList lord_list = Sanguosha->getRandomLords(Config.LordCount);
            player->invoke("getLords", lord_list.join("+"));
        }else
            player->sendProperty("role");
    }

    Q_ASSERT(lord_index != -1);
    players.swap(0, lord_index);

    for(i=0; i<players.length(); i++)
        players.at(i)->setSeat(i+1);
}

void Room::chooseCommand(ServerPlayer *player, const QStringList &args){
    QString general_name = args[1];
    player->setGeneralName(general_name);

    if(player->getRole() == "lord"){
        broadcastProperty(player, "general");

        static const int max_choice = 5;
        const int total = Sanguosha->getGeneralCount();
        const int choice_count = qMin(max_choice, (total-1) / (player_count-1));
        QStringList general_list = Sanguosha->getRandomGenerals((player_count-1) * choice_count + 1);
        general_list.removeOne(general_name);

        int i,j;
        for(i=1; i<player_count; i++){
            QStringList choices;
            for(j=0; j<choice_count; j++)
                choices << general_list[(i-1)*choice_count + j];

            players[i]->invoke("getGenerals", QString("%1+%2").arg(general_name).arg(choices.join("+")));
        }
    }

    chosen_generals ++;
    if(chosen_generals == player_count){
        startGame();
    }
}

void Room::useCardCommand(ServerPlayer *player, const QStringList &args){
    QString card_str = args.at(1);
    const Card *card = Card::Parse(card_str);

    if(card == NULL){
        emit room_message(tr("Card can not parse:\n %1").arg(card_str));
        return;
    }

    QList<ServerPlayer *> targets;
    if(args.at(2) != "."){
        QStringList target_names = args.at(2).split("+");

        foreach(QString target_name, target_names)
            targets << findChild<ServerPlayer *>(target_name);

    }

    CardUseStruct data;
    data.card = card;
    data.from = player;
    data.to = targets;

    invokePassiveSkills(CardUsed, player, QVariant::fromValue(data));
}

void Room::startGame(){
    // broadcast all generals except the lord
    int i;
    for(i=1; i<players.count(); i++)
        broadcastProperty(players.at(i), "general");

    // tell the players about the seat, and the first is always the lord
    QStringList player_circle;
    foreach(ServerPlayer *player, players){
        alive_players << player;
        player_circle << player->objectName();
    }
    broadcast("! arrangeSeats " + player_circle.join("+"));

    // set hp full state
    int lord_welfare = player_count > 4 ? 1 : 0;
    players.first()->setMaxHP(players.first()->getGeneralMaxHP() + lord_welfare);

    for(i=1; i<player_count; i++)
        players[i]->setMaxHP(players[i]->getGeneralMaxHP());

    foreach(ServerPlayer *player, players){
        player->setHp(player->getMaxHP());

        broadcastProperty(player, "max_hp");
        broadcastProperty(player, "hp");
    }

    // every player draw 4 cards and them start from the lord
    for(i=0; i<player_count; i++){
        drawCards(players.at(i), 4);
    }

    foreach(ServerPlayer *player, players){
        const General *general = player->getGeneral();

        QList<const PassiveSkill *> skills = general->findChildren<const PassiveSkill *>();
        foreach(const PassiveSkill *skill, skills){
            passive_skills.insert(skill->objectName(), skill);
        }        
    }
    GameRule *game_rule = new GameRule;
    passive_skills.insert(game_rule->objectName(), game_rule);

    // construct trigger_table
    foreach(const PassiveSkill *skill, passive_skills){
        QList<TriggerEvent> events;
        skill->getTriggerEvents(events);
        foreach(TriggerEvent event, events){
            trigger_table[event] << skill;
        }
    }

    broadcast("! startGame .");
    game_started = true;

    ServerPlayer *the_lord = players.first();
    the_lord->setPhase(Player::Start);
    broadcastProperty(the_lord, "phase", "start");
    current = the_lord;
    changePhase(the_lord);
    invokeStackTop();
}

void Room::broadcastProperty(ServerPlayer *player, const char *property_name, const QString &value){
    if(value.isNull()){
        QString real_value = player->property(property_name).toString();
        broadcast(QString("#%1 %2 %3").arg(player->objectName()).arg(property_name).arg(real_value));
    }else
        broadcast(QString("#%1 %2 %3").arg(player->objectName()).arg(property_name).arg(value));
}

void Room::drawCards(ServerPlayer *player, const QVariant &data){
    int n = data.toInt();
    QStringList cards_str;

    int i;
    for(i=0; i<n; i++){
        int card_id = drawCard();
        const Card *card = Sanguosha->getCard(card_id);
        player->drawCard(card);
        cards_str << QString::number(drawCard());
    }

    player->invoke("drawCards", cards_str.join("+"));
    broadcast(QString("! drawNCards %1:%2").arg(player->objectName()).arg(n), player);
}

void Room::throwCard(ServerPlayer *player, const Card *card){
    if(card->isVirtualCard()){
        QList<int> subcards = card->getSubcards();
        foreach(int subcard, subcards)
            throwCard(player, subcard);
    }else
        throwCard(player, card->getId());
}

void Room::throwCard(ServerPlayer *player, const QVariant &card_id){
    CardMoveStruct data;
    data.card_id = card_id.toInt();
    data.from_place = Player::Hand;
    data.to_place = Player::DiscardedPile;
    data.from = player;
    data.to = NULL;
    data.open = true;

    moveCard(NULL, QVariant::fromValue(data));
}

QList<int> *Room::getDiscardPile() const{
    return discard_pile;
}

void Room::moveCard(ServerPlayer *, const QVariant &data){
    if(!data.canConvert<CardMoveStruct>())
        return;

    CardMoveStruct move = data.value<CardMoveStruct>();
    broadcast("! moveCard " + move.toString());
    ServerPlayer::MoveCard(move);
}

QString CardMoveStruct::toString() const{
    static QMap<Player::Place, QString> place2str;
    if(place2str.isEmpty()){
        place2str.insert(Player::Hand, "hand");
        place2str.insert(Player::Equip, "equip");
        place2str.insert(Player::DelayedTrick, "delayed_trick");
        place2str.insert(Player::Special, "special");
        place2str.insert(Player::DiscardedPile, "_");
    }

    QString from_str = from ? from->objectName() : "_";
    QString to_str = to ? to->objectName() : "_";

    return QString("%1:%2@%3->%4@%5")
            .arg(card_id)
            .arg(from_str).arg(place2str.value(from_place, "_"))
            .arg(to_str).arg(place2str.value(to_place, "_"));
}

void Room::invokeSkillCommand(ServerPlayer *, const QStringList &args){
    // FIXME: checking waiting_func and stack top

    ActiveRecord *record = stack.pop();
    QString skill_name = record->data.toString();
    const PassiveSkill *skill = passive_skills.value(skill_name, NULL);
    if(skill){
        QString option = args.at(1);
        skill->onOption(record->target, option);
    }else{
        emit room_message(tr("No such skill named %1").arg(skill_name));
    }

    waiting_func = NULL;
}

void Room::replyNullificationCommand(ServerPlayer *player, const QStringList &args){
    int card_id = args.at(1).toInt();

    if(card_id == -1)
        nullificators.removeOne(player);
    nullificators_count --;

    if(nullificators_count == 0){
        waiting_func = NULL;

        if(nullificators.isEmpty()){
            // execute the trick
        }else{
            ServerPlayer *nullificator = nullificators.first();
            throwCard(nullificator, card_id);

            // FIXME
        }
    }
}

void Room::chooseCardCommand(ServerPlayer *player, const QStringList &args){
    // FIXME
}

void Room::nextPhase(ServerPlayer *player){
    Player::Phase next_phase = player->getNextPhase();

    if(next_phase == Player::NotActive){
        int index = alive_players.indexOf(player);
        int next_index = (index + 1) % alive_players.length();
        ServerPlayer *next = alive_players.at(next_index);
        // FIXME: if face is down, turn over it

        player->setPhase(Player::NotActive);
        next->setPhase(Player::Start);
        current = next;

        broadcastProperty(player, "phase", "not_active");
        broadcastProperty(next, "phase", "start");

        changePhase(next);
    }else{
        player->setPhase(next_phase);        
        broadcastProperty(player, "phase", player->getPhaseString());

        changePhase(player);
    }
}

void Room::playSkillEffect(const QString &skill_name, int index){
    broadcastInvoke("playSkillEffect", QString("%1:%2").arg(skill_name).arg(index));
}

void Room::invokeStackTop(){
    while(!stack.isEmpty()){
        ActiveRecord *top = stack.pop();

#ifndef QT_NO_DEBUG
        if(top->target)
            qDebug("pop (%s %s %s)", top->method, qPrintable(top->target->objectName()), qPrintable(top->data.toString()));
        else
            qDebug("pop (%s NULL %s)", top->method, qPrintable(top->data.toString()));
#endif

        bool invoked;

        if(top->data.isValid())
            invoked = QMetaObject::invokeMethod(this,
                                                top->method,
                                                Qt::DirectConnection,
                                                Q_ARG(ServerPlayer *, top->target),
                                                Q_ARG(QVariant, top->data)
                                                );
        else
            invoked = QMetaObject::invokeMethod(this,
                                                top->method,
                                                Qt::DirectConnection,
                                                Q_ARG(ServerPlayer *, top->target)
                                                );

        if(!invoked)
            emit room_message(tr("Unknown method :%1 ").arg(top->method));

        if(waiting_func)
            return;

        delete top;
    }

    if(current && !waiting_func)
        activate(current);
}

void Room::invokePassiveSkills(TriggerEvent event, ServerPlayer *target, const QVariant &data){
    QList<const PassiveSkill *> skills, triggers = trigger_table[event];

    foreach(const PassiveSkill *skill, triggers){
        if(skill->triggerable(target))
            skills << skill;
    }

    Sorter.target = target;
    Sorter.sort(skills);

    queue.clear();
    foreach(const PassiveSkill *skill, skills){
        if(skill->trigger(event, target, data))
            break;
    }

    QListIterator<ActiveRecord *> itor(queue);
    itor.toBack();
    while(itor.hasPrevious()){
        stack.push(itor.previous());
    }
    queue.clear();    
}

void Room::changePhase(ServerPlayer *target){
    invokePassiveSkills(PhaseChange, target);
}

void Room::predamage(ServerPlayer *target, const DamageStruct &data){
    // FIXME
}

void Room::damage(ServerPlayer *target, const DamageStruct &data){
    // FIXME
}

void Room::broadcastInvoke(const char *method, const QString &arg){
    broadcast(QString("! %1 %2").arg(method).arg(arg));
}

void Room::activate(ServerPlayer *target){
    broadcastInvoke("activate", target->objectName());
}
