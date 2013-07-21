#include "generaloverview.h"
#include "engine.h"
#include "generalmodel.h"
#include "audio.h"

#include <QFileInfo>
#include <QtDebug>
#include <QWebFrame>
#include <QWebPluginFactory>
#include <QLineEdit>
#include <QCompleter>
#include <QAbstractItemView>

class GeneralWebPage: public QWebPage{
public:
    GeneralWebPage(){

    }

    virtual void javaScriptConsoleMessage(const QString &message, int lineNumber, const QString &sourceID){
        qDebug("Error: message: %s, linenumber = %d, source ID = %s", qPrintable(message), lineNumber, qPrintable(sourceID));
    }
};

Q_GLOBAL_STATIC(GeneralModel, GlobalGeneralModel)

class GeneralPluginFactory: public QWebPluginFactory{
public:
    GeneralPluginFactory(){

    }

    virtual QList<Plugin> plugins() const{
        return QList<Plugin>();
    }

    virtual QObject *create(const QString &mimeType, const QUrl &, const QStringList &, const QStringList &) const{
        if(mimeType == "application/x-searchbox"){
            QLineEdit *edit = new QLineEdit;

            QCompleter *completer = new QCompleter(GlobalGeneralModel());
            completer->popup()->setIconSize(General::TinyIconSize);
            completer->setCaseSensitivity(Qt::CaseInsensitive);

            edit->setCompleter(completer);

            return edit;
        }else
            return NULL;
    }
};

GeneralOverview::GeneralOverview()
{
    setPage(new GeneralWebPage);

    load(QUrl::fromLocalFile(QFileInfo("web/generals.html").absoluteFilePath()));

    connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(addSelfToFrame()));

    page()->mainFrame()->addToJavaScriptWindowObject("sgs", this);
    page()->setPluginFactory(new GeneralPluginFactory);

    settings()->setAttribute(QWebSettings::PluginsEnabled, true);

    resize(900, 600);
}

QString GeneralOverview::translate(const QString &key){
    return Sanguosha->translate(key, "");
}

QStringList GeneralOverview::getKingdoms() const{
    return Sanguosha->getKingdoms();
}

QStringList GeneralOverview::getPackages() const
{
    QList<const Package *> packages = Sanguosha->findChildren<const Package *>();
    QStringList packageNames;
    foreach(const Package *package, packages){
        if(package->getType() == Package::GeneralPack || package->getType() == Package::MixedPack)
            packageNames << package->objectName();
    }

    return packageNames;
}

QStringList GeneralOverview::getGenerals(const QVariantMap &options) const
{
    QHash<QString, const General *> list = Sanguosha->getGenerals();

    QString kingdom = options["kingdom"].toString();
    QString package = options["package"].toString();
    QString gender = options["gender"].toString();
    QString type = options["type"].toString();
    QString lordtype = options["lordtype"].toString();

    QStringList results;
    QHashIterator<QString, const General *> itor(list);
    while(itor.hasNext()){
        const General *g = itor.next().value();

        if(!kingdom.isEmpty() && g->getKingdom() != kingdom)
            continue;

        if(!package.isEmpty() && g->getPackage() != package)
            continue;

        if(!gender.isEmpty() && g->getGenderString() != gender)
            continue;

        if(!type.isEmpty()){
            if(type == "civilian" && g->getMaxHp() > 3)
                continue;

            if(type == "commander" && g->getMaxHp() < 4)
                continue;
        }

        if(!lordtype.isEmpty()){
            if(lordtype == "lord" && !g->isLord())
                continue;

            if(lordtype == "non-lord" && g->isLord())
                continue;
        }

        results << itor.key();
    }

    qSort(results);

    return results;
}

QObject *GeneralOverview::getGeneral(const QString &name) const
{
    const QObject *g = Sanguosha->getGeneral(name);
    if(g == NULL){
        const QObject *s = Sanguosha->getSkill(name);
        if(s)
            g = qobject_cast<const General *>(s->parent());
    }

    return const_cast<QObject *>(g);
}

QVariantList GeneralOverview::getLines(const QString &generalName) const
{
    const General *g = Sanguosha->getGeneral(generalName);
    QVariantList lines;

    if(g){
        foreach(const Skill *skill, g->getVisibleSkillList()){
            QStringList sources = skill->getSources();

            if(sources.isEmpty()){
                lines << Sanguosha->translate(skill->objectName());
                continue;
            }

            for(int i=0; i<sources.length(); i++){
                QFileInfo info(sources.at(i));

                QVariantMap map;
                if(sources.length() == 1){
                    map["name"] = Sanguosha->translate(skill->objectName());
                }else{
                    map["name"] = QString("%1%2").arg(Sanguosha->translate(skill->objectName())).arg(i+1);
                }

                map["word"] = Sanguosha->translate("$" + info.baseName());
                map["path"] = sources.at(i);

                lines << map;
            }
        }
    }

    return lines;
}

void GeneralOverview::play(const QString &path) const
{
    Audio::play(path);
}

void GeneralOverview::addSelfToFrame()
{
    page()->mainFrame()->addToJavaScriptWindowObject("sgs", this);
}


