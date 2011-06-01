#include "cardeditor.h"
#include "mainwindow.h"
#include "engine.h"
#include "settings.h"
#include "pixmap.h"

#include <QFormLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QLabel>
#include <QGraphicsSceneMouseEvent>
#include <QApplication>
#include <QCursor>
#include <QKeyEvent>

BlackEdgeTextItem::BlackEdgeTextItem()
    :skip(0), color(Qt::white), outline(3)
{
    setFlag(ItemIsMovable);
}

QRectF BlackEdgeTextItem::boundingRect() const{
    if(text.isEmpty())
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

void BlackEdgeTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    if(text.isEmpty())
        return;

    painter->setRenderHint(QPainter::Antialiasing);

    QPen pen(Qt::black);
    pen.setWidth(outline);
    painter->setPen(pen);

    QFontMetrics metric(font);
    int height = metric.height() - metric.descent() + skip;

    int i;
    for(i=0; i<text.length(); i++){

        QString text;
        text.append(this->text.at(i));

        QPainterPath path;
        path.addText(0, (i+1) * height, font, text);

        painter->drawPath(path);
        painter->fillPath(path, color);
    }
}

SkillBox::SkillBox()
    :middle_height(0)
{
    setAcceptedMouseButtons(Qt::LeftButton);

    skill_description = new QGraphicsTextItem(tr("Skill description"), this);
    skill_description->setFont(Config.value("CardEditor/SkillDescriptionFont").value<QFont>());
    skill_description->setTextWidth(223);
    skill_description->setFlag(ItemIsMovable);
    skill_description->setTextInteractionFlags(Qt::TextEditorInteraction);
    skill_description->setX(25);

    connect(skill_description->document(), SIGNAL(blockCountChanged(int)), this, SLOT(updateLayout()));
}

void SkillBox::setKingdom(const QString &kingdom){
    this->kingdom = kingdom;
    up.load(QString("diy/%1-skill-up.png").arg(kingdom));
    down.load(QString("diy/%1-skill-down.png").arg(kingdom));
    middle.load(QString("diy/%1-skill-middle.png").arg(kingdom));

    foreach(QGraphicsTextItem *skill_title, skill_titles){
        QGraphicsItem *item = skill_title->parentItem();
        QGraphicsPixmapItem *pixmap_item = qgraphicsitem_cast<QGraphicsPixmapItem *>(item);
        pixmap_item->setPixmap(QPixmap(QString("diy/%1-skill.png").arg(kingdom)));

        if(kingdom == "god")
            skill_title->setDefaultTextColor(QColor(255, 255, 102));
        else
            skill_title->setDefaultTextColor(Qt::black);
    }
}

void SkillBox::setMiddleHeight(int height){
    int new_height = height < 0 ? middle.height() : height;
    if(middle_height != new_height){
        middle_height = new_height;
        skill_description->setY(- middle_height - down.height());
        prepareGeometryChange();
    }
}

AATextItem::AATextItem(const QString &text, QGraphicsItem *parent)
    :QGraphicsTextItem(text, parent)
{
}

void AATextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    if(hasFocus()){
        QGraphicsTextItem::paint(painter, option, widget);
        return;
    }

    painter->setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    QFontMetrics fm(font());
    path.addText(document()->documentMargin(), fm.height(), font(), toPlainText());
    painter->fillPath(path, defaultTextColor());
}

void AATextItem::keyReleaseEvent(QKeyEvent *event){
    if(!hasFocus()){
        event->ignore();
        return;
    }

    int delta_y = 0;
    switch(event->key()){
    case Qt::Key_Up: delta_y = -1; break;
    case Qt::Key_Down: delta_y = 1; break;
    default:
        break;
    }

    if(delta_y == 0){
        event->ignore();
        return;
    }

    if(event->modifiers() & Qt::ShiftModifier)
        delta_y *= 5;

    event->accept();
    parentItem()->moveBy(0, delta_y);
}

