#ifndef BANPAIRDIALOG_H
#define BANPAIRDIALOG_H

#include <QDialog>
#include <QPair>
#include <QListWidget>

struct BanPair: public QPair<QString, QString>{
    BanPair();
    BanPair(const QString &first, const QString &second);

    static void loadBanPairs();
    static void saveBanPairs();
    static bool isBanned(const BanPair &pair);
};

class BanPairDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BanPairDialog(QWidget *parent = 0);
    void addPairToList(const BanPair &pair);

public slots:
    void addPair();
    void addPair(const QString &first, const QString &second);
    void removePair();
    void save();

private:
    QListWidget *list;
};

#endif // BANPAIRDIALOG_H
