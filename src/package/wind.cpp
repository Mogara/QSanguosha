#include "settings.h"
#include "standard.h"
#include "skill.h"
#include "wind.h"
#include "client.h"
#include "engine.h"
#include "ai.h"
#include "general.h"



#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCommandLinkButton>

GuhuoDialog *GuhuoDialog::getInstance(const QString &object, bool left, bool right) {
    static GuhuoDialog *instance;
    if (instance == NULL || instance->objectName() != object)
        instance = new GuhuoDialog(object, left, right);

    return instance;
}

GuhuoDialog::GuhuoDialog(const QString &object, bool left, bool right): object_name(object) {
    setObjectName(object);
    setWindowTitle(Sanguosha->translate(object));
    group = new QButtonGroup(this);

    QHBoxLayout *layout = new QHBoxLayout;
    if (left) layout->addWidget(createLeft());
    if (right) layout->addWidget(createRight());
    setLayout(layout);

    connect(group, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(selectCard(QAbstractButton *)));
}

void GuhuoDialog::popup() {
    if (Sanguosha->currentRoomState()->getCurrentCardUseReason() != CardUseStruct::CARD_USE_REASON_PLAY)
        return;

    foreach (QAbstractButton *button, group->buttons()) {
        const Card *card = map[button->objectName()];
        bool enabled = !Self->isCardLimited(card, Card::MethodUse, true) && card->isAvailable(Self);
        button->setEnabled(enabled);
    }

    Self->tag.remove(object_name);
    exec();
}

void GuhuoDialog::selectCard(QAbstractButton *button) {
    CardStar card = map.value(button->objectName());
    Self->tag[object_name] = QVariant::fromValue(card);
    if (button->objectName().contains("slash")) {
        if (objectName() == "guhuo")
            Self->tag["GuhuoSlash"] = button->objectName();
        else if (objectName() == "nosguhuo")
            Self->tag["NosGuhuoSlash"] = button->objectName();
    }
    emit onButtonClick();
    accept();
}

QGroupBox *GuhuoDialog::createLeft() {
    QGroupBox *box = new QGroupBox;
    box->setTitle(Sanguosha->translate("basic"));

    QVBoxLayout *layout = new QVBoxLayout;

    QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
    foreach (const Card *card, cards) {
        if (card->getTypeId() == Card::TypeBasic && !map.contains(card->objectName())
            && !Config.BanPackages.contains(card->getPackage())) {
            Card *c = Sanguosha->cloneCard(card->objectName());
            c->setParent(this);
            layout->addWidget(createButton(c));

            if (card->objectName() == "slash" && !Config.BanPackages.contains("maneuvering") && objectName() != "gudan") {
                Card *c2 = Sanguosha->cloneCard(card->objectName());
                c2->setParent(this);
                layout->addWidget(createButton(c2));
            }
        }
    }

    layout->addStretch();
    box->setLayout(layout);
    return box;
}

QGroupBox *GuhuoDialog::createRight() {
    QGroupBox *box = new QGroupBox(Sanguosha->translate("ndtrick"));
    QHBoxLayout *layout = new QHBoxLayout;

    QGroupBox *box1 = new QGroupBox(Sanguosha->translate("single_target_trick"));
    QVBoxLayout *layout1 = new QVBoxLayout;

    QGroupBox *box2 = new QGroupBox(Sanguosha->translate("multiple_target_trick"));
    QVBoxLayout *layout2 = new QVBoxLayout;


    QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
    foreach (const Card *card, cards) {
        if (card->isNDTrick() && !map.contains(card->objectName())
            && !Config.BanPackages.contains(card->getPackage())) {
            Card *c = Sanguosha->cloneCard(card->objectName());
            c->setSkillName(object_name);
            c->setParent(this);

            QVBoxLayout *layout = c->isKindOf("SingleTargetTrick") ? layout1 : layout2;
            layout->addWidget(createButton(c));
        }
    }

    box->setLayout(layout);
    box1->setLayout(layout1);
    box2->setLayout(layout2);

    layout1->addStretch();
    layout2->addStretch();

    layout->addWidget(box1);
    layout->addWidget(box2);
    return box;
}

QAbstractButton *GuhuoDialog::createButton(const Card *card) {
    if (card->objectName() == "slash" && map.contains(card->objectName())
        && !map.contains("normal_slash") && objectName() != "gudan") {
        QCommandLinkButton *button = new QCommandLinkButton(Sanguosha->translate("normal_slash"));
        button->setObjectName("normal_slash");
        button->setToolTip(card->getDescription());

        map.insert("normal_slash", card);
        group->addButton(button);

        return button;
    } else {
        QCommandLinkButton *button = new QCommandLinkButton(Sanguosha->translate(card->objectName()));
        button->setObjectName(card->objectName());
        button->setToolTip(card->getDescription());

        map.insert(card->objectName(), card);
        group->addButton(button);

        return button;
    }
}

WindPackage::WindPackage()
    :Package("wind")
{

}

ADD_PACKAGE(Wind)