void SkillBox::addSkill(){
    QPixmap title_pixmap(QString("diy/%1-skill.png").arg(kingdom));    
    QGraphicsPixmapItem *skill_title = scene()->addPixmap(title_pixmap);
    qreal last_y = 389;
    if(!skill_titles.isEmpty()){
        QGraphicsItem *last = skill_titles.last()->parentItem();
        last_y = last->y() + last->boundingRect().height();
    }

    skill_title->setPos(32, last_y);

    QGraphicsTextItem *title_text = new AATextItem(tr("Skill"), skill_title);
    title_text->setFont(Config.value("CardEditor/SkillTitleFont").value<QFont>());
    title_text->setTextInteractionFlags(Qt::TextEditorInteraction);
    title_text->setPos(Config.value("CardEditor/TitleTextOffset", QPointF(10, -2)).toPointF());
    title_text->document()->setDocumentMargin(0);

    skill_titles << title_text;
}

void SkillBox::setSkillTitleFont(const QFont &font){
    Config.setValue("CardEditor/SkillTitleFont", font);

    foreach(QGraphicsTextItem *item, skill_titles){
        item->setFont(font);
    }
}

void SkillBox::setSkillDescriptionFont(const QFont &font){
    Config.setValue("CardEditor/SkillDescriptionFont", font);

    skill_description->setFont(font);
}

void SkillBox::updateLayout(){
    // dummy
}

void SkillBox::insertSuit(int index){
    Card::Suit suit = static_cast<Card::Suit>(index);
    QString suit_name = Card::Suit2String(suit);
    QImage image(QString("image/system/suit/%1.png").arg(suit_name));

    int size = skill_description->font().pointSize() + 1;
    image = image.scaled(QSize(size, size), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    skill_description->textCursor().insertImage(image);
}

void SkillBox::insertBoldText(const QString &bold_text){
    QTextCharFormat format;
    format.setFontWeight(QFont::Bold);
    skill_description->textCursor().insertText(bold_text, format);

    skill_description->textCursor().insertText(",", QTextCharFormat());
}

QRectF SkillBox::boundingRect() const{
    // left down cornor is the origin
    int height = up.height() + middle_height + down.height();
    return QRectF(0, -height, up.width(), height);
}

void SkillBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *){
    // from down to up
    painter->drawPixmap(0, -down.height(), down);
    painter->drawTiledPixmap(0, -down.height()-middle_height, middle.width(), middle_height, middle);
    painter->drawPixmap(0, -down.height()-middle_height-up.height(), up);
}

void SkillBox::mousePressEvent(QGraphicsSceneMouseEvent *event){
    QApplication::setOverrideCursor(QCursor(Qt::SizeVerCursor));
}

void SkillBox::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    int diff = event->pos().y() - event->lastPos().y();
    setMiddleHeight(middle_height - diff);
}

void SkillBox::mouseReleaseEvent(QGraphicsSceneMouseEvent *){
    QApplication::restoreOverrideCursor();
}

CardScene::CardScene()
    :QGraphicsScene(QRectF(0, 0, 366, 514))
{
    photo = new Pixmap;
    frame = new QGraphicsPixmapItem;

    name = new BlackEdgeTextItem;
    name->setObjectName("Name");
    name->setOutline(5);

    title = new BlackEdgeTextItem;
    title->setObjectName("Title");

    photo->setFlag(QGraphicsItem::ItemIsMovable);

    skill_box = new SkillBox;

    addItem(photo);
    addItem(frame);
    addItem(name);
    addItem(title);
    addItem(skill_box);

    int i;
    for(i=0; i<10; i++){
        QGraphicsPixmapItem *item = new QGraphicsPixmapItem;
        magatamas << item;
        item->hide();
        addItem(item);

        item->setPos(94 + i*(115-94), 18);
    }

    loadConfig();
}

void CardScene::setFrame(const QString &kingdom, bool is_lord){
    QString path;
    if(is_lord){
        path = QString("diy/%1-lord.png").arg(kingdom);

        static QMap<QString, QColor> color_map;
        if(color_map.isEmpty()){
            color_map["wei"] = QColor(88, 101, 205);
            color_map["shu"] = QColor(234, 137, 72);
            color_map["wu"] = QColor(167, 221, 102);
            color_map["qun"] = QColor(146, 146, 146);
            color_map["god"] = QColor(252, 219, 85);
        }
        title->setColor(color_map.value(kingdom));
    }else{
        path = QString("diy/%1.png").arg(kingdom);
        title->setColor(QColor(252, 219, 85));
    }

    frame->setPixmap(QPixmap(path));

    foreach(QGraphicsPixmapItem *item, magatamas){
        item->setPixmap(QPixmap(QString("diy/%1-magatama.png").arg(kingdom)));
    }

    skill_box->setKingdom(kingdom);
    skill_box->setMiddleHeight(-1);

    Config.setValue("CardEditor/Kingdom", kingdom);
    Config.setValue("CardEditor/IsLord", is_lord);
}

