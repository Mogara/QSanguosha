#ifndef EVENT_H
#define EVENT_H

class Player;
class Room;
class Card;

#include <QEvent>
#include <QObject>

class Event : public QEvent
{
public:
    Event(const QString &name, Player *source, Player *target);
    void addCard(const Card *card);
    QString getName() const;
    Player *getSource() const;
    Player *getTarget() const;

    static Event *Parse(Room *room, const QString &str);

protected:
    QString tag;

private:
    QString name;
    Player *source, *target;
    Room *room;
    QList<const Card *> cards;
};

#endif // EVENT_H
