#include "clientlogbox.h"
#include "settings.h"
#include "engine.h"

#include <QPalette>

ClientLogBox::ClientLogBox(QWidget *parent) :
    QTextEdit(parent)
{
    QPalette palette;
    palette.setBrush(QPalette::Base, Config.BackgroundBrush);

    setPalette(palette);
    setReadOnly(true);
}

void ClientLogBox::appendLog(const QString &log_str){
    QRegExp rx("(#\\w+):(\\w+)->(\\w*):(.*):(\\w*):(\\w*)");

    if(!rx.exactMatch(log_str))
        return;

    QStringList texts = rx.capturedTexts();   

    QString type = texts.at(1);
    QString from = texts.at(2);
    QStringList tos;
    if(!texts.at(3).isEmpty())
        tos = texts.at(3).split("+");
    QString card_str = texts.at(4);
    QString arg = texts.at(5);
    QString arg2 = texts.at(6);

    if(!from.isEmpty()){
        from = Sanguosha->translate(from);
        from = QString("<b>%1</b>").arg(from);
    }

    QString to;
    if(!tos.isEmpty()){
        QStringList to_list;
        foreach(QString to, tos)
            to_list << Sanguosha->translate(to);
        to = to_list.join(",");
        arg = Sanguosha->translate(arg);

        to = QString("<b>%1</b>").arg(to);
    }

    QString log;

    if(!card_str.isEmpty()){
        const Card *card = Card::Parse(card_str);
        QString card_name = QString("<font color='white'>%1</font>").arg(card->getLogName());

        if(card->isVirtualCard()){
            QString skill_name = Sanguosha->translate(card->getSkillName());
            if(card->inherits("SkillCard")){            
                log = tr("%from use skill [%1]").arg(skill_name);
            }else{
                QList<int> card_ids = card->getSubcards();
                QStringList subcard_list;
                foreach(int card_id, card_ids){
                    const Card *subcard = Sanguosha->getCard(card_id);
                    subcard_list << subcard->getFullName(true);
                }

                QString subcard_str = subcard_list.join(",");
                subcard_str = QString("<font color='white'>%1</font>").arg(subcard_str);

                log = tr("%from use skill [%1] use %2 as %3")
                      .arg(skill_name)
                      .arg(subcard_str)
                      .arg(card_name);
            }
        }else
            log = tr("%from use %1").arg(card_name);

        if(!to.isEmpty())
            log.append(tr(", target is %to"));

        // delete card;
    }else
        log = Sanguosha->translate(type);

    log.replace("%from", from);
    log.replace("%to", to);

    if(!arg2.isEmpty()){
        arg2 = Sanguosha->translate(arg2);
        log.replace("%arg2", arg2);
    }

    if(!arg.isEmpty()){
        arg = Sanguosha->translate(arg);
        log.replace("%arg", arg);
    }

    append(log);
}
