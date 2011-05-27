#include "cardeditor.h"
#include "mainwindow.h"
#include "engine.h"
#include "settings.h"

#include <QFormLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QCommandLinkButton>
#include <QFontDatabase>
#include <QLabel>
#include <QFontComboBox>
#include <QGraphicsSceneMouseEvent>
#include <QApplication>
#include <QCursor>

BlackEdgeTextItem::BlackEdgeTextItem()
    :skip(0), color(Qt::white)
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

void BlackEdgeTextItem::setText(const QString &text){
    this->text = text;

    prepareGeometryChange();

    Config.setValue("CardEditor/" + objectName() + "Text", text);
}

void BlackEdgeTextItem::setFont(const QFont &font){
    int size = this->font.pointSize();
    this->font = font;
    this->font.setPointSize(size);

    prepareGeometryChange();

    Config.setValue("CardEditor/" + objectName() + "Font", font);
}

void BlackEdgeTextItem::setFontSize(int size){
    font.setPointSize(size);
    prepareGeometryChange();

    Config.setValue("CardEditor/" + objectName() + "FontSize", size);
}

void BlackEdgeTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    if(text.isEmpty())
        return;

    painter->setRenderHint(QPainter::Antialiasing);

    QPen pen(Qt::black);
    pen.setWidth(3);
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
}

void SkillBox::setKingdom(const QString &kingdom){
    this->kingdom = kingdom;
    up.load(QString("diy/%1-skill-up.png").arg(kingdom));
    down.load(QString("diy/%1-skill-down.png").arg(kingdom));
    middle.load(QString("diy/%1-skill-middle.png").arg(kingdom));
}

void SkillBox::setMiddleHeight(int height){
    if(height < 0){
        middle_height = middle.height();
        prepareGeometryChange();
    }

    if(height >= middle.height()){
        middle_height = height;
        prepareGeometryChange();
    }
}

void SkillBox::addSkill(){
    QGraphicsTextItem *text_item = new QGraphicsTextItem(tr("Skill description"), this);
    text_item->setTextWidth(middle.width());
    text_item->setFlag(ItemIsMovable);
    //text_item->setTextInteractionFlags(Qt::TextEditorInteraction);

    QPixmap title_pixmap(QString("diy/%1-skill.png").arg(kingdom));
    QGraphicsPixmapItem *skill_title = new QGraphicsPixmapItem(title_pixmap, text_item);
    skill_title->setX(-39);

    QGraphicsTextItem *title_text = new QGraphicsTextItem(tr("Skill"), skill_title);
    //title_text->setTextInteractionFlags(Qt::TextEditorInteraction);
    title_text->setPos(0, 0);

    skill_titles << title_text;
    skill_descriptions << text_item;

    connect(text_item->document(), SIGNAL(contentsChanged()), this, SLOT(updateLayout()));
    updateLayout();
}

void SkillBox::setSkillTitleFont(const QFont &font){
    Config.setValue("CardEditor/SkillNameFont", font);

    foreach(QGraphicsTextItem *item, skill_titles){
        item->setFont(font);
    }
}

void SkillBox::setSkillDescriptionFont(const QFont &font){
    Config.setValue("CardEditor/SkillDescriptionFont", font);

    foreach(QGraphicsTextItem *item, skill_descriptions){
        item->setFont(font);
    }
}

void SkillBox::updateLayout(){
    int i, n = skill_titles.length();
    qreal height = 0;
    for(i=0; i<n; i++){
        QGraphicsTextItem *item = skill_descriptions.at(n-1-i);
        QRectF rect = item->boundingRect();
        height += rect.height();
        item->setY(- height);
    }
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
    photo = new QGraphicsPixmapItem;
    frame = new QGraphicsPixmapItem;

    name = new BlackEdgeTextItem;
    name->setPos(28, 206);
    name->setObjectName("Name");

    title = new BlackEdgeTextItem;
    title->setPos(49, 128);
    title->setObjectName("Title");

    photo->setFlag(QGraphicsItem::ItemIsMovable);

    skill_box = new SkillBox;
    skill_box->setPos(70, 484);

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
    photo->setPixmap(QPixmap(filename));
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
    card_scene = new CardScene;
    view->setScene(card_scene);
    view->setMinimumSize(380, 530);

    layout->addWidget(createLeft());
    layout->addWidget(view);

    QWidget *central_widget = new QWidget;
    central_widget->setLayout(layout);
    setCentralWidget(central_widget);

    QMenuBar *menu_bar = new QMenuBar;
    setMenuBar(menu_bar);

    QMenu *file_menu = new QMenu(tr("File"));
    QAction *import = new QAction(tr("Import ..."), file_menu);
    QAction *save = new QAction(tr("Save ..."), file_menu);
    file_menu->addAction(import);
    file_menu->addAction(save);

    menu_bar->addMenu(file_menu);

    connect(import, SIGNAL(triggered()), this, SLOT(import()));
    connect(save, SIGNAL(triggered()), this, SLOT(saveImage()));

    QMenu *tool_menu = new QMenu(tr("Tool"));
    QAction *add_skill = new QAction(tr("Add skill"), tool_menu);
    connect(add_skill, SIGNAL(triggered()), card_scene->getSkillBox(), SLOT(addSkill()));
    tool_menu->addAction(add_skill);

    menu_bar->addMenu(tool_menu);
}

