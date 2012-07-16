#ifndef CLIENTPLAYER_H
#define CLIENTPLAYER_H

#include "player.h"
#include "clientstruct.h"

class Client;
class QTextDocument;

class ClientPlayer : public Player
{
    Q_OBJECT
    Q_PROPERTY(int handcard READ getHandcardNum WRITE setHandcardNum)

public:
    explicit ClientPlayer(Client *client);
    void handCardChange(int delta);
    QList<const Card *> getCards() const;
    void setCards(const QList<int> &card_ids);
    QTextDocument *getMarkDoc() const;
    void changePile(const QString &name, bool add, QList<int> card_ids);
    QString getDeathPixmapPath() const;
    void setHandcardNum(int n);
    virtual QString getGameMode() const;

    virtual void setFlags(const QString &flag);
    virtual int aliveCount() const;
    virtual int getHandcardNum() const;
    virtual void removeCard(const Card *card, Place place);
    virtual void addCard(const Card *card, Place place);
    virtual void addKnownHandCard(const Card *card);
    virtual bool isLastHandCard(const Card *card) const;
    virtual void setMark(const QString &mark, int value);

private:
    int handcard_num;
    QList<int> known_cards;
    QTextDocument *mark_doc;

signals:
    void pile_changed(const QString &name);
    void drank_changed();
    void action_taken();
};

extern ClientPlayer *Self;

#endif // CLIENTPLAYER_H
