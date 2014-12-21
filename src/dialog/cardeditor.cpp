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

#include "cardeditor.h"
#include "settings.h"
#include "card.h"
#include "engine.h"
#include "qsanselectableitem.h"

#include <QPainter>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QGraphicsView>
#include <QTextDocument>
#include <QKeyEvent>
#include <QAction>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenuBar>
#include <QPushButton>
#include <QFileDialog>
#include <QGroupBox>
#include <QSpinBox>
#include <QFormLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QBitmap>
#include <QFontDialog>
#include <QApplication>
#include <QInputDialog>
#include <QClipboard>

BlackEdgeTextItem::BlackEdgeTextItem()
    :skip(0), color(Qt::white), outline(3)
{
    setFlags(ItemIsMovable | ItemIsFocusable);
}

QRectF BlackEdgeTextItem::boundingRect() const{
    if (text.isEmpty())
        return QRectF();

    QFontMetrics metric(font);

    QRectF rect;
    rect.setWidth(metric.width(text.at(0)));
    rect.setHeight(text.length() * (metric.height() - metric.descent() + skip) + 10);
    return rect;
}

void BlackEdgeTextItem::setSkip(int skip){
    this->skip = skip;
    prepareGeometryChange();

    Config.setValue("CardEditor/" + objectName() + "Skip", skip);
}

void BlackEdgeTextItem::setColor(const QColor &color){
    this->color = color;
}

void BlackEdgeTextItem::setOutline(int outline){
    this->outline = outline;
}

void BlackEdgeTextItem::toCenter(const QRectF &rect){
    if (text.isEmpty())
        return;

    QFontMetrics metric(font);
    setX((rect.width() - metric.width(text.at(0))) / 2);

    int total_height = (metric.height() - metric.descent()) * text.length();
    setY((rect.height() - total_height) / 2);
}

void BlackEdgeTextItem::setText(const QString &text){
    this->text = text;
    prepareGeometryChange();

    Config.setValue("CardEditor/" + objectName() + "Text", text);
}

void BlackEdgeTextItem::setFont(const QFont &font){
    this->font = font;
    prepareGeometryChange();

    Config.setValue("CardEditor/" + objectName() + "Font", font);
}

void BlackEdgeTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *){
    if (text.isEmpty())
        return;

    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    if (outline > 0){
        QPen pen(Qt::black);
        pen.setWidth(outline);
        painter->setPen(pen);
    }

    QFontMetrics metric(font);
    int height = metric.height() - metric.descent() + skip;

    for (int i = 0; i < text.length(); i++){

        QString text;
        text.append(this->text.at(i));

        QPainterPath path;
        path.addText(0, (i + 1) * height, font, text);

        if (outline > 0)
            painter->drawPath(path);

        painter->fillPath(path, color);
    }

    if (hasFocus()){
        QPen red_pen(Qt::red);
        painter->setPen(red_pen);
        QRectF rect = boundingRect();
        painter->drawRect(-1, -1, rect.width() + 2, rect.height() + 2);
    }
}

static QAction *EditAction;
static QAction *DeleteAction;

class SkillTitle : public QGraphicsPixmapItem{
public:
    SkillTitle(const QString &kingdom, const QString &text)
        :title_text(NULL), frame(NULL)
    {
        title_text = new AATextItem(text, this);
        title_text->setFont(Config.value("CardEditor/SkillTitleFont").value<QFont>());
        title_text->setPos(Config.value("CardEditor/TitleTextOffset", QPointF(10, -2)).toPointF());
        title_text->document()->setDocumentMargin(0);

        setKingdom(kingdom);
        setFlag(QGraphicsItem::ItemIsFocusable);

        frame = new QGraphicsRectItem(this);
        frame->setRect(-1, -1, 58, 21);
        QPen red_pen(Qt::red);
        frame->setPen(red_pen);
        frame->hide();
    }

    void setKingdom(const QString &kingdom){
        QPixmap title_pixmap(QString("diy/%1-skill.png").arg(kingdom));
        setPixmap(title_pixmap);

        title_text->setDefaultTextColor(Qt::black);
    }

    void setText(const QString &text){
        title_text->setPlainText(text);
    }

    QString text() const{
        return title_text->toPlainText();
    }

    void setFont(const QFont &_font){
        QFont font = _font;
        font.setStyleHint(QFont::AnyStyle, QFont::PreferAntialias);
        title_text->setFont(font);
    }

    QFont font() const{
        return title_text->font();
    }

    virtual void keyPressEvent(QKeyEvent *event){
        if (!hasFocus()){
            event->ignore();
            return;
        }

        int delta_y = 0;
        switch (event->key()){
        case Qt::Key_Up: delta_y = -1; break;
        case Qt::Key_Down: delta_y = 1; break;
        case Qt::Key_Delete:{
            if (DeleteAction)
                DeleteAction->trigger();
            return;
        }

        default:
            break;
        }

        if (delta_y == 0){
            event->ignore();
            return;
        }

        if (event->modifiers() & Qt::ShiftModifier)
            delta_y *= 5;

        event->accept();
        moveBy(0, delta_y);
    }

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event){
        event->accept();
    }

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event){
        this->setPos(x(), event->scenePos().y() - event->buttonDownPos(Qt::LeftButton).y());
    }

    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *){
        if (EditAction)
            EditAction->trigger();
    }

    virtual void focusInEvent(QFocusEvent *){
        frame->show();
    }

    virtual void focusOutEvent(QFocusEvent *){
        frame->hide();
    }

private:
    AATextItem *title_text;
    QGraphicsRectItem *frame;
};

