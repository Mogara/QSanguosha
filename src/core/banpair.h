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
    static bool isBanned(const QString &general);
    static bool isBanned(const QString &first, const QString &second);
    static const QSet<BanPair> getBanPairSet();
    static const QSet<QString> getAllBanSet();
    static const QSet<QString> getSecondBanSet();
};

#endif // BANPAIRDIALOG_H
