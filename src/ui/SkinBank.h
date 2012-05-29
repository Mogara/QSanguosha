#ifndef _SKIN_BANK_H
#define _SKIN_BANK_H

#include <json/json.h>
#include <QString>
#include <QPixmap>
#include <QHash>
#include <QFont>
#include <QPen>
#include <QPainter>

class QSanPixmapCache
{
public:
    // Load pixmap from a file and map it to the given key.
    static const QPixmap& getPixmap(const QString &key, const QString &fileName);
    // Load pixmap from a existing key.
    static const QPixmap& getPixmap(const QString &key);
    static bool contains(const QString &key); 
private:
    static QHash<QString, QPixmap> _m_pixmapBank;
};

class IQSanComponentSkin // interface class
{
public:
    bool load(const QString &layoutConfigFileName, const QString &imageConfigFileName);
    QPixmap getPixmap(const QString &key) const;
protected:
    virtual bool _loadLayoutConfig() = 0;
    Json::Value _m_layoutConfig;
    Json::Value _m_imageConfig;
};

class QSanRoomSkin : public IQSanComponentSkin
{
public:
    struct QSanTextFont {
        QFont m_font;
        QPen m_backgroundPen;
        QPen m_foregroundPen;
        bool m_drawShadow;
        bool tryParse(Json::Value arg);
        void paintText(QPainter* painter, QRect pos, Qt::AlignmentFlag align, const QString &text) const;
    };
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
        QRect m_cardMoveRegion;
        QRect m_phaseArea;
        QRect m_mainFrameArea;
        QRect m_progressBarArea;
        bool m_isProgressBarHorizontal;
    };
    struct CommonLayout
    {
        int m_cardNormalWidth;
        int m_cardNormalHeight;
        QRect m_cardSuitArea;
        QRect m_cardNumberArea;
        QRect m_cardFootnoteArea;
        QSanTextFont m_cardFootnoteFont;
    };
    const RoomLayout& getRoomLayout() const;
    const PhotoLayout& getPhotoLayout() const;
    const CommonLayout& getCommonLayout() const;
    // static consts
    static const char S_SKIN_KEY_PHOTO[];
    static const char S_SKIN_KEY_COMMON[];
    static const char S_SKIN_KEY_ROOM[];
    static const char S_SKIN_KEY_PHOTO_MAINFRAME[];
    static const char S_SKIN_KEY_PHOTO_HANDCARDNUM[];
    static const char S_SKIN_KEY_PHOTO_FACETURNEDMASK[];
    static const char S_SKIN_KEY_PHOTO_CHAIN[];
    static const char S_SKIN_KEY_PHOTO_PHASE[];
    static const char S_SKIN_KEY_HAND_CARD_BACK[];
protected:
    RoomLayout _m_roomLayout;
    PhotoLayout _m_photoLayout;
    CommonLayout _m_commonLayout;
    virtual bool _loadLayoutConfig();

};

class QSanSkinScheme
{
// Why do we need another layer above room skin? Because we may add lobby, login interface
// in the future; and we may need to assemble a set of different skins into a scheme.
public:
    bool load(Json::Value configs);
    const QSanRoomSkin& getRoomSkin() const;
protected:
    QSanRoomSkin _m_roomSkin;
};

class QSanSkinFactory
{
public:
    static QSanSkinFactory& getInstance();
    const QSanSkinScheme& getCurrentSkinScheme() const;
    bool switchSkin(QString skinName);
protected:
    QSanSkinFactory(const char* fileName);
    static QSanSkinFactory* _sm_singleton;
    QSanSkinScheme _sm_currentSkin;
    Json::Value _m_skinList;
};

#endif