class CompanionBox : public QGraphicsPixmapItem{
public:
    CompanionBox(const QString &text = QString())
        :title_text(NULL)
    {
        title_text = new AATextItem(text, this);
        title_text->setFont(Config.value("CardEditor/CompanionFont", QFont("", 9)).value<QFont>());
        title_text->setPlainText(Config.value("CardEditor/Companion").toString());
        title_text->setPos(10, 4);
        title_text->document()->setDocumentMargin(0);
        title_text->setDefaultTextColor(QColor(255, 255, 255));

        setPixmap(QPixmap("diy/companion.png"));

        setFlags(ItemIsFocusable);
    }

    void setText(const QString &text){
        title_text->setPlainText(text);
    }

    QString text() const{
        return title_text->toPlainText();
    }

    void setFont(const QFont &_font){
        QFont font = _font;
        font.setStyleHint(QFont::AnyStyle, QFont::PreferAntialias);
        title_text->setFont(font);
    }

    QFont font() const{
        return title_text->font();
    }

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event){
        event->accept();
    }

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event){
        this->setPos(qBound(50.0, event->scenePos().x() - event->buttonDownPos(Qt::LeftButton).x(), 318.0), y());
    }

private:
    AATextItem *title_text;
};

SkillBox::SkillBox()
{
    setAcceptedMouseButtons(Qt::LeftButton);

    skill_description = new AATextItem(tr("Skill description"), this);
    skill_description->setTextWidth(273);
    skill_description->setFlag(ItemIsMovable);
    skill_description->setTextInteractionFlags(Qt::TextEditorInteraction);

    QFont font = Config.value("CardEditor/SkillDescriptionFont").value<QFont>();

    copyright_text = new AATextItem(tr("Copyright text"), this);
    copyright_text->setFont(Config.value("CardEditor/TinyFont").value<QFont>());
    copyright_text->setTextWidth(246);
    copyright_text->setPos(10, 105);
    copyright_text->setTextInteractionFlags(Qt::TextEditorInteraction);

    setSkillDescriptionFont(font);
}

void SkillBox::setKingdom(const QString &kingdom){
    this->kingdom = kingdom;

    foreach(SkillTitle *skill_title, skill_titles)
        skill_title->setKingdom(kingdom);
}

AATextItem::AATextItem(const QString &text, QGraphicsItem *parent)
    :QGraphicsTextItem(text, parent)
{
}

void AATextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    if (hasFocus()){
        QGraphicsTextItem::paint(painter, option, widget);
        return;
    }

    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    QPainterPath path;
    QString s = toPlainText();
    QTextDocument *doc = document();
    qreal margin = doc->documentMargin();
    QStringList lines = s.split("\n", QString::SkipEmptyParts);

    QFont f = font();
    f.setStyleHint(QFont::AnyStyle, QFont::PreferAntialias);
    QFontMetrics fm(f);
    QFont f_bold = f;
    f_bold.setBold(true);
    QFontMetrics fm_bold(f_bold);

    int line = 0;
    for (int i = 0; i < lines.length(); i++){
        QString thisline = lines[i];
        QStringList s1 = thisline.split(']', QString::SkipEmptyParts);
        QStringList s_non_bold, s_bold;
        foreach(QString s, s1){
            if (!s.contains('[')){
                s_non_bold << s;
                s_bold << QString();
            }
            else if (!s.startsWith('[')){
                QStringList s2 = s.split('[', QString::SkipEmptyParts);
                s_non_bold << s2.first();
                s_bold << s2.last();
            }
            else {
                s = s.mid(1);
                s_non_bold << QString();
                s_bold << s;
            }
        }

        int width = 0;
        for (int j = 0; j < s_non_bold.length(); j++){
            QString non_bold = s_non_bold[j];
            for (int k = 0; k < non_bold.length(); k++){
                QChar c = non_bold[k];
                QString str = c;
                int width_c = fm.width(str);
                if (width + width_c > doc->size().width()){
                    ++line;
                    width = 0;
                }
                path.addText(margin + width, fm.height() * (line + 1), f, str);
                width += width_c;
            }
            QString bold = s_bold[j];
            for (int k = 0; k < bold.length(); k++){
                QChar c = bold[k];
                QString str = c;
                int width_c = fm_bold.width(str);
                if (width + width_c > doc->size().width()){
                    ++line;
                    width = 0;
                }
                path.addText(margin + width, fm.height() * (line + 1), f_bold, str); //use fm.height not fm_bold.height, for sometimes the bold characters are not as high as the non-bold ones
                width += width_c;
            }
        }
        ++line;
    }
    painter->fillPath(path, defaultTextColor());
}

void SkillBox::addSkill(const QString &text){
    SkillTitle *skill_title = new SkillTitle(kingdom, text);
    skill_title->setPos(20, 389);
    skill_titles << skill_title;

    scene()->addItem(skill_title);
}

SkillTitle *SkillBox::getFocusTitle() const{
    if (skill_titles.length() == 1)
        return skill_titles.first();
    else{
        foreach(SkillTitle *skill_title, skill_titles){
            if (skill_title->hasFocus()){
                return skill_title;
            }
        }
    }

    return NULL;
}

void SkillBox::removeSkill(){
    SkillTitle *to_remove = getFocusTitle();
    if (to_remove){
        skill_titles.removeOne(to_remove);
        delete to_remove;
    }
}

