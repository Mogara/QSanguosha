#include "clientlogbox.h"
#include "settings.h"
#include "engine.h"
#include "clientplayer.h"

#include <QPalette>

ClientLogBox::ClientLogBox(QWidget *parent) :
    QTextEdit(parent)
{
    setReadOnly(true);
}

void ClientLogBox::appendLog(
        const QString &type,
        const QString &from_general,
        const QStringList &tos,
        QString card_str,
        QString arg,
        QString arg2)
{
    QString from;
    if(!from_general.isEmpty()){
        from = Sanguosha->translate(from_general);
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

    if(type.startsWith("$")){
        const Card *card = Sanguosha->getCard(card_str.toInt());
        QString log_name = QString("<font color='white'>%1</font>").arg(card->getLogName());

        log = Sanguosha->translate(type);
        log.replace("%from", from);
        log.replace("%to", to);
        log.replace("%card", log_name);

        append(log);

        return;
    }

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
                    subcard_list << subcard->getLogName();
                }

                QString subcard_str = subcard_list.join(",");
                subcard_str = QString("<font color='white'>%1</font>").arg(subcard_str);

                if(subcard_list.isEmpty())
                    log = tr("%from use skill [%1], played [%2]").arg(skill_name).arg(card_name);
                else
                    log = tr("%from use skill [%1] use %2 as %3")
                          .arg(skill_name)
                          .arg(subcard_str)
                          .arg(card_name);
            }

            delete card;
        }else
            log = tr("%from use %1").arg(card_name);

        if(!to.isEmpty())
            log.append(tr(", target is %to"));

    }else
        log = Sanguosha->translate(type);

    log.replace("%from", from);
    log.replace("%to", to);

    if(!arg2.isEmpty()){
        arg2 = QString("<b>%1</b>").arg(Sanguosha->translate(arg2));
        log.replace("%arg2", arg2);
    }

    if(!arg.isEmpty()){
        arg = QString("<b>%1</b>").arg(Sanguosha->translate(arg));
        log.replace("%arg", arg);
    }

    log = QString("<font color='%2'>%1</font>").arg(log).arg(Config.TextEditColor.name());

    append(log);
}

void ClientLogBox::appendLog(const QString &log_str){
    QRegExp rx("([#$]\\w+):(\\w*)->([+\\w]*):(.*):(@?\\w*):(\\w*)");

    if(!rx.exactMatch(log_str)){
        append(tr("Log string is not well formatted: %1, error string is %2")
               .arg(log_str).arg(rx.errorString()));
        return;
    }

    QStringList texts = rx.capturedTexts();   

    QString type = texts.at(1);
    QString from = texts.at(2);
    QStringList tos;
    if(!texts.at(3).isEmpty())
        tos = texts.at(3).split("+");
    QString card_str = texts.at(4);
    QString arg = texts.at(5);
    QString arg2 = texts.at(6);

    appendLog(type, from, tos, card_str, arg, arg2);
}

void ClientLogBox::appendSeparator(){
    append("------------------------");
}