void CardScene::setGeneralPhoto(const QString &filename){
    photo->changePixmap(filename);

    Config.setValue("CardEditor/Photo", filename);
}

void SkillBox::setTextEditable(bool editable){
    Qt::TextInteractionFlags flags = editable ? Qt::TextEditorInteraction : Qt::NoTextInteraction;

    foreach(QGraphicsTextItem *item, skill_titles)
        item->setTextInteractionFlags(flags);

    skill_description->setTextInteractionFlags(flags);
}

void CardScene::saveConfig(){
    Config.beginGroup("CardEditor");
    Config.setValue("NamePos", name->pos());
    Config.setValue("TitlePos", title->pos());
    Config.setValue("PhotoPos", photo->pos());
    Config.setValue("SkillBoxPos", skill_box->pos());
    Config.endGroup();
}

void CardScene::loadConfig(){
    Config.beginGroup("CardEditor");
    name->setPos(Config.value("NamePos", QPointF(28, 206)).toPointF());
    title->setPos(Config.value("TitlePos", QPointF(49, 128)).toPointF());
    photo->setPos(Config.value("PhotoPos").toPointF());
    skill_box->setPos(Config.value("SkillBoxPos", QPointF(70, 484)).toPointF());
    Config.endGroup();
}


BlackEdgeTextItem *CardScene::getNameItem() const{
    return name;
}

BlackEdgeTextItem *CardScene::getTitleItem() const{
    return title;
}

SkillBox *CardScene::getSkillBox() const{
    return skill_box;
}

#ifdef QT_DEBUG

#include <QKeyEvent>
#include <QMessageBox>

void CardScene::keyPressEvent(QKeyEvent *event){
    QGraphicsScene::keyPressEvent(event);

    if(event->key() == Qt::Key_D){
        //QMessageBox::information(NULL, "", QString("%1, %2").arg(skill_box->x()).arg(skill_box->y()));
    }
}

#endif

void CardScene::setRatio(int ratio){
    photo->setScale(ratio / 100.0);

    Config.setValue("CardEditor/ImageRatio", ratio);
}

void CardScene::setMaxHp(int max_hp){    
    int n = magatamas.length();
    max_hp = qBound(0, max_hp, n-1);

    int i;
    for(i=0; i<n; i++)
        magatamas.at(i)->setVisible(i < max_hp);

    Config.setValue("CardEditor/MaxHP", max_hp);
}

#include <QMenu>
#include <QMenuBar>


CardEditor::CardEditor(QWidget *parent) :
    QMainWindow(parent)
{
    setWindowTitle(tr("Card editor"));

    QHBoxLayout *layout = new QHBoxLayout;
    QGraphicsView *view = new QGraphicsView;

    view->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
                        QPainter::SmoothPixmapTransform	|
                        QPainter::HighQualityAntialiasing
                        );

    card_scene = new CardScene;
    view->setScene(card_scene);
    view->setFixedSize(card_scene->sceneRect().width() + 2,
                       card_scene->sceneRect().height() + 2);

    layout->addWidget(createLeft());
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
    file_menu->addAction(import);
    file_menu->addAction(save);

    menu_bar->addMenu(file_menu);

    connect(import, SIGNAL(triggered()), this, SLOT(import()));
    connect(save, SIGNAL(triggered()), this, SLOT(saveImage()));

    QMenu *tool_menu = new QMenu(tr("Tool"));
    QAction *add_skill = new QAction(tr("Add skill"), tool_menu);
    add_skill->setShortcut(Qt::ALT + Qt::Key_S);
    connect(add_skill, SIGNAL(triggered()), card_scene->getSkillBox(), SLOT(addSkill()));
    tool_menu->addAction(add_skill);

    menu_bar->addMenu(tool_menu);
}