QGroupBox *CardEditor::createTextItemBox(const QString &text, const QFont &font, int size, int skip, BlackEdgeTextItem *item){
    QGroupBox *box = new QGroupBox;

    QLineEdit *edit = new QLineEdit;
    QFontComboBox *font_combobox = new QFontComboBox;
    QSpinBox *size_spinbox = new QSpinBox;
    size_spinbox->setRange(1, 96);
    QSpinBox *space_spinbox = new QSpinBox;
    space_spinbox->setRange(0, 100);

    QFormLayout *layout = new QFormLayout;
    layout->addRow(tr("Text"), edit);
    layout->addRow(tr("Font"), font_combobox);
    layout->addRow(tr("Size"), size_spinbox);
    layout->addRow(tr("Line spacing"), space_spinbox);

    connect(edit, SIGNAL(textChanged(QString)), item, SLOT(setText(QString)));
    connect(font_combobox, SIGNAL(currentFontChanged(QFont)), item, SLOT(setFont(QFont)));
    connect(size_spinbox, SIGNAL(valueChanged(int)), item, SLOT(setFontSize(int)));
    connect(space_spinbox, SIGNAL(valueChanged(int)), item, SLOT(setSkip(int)));

    edit->setText(text);
    size_spinbox->setValue(size);
    font_combobox->setCurrentFont(font);
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


    QFormLayout *layout = new QFormLayout;
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(kingdom_combobox);
    hlayout->addWidget(lord_checkbox);
    layout->addRow(tr("Kingdom"), hlayout);
    layout->addRow(tr("Max HP"), hp_spinbox);

    connect(kingdom_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(setCardFrame()));
    connect(lord_checkbox, SIGNAL(toggled(bool)), this, SLOT(setCardFrame()));
    connect(hp_spinbox, SIGNAL(valueChanged(int)), card_scene, SLOT(setMaxHp(int)));

    QString kingdom = Config.value("CardEditor/Kingdom", "wei").toString();
    int is_lord = Config.value("CardEditor/IsLord", false).toBool();
    kingdom_combobox->setCurrentIndex(kingdom_names.indexOf(kingdom));
    lord_checkbox->setChecked(is_lord);
    hp_spinbox->setValue(Config.value("CardEditor/MaxHP", 3).toInt());

    setCardFrame();

    return layout;
}

QWidget *CardEditor::createSkillBox(){
    QGroupBox *box = new QGroupBox(tr("Skill"));

    QFormLayout *layout = new QFormLayout;
    QFontComboBox *title_font_combobox = new QFontComboBox;
    layout->addRow(tr("Title font"), title_font_combobox);
    QFontComboBox *desc_font_combobox = new QFontComboBox;
    layout->addRow(tr("Description font"), desc_font_combobox);

    SkillBox *skill_box = card_scene->getSkillBox();
    connect(title_font_combobox, SIGNAL(currentFontChanged(QFont)), skill_box, SLOT(setSkillTitleFont(QFont)));
    connect(desc_font_combobox, SIGNAL(currentFontChanged(QFont)), skill_box, SLOT(setSkillDescriptionFont(QFont)));

    box->setLayout(layout);
    return box;
}

QWidget *CardEditor::createLeft(){
    QFormLayout *layout = new QFormLayout;
    QGroupBox *box = createTextItemBox(Config.value("CardEditor/TitleText", tr("Title")).toString(),
                                       Config.value("CardEditor/TitleFont").value<QFont>(),
                                       Config.value("CardEditor/TitleFontSize", 20).toInt(),
                                       Config.value("CardEditor/TitleSkip", 0).toInt(),
                                       card_scene->getTitleItem());
    box->setTitle(tr("Title"));
    layout->addRow(box);

    box = createTextItemBox(Config.value("CardEditor/NameText", tr("Name")).toString(),
                            Config.value("CardEditor/NameFont").value<QFont>(),
                            Config.value("CardEditor/NameFontSize", 36).toInt(),
                            Config.value("CardEditor/NameSkip", 0).toInt(),
                            card_scene->getNameItem());

    box->setTitle(tr("Name"));
    layout->addRow(box);

    layout->addRow(createGeneralLayout());
    layout->addRow(createSkillBox());

    QSpinBox *ratio_spinbox = new QSpinBox;
    ratio_spinbox->setRange(1, 1600);
    ratio_spinbox->setValue(100);
    ratio_spinbox->setSuffix(" %");
    connect(ratio_spinbox, SIGNAL(valueChanged(int)), card_scene, SLOT(setRatio(int)));


    /*


    {
        QHBoxLayout *hlayout = new QHBoxLayout;
        hlayout->addWidget(import_button);
        hlayout->addWidget(new QLabel(tr("Image ratio")));
        hlayout->addWidget(ratio_spinbox);
        layout->addRow(hlayout);
    }

    */

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
                                                    QString(),
                                                    tr("Images (*.png *.bmp *.jpg)")
                                                    );

    if(!filename.isEmpty())
        card_scene->setGeneralPhoto(filename);
}

void CardEditor::saveImage(){
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Select a photo file ..."),
                                                    QString(),
                                                    tr("Images (*.png *.bmp *.jpg)")
                                                    );

    if(!filename.isEmpty()){
        QImage image(card_scene->sceneRect().size().toSize(), QImage::Format_ARGB32);
        QPainter painter(&image);
        card_scene->render(&painter);

        image.save(filename);
    }
}

void MainWindow::on_actionCard_editor_triggered()
{
    CardEditor *editor = new CardEditor(this);
    editor->show();
}
