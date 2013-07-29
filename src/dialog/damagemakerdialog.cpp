#include "damagemakerdialog.h"
#include "dialogutil.h"
#include "client.h"
#include "engine.h"
#include "clientplayerlistmodel.h"

#include <QListView>
#include <QAbstractListModel>
#include <QGroupBox>

DamageMakerOperationsModel::DamageMakerOperationsModel(QObject *parent)
    :QAbstractListModel(parent)
{

}

int DamageMakerOperationsModel::rowCount(const QModelIndex &parent) const{
    return parent.isValid() ? 0 : OperationCount;
}

QVariant DamageMakerOperationsModel::data(const QModelIndex &index, int role) const{
    int i = index.row();
    if(i < 0 || i>= OperationCount)
        return QVariant();

    Operation op = static_cast<Operation>(i);
    if(role == Qt::DisplayRole){
        switch(op){
        case Normal: return tr("Normal");
        case Thunder: return tr("Thunder");
        case Fire: return tr("Fire");
        case Recover: return tr("Recover");
        case LoseHp: return tr("Lose HP");
        case LostMaxHp: return tr("Lose Max HP");
        case ResetMaxHp: return tr("Reset Max HP");
        default:
            return QVariant();
        }
    }else if(role == Qt::UserRole){
        //return QChar(i["NTFRLME"]);
        return op;
    }

    return QVariant();
}

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpinBox>

DamageMakerDialog::DamageMakerDialog(QWidget *parent)
    :QDialog(parent)
{
    setWindowTitle(tr("Damage maker"));

    int n = ClientInstance->getPlayers().length();

    QListView *source = new QListView;
    source->setObjectName("source");
    source->setModel(new ClientPlayerListModel(this, true));
    source->setIconSize(General::TinyIconSize);
    source->setMinimumHeight((n+1) * General::TinyIconSize.height() + 20);
    QGroupBox *sourceBox = CreateGroupBoxWithWidget(source);
    sourceBox->setTitle("Source");

    QListView *target = new QListView;
    target->setObjectName("target");
    target->setModel(new ClientPlayerListModel(this, false));
    target->setIconSize(General::TinyIconSize);
    target->setMinimumHeight(n * General::TinyIconSize.height() + 20);
    target->setCurrentIndex(target->model()->index(0, 0));

    QGroupBox *targetBox = CreateGroupBoxWithWidget(target);
    targetBox->setTitle("Target");

    QListView *operation = new QListView;
    operation->setObjectName("operation");
    operation->setModel(new DamageMakerOperationsModel(this));
    QGroupBox *operationBox = CreateGroupBoxWithWidget(operation);
    operationBox->setTitle(tr("Operation"));

    QHBoxLayout *hlayout = new QHBoxLayout;

    hlayout->addWidget(sourceBox);
    hlayout->addWidget(targetBox);
    hlayout->addWidget(operationBox);

    QSpinBox *point = new QSpinBox;
    point->setObjectName("point");
    point->setRange(0, 1000);
    point->setValue(1);
    point->setSuffix(tr(" Point"));

    operationBox->layout()->addWidget(point);

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addLayout(hlayout);
    vlayout->addLayout(CreateOKCancelLayout(this));

    setLayout(vlayout);

    connect(operation, SIGNAL(clicked(QModelIndex)), this, SLOT(disableSource()));
}

void DamageMakerDialog::disableSource(){
    QListView *listView = qobject_cast<QListView *>(sender());
    QModelIndex index = listView->currentIndex();
    int op = listView->model()->data(index, Qt::UserRole).toInt();
    QListView *source = findChild<QListView *>("source");

    switch(static_cast<DamageMakerOperationsModel::Operation>(op)){
    case DamageMakerOperationsModel::LoseHp:
    case DamageMakerOperationsModel::LostMaxHp:
    case DamageMakerOperationsModel::ResetMaxHp: {
        source->setDisabled(true);
        break;
    }
    default:{
        source->setDisabled(false);
        break;
    }
    }
}

void DamageMakerDialog::accept(){
    QListView *source = findChild<QListView *>("source");
    QString sourceName;
    if(source->isEnabled())
        sourceName = source->model()->data(source->currentIndex(), Qt::UserRole).toString();
    else
        sourceName = ".";

    QListView *target = findChild<QListView *>("target");
    QString targetName = target->model()->data(target->currentIndex(), Qt::UserRole).toString();

    QListView *operation = findChild<QListView *>("operation");
    int op = operation->model()->data(operation->currentIndex(), Qt::UserRole).toInt();
    char opchar = op["NTFRLME"];

    QSpinBox *damageSpinBox = findChild<QSpinBox *>("point");
    int point = damageSpinBox->value();

    ClientInstance->request(QString("useCard :%1->%2:%3%4")
                            .arg(sourceName).arg(targetName).arg(opchar).arg(point));

    QDialog::accept();
}