void CardEditor::updateButtonText(const QFont &font){
    QFontDialog *dialog = qobject_cast<QFontDialog *>(sender());
    if(dialog){
        QPushButton *button = dialog2button.value(dialog, NULL);
        if(button)
            button->setText(QString("%1[%2]").arg(font.family()).arg(font.pointSize()));
    }
}

void CardEditor::setMapping(QFontDialog *dialog, QPushButton *button){
    dialog2button.insert(dialog, button);

    connect(dialog, SIGNAL(currentFontChanged(QFont)), this, SLOT(updateButtonText(QFont)));
    connect(button, SIGNAL(clicked()), dialog, SLOT(exec()));
}

QGroupBox *CardEditor::createTextItemBox(const QString &text, const QFont &font, int skip, BlackEdgeTextItem *item){
    QGroupBox *box = new QGroupBox;

    QLineEdit *edit = new QLineEdit;
    QPushButton *font_button = new QPushButton(font.family());
    QSpinBox *size_spinbox = new QSpinBox;
    size_spinbox->setRange(1, 96);
    QSpinBox *space_spinbox = new QSpinBox;
    space_spinbox->setRange(0, 100);

    QFormLayout *layout = new QFormLayout;
    layout->addRow(tr("Text"), edit);
    layout->addRow(tr("Font"), font_button);
    layout->addRow(tr("Line spacing"), space_spinbox);

    QFontDialog *font_dialog = new QFontDialog(this);
    setMapping(font_dialog, font_button);

    connect(edit, SIGNAL(textChanged(QString)), item, SLOT(setText(QString)));
    connect(font_dialog, SIGNAL(currentFontChanged(QFont)), item, SLOT(setFont(QFont)));
    connect(space_spinbox, SIGNAL(valueChanged(int)), item, SLOT(setSkip(int)));

    edit->setText(text);
    font_dialog->setCurrentFont(font);
    space_spinbox->setValue(skip);

    box->setLayout(layout);

    return box;
}

QLayout *CardEditor::createGeneralLayout(){
    kingdom_combobox = new QComboBox;
    lord_checkbox = new QCheckBox(tr("Lord"));
    QStringList kingdom_names = Sanguosha->getKingdoms();
    foreach(QString kingdom, kingdom_names){
        QIcon icon(QString("image/kingdom/icon/%1.png").arg(kingdom));
        kingdom_combobox->addItem(icon, Sanguosha->translate(kingdom), kingdom);
    }

    QSpinBox *hp_spinbox = new QSpinBox;
    hp_spinbox->setRange(0, 10);

    ratio_spinbox = new QSpinBox;
    ratio_spinbox->setRange(1, 1600);
    ratio_spinbox->setValue(100);
    ratio_spinbox->setSuffix(" %");

    QFormLayout *layout = new QFormLayout;
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(kingdom_combobox);
    hlayout->addWidget(lord_checkbox);
    layout->addRow(tr("Kingdom"), hlayout);
    layout->addRow(tr("Max HP"), hp_spinbox);
    layout->addRow(tr("Image ratio"), ratio_spinbox);

    connect(kingdom_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(setCardFrame()));
    connect(lord_checkbox, SIGNAL(toggled(bool)), this, SLOT(setCardFrame()));
    connect(hp_spinbox, SIGNAL(valueChanged(int)), card_scene, SLOT(setMaxHp(int)));
    connect(ratio_spinbox, SIGNAL(valueChanged(int)), card_scene, SLOT(setRatio(int)));

    QString kingdom = Config.value("CardEditor/Kingdom", "wei").toString();
    int is_lord = Config.value("CardEditor/IsLord", false).toBool();
    kingdom_combobox->setCurrentIndex(kingdom_names.indexOf(kingdom));
    lord_checkbox->setChecked(is_lord);
    hp_spinbox->setValue(Config.value("CardEditor/MaxHP", 3).toInt());
    ratio_spinbox->setValue(Config.value("CardEditor/ImageRatio", 100).toInt());
    QString photo = Config.value("CardEditor/Photo").toString();
    if(!photo.isEmpty())
        card_scene->setGeneralPhoto(photo);

    setCardFrame();

    return layout;
}

