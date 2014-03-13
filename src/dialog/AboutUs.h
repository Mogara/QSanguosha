#ifndef _ABOUT_US_H
#define _ABOUT_US_H

#include <QDialog>

class QListWidget;
class QTextBrowser;

class AboutUsDialog: public QDialog {
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
    // QSanguosha-Hegemony Team
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