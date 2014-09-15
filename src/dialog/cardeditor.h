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

class QSanSelectableItem;

class BlackEdgeTextItem : public QGraphicsObject{
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

class AATextItem : public QGraphicsTextItem{
public:
    AATextItem(const QString &text, QGraphicsItem *parent);

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

class SkillTitle;
class CompanionBox;

class SkillBox : public QGraphicsObject{
    Q_OBJECT

public:
    SkillBox();
    void setKingdom(const QString &kingdom);

    void setTextEditable(bool editable);
    void addSkill(const QString &text);
    SkillTitle *getFocusTitle() const;

    virtual QRectF boundingRect() const;

public slots:
    void removeSkill();
    void setSkillTitleFont(const QFont &font);
    void setSkillDescriptionFont(const QFont &font);
    void setTinyFont(const QFont &font);
    void insertSuit(int index);
    void insertBoldText(const QString &bold_text);
    void saveConfig();
    void loadConfig();

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);


private:

    QString kingdom;
    QList<SkillTitle *> skill_titles;
    AATextItem *skill_description;
    AATextItem *copyright_text;
};

class AvatarRectItem : public QGraphicsRectItem{
public:
    AvatarRectItem(qreal width, qreal height, const QRectF &box_rect, int font_size);
    void toCenter(QGraphicsScene *scene);
    void setKingdom(const QString &kingdom);
    void setName(const QString &name);

private:
    QGraphicsRectItem *name_box;
    BlackEdgeTextItem *name;
};

class CardScene : public QGraphicsScene{
    Q_OBJECT

public:
    explicit CardScene();

    void setFrame(const QString &kingdom, bool is_lord);
    void setGeneralPhoto(const QString &filename);
    BlackEdgeTextItem *getNameItem() const;
    BlackEdgeTextItem *getTitleItem() const;
    inline SkillBox *getSkillBox() const { return skill_box; }
    inline CompanionBox *getCompanionBox() const { return companion_box; }
    void saveConfig();
    void loadConfig();
    void setMenu(QMenu *menu);

public slots:
    void setRatio(int ratio);
    void setMaxHp(int max_hp);
    void setTransMaxHp(int trans_max_hp);
    void makeBigAvatar();
    void makeSmallAvatar();
    void makeTinyAvatar();
    void doneMakingAvatar();
    void hideAvatarRects();
    void setAvatarNameBox(const QString &text);
    void resetPhoto();
    void setCompanion(const QString &text);
    void setCompanionFont(const QFont &font);
    void setCompanionVisible(bool visible);

protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

#ifdef QT_DEBUG
protected:
    virtual void keyPressEvent(QKeyEvent *);
#endif

private:
    QSanSelectableItem *photo;
    QGraphicsPixmapItem *frame;
    QList<QGraphicsPixmapItem *> magatamas;
    BlackEdgeTextItem *name, *title;
    SkillBox *skill_box;
    AvatarRectItem *big_avatar_rect, *small_avatar_rect, *tiny_avatar_rect;
    QMenu *menu, *done_menu;

    CompanionBox *companion_box;

    int max_hp, trans_max_hp;

    void makeAvatar(AvatarRectItem *item);
    void _redrawTransMaxHp();

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
    QCheckBox *lord_checkbox;
    QComboBox *kingdom_ComboBox;
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
    void copyPhoto();
    void updateButtonText(const QFont &font);
    void saveAvatar(const QRectF &rect);
    void addSkill();
    void editSkill();
};

#endif // CARDEDITOR_H
