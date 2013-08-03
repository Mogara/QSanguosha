#ifndef CARDBUTTON_H
#define CARDBUTTON_H

#include <QtGlobal>

#ifdef Q_OS_WIN
#define CommandLinkButton QCommandLinkButton
#include <QCommandLinkButton>
#else
#define CommandLinkButton QPushButton
#include <QPushButton>
#endif

class Card;

class CardButton : public CommandLinkButton
{
    Q_OBJECT
public:
    explicit CardButton(const Card *card);

private:
    const Card *card;

private slots:
    void onClicked();

signals:
    void idSelected(int id);
};

#endif // CARDBUTTON_H
