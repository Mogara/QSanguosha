#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QObject>
#include <QIcon>
#include <QPixmap>
#include <QGraphicsObject>
#include <QPushButton>
#include <QGraphicsProxyWidget>
#include <QGraphicsPixmapItem>

// --> class MyPixmapItem
class MyPixmapItem : public QObject , public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    MyPixmapItem(const QPixmap &pixmap, QGraphicsItem *parentItem=0);
    ~MyPixmapItem();

public:
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget=0);
    void setSize(int x, int y);
    QString itemName;

private:
    virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    virtual void hoverMoveEvent ( QGraphicsSceneHoverEvent * event );
    void initFaceBoardPos();
    void initEasyTextPos();
    int mouseCanClick(int x, int y);
    int mouseOnIcon(int x, int y);
    int mouseOnText(int x, int y);

    int sizex;
    int sizey;
    QList <QRect> faceboardPos;
    QList <QRect> easytextPos;
    QList <QString> easytext;

signals:
    void my_pixmap_item_msg(QString);
};

// --> class ChatWidget
class ChatWidget : public QGraphicsObject
{
    Q_OBJECT

public:
    ChatWidget();
    ~ChatWidget();
    virtual QRectF boundingRect() const;
protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget); 
private:
    QPixmap base_pixmap;
    QPushButton *returnButton;
    QPushButton *chatfaceButton;
    QPushButton *easytextButton;
    MyPixmapItem *chat_face_board, *easy_text_board;
    QGraphicsRectItem *base;

    QGraphicsProxyWidget *addWidget(QWidget *widget, int x);
    QPushButton *addButton(const QString &name, int x);
    QPushButton *createButton(const QString &name);    
private slots:
    void showEasyTextBoard();
    void showFaceBoard();
    void sendText();
signals:
    void chat_widget_msg(QString);
    void return_button_click();
};

#endif // CHATWIDGET_H