void SkillBox::saveConfig(){
    Config.beginGroup("CardEditor");

    Config.beginWriteArray("SkillTitles");
    for (int i = 0; i < skill_titles.length(); i++){
        Config.setArrayIndex(i);

        Config.setValue("TitleText", skill_titles.at(i)->text());
        Config.setValue("TitlePos", skill_titles.at(i)->pos());
    }

    Config.endArray();

    Config.setValue("SkillDescription", skill_description->toHtml());
    Config.setValue("SkillDescriptionFont", skill_description->font());
    Config.setValue("TinyFont", copyright_text->font());
    if (!skill_titles.isEmpty()){
        Config.setValue("SkillTitleFont", skill_titles.first()->font());
    }

    Config.setValue("CopyrightText", copyright_text->toPlainText());

    Config.endGroup();
}

void SkillBox::loadConfig(){
    Config.beginGroup("CardEditor");

    int size = Config.beginReadArray("SkillTitles");
    for (int i = 0; i < size; i++){
        Config.setArrayIndex(i);

        addSkill(Config.value("TitleText").toString());

        SkillTitle *item = skill_titles.last();

        if (Config.contains("TitlePos"))
            item->setPos(Config.value("TitlePos").toPoint());
    }

    Config.endArray();
    skill_description->setHtml(Config.value("SkillDescription").toString());
    copyright_text->setPlainText(Config.value("CopyrightText").toString());

    Config.endGroup();
}


void SkillBox::setSkillTitleFont(const QFont &font){
    foreach(SkillTitle *item, skill_titles){
        item->setFont(font);
    }
}

void SkillBox::setSkillDescriptionFont(const QFont &font){
    skill_description->setFont(font);
}

void SkillBox::setTinyFont(const QFont &font){
    copyright_text->setFont(font);
}

void SkillBox::insertSuit(int index){
    Card::Suit suit = static_cast<Card::Suit>(index);
    QString suit_name = Card::Suit2String(suit);
    QString suit_path = QString("image/system/suit/%1.png").arg(suit_name);
    int size = skill_description->font().pointSize() + 1;

    // QString code = QString("<img src='%1' width='%2' height='%2'>").arg(suit_path).arg(size);
    // skill_description->textCursor().insertHtml(code);

    QImage image(suit_path);
    image = image.scaled(QSize(size, size), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    skill_description->textCursor().insertImage(image);
}

void SkillBox::insertBoldText(const QString &bold_text){
    skill_description->textCursor().insertText("[");
    skill_description->textCursor().insertText(bold_text);
    skill_description->textCursor().insertText(tr(","));
    skill_description->textCursor().insertText("]");
}

QRectF SkillBox::boundingRect() const{
    return QRectF(0, 0, 0, 0);
}

void SkillBox::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *){
}

AvatarRectItem::AvatarRectItem(qreal width, qreal height, const QRectF &box_rect, int font_size)
    : QGraphicsRectItem(0, 0, width, height)
{
    QPen thick_pen(Qt::black);
    thick_pen.setWidth(4);
    setPen(thick_pen);

    setFlag(ItemIsMovable);

    name_box = new QGraphicsRectItem(this);
    name_box->setRect(0, 0, box_rect.width(), box_rect.height());
    name_box->setPos(box_rect.x(), box_rect.y());
    name_box->setOpacity(0.7);
    name_box->setPen(Qt::NoPen);

#ifdef Q_OS_LINUX
    QFont font("DroidSansFallback");
#else
    QFont font("SimHei");
#endif
    font.setPixelSize(font_size);
    name = new BlackEdgeTextItem;
    name->setFont(font);
    name->setOutline(0);
    name->setParentItem(name_box);
    name->setFlag(ItemIsMovable, false);
}

void AvatarRectItem::toCenter(QGraphicsScene *scene){
    QRectF scene_rect = scene->sceneRect();
    setPos((scene_rect.width() - rect().width()) / 2,
        (scene_rect.height() - rect().height()) / 2);
}

void AvatarRectItem::setKingdom(const QString &kingdom){
    QColor color = Sanguosha->getKingdomColor(kingdom);
    name_box->setBrush(color);
}

void AvatarRectItem::setName(const QString &name){
    this->name->setText(name);
    this->name->toCenter(name_box->rect());
}

CardScene::CardScene()
    : QGraphicsScene(QRectF(0, 0, 366, 514)), menu(NULL), max_hp(0), trans_max_hp(0)
{
    photo = NULL;
    frame = new QGraphicsPixmapItem;

    name = new BlackEdgeTextItem;
    name->setObjectName("Name");
    name->setOutline(5);

    title = new BlackEdgeTextItem;
    title->setObjectName("Title");

    skill_box = new SkillBox;

    companion_box = new CompanionBox;
    companion_box->setZValue(-1);
    companion_box->hide();

    addItem(companion_box);
    addItem(frame);
    addItem(name);
    addItem(title);
    addItem(skill_box);

    resetPhoto();

    QGraphicsItemGroup *magatama_group = new QGraphicsItemGroup;
    addItem(magatama_group);

    for (int i = 0; i < 7; i++){
        QGraphicsPixmapItem *item = new QGraphicsPixmapItem;
        magatamas << item;
        item->hide();
        addItem(item);

        if (i % 2 == 0)
            item->setPos(69 + i * 14, 14);
        else
            item->setPos(63 + i * 14, 14);

        magatama_group->addToGroup(item);
    }

    loadConfig();

    QGraphicsRectItem *name_box = new QGraphicsRectItem(0, 0, 16, 48);
    name_box->setPos(78, 24);

    QRectF big_rect(78, 24, 16, 48);
    big_avatar_rect = new AvatarRectItem(94, 96, big_rect, 13);
    big_avatar_rect->hide();
    big_avatar_rect->toCenter(this);
    addItem(big_avatar_rect);

    QRectF small_rect(6, 0, 16, 50);
    small_avatar_rect = new AvatarRectItem(122, 50, small_rect, 11);
    small_avatar_rect->hide();
    small_avatar_rect->toCenter(this);
    addItem(small_avatar_rect);

    QRectF tiny_rect(0, 0, 10, 36);
    tiny_avatar_rect = new AvatarRectItem(42, 36, tiny_rect, 9);
    tiny_avatar_rect->hide();
    tiny_avatar_rect->toCenter(this);
    addItem(tiny_avatar_rect);

    done_menu = new QMenu();

    QAction *done_action = new QAction(tr("Done"), done_menu);

    connect(done_action, &QAction::triggered, this, &CardScene::doneMakingAvatar);
    done_menu->addAction(done_action);
}

