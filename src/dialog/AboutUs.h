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

#ifndef _ABOUT_US_H
#define _ABOUT_US_H

#include <QDialog>

class QListWidget;
class QTextBrowser;

class AboutUsDialog : public QDialog {
    Q_OBJECT

public:

    //************************************
    // Method:    AboutUsDialog
    // FullName:  AboutUsDialog::AboutUsDialog
    // Access:    public
    // Returns:
    // Qualifier:
    // Parameter: QWidget * parent
    // Description: Construct a dialog to provide information about developers.
    //              The dialog can also be used to introduce this program.
    //
    // Last Updated By Yanguam Siliagim
    // To make the characters clearer
    //
    // QSanguosha-Rara
    // March 14 2014
    //************************************
    AboutUsDialog(QWidget *parent);

private:
    QListWidget *list;
    QTextBrowser *content_box;

private slots:
    void loadContent(int row);
};

#endif
