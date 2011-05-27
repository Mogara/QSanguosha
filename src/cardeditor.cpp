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
}

void BlackEdgeTextItem::setColor(const QColor &color){
    this->color = color;
}

void BlackEdgeTextItem::setText(const QString &text){
    this->text = text;

    prepareGeometryChange();
}

void BlackEdgeTextItem::setFont(const QFont &font){
    int size = this->font.pointSize();
    this->font = font;
    this->font.setPointSize(size);
    prepareGeometryChange();
}

void BlackEdgeTextItem::setFontSize(int size){
    font.setPointSize(size);
    prepareGeometryChange();
}

void BlackEdgeTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    if(text.isEmpty())
        return;

    painter->setRenderHint(QPainter::Antialiasing);

    QPen pen(Qt::black);
    pen.setWidth(5);
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
}

void SkillBox::setKingdom(const QString &kingdom){
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

QRectF SkillBox::boundingRect() const{
    // left down cornor is the origin
    int height = up.height() + middle_height + down.height();
    return QRectF(0, -height, up.width(), height);
}

void SkillBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
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

    title = new BlackEdgeTextItem;
    title->setPos(49, 128);

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

#ifdef QT_DEBUG

#include <QKeyEvent>
#include <QMessageBox>

void CardScene::keyPressEvent(QKeyEvent *event){
    QGraphicsScene::keyPressEvent(event);

    if(event->key() == Qt::Key_D){
        QMessageBox::information(NULL, "", QString("%1, %2").arg(skill_box->x()).arg(skill_box->y()));
    }
}

#endif

void CardScene::setMaxHp(int max_hp){    
    int n = magatamas.length();
    max_hp = qBound(0, max_hp, n-1);

    int i;
    for(i=0; i<n; i++)
        magatamas.at(i)->setVisible(i < max_hp);
}

void CardScene::setRatio(int ratio){
    photo->setScale(ratio / 100.0);
}

static QLayout *HLay(QWidget *left, QWidget *right){
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(left);
    layout->addWidget(right);

    return layout;
}

CardEditor::CardEditor(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("Card editor"));

    QHBoxLayout *layout = new QHBoxLayout;
    QGraphicsView *view = new QGraphicsView;
    card_scene = new CardScene;
    view->setScene(card_scene);
    view->setMinimumSize(380, 530);

    layout->addWidget(createLeft());
    layout->addWidget(view);

    setLayout(layout);

    card_scene->setFrame("wei", false);
}

QHBoxLayout *CardEditor::createTextItemLayout(const QString &text, const QFont &font, int size, BlackEdgeTextItem *item){
    QLineEdit *title_edit = new QLineEdit;
    QFontComboBox *title_font_combobox = new QFontComboBox;
    QSpinBox *title_size_spinbox = new QSpinBox;
    title_size_spinbox->setRange(1, 96);
    QSpinBox *title_space_spinbox = new QSpinBox;
    title_space_spinbox->setRange(0, 100);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(new QLabel(text));
    hlayout->addWidget(title_edit);
    hlayout->addWidget(title_font_combobox);
    hlayout->addWidget(new QLabel(tr("Size")));
    hlayout->addWidget(title_size_spinbox);
    hlayout->addWidget(new QLabel(tr("Line spacing")));
    hlayout->addWidget(title_space_spinbox);

    BlackEdgeTextItem *title_item = item;
    connect(title_edit, SIGNAL(textChanged(QString)), title_item, SLOT(setText(QString)));
    connect(title_font_combobox, SIGNAL(currentFontChanged(QFont)), title_item, SLOT(setFont(QFont)));
    connect(title_size_spinbox, SIGNAL(valueChanged(int)), title_item, SLOT(setFontSize(int)));
    connect(title_space_spinbox, SIGNAL(valueChanged(int)), title_item, SLOT(setSkip(int)));

    title_edit->setText(text);
    title_size_spinbox->setValue(size);

    return hlayout;
}

