#include "SkinBank.h"
#include "jsonutils.h"
#include <fstream>

using namespace std;

const char QSanRoomSkin::S_SKIN_KEY_PHOTO[]("photo");
const char QSanRoomSkin::S_SKIN_KEY_ROOM[]("room");
const char QSanRoomSkin::S_SKIN_KEY_COMMON[]("common");
const char QSanRoomSkin::S_SKIN_KEY_PHOTO_MAINFRAME[]("photoMainFrame");
const char QSanRoomSkin::S_SKIN_KEY_PHOTO_HANDCARDNUM[]("photoHandCardNum");
const char QSanRoomSkin::S_SKIN_KEY_PHOTO_FACETURNEDMASK[]("photoFaceTurnedMask");
const char QSanRoomSkin::S_SKIN_KEY_PHOTO_CHAIN[]("photoChain");
const char QSanRoomSkin::S_SKIN_KEY_PHOTO_PHASE[]("photoPhase%1");
const char QSanRoomSkin::S_SKIN_KEY_HAND_CARD_BACK[]("handCardBack");

QSanSkinFactory* QSanSkinFactory::_sm_singleton = NULL;
QHash<QString, QPixmap> QSanPixmapCache::_m_pixmapBank;

bool QSanRoomSkin::QSanTextFont::tryParse(Json::Value arg)
{
    if (!arg.isArray()) return false;
    m_drawShadow = false;
    m_font = QFont(arg[0].asCString(), arg[1].asInt(), arg[2].asInt());
    m_foregroundPen = QPen(QColor(arg[3][0].asInt(), arg[3][1].asInt(), arg[3][2].asInt(), arg[3][3].asInt()));
    if (arg.size() == 5)
    {
        m_backgroundPen = QPen(QColor(arg[4][0].asInt(), arg[4][1].asInt(), arg[4][2].asInt(), arg[4][3].asInt()));
        m_drawShadow = true;
    }
    return true;
}
void QSanRoomSkin::QSanTextFont::paintText(QPainter* painter, QRect pos, Qt::AlignmentFlag align, const QString &text) const
{
    painter->setFont(m_font);
    if (m_drawShadow)
    {
        painter->setPen(m_backgroundPen);
        pos.translate(-1, -1);
        painter->drawText(pos, align, text);
        pos.translate(2, 2);
        painter->drawText(pos, align, text);
        pos.translate(-2, 0);
        painter->drawText(pos, align, text);
        pos.translate(2, -2);
        painter->drawText(pos, align, text);
        pos.translate(-1, 1);
    }    
    painter->setPen(m_foregroundPen);
    painter->drawText(pos, align, text);
}

// Load pixmap from a file and map it to the given key.
const QPixmap& QSanPixmapCache::getPixmap(const QString &key, const QString &fileName)
{
    if (!_m_pixmapBank.contains(key))
        _m_pixmapBank[key].load(fileName);
    return _m_pixmapBank[key];
}
 
// Load pixmap from a existing key.
const QPixmap& QSanPixmapCache::getPixmap(const QString &key)
{
    return _m_pixmapBank[key];
}

bool QSanPixmapCache::contains(const QString &key)
{
    return _m_pixmapBank.contains(key);
}

bool IQSanComponentSkin::load(const QString &layoutConfigName, const QString &imageConfigName)
{
    Json::Reader layoutReader, imageReader;
    ifstream layoutFile(layoutConfigName.toAscii()), imageFile(imageConfigName.toAscii());
    bool success = (!layoutFile.bad() && !imageFile.bad());
    if (success) success = layoutReader.parse(layoutFile, this->_m_layoutConfig);
    if (success) success = imageReader.parse(imageFile, this->_m_imageConfig);
    layoutFile.close(); imageFile.close();
    if (success) success = _m_layoutConfig.isObject() && _m_imageConfig.isObject();
    if (success) success = _loadLayoutConfig();
    return success;
}

QPixmap IQSanComponentSkin::getPixmap(const QString &key) const
{
    return QSanPixmapCache::getPixmap(key, _m_imageConfig[key.toAscii().constData()].asCString());
}

const QSanRoomSkin::RoomLayout& QSanRoomSkin::getRoomLayout() const
{
    return this->_m_roomLayout;
}

const QSanRoomSkin::PhotoLayout& QSanRoomSkin::getPhotoLayout() const
{
    return _m_photoLayout;
}

const QSanRoomSkin::CommonLayout& QSanRoomSkin::getCommonLayout() const
{
    return _m_commonLayout;
}
    