void CardScene::setFrame(const QString &kingdom, bool is_lord){
    QString path;
    if (is_lord) {
        path = QString("diy/%1-lord.png").arg(kingdom);
        title->setColor(QColor(171, 151, 90));
    }
    else {
        path = QString("diy/%1.png").arg(kingdom);
        title->setColor(QColor(246, 241, 125));
    }

    frame->setPixmap(QPixmap(path));

    for (int i = 0; i < magatamas.length(); i++){
        QGraphicsPixmapItem *item = magatamas[i];
        if (i % 2 == 0)
            item->setPixmap(QPixmap(QString("diy/%1-magatama-l.png").arg(is_lord ? "lord" : kingdom)));
        else
            item->setPixmap(QPixmap(QString("diy/%1-magatama-r.png").arg(is_lord ? "lord" : kingdom)));
    }

    skill_box->setKingdom(kingdom);

    big_avatar_rect->setKingdom(kingdom);
    small_avatar_rect->setKingdom(kingdom);
    tiny_avatar_rect->setKingdom(kingdom);

    Config.setValue("CardEditor/Kingdom", kingdom);
    Config.setValue("CardEditor/IsLord", is_lord);

    _redrawTransMaxHp();
}

void CardScene::setGeneralPhoto(const QString &filename){
    photo->load(filename);

    Config.setValue("CardEditor/Photo", filename);
}

void SkillBox::setTextEditable(bool editable){
    Qt::TextInteractionFlags flags = editable ? Qt::TextEditorInteraction : Qt::NoTextInteraction;

    skill_description->setTextInteractionFlags(flags);
}

void CardScene::saveConfig(){
    Config.beginGroup("CardEditor");
    Config.setValue("NamePos", name->pos());
    Config.setValue("TitlePos", title->pos());
    Config.setValue("PhotoPos", photo->pos());
    Config.setValue("SkillBoxPos", skill_box->pos());
    Config.setValue("CompanionBoxPos", companion_box->pos());
    Config.setValue("Companion", companion_box->text());
    Config.setValue("CompanionFont", companion_box->font());
    Config.setValue("CompanionVisible", companion_box->isVisible());
    Config.endGroup();

    skill_box->saveConfig();
}

void CardScene::loadConfig(){
    Config.beginGroup("CardEditor");
    name->setPos(Config.value("NamePos", QPointF(28, 206)).toPointF());
    title->setPos(Config.value("TitlePos", QPointF(49, 128)).toPointF());
    photo->setPos(Config.value("PhotoPos").toPointF());
    skill_box->setPos(Config.value("SkillBoxPos", QPointF(67, 389)).toPointF());
    companion_box->setPos(Config.value("CompanionBoxPos", QPointF(318.50, 359)).toPointF());
    companion_box->setVisible(Config.value("CompanionVisible", false).toBool());
    Config.endGroup();

    skill_box->loadConfig();
}


BlackEdgeTextItem *CardScene::getNameItem() const{
    return name;
}

BlackEdgeTextItem *CardScene::getTitleItem() const{
    return title;
}

#ifdef QT_DEBUG

#include <QKeyEvent>
#include <QMessageBox>

void CardScene::keyPressEvent(QKeyEvent *event){
    QGraphicsScene::keyPressEvent(event);
#ifdef QT_DEBUG
    if (event->key() == Qt::Key_D){
        QMessageBox::information(NULL, "", QString("%1, %2").arg(skill_box->x()).arg(skill_box->y()));
    }
#endif
}

#endif

void CardScene::setRatio(int ratio){
    photo->setScale(ratio / 100.0);

    Config.setValue("CardEditor/ImageRatio", ratio);
}

void CardScene::setMaxHp(int max_hp){
    int n = magatamas.length();
    this->max_hp = max_hp = qBound(0, max_hp, n - 1);

    for (int i = 0; i < n; i++)
        magatamas.at(i)->setVisible(i < max_hp);

    Config.setValue("CardEditor/MaxHP", max_hp);

    _redrawTransMaxHp();
}

void CardScene::setTransMaxHp(int trans_max_hp){
    this->trans_max_hp = trans_max_hp;

    Config.setValue("CardEditor/TransMaxHP", trans_max_hp);

    _redrawTransMaxHp();
}

void CardScene::_redrawTransMaxHp(){
    int start = max_hp - trans_max_hp + 1;
    if (start < 0)
        return;

    QString kingdom = Config.value("CardEditor/Kingdom", "wei").toString();
    bool is_lord = Config.value("CardEditor/IsLord", false).toBool();

    for (int i = 0; i < magatamas.length(); i++){
        QGraphicsPixmapItem *item = magatamas[i];
        QString suffix = "";
        if (i >= start - 1)
            suffix = "t";
        if (i % 2 == 0)
            item->setPixmap(QPixmap(QString("diy/%1-magatama-l%2.png").arg(is_lord ? "lord" : kingdom).arg(suffix)));
        else
            item->setPixmap(QPixmap(QString("diy/%1-magatama-r%2.png").arg(is_lord ? "lord" : kingdom).arg(suffix)));
    }
}

