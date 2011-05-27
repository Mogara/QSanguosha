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
#include <QHBoxLayout>

class BlackEdgeTextItem: public QGraphicsObject{
    Q_OBJECT

public:
    BlackEdgeTextItem();  
    void setColor(const QColor &color);

public slots:
    void setText(const QString &text);
    void setFont(const QFont &font);
    void setFontSize(int size);
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

class CardEditor;

class CardScene: public QGraphicsScene{
    Q_OBJECT

public:
    explicit CardScene();

    void setFrame(const QString &kingdom, bool is_lord);
    void setGeneralPhoto(const QString &filename);
    BlackEdgeTextItem *getNameItem() const;
    BlackEdgeTextItem *getTitleItem() const;

public slots:
    void setMaxHp(int max_hp);
    void setRatio(int ratio);

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
    QComboBox *kingdom_combobox;
    QCheckBox *lord_checkbox;
    QSpinBox *ratio_spinbox;
    QTabWidget *skill_tabs;

    QGroupBox *createLeft();
    QHBoxLayout *createTextItemLayout(const QString &text,
                                      const QFont &font,
                                      int size,
                                      BlackEdgeTextItem *item
                                      );

private slots:
    void setCardFrame();
    void browseGeneralPhoto();
    void saveImage();
};

#endif // CARDEDITOR_H