QWidget *CardEditor::createSkillBox(){
    QGroupBox *box = new QGroupBox(tr("Skill"));

    QFormLayout *layout = new QFormLayout;
    QPushButton *title_font_button = new QPushButton;
    QPushButton *desc_font_button = new QPushButton;
    QFontDialog *title_font_dialog = new QFontDialog(this);
    QFontDialog *desc_font_dialog = new QFontDialog(this);

    layout->addRow(tr("Title font"), title_font_button);
    layout->addRow(tr("Description font"), desc_font_button);

    SkillBox *skill_box = card_scene->getSkillBox();
    connect(title_font_dialog, SIGNAL(currentFontChanged(QFont)), skill_box, SLOT(setSkillTitleFont(QFont)));
    connect(desc_font_dialog, SIGNAL(currentFontChanged(QFont)), skill_box, SLOT(setSkillDescriptionFont(QFont)));

    setMapping(title_font_dialog, title_font_button);
    setMapping(desc_font_dialog, desc_font_button);

    title_font_dialog->setCurrentFont(Config.value("CardEditor/SkillTitleFont", QFont("", 15)).value<QFont>());
    desc_font_dialog->setCurrentFont(Config.value("CardEditor/SkillDescriptionFont", QFont("", 9)).value<QFont>());

    QComboBox *suit_combobox = new QComboBox;
    const Card::Suit *suits = Card::AllSuits;
    int i;
    for(i=0; i<4; i++){
        QString suit_name = Card::Suit2String(suits[i]);
        QIcon suit_icon(QString("image/system/suit/%1.png").arg(suit_name));
        suit_combobox->addItem(suit_icon, Sanguosha->translate(suit_name), suit_name);
    }
    layout->addRow(tr("Insert suit"), suit_combobox);

    connect(suit_combobox, SIGNAL(activated(int)), skill_box, SLOT(insertSuit(int)));

    QComboBox *bold_combobox = new QComboBox;
    bold_combobox->setEditable(true);
    bold_combobox->addItem(tr("Compulsory"));
    bold_combobox->addItem(tr("Limited"));
    layout->addRow(tr("Insert bold text"), bold_combobox);

    connect(bold_combobox, SIGNAL(activated(QString)), skill_box, SLOT(insertBoldText(QString)));

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
                                       Config.value("CardEditor/TitleFont", QFont("Times", 20)).value<QFont>(),
                                       Config.value("CardEditor/TitleSkip", 0).toInt(),
                                       card_scene->getTitleItem());
    box->setTitle(tr("Title"));
    layout->addWidget(box);

    box = createTextItemBox(Config.value("CardEditor/NameText", tr("Name")).toString(),
                            Config.value("CardEditor/NameFont", QFont("Times", 36)).value<QFont>(),
                            Config.value("CardEditor/NameSkip", 0).toInt(),
                            card_scene->getNameItem());

    box->setTitle(tr("Name"));
    layout->addWidget(box);

    layout->addLayout(createGeneralLayout());
    layout->addWidget(createSkillBox());
    layout->addStretch();
    layout->addWidget(new QLabel(tr("Thanks to BeiwanLufen <img width='50' height='50' src='diy/lufen.jpg' />")));

    QWidget *widget = new QWidget;
    widget->setLayout(layout);
    return widget;
}

void CardEditor::setCardFrame(){
    QString kingdom = kingdom_combobox->itemData(kingdom_combobox->currentIndex()).toString();
    if(kingdom == "god")
        card_scene->setFrame("god", false);
    else
        card_scene->setFrame(kingdom, lord_checkbox->isChecked());
}

void CardEditor::import(){
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Select a photo file ..."),
                                                    Config.value("CardEditor/ImportPath").toString(),
                                                    tr("Images (*.png *.bmp *.jpg)")
                                                    );

    if(!filename.isEmpty()){
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

    if(!filename.isEmpty()){
        QPixmap::grabWidget(card_scene->views().first()).save(filename);
        Config.setValue("CardEditor/ExportPath", QFileInfo(filename).absolutePath());
    }
}

void MainWindow::on_actionCard_editor_triggered()
{
    CardEditor *editor = new CardEditor(this);
    editor->show();
}
