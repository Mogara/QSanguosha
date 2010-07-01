#ifndef CARDCLASS_H
#define CARDCLASS_H

#include <QObject>

class Card;

class CardClass : public QObject
{
    Q_OBJECT
public:
    enum Type {Basic, Equip, Trick, UserDefined};
    friend class Card;

    explicit CardClass(const QString &name, enum Type type, int id, const QString &pixmap_path);
    QString getPixmapPath() const;

private:
    enum Type type;
    int id;
    QString pixmap_path;
};

#endif // CARDCLASS_H
