#ifndef CARDCLASS_H
#define CARDCLASS_H

#include <QObject>

class Card;

class CardClass : public QObject
{
    Q_OBJECT
public:
    friend class Card;

    explicit CardClass(const QString &name, const QString &type, const QString &subtype, int id, const QString &pixmap_dir);
    QString getPixmapPath() const;

private:
    QString type, subtype;
    int id;
    QString pixmap_dir;
};

#endif // CARDCLASS_H