QGroupBox *CardEditor::createLeft(){
    QGroupBox *box = new QGroupBox;
    box->setTitle(tr("Properties"));

    QFormLayout *layout = new QFormLayout;
    layout->addRow(createTextItemLayout(tr("Title"), QFont(), 20, card_scene->getTitleItem()));
    layout->addRow(createTextItemLayout(tr("Name"), QFont(), 36, card_scene->getNameItem()));

    /*
    {
        QLineEdit *name_edit = new QLineEdit;
        QFontComboBox *name_font_combobox = new QFontComboBox;
        QSpinBox *name_size_spinbox = new QSpinBox;
        name_size_spinbox->setRange(1, 96);
        QSpinBox *name_space_spinbox = new QSpinBox;
        name_space_spinbox->setRange(0, 100);

        QHBoxLayout *hlayout = new QHBoxLayout;
        hlayout->addWidget(new QLabel(tr("Name")));
        hlayout->addWidget(name_edit);
        hlayout->addWidget(name_font_combobox);
        hlayout->addWidget(new QLabel(tr("Size")));
        hlayout->addWidget(name_size_spinbox);
        hlayout->addWidget(new QLabel(tr("Line spacing")));
        hlayout->addWidget(name_space_spinbox);
        layout->addRow(hlayout);

        BlackEdgeTextItem *name_item = card_scene->getNameItem();
        connect(name_edit, SIGNAL(textChanged(QString)), name_item, SLOT(setText(QString)));
        connect(name_font_combobox, SIGNAL(currentFontChanged(QFont)), name_item, SLOT(setFont(QFont)));
        connect(name_size_spinbox, SIGNAL(valueChanged(int)), name_item, SLOT(setFontSize(int)));
        connect(name_space_spinbox, SIGNAL(valueChanged(int)), name_item, SLOT(setSkip(int)));

        name_edit->setText(tr("Name"));
        name_size_spinbox->setValue(36);
    }

*/

    kingdom_combobox = new QComboBox;
    lord_checkbox = new QCheckBox(tr("Lord"));
    foreach(QString kingdom, Sanguosha->getKingdoms()){
        QIcon icon(QString("image/kingdom/icon/%1.png").arg(kingdom));
        kingdom_combobox->addItem(icon, Sanguosha->translate(kingdom), kingdom);
    }

    QSpinBox *hp_spinbox = new QSpinBox;
    hp_spinbox->setRange(0, 10);

    QPushButton *browse_button = new QPushButton(tr("Photo file ..."));
    ratio_spinbox = new QSpinBox;
    ratio_spinbox->setRange(1, 1600);
    ratio_spinbox->setValue(100);
    ratio_spinbox->setSuffix(" %");


    {
        QHBoxLayout *hlayout = new QHBoxLayout;
        hlayout->addWidget(new QLabel(tr("Kingdom")));
        hlayout->addWidget(kingdom_combobox);
        hlayout->addWidget(lord_checkbox);
        hlayout->addWidget(new QLabel(tr("Max HP")));
        hlayout->addWidget(hp_spinbox);
        layout->addRow(hlayout);
    }

    layout->addRow(tr("General"), HLay(browse_button, ratio_spinbox));

    skill_tabs = new QTabWidget;

    int i;
    for(i=1; i<=4; i++){
        SkillTab *tab = new SkillTab;
        skill_tabs->addTab(tab, tr("Skill %1").arg(i));



        QGraphicsTextItem *text_item = new QGraphicsTextItem;
        text_item->setPlainText("hello, world");
        text_item->setFlag(QGraphicsItem::ItemIsMovable);
        text_item->setTextInteractionFlags(Qt::TextEditorInteraction);
        text_item->setTextWidth(100);

        //text_item->setDocument(tab->getDoc());
        //text_item->document()->setTextWidth(100);

        text_item->setPos(100, 100 + i * 100);
        card_scene->addItem(text_item);
    }

    layout->addRow(skill_tabs);

    QCommandLinkButton *save_button = new QCommandLinkButton(tr("Save"));
    save_button->setDescription(tr("Save the image"));
    layout->addRow(save_button);

    connect(kingdom_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(setCardFrame()));
    connect(lord_checkbox, SIGNAL(toggled(bool)), this, SLOT(setCardFrame()));

    connect(browse_button, SIGNAL(clicked()), this, SLOT(browseGeneralPhoto()));
    connect(hp_spinbox, SIGNAL(valueChanged(int)), card_scene, SLOT(setMaxHp(int)));
    connect(ratio_spinbox, SIGNAL(valueChanged(int)), card_scene, SLOT(setRatio(int)));
    connect(save_button, SIGNAL(clicked()), this, SLOT(saveImage()));

    box->setLayout(layout);

    hp_spinbox->setValue(3);

    return box;
}

void CardEditor::setCardFrame(){
    QString kingdom = kingdom_combobox->itemData(kingdom_combobox->currentIndex()).toString();
    if(kingdom == "god")
        card_scene->setFrame("god", false);
    else
        card_scene->setFrame(kingdom, lord_checkbox->isChecked());
}

SkillTab::SkillTab()
{
    QFormLayout *layout = new QFormLayout;

    name_edit = new QLineEdit;
    description_edit = new QTextEdit;

    layout->addRow(tr("Name"), name_edit);
    layout->addRow(tr("Description"), description_edit);

    setLayout(layout);

    connect(name_edit, SIGNAL(textChanged(QString)), this, SLOT(setDocTitle(QString)));
}

QTextDocument *SkillTab::getDoc() const{
    return description_edit->document();
}

void SkillTab::setDocTitle(const QString &title){
    description_edit->setDocumentTitle(title);
}

void CardEditor::browseGeneralPhoto(){
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
    editor->exec();
}