void CardScene::makeAvatar(AvatarRectItem *item){
    hideAvatarRects();

    item->setName(Config.value("CardEditor/NameText").toString());
    item->show();
}

void CardScene::makeBigAvatar(){
    makeAvatar(big_avatar_rect);
}

void CardScene::makeSmallAvatar(){
    makeAvatar(small_avatar_rect);
}

void CardScene::makeTinyAvatar(){
    makeAvatar(tiny_avatar_rect);
}

void CardScene::doneMakingAvatar(){
    QGraphicsRectItem *avatar_rect = NULL;

    if (big_avatar_rect->isVisible())
        avatar_rect = big_avatar_rect;
    else if (small_avatar_rect->isVisible())
        avatar_rect = small_avatar_rect;
    else
        avatar_rect = tiny_avatar_rect;

    if (avatar_rect){
        avatar_rect->setPen(Qt::NoPen);

        QRectF rect(avatar_rect->scenePos(), avatar_rect->rect().size());
        emit avatar_snapped(rect);

        QPen thick_pen;
        thick_pen.setWidth(4);
        avatar_rect->setPen(thick_pen);
    }
}

void CardScene::hideAvatarRects(){
    big_avatar_rect->hide();
    small_avatar_rect->hide();
    tiny_avatar_rect->hide();
}

void CardScene::setAvatarNameBox(const QString &text){
    big_avatar_rect->setName(text);
    small_avatar_rect->setName(text);
    tiny_avatar_rect->setName(text);
}

void CardScene::resetPhoto(){
    if (photo){
        photo->deleteLater();
        Config.remove("CardEditor/Photo");
    }

    photo = new QSanSelectableItem;
    photo->setZValue(-2);
    photo->setFlag(QGraphicsItem::ItemIsMovable);
    addItem(photo);
}

void CardScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event){
    QGraphicsItem *item = itemAt(event->scenePos(), QTransform());
    if (item){
        if (item->parentItem() == skill_box){
            QGraphicsScene::contextMenuEvent(event);
            return;
        }
        else if (item == big_avatar_rect || item == small_avatar_rect || item == tiny_avatar_rect){
            done_menu->popup(event->screenPos());
            return;
        }
    }

    if (!skill_box->hasFocus() && menu){
        menu->popup(event->screenPos());
    }
}

void CardScene::setMenu(QMenu *menu){
    this->menu = menu;
}

void CardScene::setCompanion(const QString &text){
    companion_box->setText(text);
}

void CardScene::setCompanionFont(const QFont &font)
{
    companion_box->setFont(font);
}

void CardScene::setCompanionVisible(bool visible)
{
    companion_box->setVisible(visible);
}

CardEditor::CardEditor(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Card editor"));

    QHBoxLayout *layout = new QHBoxLayout;
    QGraphicsView *view = new QGraphicsView;

    view->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
        QPainter::SmoothPixmapTransform |
        QPainter::HighQualityAntialiasing
        );

    card_scene = new CardScene;
    connect(card_scene, &CardScene::avatar_snapped, this, &CardEditor::saveAvatar);

    view->setScene(card_scene);
    view->setFixedSize(card_scene->sceneRect().width() + 2,
        card_scene->sceneRect().height() + 2);

    layout->addWidget(createLeft());
    layout->addWidget(createMiddle());
    layout->addWidget(view);

    QWidget *central_widget = new QWidget;
    central_widget->setLayout(layout);
    setCentralWidget(central_widget);

    QMenuBar *menu_bar = new QMenuBar;
    setMenuBar(menu_bar);

    QMenu *file_menu = new QMenu(tr("File"));
    QAction *import = new QAction(tr("Import ..."), file_menu);
    import->setShortcut(Qt::CTRL + Qt::Key_O);
    QAction *save = new QAction(tr("Save ..."), file_menu);
    save->setShortcut(Qt::CTRL + Qt::Key_S);
    QAction *exit = new QAction(tr("Exit"), file_menu);
    exit->setShortcut(Qt::CTRL + Qt::Key_Q);

    file_menu->addAction(import);
    file_menu->addAction(save);
    file_menu->addSeparator();
    file_menu->addAction(exit);

    menu_bar->addMenu(file_menu);

    connect(import, &QAction::triggered, this, &CardEditor::import);
    connect(save, &QAction::triggered, this, &CardEditor::saveImage);
    connect(exit, &QAction::triggered, this, &CardEditor::close);

    QMenu *tool_menu = new QMenu(tr("Tool"));
    QAction *add_skill = new QAction(tr("Add skill"), tool_menu);
    add_skill->setShortcut(Qt::ALT + Qt::Key_S);
    connect(add_skill, &QAction::triggered, this, &CardEditor::addSkill);
    tool_menu->addAction(add_skill);

    QAction *remove_skill = new QAction(tr("Remove skill"), tool_menu);
    remove_skill->setShortcut(Qt::ALT + Qt::Key_D);
    connect(remove_skill, &QAction::triggered, card_scene->getSkillBox(), &SkillBox::removeSkill);
    tool_menu->addAction(remove_skill);

    QAction *edit_skill = new QAction(tr("Edit skill title ..."), tool_menu);
    edit_skill->setShortcut(Qt::ALT + Qt::Key_E);
    connect(edit_skill, &QAction::triggered, this, &CardEditor::editSkill);
    tool_menu->addAction(edit_skill);

    tool_menu->addSeparator();

    QAction *making_big = new QAction(tr("Make big avatar"), tool_menu);
    making_big->setShortcut(Qt::ALT + Qt::Key_B);
    connect(making_big, &QAction::triggered, card_scene, &CardScene::makeBigAvatar);
    tool_menu->addAction(making_big);

    QAction *making_small = new QAction(tr("Make small avatar"), tool_menu);
    making_small->setShortcut(Qt::ALT + Qt::Key_M);
    connect(making_small, &QAction::triggered, card_scene, &CardScene::makeSmallAvatar);
    tool_menu->addAction(making_small);

    QAction *making_tiny = new QAction(tr("Make tiny avatar"), tool_menu);
    making_tiny->setShortcut(Qt::ALT + Qt::Key_T);
    connect(making_tiny, &QAction::triggered, card_scene, &CardScene::makeTinyAvatar);
    tool_menu->addAction(making_tiny);

    QAction *hiding_rect = new QAction(tr("Hide avatar rect"), tool_menu);
    hiding_rect->setShortcut(Qt::ALT + Qt::Key_H);
    connect(hiding_rect, &QAction::triggered, card_scene, &CardScene::hideAvatarRects);
    tool_menu->addAction(hiding_rect);

    tool_menu->addSeparator();

    QAction *reset_photo = new QAction(tr("Reset photo"), tool_menu);
    reset_photo->setShortcut(Qt::ALT + Qt::Key_R);
    connect(reset_photo, &QAction::triggered, card_scene, &CardScene::resetPhoto);
    tool_menu->addAction(reset_photo);

    QAction *copy_photo = new QAction(tr("Copy photo to clipboard"), tool_menu);
    copy_photo->setShortcut(Qt::CTRL + Qt::Key_C);
    connect(copy_photo, &QAction::triggered, this, &CardEditor::copyPhoto);
    tool_menu->addAction(copy_photo);

    menu_bar->addMenu(tool_menu);

    card_scene->setMenu(tool_menu);

    DeleteAction = remove_skill;
    EditAction = edit_skill;
}

