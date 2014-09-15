/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Rara
    *********************************************************************/

#include "ChooseOptionsBox.h"
#include "engine.h"
#include "button.h"
#include "client.h"
#include "clientstruct.h"

#include <QGraphicsProxyWidget>

ChooseOptionsBox::ChooseOptionsBox()
    : optionsNumber(0), progressBar(NULL)
{
}

void ChooseOptionsBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    //====================
    //||================||
    //|| Please Choose: ||
    //||    _______     ||
    //||   |   1   |    ||
    //||    -------     ||
    //||    _______     ||
    //||   |   2   |    ||
    //||    -------     ||
    //||    _______     ||
    //||   |   3   |    ||
    //||    -------     ||
    //====================

    title = QString("%1 %2").arg(Sanguosha->translate(skillName)).arg(tr("Please choose:"));
    GraphicsBox::paint(painter, option, widget);
}

QRectF ChooseOptionsBox::boundingRect() const
{
    const int width = getButtonWidth() + outerBlankWidth * 2;

    int height = topBlankWidth + optionsNumber * defaultButtonHeight + (optionsNumber - 1) * interval + bottomBlankWidth;

    if (ServerInfo.OperationTimeout != 0)
        height += 12;

    return QRectF(0, 0, width, height);
}

void ChooseOptionsBox::chooseOption(const QStringList &options)
{
    //repaint background
    this->options = options;
    optionsNumber = options.length();
    prepareGeometryChange();

    const int buttonWidth = getButtonWidth();
    foreach (QString option, options) {
        Button *button = new Button(translate(option), QSizeF(buttonWidth,
                                                      defaultButtonHeight), true);
        button->setFlag(QGraphicsItem::ItemIsFocusable);
        button->setObjectName(option);
        buttons << button;
        button->setParentItem(this);

        QString original_tooltip = QString(":%1").arg(title);
        QString tooltip = Sanguosha->translate(original_tooltip);
        if (tooltip == original_tooltip) {
            original_tooltip = QString(":%1").arg(option);
            tooltip = Sanguosha->translate(original_tooltip);
        }
        connect(button, SIGNAL(clicked()), this, SLOT(reply()));
        if (tooltip != original_tooltip)
            button->setToolTip(QString("<font color=%1>%2</font>")
                               .arg(Config.SkillDescriptionInToolTipColor.name())
                               .arg(tooltip));
    }

    moveToCenter();
    show();

    for (int i = 0; i < buttons.length(); ++i) {
        Button *button = buttons.at(i);

        QPointF pos;
        pos.setX(outerBlankWidth);
        pos.setY(topBlankWidth + defaultButtonHeight * i + (i - 1) * interval + defaultButtonHeight / 2);

        button->setPos(pos);
    }

    if (ServerInfo.OperationTimeout != 0) {
        if (!progressBar) {
            progressBar = new QSanCommandProgressBar();
            progressBar->setMaximumWidth(boundingRect().width() - 16);
            progressBar->setMaximumHeight(12);
            progressBar->setTimerEnabled(true);
            progressBarItem = new QGraphicsProxyWidget(this);
            progressBarItem->setWidget(progressBar);
            progressBarItem->setPos(boundingRect().center().x() - progressBarItem->boundingRect().width() / 2, boundingRect().height() - 20);
            connect(progressBar, SIGNAL(timedOut()), this, SLOT(reply()));
        }
        progressBar->setCountdown(QSanProtocol::S_COMMAND_MULTIPLE_CHOICE);
        progressBar->show();
    }
}

void ChooseOptionsBox::reply()
{
    QString choice = sender()->objectName();
    if (choice.isEmpty())
        choice = options.first();
    ClientInstance->onPlayerMakeChoice(choice);
    clear();
}

int ChooseOptionsBox::getButtonWidth() const
{
    if (options.isEmpty())
        return minButtonWidth;

    QFontMetrics fontMetrics(Button::defaultFont());
    int biggest = 0;
    foreach (const QString &choice, options) {
        const int width = fontMetrics.width(translate(choice));
        if (width > biggest)
            biggest = width;
    }

    // Otherwise it would look compact
    biggest += 20;

    int width = minButtonWidth;
    return qMax(biggest, width);
}

QString ChooseOptionsBox::translate(const QString &option) const
{
    QString title = QString("%1:%2").arg(skillName).arg(option);
    QString translated = Sanguosha->translate(title);
    if (translated == title)
        translated = Sanguosha->translate(option);
    return translated;
}

void ChooseOptionsBox::clear()
{
    if (progressBar != NULL){
        progressBar->hide();
        progressBar->deleteLater();
        progressBar = NULL;
    }

    foreach(Button *button, buttons)
        button->deleteLater();

    buttons.clear();

    disappear();
}
