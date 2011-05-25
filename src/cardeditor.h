#ifndef CARDEDITOR_H
#define CARDEDITOR_H

#include <QDialog>
#include <QGraphicsView>
#include <QGroupBox>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QTabWidget>
#include <QGraphicsPixmapItem>

class CardScene: public QGraphicsScene{
    Q_OBJECT

public:
    explicit CardScene();

    void setFrame(const QString &kingdom, bool is_lord);
    void setGeneralPhoto(const QString &filename);

public slots:
    void setName(const QString &name);
    void setTitle(const QString &title);
    void setMaxHp(int max_hp);
    void setRatio(int ratio);

private:
    QGraphicsPixmapItem *photo, *frame;
    QList<QGraphicsPixmapItem *> magatamas;
    QGraphicsTextItem *name, *title;
};

class CardEditor : public QDialog
{
    Q_OBJECT

public:
    explicit CardEditor(QWidget *parent = 0);

private:
    CardScene *card_scene;
    QLineEdit *name_edit;
    QLineEdit *title_edit;
    QComboBox *kingdom_combobox;
    QCheckBox *lord_checkbox;
    QSpinBox *hp_spinbox, *ratio_spinbox;
    QTabWidget *skill_tabs;

    QGroupBox *createLeft();
    QWidget *createSkillTab();

private slots:
    void setCardFrame();
    void browseGeneralPhoto();
    void saveImage();
};

#endif // CARDEDITOR_H
