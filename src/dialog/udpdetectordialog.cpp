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

#include "udpdetectordialog.h"
#include "detector.h"
#include "stylehelper.h"

#include <QTimer>
#include <QPushButton>
#include <QHBoxLayout>
#include <QListWidget>
#include <QScrollBar>

UdpDetectorDialog::UdpDetectorDialog(QDialog *parent)
    : FlatDialog(parent)
{
    setWindowTitle(tr("Detect available server's addresses at LAN"));
    detect_button = new QPushButton(tr("Refresh"));
    cancel_button = new QPushButton(tr("Cancel"));

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    hlayout->addWidget(detect_button);
    hlayout->addWidget(cancel_button);

    list = new QListWidget;
    layout->addWidget(list);
    layout->addLayout(hlayout);

    QScrollBar *bar = list->verticalScrollBar();
    bar->setStyleSheet(StyleHelper::styleSheetOfScrollBar());

    setLayout(layout);

    detector = NULL;
    connect(detect_button, SIGNAL(clicked()), this, SLOT(startDetection()));
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
    connect(list, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(chooseAddress(QListWidgetItem *)));

    detect_button->click();
}

void UdpDetectorDialog::startDetection() {
    list->clear();
    detect_button->setEnabled(false);

    detector = new UdpDetector;
    connect(detector, SIGNAL(detected(QString, QString)), this, SLOT(addServerAddress(QString, QString)));
    QTimer::singleShot(2000, this, SLOT(stopDetection()));

    detector->detect();
}

void UdpDetectorDialog::stopDetection() {
    detect_button->setEnabled(true);
    detector->stop();
    delete detector;
    detector = NULL;
}

void UdpDetectorDialog::addServerAddress(const QString &server_name, const QString &address) {
    QString label = QString("%1 [%2]").arg(server_name).arg(address);
    QListWidgetItem *item = new QListWidgetItem(label);
    item->setData(Qt::UserRole, address);

    list->addItem(item);
}

void UdpDetectorDialog::chooseAddress(QListWidgetItem *item) {
    accept();

    QString address = item->data(Qt::UserRole).toString();
    emit address_chosen(address);
}