void CardEditor::updateButtonText(const QFont &font){
    QFontDialog *dialog = qobject_cast<QFontDialog *>(sender());
    if (dialog){
        QPushButton *button = dialog2button.value(dialog, NULL);
        if (button)
            button->setText(QString("%1[%2]").arg(font.family()).arg(font.pointSize()));
    }
}

void CardEditor::saveAvatar(const QRectF &rect){
    QString filename = QFileDialog::getSaveFileName(this,
        tr("Select a avatar file"),
        QString(),
        tr("Image file (*.png *.jpg *.bmp)"));

    if (!filename.isEmpty()){
        QImage image(rect.width(), rect.height(), QImage::Format_ARGB32);
        QPainter painter(&image);

        QPixmap pixmap = QPixmap::grabWidget(card_scene->views().first());
        pixmap = pixmap.copy(rect.toRect());

        QBitmap mask("diy/mask.png");
        pixmap.setMask(mask);

        painter.drawPixmap(0, 0, pixmap);

        image.save(filename);
    }
}

void CardEditor::setMapping(QFontDialog *dialog, QPushButton *button){
    dialog2button.insert(dialog, button);

    connect(dialog, &QFontDialog::currentFontChanged, this, &CardEditor::updateButtonText);
    connect(button, &QPushButton::clicked, dialog, &QFontDialog::exec);
}

QGroupBox *CardEditor::createTextItemBox(const QString &text, const QFont &font, int skip, BlackEdgeTextItem *item) {
    QGroupBox *box = new QGroupBox;

    QLineEdit *edit = new QLineEdit;
    QPushButton *font_button = new QPushButton(font.family());
    QSpinBox *size_spinbox = new QSpinBox;
    size_spinbox->setRange(1, 96);
    QSpinBox *space_spinbox = new QSpinBox;
    space_spinbox->setRange(0, 100);

    edit->setObjectName("name");
    size_spinbox->setObjectName("font");
    space_spinbox->setObjectName("space");

    QFormLayout *layout = new QFormLayout;
    layout->addRow(tr("Text"), edit);
    layout->addRow(tr("Font"), font_button);
    layout->addRow(tr("Line spacing"), space_spinbox);

    QFontDialog *font_dialog = new QFontDialog(this);
    setMapping(font_dialog, font_button);

    connect(edit, &QLineEdit::textChanged, item, &BlackEdgeTextItem::setText);
    connect(font_dialog, &QFontDialog::currentFontChanged, item, &BlackEdgeTextItem::setFont);
    connect(space_spinbox, (void (QSpinBox::*)(int))(&QSpinBox::valueChanged), item, &BlackEdgeTextItem::setSkip);

    edit->setText(text);
    font_dialog->setCurrentFont(font);
    space_spinbox->setValue(skip);

    box->setLayout(layout);

    return box;
}

