#include "roleassigndialog.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <qlist.h>
#include "general.h"
#include "player.h"
#include "client.h"
#include "engine.h"
#include "roomscene.h"
#include "settings.h"

RoleAssignDialog::RoleAssignDialog(QWidget *parent)
    :QDialog(parent)
{
    setWindowTitle(tr("Assign roles and seats"));

    list = new QListWidget;
    list->setFlow(QListView::TopToBottom);
    list->setMovement(QListView::Static);

    QStringList role_list = Sanguosha->getRoleList(ServerInfo.GameMode);

    if(Config.FreeAssignSelf){
        QString text = QString("%1[%2]")
                       .arg(Self->screenName())
                       .arg(Sanguosha->translate("lord"));

        QListWidgetItem *item = new QListWidgetItem(text, list);
        item->setData(Qt::UserRole, Self->objectName());

        role_mapping.insert(Self->objectName(), "lord");
    }
    else{
        QList<const ClientPlayer *> players = ClientInstance->getPlayers();
        for(int i=0; i<players.length(); i++){
            QString role = role_list.at(i);
            const ClientPlayer *player = players.at(i);
            QString text = QString("%1[%2]")
                           .arg(player->screenName())
                           .arg(Sanguosha->translate(role));

            QListWidgetItem *item = new QListWidgetItem(text, list);
            item->setData(Qt::UserRole, player->objectName());

            role_mapping.insert(player->objectName(), role);
        }
    }

    QVBoxLayout *vlayout = new QVBoxLayout;

    role_combobox = new QComboBox;
    role_combobox->addItem(tr("Lord"), "lord");
    role_combobox->addItem(tr("Loyalist"), "loyalist");
    role_combobox->addItem(tr("Renegade"), "renegade");
    role_combobox->addItem(tr("Rebel"), "rebel");

    QPushButton *moveUpButton = new QPushButton(tr("Move up"));
    QPushButton *moveDownButton = new QPushButton(tr("Move down"));
    QPushButton *okButton = new QPushButton(tr("OK"));
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));

    if(Config.FreeAssignSelf){
        moveUpButton->setEnabled(false);
        moveDownButton->setEnabled(false);
    }

    vlayout->addWidget(role_combobox);
    vlayout->addWidget(moveUpButton);
    vlayout->addWidget(moveDownButton);
    vlayout->addStretch();
    vlayout->addWidget(okButton);
    vlayout->addWidget(cancelButton);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(list);
    layout->addLayout(vlayout);
    QVBoxLayout *mainlayout = new QVBoxLayout();
    mainlayout->addLayout(layout);
    setLayout(mainlayout);

    connect(role_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateRole(int)));
    connect(list, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(updateRole(QListWidgetItem*)));
    connect(moveUpButton, SIGNAL(clicked()), this, SLOT(moveUp()));
    connect(moveDownButton, SIGNAL(clicked()), this, SLOT(moveDown()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void RoleAssignDialog::accept(){
    QStringList role_list = Sanguosha->getRoleList(ServerInfo.GameMode);
    QStringList real_list;

    QList<QString> names;
    QList<QString> roles;
    if(Config.FreeAssignSelf){
        QString name = list->item(0)->data(Qt::UserRole).toString();
        QString role = role_mapping.value(name);
        names.push_back(name);
        roles.push_back(role);
        ClientInstance->onPlayerAssignRole(names, roles);
        QDialog::accept();
        return;
    }
    
    for(int i = 0; i < list->count(); i++){
        QString name = list->item(i)->data(Qt::UserRole).toString();
        QString role = role_mapping.value(name);

        if(i == 0 && role != "lord"){
            QMessageBox::warning(this, tr("Warning"), tr("The first assigned role must be lord!"));
            return;
        }

        real_list << role;
        names.push_back(name);
        roles.push_back(role);
    }

    role_list.sort();
    real_list.sort();

    if(role_list == real_list){
        ClientInstance->onPlayerAssignRole(names, roles);
        QDialog::accept();
    }else{
        QMessageBox::warning(this, tr("Warning"),
                             tr("The roles that you assigned do not comform with the current game mode"));
    }
}

void RoleAssignDialog::reject(){
    ClientInstance->request("assignRoles .");
    QDialog::reject();
}

void RoleAssignDialog::updateRole(int index){
    QString name = list->currentItem()->data(Qt::UserRole).toString();
    QString role = role_combobox->itemData(index).toString();
    ClientPlayer *player = ClientInstance->getPlayer(name);
    QString text = QString("%1[%2]").arg(player->screenName()).arg(Sanguosha->translate(role));
    list->currentItem()->setText(text);
    role_mapping[name] = role;
}

void RoleAssignDialog::updateRole(QListWidgetItem *current){
    static QMap<QString, int> mapping;
    if(mapping.isEmpty()){
        mapping["lord"] = 0;
        mapping["loyalist"] = 1;
        mapping["renegade"] = 2;
        mapping["rebel"] = 3;
    }

    QString name = current->data(Qt::UserRole).toString();
    QString role = role_mapping.value(name);
    int index = mapping.value(role);
    role_combobox->setCurrentIndex(index);
}

void RoleAssignDialog::moveUp(){
    int index = list->currentRow();
    QListWidgetItem *item = list->takeItem(index);
    list->insertItem(index - 1, item);
    list->setCurrentItem(item);
}

void RoleAssignDialog::moveDown(){
    int index = list->currentRow();
    QListWidgetItem *item = list->takeItem(index);
    list->insertItem(index + 1, item);
    list->setCurrentItem(item);
}

void RoomScene::startAssign(){
    RoleAssignDialog *dialog = new RoleAssignDialog(main_window);
    dialog->exec();
}
