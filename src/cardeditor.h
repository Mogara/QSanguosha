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
#include <QMainWindow>
#include <QFontDialog>

class Pixmap;

class BlackEdgeTextItem: public QGraphicsObject{
    Q_OBJECT

public:
    BlackEdgeTextItem();  
    void setColor(const QColor &color);
    void setOutline(int outline);
    void toCenter(const QRectF &rect);

public slots:
    void setText(const QString &text);
    void setFont(const QFont &font);
    void setSkip(int skip);

    virtual QRectF boundingRect() const;

protected:    
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QString text;
    QFont font;
    int skip;
    QColor color;
    int outline;
};

class AATextItem: public QGraphicsTextItem{
public:
    AATextItem(const QString &text, QGraphicsItem *parent);

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void keyReleaseEvent(QKeyEvent *event);
};

class SkillBox: public QGraphicsObject{
    Q_OBJECT

public:
    SkillBox();
    void setKingdom(const QString &kingdom);
    void setMiddleHeight(int height);
    void setTextEditable(bool editable);

    virtual QRectF boundingRect() const;

public slots:
    void addSkill();
    void removeSkill();
    void setSkillTitleFont(const QFont &font);
    void setSkillDescriptionFont(const QFont &font);
    void insertSuit(int index);
    void insertBoldText(const QString &bold_text);
    void saveConfig();
    void loadConfig();

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    int middle_height;
    QPixmap up, middle, down;
    QString kingdom;
    QList<QGraphicsTextItem *> skill_titles;
    QGraphicsTextItem *skill_description;
    QGraphicsTextItem *copyright_text;
};

class AvatarRectItem: public QGraphicsRectItem{
public:
    AvatarRectItem(qreal width, qreal height, const QRectF &box_rect, int font_size);
    void toCenter(QGraphicsScene *scene);
    void setKingdom(const QString &kingdom);
    void setName(const QString &name);

private:
    QGraphicsRectItem *name_box;
    BlackEdgeTextItem *name;
};

class CardScene: public QGraphicsScene{
    Q_OBJECT

public:
    explicit CardScene();

    void setFrame(const QString &kingdom, bool is_lord);
    void setGeneralPhoto(const QString &filename);
    BlackEdgeTextItem *getNameItem() const;
    BlackEdgeTextItem *getTitleItem() const;
    SkillBox *getSkillBox() const;
    void saveConfig();
    void loadConfig();
    void setMenu(QMenu *menu);

public slots:
    void setRatio(int ratio);
    void setMaxHp(int max_hp);
    void makeBigAvatar();
    void makeSmallAvatar();
    void makeTinyAvatar();
    void doneMakingAvatar();
    void hideAvatarRects();
    void setAvatarNameBox(const QString &text);

protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

#ifdef QT_DEBUG
protected:
    virtual void keyPressEvent(QKeyEvent *);
#endif

private:
    Pixmap *photo;
    QGraphicsPixmapItem *frame;
    QList<QGraphicsPixmapItem *> magatamas;
    BlackEdgeTextItem *name, *title;
    SkillBox *skill_box;
    AvatarRectItem *big_avatar_rect, *small_avatar_rect, *tiny_avatar_rect;
    QMenu *menu, *done_menu;

    void makeAvatar(AvatarRectItem *item);

signals:
    void avatar_snapped(const QRectF &rect);
};

class CardEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit CardEditor(QWidget *parent = 0);

private:
    CardScene *card_scene;
    QComboBox *kingdom_combobox;
    QCheckBox *lord_checkbox;
    QSpinBox *ratio_spinbox;
    QMap<QFontDialog *, QPushButton *> dialog2button;

    QWidget *createLeft();
    QGroupBox *createTextItemBox(const QString &text,
                                 const QFont &font,
                                 int skip,
                                 BlackEdgeTextItem *item
                                 );
    QLayout *createGeneralLayout();
    QWidget *createSkillBox();

protected:
    virtual void closeEvent(QCloseEvent *);

private:
    void setMapping(QFontDialog *dialog, QPushButton *button);

private slots:
    void setCardFrame();
    void import();
    void saveImage();
    void updateButtonText(const QFont &font);
    void saveAvatar(const QRectF &rect);
};

#endif // CARDEDITOR_H
