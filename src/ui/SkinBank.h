//#ifndef _SKIN_BANK_H
//#define _SKIN_BANK_H

#include <json/json.h>
#include <QString>
#include <QPixmap>
#include <QHash>

namespace QSanLayout
{
    struct RoomLayout {
        int m_scenePadding;
        int m_roleBoxHeight;
        int m_chatTextBoxHeight;
        int m_discardPileMinWidth;
        int m_discardPilePadding;
        double m_logBoxHeightPercentage;
        double m_chatBoxHeightPercentage;
        double m_infoPlaneWidthPercentage;
        double m_photoRoomPadding;
        double m_photoPhotoPadding;    
        QSize m_minimumSceneSize;    
    };
    struct PhotoLayout
    {
        int m_normalWidth;
        int m_normalHeight;
        int m_widthIncludeShadow;
        int m_heightIncludeShadow;
        int m_widthIncludeMarkAndControl;
        int m_heightIncludeMarkAndControl;
    };
}

class QSanSkin
{
public:
    QSanSkin();
    void load(QString layoutConfigName, QString skinConfigName);
    const QSanLayout::RoomLayout& getRoomLayout() const;
    const QSanLayout::PhotoLayout& getPhotoLayout() const;
    const QPixmap& getPixmap(const QString &key) const;

    //static consts
    struct PhotoSkin
    {
        static const QString S_SKIN_KEY_MAINFRAME;
        static const QString S_SKIN_KEY_HANDCARDNUM;
        static const QString S_SKIN_KEY_FACETURNEDMASK;
        static const QString S_SKIN_KEY_CHAIN;
    };

protected:
    QSanLayout::RoomLayout layout;
    QHash<QString, QPixmap> _m_pixmapBank;
};

class SkinBankFactory
{
public:
    static const SkinBankFactory& getInstance();
    QSanSkin getQSanSkin(QString skinName) const;
    const QSanSkin& getCurrentSkin() const;
    void switchSkin(QString skinName);
protected:
    SkinBankFactory();
    static SkinBankFactory* _sm_singleton;
    static QSanSkin _sm_currentSkin;
};

QSanSkin* g_currentSkin;
//#endif