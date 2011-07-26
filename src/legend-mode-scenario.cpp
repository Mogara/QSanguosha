
#include "legend-mode-scenario.h"
#include "engine.h"
#include "clientplayer.h"
#include "client.h"
#include "carditem.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCommandLinkButton>
#include <QFile>
#include <QTextStream>

class LegendRule: public ScenarioRule{
public:

    LegendRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameStart;


    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

                room->acquireSkill(player, "chuanqi");
        return false;
    }
private:
};


void LegendScenario::onTagSet(Room *room, const QString &key) const{
    // dummy
}

bool LegendScenario::generalSelection() const{
    return true;
}

int LegendScenario::getPlayerCount() const{
    return 8;
}

void LegendScenario::getRoles(char *roles) const{
    strcpy(roles, "ZCCFFFFN");
}

bool LegendScenario::exposeRoles() const{
    return false;
}

void LegendScenario::assign(QStringList &generals, QStringList &roles) const{

    roles <<"lord"<<"loyalist"<<"loyalist"
             <<"rebel"<<"rebel"<<"rebel"<<"rebel"
                <<"renegade";
    qShuffle(roles);
}

ChuanqiDialog *ChuanqiDialog::GetInstance(){
    static ChuanqiDialog *instance;
    if(instance == NULL)
        instance = new ChuanqiDialog;

    return instance;
}

ChuanqiDialog::ChuanqiDialog()
{
    setWindowTitle(tr("Chuanqi"));

    group = new QButtonGroup(this);

    QHBoxLayout *layout = new QHBoxLayout;
    int code=ChuanqiCard::card_map.value(Self->getGeneralName());
    int thresh=ChuanqiCard::thresh_map.value(Self->getGeneralName());
    layout->addWidget(createSelection(code,thresh,1));
    code=ArcChuanqiCard::card_map.value(Self->getGeneralName());
    thresh=ArcChuanqiCard::thresh_map.value(Self->getGeneralName());
    layout->addWidget(createSelection(code,thresh,2));

    setLayout(layout);

    connect(group, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(selectCard(QAbstractButton*)));
}

void ChuanqiDialog::popup(){

    Self->tag["SelectedChuanqi"]=0;
    foreach(QAbstractButton *button, group->buttons()){
        if(button->objectName()=="0")
                button->setEnabled(true);
       else if(button->objectName()=="1")
        {
            int req=ChuanqiCard::thresh_map.value(Self->getGeneralName());
            int cur=Self->getMark("@chuanqi");
            button->setEnabled((cur>=req) && (req>0));
        }
        else if(button->objectName()=="2")
         {
             int req=ArcChuanqiCard::thresh_map.value(Self->getGeneralName());
             int cur=Self->getMark("@chuanqi");
             button->setEnabled((cur>=req) && (req>0));
         }
        else button->setEnabled(false);
    }
    exec();
}

void ChuanqiDialog::selectCard(QAbstractButton *button){
    int x= button->objectName().toInt();
    Self->tag["SelectedChuanqi"] =x;
    accept();
}

QGroupBox *ChuanqiDialog::createSelection(int code, int thresh,int id){
    QGroupBox *box = new QGroupBox;

    QString name="..";
    if(code>=0)name=Sanguosha->getCard(code)->objectName();
    box->setTitle(QString("%1").arg(thresh));

    QVBoxLayout *layout = new QVBoxLayout;

    layout->addWidget(createButton(code,id));

    layout->addStretch();

    box->setLayout(layout);
    return box;
}

QAbstractButton *ChuanqiDialog::createButton(int code,int id){
    QString name;
    if(code<0)name=Sanguosha->translate("Cancel");
    else name=Sanguosha->translate(Sanguosha->getCard(code)->objectName());
    QCommandLinkButton *button = new QCommandLinkButton(name);
    button->setObjectName(QString("%1").arg(id));
    button->setToolTip(name);

    group->addButton(button);

    return button;
}

QMap<QString,int> ChuanqiCard::card_map;
QMap<QString,int> ChuanqiCard::thresh_map;

ChuanqiCard::ChuanqiCard(){
    target_fixed = true;
    will_throw=false;
    if(card_map.size()<1)loadChuanqiConfig();
}

