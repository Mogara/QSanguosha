#ifndef _GENERAL_OVERVIEW_H
#define _GENERAL_OVERVIEW_H

class General;
class Skill;
class QCommandLinkButton;

#include <QDialog>
#include <QTableWidgetItem>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QGroupBox>

class GeneralOverview;

class GeneralSearch: public QDialog {
    Q_OBJECT

public:
    GeneralSearch(GeneralOverview *parent);

private:
    QCheckBox *include_hidden_checkbox;
    QLabel *nickname_label;
    QLineEdit *nickname_edit;
    QLabel *name_label;
    QLineEdit *name_edit;
    QButtonGroup *gender_buttons;
    QButtonGroup *kingdom_buttons;
    QLabel *maxhp_lower_label, *maxhp_upper_label;
    QSpinBox *maxhp_lower_spinbox;
    QSpinBox *maxhp_upper_spinbox;
    QPushButton *select_all_button, *unselect_all_button;
    QButtonGroup *package_buttons;

signals:
    void search(bool include_hidden, const QString &nickname, const QString &name, const QStringList &genders,
                const QStringList &kingdoms, int lower, int upper, const QStringList &packages);

protected:
    virtual void accept();

private:
    QWidget *createInfoTab();
    QLayout *createButtonLayout();

private slots:
    void clearAll();
    void selectAllPackages();
    void unselectAllPackages();
};

namespace Ui {
    class GeneralOverview;
}

class GeneralOverview: public QDialog {
    Q_OBJECT

public:
    GeneralOverview(QWidget *parent = 0);
    ~GeneralOverview();
    void fillGenerals(const QList<const General *> &generals, bool init = true);

    static GeneralOverview *getInstance(QWidget *main_window);

private:
    Ui::GeneralOverview *ui;
    QVBoxLayout *button_layout;
    GeneralSearch *general_search;

    QString origin_window_title;
    QList<const General *> all_generals;

    void resetButtons();
    void addLines(const Skill *skill);
    void addCopyAction(QCommandLinkButton *button);
    bool hasSkin(const QString &general_name);
    QString getIllustratorInfo(const QString &general_name);

public slots:
    void startSearch(bool include_hidden, const QString &nickname, const QString &name, const QStringList &genders,
                     const QStringList &kingdoms, int lower, int upper, const QStringList &packages);

private slots:
    void playAudioEffect();
    void copyLines();
    void askTransfiguration();
    void askChangeSkin();
    void fillAllGenerals();
    void on_tableWidget_itemSelectionChanged();
    void on_tableWidget_itemDoubleClicked(QTableWidgetItem *item);
};

#endif