bool QSanRoomSkin::_loadLayoutConfig()
{
    Json::Value config = _m_layoutConfig[S_SKIN_KEY_PHOTO];
    _m_photoLayout.m_heightIncludeMarkAndControl = config["heightIncludeMarkAndControl"].asInt();
    _m_photoLayout.m_heightIncludeShadow = config["heightIncludeShadow"].asInt();
    _m_photoLayout.m_normalHeight = config["normalHeight"].asInt();
    _m_photoLayout.m_normalWidth = config["normalWidth"].asInt();
    _m_photoLayout.m_widthIncludeMarkAndControl = config["widthIncludeMarkAndControl"].asInt();
    _m_photoLayout.m_widthIncludeShadow = config["widthIncludeShadow"].asInt();
    QSanProtocol::Utils::tryParse(config["cardMoveArea"], _m_photoLayout.m_cardMoveRegion);
    QSanProtocol::Utils::tryParse(config["phaseArea"], _m_photoLayout.m_phaseArea);
    QSanProtocol::Utils::tryParse(config["mainFrameArea"], _m_photoLayout.m_mainFrameArea);
    QSanProtocol::Utils::tryParse(config["progressBarArea"], _m_photoLayout.m_progressBarArea);
    _m_photoLayout.m_isProgressBarHorizontal = config["progressBarHorizontal"].asBool();
    config = _m_layoutConfig[S_SKIN_KEY_ROOM];
    _m_roomLayout.m_chatBoxHeightPercentage = config["chatBoxHeightPercentage"].asDouble();
    _m_roomLayout.m_chatTextBoxHeight = config["chatTextBoxHeight"].asInt();
    _m_roomLayout.m_discardPileMinWidth = config["discardPileMinWidth"].asInt();
    _m_roomLayout.m_discardPilePadding = config["discardPilePadding"].asInt();
    _m_roomLayout.m_infoPlaneWidthPercentage = config["infoPlaneWidthPercentage"].asDouble();
    _m_roomLayout.m_logBoxHeightPercentage = config["logBoxHeightPercentage"].asDouble();
    _m_roomLayout.m_minimumSceneSize = QSize(config["minimumSceneSize"][0].asInt(), config["minimumSceneSize"][1].asInt());
    _m_roomLayout.m_photoPhotoPadding = config["photoPhotoPadding"].asInt();
    _m_roomLayout.m_photoRoomPadding = config["photoRoomPadding"].asInt();
    _m_roomLayout.m_roleBoxHeight = config["roleBoxHeight"].asInt();
    _m_roomLayout.m_scenePadding = config["scenePadding"].asInt();
    config = _m_layoutConfig[S_SKIN_KEY_COMMON];
    _m_commonLayout.m_cardNormalHeight = config["cardNormalHeight"].asInt();
    _m_commonLayout.m_cardNormalWidth = config["cardNormalWidth"].asInt();
    QSanProtocol::Utils::tryParse(config["cardSuitArea"], _m_commonLayout.m_cardSuitArea);
    QSanProtocol::Utils::tryParse(config["cardNumberArea"], _m_commonLayout.m_cardNumberArea);
    QSanProtocol::Utils::tryParse(config["cardFootnoteArea"], _m_commonLayout.m_cardFootnoteArea);
    _m_commonLayout.m_cardFootnoteFont.tryParse(config["cardFootnoteFont"]);
    return true;
}


bool QSanSkinScheme::load(Json::Value configs)
{
    if (!configs.isObject()) return false;
    return _m_roomSkin.load(configs["roomLayoutConfigFile"].asCString(), configs["roomImageConfigFile"].asCString());
    return true;
}

const QSanRoomSkin& QSanSkinScheme::getRoomSkin() const
{
    return _m_roomSkin;
}

QSanSkinFactory& QSanSkinFactory::getInstance()
{
    if (_sm_singleton == NULL)
        _sm_singleton = new QSanSkinFactory("skins\\skinList.json");
    return *_sm_singleton;
}

const QSanSkinScheme& QSanSkinFactory::getCurrentSkinScheme() const
{
    return this->_sm_currentSkin;
}

bool QSanSkinFactory::switchSkin(QString skinName)
{
    return _sm_currentSkin.load(_m_skinList[skinName.toAscii().constData()]);
}

QSanSkinFactory::QSanSkinFactory(const char* fileName)
{
    Json::Reader reader;
    reader.parse(ifstream(fileName), this->_m_skinList);
}
