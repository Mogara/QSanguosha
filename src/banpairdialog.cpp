#include "banpairdialog.h"
#include "engine.h"
#include "choosegeneraldialog.h"

#include <QSet>
#include <QFile>
#include <QTextStream>
#include <QHBoxLayout>
#include <QPushButton>

static QSet<BanPair> BanPairSet;
static const QString BanPairFilename = "banpairs.txt";
static QSet<QString> AllBanSet;
static QSet<QString> SecondBanSet;

BanPair::BanPair(){

}

BanPair::BanPair(const QString &first, const QString &second)
    :QPair<QString, QString>(first, second)
{
    if(first > second){
        qSwap(this->first, this->second);
    }
}

Q_DECLARE_METATYPE(BanPair);

bool BanPair::isBanned(const QString &first, const QString &second){
    if(SecondBanSet.contains(second))
        return true;

    if(AllBanSet.contains(first) || AllBanSet.contains(second))
        return true;

    BanPair pair(first, second);
    return BanPairSet.contains(pair);    
}

void BanPair::loadBanPairs(){
    // special cases
    AllBanSet << "shencaocao" << "dongzhuo";
    SecondBanSet << "jiangboyue" << "luboyan";

    QFile file(BanPairFilename);
    if(file.open(QIODevice::ReadOnly)){
        QTextStream stream(&file);

        while(!stream.atEnd()){
            QString line = stream.readLine();
            QStringList names = line.split(" ");
            if(names.length() != 2)
                continue;

            QString first = names.at(0);
            QString second = names.at(1);

            BanPair pair(first, second);
            BanPairSet.insert(pair);
        }
    }
}

void BanPair::saveBanPairs(){
    QFile file(BanPairFilename);
    if(file.open(QIODevice::WriteOnly)){
        QTextStream stream(&file);

        foreach(BanPair pair, BanPairSet)
            stream << pair.first << " " << pair.second << "\n";
    }
}

BanPairDialog::BanPairDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("Ban pair table"));

    list = new QListWidget;

    foreach(BanPair pair, BanPairSet)
        addPairToList(pair);

    QPushButton *add_button = new QPushButton(tr("Add"));
    QPushButton *remove_button = new QPushButton(tr("Remove"));
    QPushButton *save_button = new QPushButton(tr("Save"));

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    hlayout->addWidget(add_button);
    hlayout->addWidget(remove_button);
    hlayout->addWidget(save_button);

    connect(add_button, SIGNAL(clicked()), this, SLOT(addPair()));
    connect(remove_button, SIGNAL(clicked()), this, SLOT(removePair()));
    connect(save_button, SIGNAL(clicked()), this, SLOT(save()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(list);
    layout->addLayout(hlayout);

    setLayout(layout);
}

void BanPairDialog::addPairToList(const BanPair &pair){
    QString first = Sanguosha->translate(pair.first);
    QString second = Sanguosha->translate(pair.second);

    QListWidgetItem *item = new QListWidgetItem(QString("%1 + %2").arg(first).arg(second));
    item->setData(Qt::UserRole, QVariant::fromValue(pair));
    list->addItem(item);
}

void BanPairDialog::addPair(){
    FreeChooseDialog *chooser = new FreeChooseDialog(this, true);
    connect(chooser, SIGNAL(pair_chosen(QString,QString)),
            this, SLOT(addPair(QString,QString)));

    chooser->exec();
}

void BanPairDialog::addPair(const QString &first, const QString &second){
    BanPair pair(first, second);
    if(!BanPairSet.contains(pair)){
        BanPairSet.insert(pair);
        addPairToList(pair);
    }
}

void BanPairDialog::removePair(){
    QListWidgetItem *item = list->currentItem();
    if(item){
        list->takeItem(list->currentRow());
        BanPair pair = item->data(Qt::UserRole).value<BanPair>();
        BanPairSet.remove(pair);
    }
}

void BanPairDialog::save(){
    BanPair::saveBanPairs();
}

