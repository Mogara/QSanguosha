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
#include <QFontDatabase>
#include <QTextEdit>

class BlackEdgeTextItem: public QGraphicsObject{
    Q_OBJECT

public:
    BlackEdgeTextItem();
    void setText(const QString &text);
    void setFont(const QFont &font);
    void setColor(const QColor &color);

public slots:
    void setSkip(int skip);

    virtual QRectF boundingRect() const;

protected:    
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QString text;
    QFont font;
    int skip;
    QColor color;
};

class SkillBox: public QGraphicsObject{
    Q_OBJECT

public:
    SkillBox();
    void setKingdom(const QString &kingdom);
    void setMiddleHeight(int height);

    virtual QRectF boundingRect() const;

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    int middle_height;
    QPixmap up, middle, down;
};

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

    void setNameFont(const QString &family);
    void setTitleFont(const QString &family);

#ifdef QT_DEBUG
protected:
    virtual void keyPressEvent(QKeyEvent *);
#endif

private:
    QGraphicsPixmapItem *photo, *frame;
    QList<QGraphicsPixmapItem *> magatamas;
    BlackEdgeTextItem *name, *title;
    SkillBox *skill_box;
};

class SkillTab: public QWidget{
    Q_OBJECT

public:
    SkillTab();
    QTextDocument *getDoc() const;

private:
    QLineEdit *name_edit;
    QTextEdit *description_edit;

private slots:
    void setDocTitle(const QString &title);
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

private slots:
    void setCardFrame();
    void browseGeneralPhoto();
    void saveImage();
};

#endif // CARDEDITOR_H