QWidget *CardEditor::createPropertiesBox() {
    kingdom_ComboBox = new QComboBox;
    lord_checkbox = new QCheckBox(tr("Lord"));
    QStringList kingdom_names = Sanguosha->getKingdoms();
    foreach(QString kingdom, kingdom_names){
        if ("god" == kingdom) continue;
        QIcon icon(QString("image/kingdom/icon/%1.png").arg(kingdom));
        kingdom_ComboBox->addItem(icon, Sanguosha->translate(kingdom), kingdom);
    }

    QSpinBox *hp_spinbox = new QSpinBox;
    hp_spinbox->setRange(0, 6);

    QSpinBox *trans_hp_spinbox = new QSpinBox;
    trans_hp_spinbox->setRange(0, 6);

    ratio_spinbox = new QSpinBox;
    ratio_spinbox->setRange(1, 1600);
    ratio_spinbox->setValue(100);
    ratio_spinbox->setSuffix(" %");

    QLineEdit *companion_edit = new QLineEdit;
    connect(companion_edit, &QLineEdit::textChanged, card_scene, &CardScene::setCompanion);

    QFormLayout *layout = new QFormLayout;
    QHBoxLayout *klayout = new QHBoxLayout;
    klayout->addWidget(kingdom_ComboBox);
    klayout->addWidget(lord_checkbox);
    layout->addRow(tr("Kingdom"), klayout);
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(hp_spinbox);
    hlayout->addWidget(new QLabel(tr("Trans Max HP")));
    hlayout->addWidget(trans_hp_spinbox);
    layout->addRow(tr("Max HP"), hlayout);
    layout->addRow(tr("Image ratio"), ratio_spinbox);

    QHBoxLayout *hlayout2 = new QHBoxLayout;

    QPushButton *companion_font_button = new QPushButton;
    QFontDialog *companion_font_dialog = new QFontDialog(this);

    connect(companion_font_dialog, &QFontDialog::currentFontChanged, card_scene, &CardScene::setCompanionFont);
    setMapping(companion_font_dialog, companion_font_button);

    companion_font_dialog->setCurrentFont(Config.value("CardEditor/CompanionFont", QFont("", 9)).value<QFont>());

    hlayout2->addWidget(companion_font_button);

    QCheckBox *show_companion_box = new QCheckBox(tr("Show companion box"));
    hlayout2->addWidget(show_companion_box);
    layout->addRow(tr("Companion"), companion_edit);
    layout->addRow(hlayout2);

    connect(kingdom_ComboBox, (void (QComboBox::*)(int))(&QComboBox::currentIndexChanged), this, &CardEditor::setCardFrame);
    connect(lord_checkbox, &QCheckBox::toggled, this, &CardEditor::setCardFrame);
    connect(hp_spinbox, (void (QSpinBox::*)(int))(&QSpinBox::valueChanged), card_scene, &CardScene::setMaxHp);
    connect(trans_hp_spinbox, (void (QSpinBox::*)(int))(&QSpinBox::valueChanged), card_scene, &CardScene::setTransMaxHp);
    connect(ratio_spinbox, (void (QSpinBox::*)(int))(&QSpinBox::valueChanged), card_scene, &CardScene::setRatio);
    connect(show_companion_box, &QCheckBox::toggled, card_scene, &CardScene::setCompanionVisible);
    QString kingdom = Config.value("CardEditor/Kingdom", "wei").toString();
    bool is_lord = Config.value("CardEditor/IsLord", false).toBool();

    kingdom_ComboBox->setCurrentIndex(kingdom_names.indexOf(kingdom));
    lord_checkbox->setChecked(is_lord);
    hp_spinbox->setValue(Config.value("CardEditor/MaxHP", 3).toInt());
    trans_hp_spinbox->setValue(Config.value("CardEditor/TransMaxHP", 0).toInt());
    ratio_spinbox->setValue(Config.value("CardEditor/ImageRatio", 100).toInt());
    companion_edit->setText(Config.value("CardEditor/Companion").toString());
    show_companion_box->setChecked(Config.value("CardEditor/CompanionVisible", false).toBool());
    QString photo = Config.value("CardEditor/Photo").toString();
    if (!photo.isEmpty())
        card_scene->setGeneralPhoto(photo);

    setCardFrame();

    QGroupBox *box = new QGroupBox(tr("Properties"));
    box->setLayout(layout);
    return box;
}

QWidget *CardEditor::createSkillBox() {
    QGroupBox *box = new QGroupBox(tr("Skill"));

    QFormLayout *layout = new QFormLayout;

    QPushButton *title_font_button = new QPushButton;
    QPushButton *desc_font_button = new QPushButton;
    QPushButton *tiny_font_button = new QPushButton;

    QFontDialog *title_font_dialog = new QFontDialog(this);
    QFontDialog *desc_font_dialog = new QFontDialog(this);
    QFontDialog *tiny_font_dialog = new QFontDialog(this);

    layout->addRow(tr("Title font"), title_font_button);
    layout->addRow(tr("Description font"), desc_font_button);
    layout->addRow(tr("Tiny font"), tiny_font_button);

    SkillBox *skill_box = card_scene->getSkillBox();
    connect(title_font_dialog, &QFontDialog::currentFontChanged, skill_box, &SkillBox::setSkillTitleFont);
    connect(desc_font_dialog, &QFontDialog::currentFontChanged, skill_box, &SkillBox::setSkillDescriptionFont);
    connect(tiny_font_dialog, &QFontDialog::currentFontChanged, skill_box, &SkillBox::setTinyFont);

    setMapping(title_font_dialog, title_font_button);
    setMapping(desc_font_dialog, desc_font_button);
    setMapping(tiny_font_dialog, tiny_font_button);

    title_font_dialog->setCurrentFont(Config.value("CardEditor/SkillTitleFont", QFont("", 15)).value<QFont>());
    desc_font_dialog->setCurrentFont(Config.value("CardEditor/SkillDescriptionFont", QFont("", 9)).value<QFont>());
    tiny_font_dialog->setCurrentFont(Config.value("CardEditor/TinyFont", QFont("", 7)).value<QFont>());

    QComboBox *suit_ComboBox = new QComboBox;
    const Card::Suit *suits = Card::AllSuits;
    int i;
    for (i = 0; i < 4; i++){
        QString suit_name = Card::Suit2String(suits[i]);
        QIcon suit_icon(QString("image/system/suit/%1.png").arg(suit_name));
        suit_ComboBox->addItem(suit_icon, Sanguosha->translate(suit_name), suit_name);
    }
    layout->addRow(tr("Insert suit"), suit_ComboBox);
    //temp solution
    suit_ComboBox->setEnabled(false);
    suit_ComboBox->setToolTip(tr("<font color=%1>This function cannot be used now, we will try to fix it.</font>").arg(Config.SkillDescriptionInToolTipColor.name()));

    connect(suit_ComboBox, (void (QComboBox::*)(int))(&QComboBox::activated), skill_box, &SkillBox::insertSuit);

    QComboBox *bold_ComboBox = new QComboBox;
    bold_ComboBox->setEditable(true);
    bold_ComboBox->addItem(tr("Compulsory"));
    bold_ComboBox->addItem(tr("Limited"));
    bold_ComboBox->addItem(tr("Formation skill"));
    bold_ComboBox->addItem(tr("Head skill"));
    bold_ComboBox->addItem(tr("Deputy skill"));
    layout->addRow(tr("Insert bold text"), bold_ComboBox);

    connect(bold_ComboBox, (void (QComboBox::*)(const QString &))(&QComboBox::activated), skill_box, &SkillBox::insertBoldText);

    box->setLayout(layout);
    return box;
}

