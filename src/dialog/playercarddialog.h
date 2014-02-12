#ifndef _PLAYER_CARD_DIALOG_H
#define _PLAYER_CARD_DIALOG_H

#include "clientplayer.h"

#include <QDialog>
#include <QMap>
#include <QCommandLinkButton>

class MagatamaWidget: public QWidget {
    Q_OBJECT

public:
    explicit MagatamaWidget(int hp, Qt::Orientation orientation);

    static QPixmap GetMagatama(int index);
    static QPixmap GetSmallMagatama(int index);
};

class PlayerCardDialog: public QDialog {
    Q_OBJECT

public:
    explicit PlayerCardDialog(const ClientPlayer *player, const QString &flags = "hej",
                                bool handcard_visible = false, Card::HandlingMethod method = Card::MethodNone,
                                const QList<int> &disabled_ids = QList<int>());

private:
    QWidget *createAvatar();
    QWidget *createHandcardButton();
    QWidget *createEquipArea();
    QWidget *createJudgingArea();

    const ClientPlayer *player;
    QMap<QObject *, int> mapper;
    bool handcard_visible;
    Card::HandlingMethod method;
    QList<int> disabled_ids;

private slots:
    void emitId();

signals:
    void card_id_chosen(int card_id);
};

class PlayerCardButton: public QCommandLinkButton {
public:
    explicit PlayerCardButton(const QString &name);
    virtual QSize sizeHint() const{
        QSize size = QCommandLinkButton::sizeHint();
        return QSize(size.width() * scale, size.height());
    }

    inline void setScale(double scale) { this->scale = scale; }

private:
    double scale;
};

#endif

