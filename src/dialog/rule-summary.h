#ifndef _SCENARIO_OVERVIEW_H
#define _SCENARIO_OVERVIEW_H

#include <QDialog>

class QListWidget;
class QTextEdit;

class RuleSummary: public QDialog {
    Q_OBJECT

public:
    RuleSummary(QWidget *parent);

private:
    QListWidget *list;
    QTextEdit *content_box;

private slots:
    void loadContent(int row);
};

#endif

