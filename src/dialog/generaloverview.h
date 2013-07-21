#ifndef GENERALOVERVIEW_H
#define GENERALOVERVIEW_H

#include <QWebView>
#include <QVariantList>

class General;

class GeneralOverview : public QWebView {
    Q_OBJECT

public:
    GeneralOverview();

public slots:
    QString translate(const QString &key);
    QStringList getKingdoms() const;
    QStringList getPackages() const;
    QStringList getGenerals(const QVariantMap &options) const;
    QObject *getGeneral(const QString &name) const;
    QVariantList getLines(const QString &generalName) const;
    void play(const QString &path) const;

private slots:
    void addSelfToFrame();
};

#endif // GENERALOVERVIEW_H