void CardEditor::closeEvent(QCloseEvent *event){
    QMainWindow::closeEvent(event);

    card_scene->saveConfig();
}

QWidget *CardEditor::createLeft(){
    QVBoxLayout *layout = new QVBoxLayout;
    QGroupBox *box = createTextItemBox(Config.value("CardEditor/TitleText", tr("Title")).toString(),
#ifdef Q_OS_LINUX
        Config.value("CardEditor/TitleFont", QFont("DroidSansFallback", 20)).value<QFont>(),
#else
        Config.value("CardEditor/TitleFont", QFont("Times", 20)).value<QFont>(),
#endif
        Config.value("CardEditor/TitleSkip", 0).toInt(),
        card_scene->getTitleItem());
    box->setTitle(tr("Title"));
    layout->addWidget(box);

    box = createTextItemBox(Config.value("CardEditor/NameText", tr("Name")).toString(),
#ifdef Q_OS_LINUX
        Config.value("CardEditor/NameFont", QFont("DroidSansFallback", 36)).value<QFont>(),
#else
        Config.value("CardEditor/NameFont", QFont("Times", 36)).value<QFont>(),
#endif
        Config.value("CardEditor/NameSkip", 0).toInt(),
        card_scene->getNameItem());

    QLineEdit *name_edit = box->findChild<QLineEdit *>("name");
    connect(name_edit, &QLineEdit::textChanged, card_scene, &CardScene::setAvatarNameBox);

    box->setTitle(tr("Name"));
    layout->addWidget(box);

    QWidget *widget = new QWidget;
    widget->setLayout(layout);
    return widget;
}

QWidget *CardEditor::createMiddle()
{
    QVBoxLayout *layout = new QVBoxLayout;

    layout->addWidget(createSkillBox());
    layout->addWidget(createPropertiesBox());

    QWidget *widget = new QWidget;
    widget->setLayout(layout);
    return widget;
}

void CardEditor::setCardFrame(){
    QString kingdom = kingdom_ComboBox->itemData(kingdom_ComboBox->currentIndex()).toString();
    card_scene->setFrame(kingdom, lord_checkbox->isChecked());
}

void CardEditor::import(){
    QString filename = QFileDialog::getOpenFileName(this,
        tr("Select a photo file ..."),
        Config.value("CardEditor/ImportPath").toString(),
        tr("Images (*.png *.bmp *.jpg)")
        );

    if (!filename.isEmpty()){
        card_scene->setGeneralPhoto(filename);
        Config.setValue("CardEditor/ImportPath", QFileInfo(filename).absolutePath());
    }
}

void CardEditor::saveImage(){
    QString filename = QFileDialog::getSaveFileName(this,
        tr("Select a photo file ..."),
        Config.value("CardEditor/ExportPath").toString(),
        tr("Images (*.png *.bmp *.jpg)")
        );

    if (!filename.isEmpty()){
        card_scene->clearFocus();
        QPixmap::grabWidget(card_scene->views().first()).save(filename);
        Config.setValue("CardEditor/ExportPath", QFileInfo(filename).absolutePath());
    }
}

void CardEditor::copyPhoto(){
    card_scene->clearFocus();

    QPixmap pixmap = QPixmap::grabWidget(card_scene->views().first());
    qApp->clipboard()->setPixmap(pixmap);
}

void CardEditor::addSkill(){
    QString text = QInputDialog::getText(this, tr("Add skill"), tr("Please input the skill title:"));
    if (!text.isEmpty())
        card_scene->getSkillBox()->addSkill(text);
}

void CardEditor::editSkill(){
    SkillTitle *to_edit = card_scene->getSkillBox()->getFocusTitle();
    if (to_edit == NULL)
        return;

    QString text = QInputDialog::getText(this,
        tr("Edit skill title"),
        tr("Please input the skill title:"),
        QLineEdit::Normal,
        to_edit->text());
    if (!text.isEmpty())
        to_edit->setText(text);
}
