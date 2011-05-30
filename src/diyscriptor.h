#ifndef DIYSCRIPTOR_H
#define DIYSCRIPTOR_H

#include <QMainWindow>
#include <QTextBrowser>

namespace Ui {
    class DIYScriptor;
}

class DIYScriptor : public QMainWindow
{
    Q_OBJECT

public:
    explicit DIYScriptor(QWidget *parent = 0);
    ~DIYScriptor();

private slots:
    void on__gb_clicked();
    void syncText();
    void backSync();


    void on__vf_selectionChanged();

    void on__va_1_selectionChanged();

    void on__va_2_selectionChanged();

    void on__ea_1_selectionChanged();

    void on__ea_2_selectionChanged();

    void on__cv_textChanged();

    void on__sb_clicked();

private:
    Ui::DIYScriptor *ui;
    QTextBrowser *editionArea;
};

#endif // DIYSCRIPTOR_H