void ChuanqiCard::loadChuanqiConfig()
{
    QFile file("etc/ChuanqiModeConfig.txt");
    if(file.open(QIODevice::ReadOnly)){
        QTextStream stream(&file);
        while(!stream.atEnd()){
            QString name;
            stream >> name;

            int a,b,c,d;

            stream >> a >> b >> c >> d;

            ChuanqiCard::card_map.insert(name,a-1);
            ChuanqiCard::thresh_map.insert(name,b);
            ArcChuanqiCard::card_map.insert(name,c-1);
            ArcChuanqiCard::thresh_map.insert(name,d);
            }

        file.close();
    }
}


void ChuanqiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{

    int code=card_map.value(source->getGeneralName());
    if(!(room->getCardPlace(code)==Player::DiscardedPile))
    {
        LogMessage log;
        log.type = "#ChuanqiUnavailable";
        log.arg = Sanguosha->getCard(card_map.value(source->getGeneralName()))->objectName();
        room->sendLog(log);
        return;
    }
        int marks=source->getMark("@chuanqi");
        int cost= thresh_map.value(source->getGeneralName());
        room->setPlayerMark(source,"@chuanqi",marks-cost);
        room->obtainCard(source,code);

        const Card* c=Sanguosha->getCard(this->subcards.first());
        int suit=c->getSuit();
        QString kingdom=source->getKingdom();

        static QMap<QString,int> amap;
        if(amap.isEmpty()){
            amap.insert("wu",Card::Diamond);
            amap.insert("shu",Card::Heart);
            amap.insert("wei",Card::Spade);
            amap.insert("qun",Card::Club);
        }

        if(amap.value(kingdom)!=suit)room->throwCard(this);
        else room->showCard(source,this->subcards.first());
}

QMap<QString,int> ArcChuanqiCard::card_map;
QMap<QString,int> ArcChuanqiCard::thresh_map;


ArcChuanqiCard::ArcChuanqiCard(){
    target_fixed = true;
    will_throw=false;
    if(card_map.size()<1)loadChuanqiConfig();
}

void ArcChuanqiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{

    int code=card_map.value(source->getGeneralName());
    if(!(room->getCardPlace(code)==Player::DiscardedPile))
    {
        LogMessage log;
        log.type = "#ChuanqiUnavailable";
        log.arg = Sanguosha->getCard(card_map.value(source->getGeneralName()))->objectName();
        room->sendLog(log);
        return;
    }
        room->setPlayerMark(source,"@chuanqi",0);
        room->obtainCard(source,code);

        const Card* c=Sanguosha->getCard(this->subcards.first());
        int suit=c->getSuit();
        QString kingdom=source->getKingdom();

        static QMap<QString,int> amap;
        if(amap.isEmpty()){
            amap.insert("wu",Card::Diamond);
            amap.insert("shu",Card::Heart);
            amap.insert("wei",Card::Spade);
            amap.insert("qun",Card::Club);
        }

        if(amap.value(kingdom)!=suit)room->throwCard(this);
        else room->showCard(source,this->subcards.first());
}

class ChuanqiViewAs: public OneCardViewAsSkill{
public:
    ChuanqiViewAs():OneCardViewAsSkill("chuanqi"){

    }
    virtual const Card *viewAs(CardItem *card_item) const{
        if(Self->tag["SelectedChuanqi"].toInt()==1)
        {
            ChuanqiCard* c=new ChuanqiCard();
            c->addSubcard(card_item->getCard());
            return c;
        }
        else if(Self->tag["SelectedChuanqi"].toInt()==2)
        {
            ArcChuanqiCard* c=new ArcChuanqiCard();
            c->addSubcard(card_item->getCard());
            return c;
        }
        return NULL;
    }
    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }
};

class Chuanqi: public TriggerSkill{
public:
    Chuanqi():TriggerSkill("chuanqi"){

                view_as_skill = new ChuanqiViewAs;
        events << PhaseChange ;


    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *hero, QVariant &data) const{
        if(hero->getPhase() == Player::Start){
                        hero->gainMark("@chuanqi",hero->getHandcardNum());
                        return false;
        }
        return false;
    }
    virtual QDialog *getDialog() const{
            new ChuanqiCard;
            new ArcChuanqiCard;
            return ChuanqiDialog::GetInstance();
        }
};

LegendScenario::LegendScenario()
    :Scenario("legend_mode")
{
    rule = new LegendRule(this);

    skills<< new Chuanqi;

    addMetaObject<ChuanqiCard>();
    addMetaObject<ArcChuanqiCard>();
}

ADD_SCENARIO(Legend)
