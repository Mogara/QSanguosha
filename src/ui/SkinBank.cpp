#include "SkinBank.h"
#include "jsonutils.h"
#include "protocol.h"
#include "uiUtils.h"
#include "engine.h"
#include <fstream>
#include <QGraphicsPixmapItem>
#include <QTextItem>
#include <QStyleOptionGraphicsItem>
#include <QMessageBox>

using namespace std;
using namespace QSanProtocol::Utils;

const char* IQSanComponentSkin::S_SKIN_KEY_DEFAULT = "default";
const char* QSanRoomSkin::S_SKIN_KEY_PHOTO = "photo";
const char* QSanRoomSkin::S_SKIN_KEY_ROOM = "room";
const char* QSanRoomSkin::S_SKIN_KEY_COMMON = "common";
const char* QSanRoomSkin::S_SKIN_KEY_DASHBOARD = "dashboard";

// buttons
const char* QSanRoomSkin::S_SKIN_KEY_BUTTON = "button-%1";
const char* QSanRoomSkin::S_SKIN_KEY_DASHBOARD_BUTTON_SET_BG = "dashboardButtonSetBg";
const char* QSanRoomSkin::S_SKIN_KEY_BUTTON_SKILL = "skill";

// player container
const char* QSanRoomSkin::S_SKIN_KEY_EQUIP_ICON = "%1Equip-%2";
const char* QSanRoomSkin::S_SKIN_KEY_MAINFRAME = "%1MainFrame";
const char* QSanRoomSkin::S_SKIN_KEY_LEFTFRAME = "%1LeftFrame";
const char* QSanRoomSkin::S_SKIN_KEY_RIGHTFRAME = "%1RightFrame";
const char* QSanRoomSkin::S_SKIN_KEY_MIDDLEFRAME = "%1MiddleFrame";
const char* QSanRoomSkin::S_SKIN_KEY_HANDCARDNUM = "%1HandCardNum";
const char* QSanRoomSkin::S_SKIN_KEY_FACETURNEDMASK = "%1FaceTurnedMask";
const char* QSanRoomSkin::S_SKIN_KEY_BLANK_GENERAL = "%1BlankGeneral";
const char* QSanRoomSkin::S_SKIN_KEY_CHAIN = "%1Chain";
const char* QSanRoomSkin::S_SKIN_KEY_PHASE = "%1Phase%2";
const char* QSanRoomSkin::S_SKIN_KEY_SELECTED_FRAME = "%1FrameWhenSelected";
const char* QSanRoomSkin::S_SKIN_KEY_FOCUS_FRAME = "%1FocusFrame%2";
const char* QSanRoomSkin::S_SKIN_KEY_KINGDOM_ICON = "kingdomIcon-%1";
const char* QSanRoomSkin::S_SKIN_KEY_KINGDOM_COLOR_MASK = "kingdomColorMask-%1";
const char* QSanRoomSkin::S_SKIN_KEY_VOTES_NUMBER = "votesNum-%1";
const char* QSanRoomSkin::S_SKIN_KEY_SAVE_ME_ICON = "saveMe";
const char* QSanRoomSkin::S_SKIN_KEY_ACTIONED_ICON = "playerActioned";
const char* QSanRoomSkin::S_SKIN_KEY_READY_ICON = "playerReady";
const char* QSanRoomSkin::S_SKIN_KEY_HAND_CARD_BACK = "handCardBack";
const char* QSanRoomSkin::S_SKIN_KEY_HAND_CARD_SUIT = "handCardSuit-%1";
const char* QSanRoomSkin::S_SKIN_KEY_JUDGE_CARD_ICON = "judgeCardIcon-%1";
const char* QSanRoomSkin::S_SKIN_KEY_HAND_CARD_FRAME = "handCardFrame-%1";
const char* QSanRoomSkin::S_SKIN_KEY_HAND_CARD_AVATAR = "handCardAvatar-%1";
const char* QSanRoomSkin::S_SKIN_KEY_HAND_CARD_MAIN_PHOTO = "handCardMainPhoto-%1";
const char* QSanRoomSkin::S_SKIN_KEY_HAND_CARD_NUMBER_BLACK = "handCardNumber-black-%1";
const char* QSanRoomSkin::S_SKIN_KEY_HAND_CARD_NUMBER_RED = "handCardNumber-red-%1";
const char* QSanRoomSkin::S_SKIN_KEY_PLAYER_AUDIO_EFFECT = "playerAudioEffect-%1-%2";
const char* QSanRoomSkin::S_SKIN_KEY_SYSTEM_AUDIO_EFFECT = "systemAudioEffect-%1";
const char* QSanRoomSkin::S_SKIN_KEY_PLAYER_GENERAL_ICON = "playerGeneralIcon-%1-%2";
const char* QSanRoomSkin::S_SKIN_KEY_MAGATAMAS_BG = "magatamasBg%1";
const char* QSanRoomSkin::S_SKIN_KEY_MAGATAMAS = "magatamas%1";
const char* QSanRoomSkin::S_SKIN_KEY_PROGRESS_BAR_IMAGE = "progressBar";

QSanSkinFactory* QSanSkinFactory::_sm_singleton = NULL;
QHash<QString, QPixmap> QSanPixmapCache::_m_pixmapBank;
QHash<QString, int*> IQSanComponentSkin::QSanSimpleTextFont::_m_fontBank;

bool IQSanComponentSkin::QSanSimpleTextFont::tryParse(Json::Value arg)
{
    if (!arg.isArray() || arg.size() < 4) return false;
    // m_font = QFont(toQString(arg[0]), arg[1].asInt(), arg[2].asInt());
    m_vertical = false;
    QString fontPath = toQString(arg[0]);
    if (fontPath.startsWith("@"))
    {
        m_vertical = true;
        fontPath.remove(0, 1);
    }
    if (_m_fontBank.contains(fontPath))
        m_fontFace = _m_fontBank[fontPath];
    else {
        m_fontFace = QSanUiUtils::QSanFreeTypeFont::loadFont(fontPath);
        _m_fontBank[fontPath] = m_fontFace;
    }
    if (arg[1].isInt())
    {
        m_fontSize.setWidth(arg[1].asInt());
        m_fontSize.setHeight(arg[1].asInt());
        m_spacing = 0;
    }
    else
    {
        m_fontSize.setWidth(arg[1][0].asInt());
        m_fontSize.setHeight(arg[1][1].asInt());
        m_spacing = arg[1][2].asInt();
    }
    m_color = QColor(arg[3][0].asInt(), arg[3][1].asInt(), arg[3][2].asInt(), arg[3][3].asInt());
    return true;
}

bool IQSanComponentSkin::QSanShadowTextFont::tryParse(Json::Value arg)
{
    if (!arg.isArray() || arg.size() < 4) return false;
    if (!QSanSimpleTextFont::tryParse(arg)) return false;
    if (arg.size() >= 8)
    {
        m_shadowRadius = arg[4].asInt();
        m_shadowDecadeFactor = arg[5].asDouble();
        QSanProtocol::Utils::tryParse(arg[6], m_shadowOffset);
        m_shadowColor = QColor(arg[7][0].asInt(), arg[7][1].asInt(), arg[7][2].asInt(), arg[7][3].asInt());
    }
    else
    {
        m_shadowRadius = -1;
    }
    return true;
}

bool IQSanComponentSkin::isImageKeyDefined(const QString &key) const
{
    Json::Value val = _m_imageConfig[key.toAscii().constData()];
    return val.isArray() || val.isString();
}

void IQSanComponentSkin::QSanSimpleTextFont::paintText(QPainter* painter, QRect pos, Qt::Alignment align,
                                                       const QString &text) const
{
    if (pos.width() < 0 || pos.height() < 0) return;
    QSize actualSize = m_fontSize;
    if ((align & Qt::TextWrapAnywhere) && !m_vertical)
    {
        QSanUiUtils::QSanFreeTypeFont::paintQStringMultiLine(
            painter, text, m_fontFace, m_color, actualSize, m_spacing, pos, align);
    }
    else
    {
        QSanUiUtils::QSanFreeTypeFont::paintQString(painter, text, m_fontFace, m_color,
                                                    actualSize, m_spacing, pos,
                                                    m_vertical ? Qt::Vertical : Qt::Horizontal, align);
    }
}


void IQSanComponentSkin::QSanSimpleTextFont::paintText(QGraphicsPixmapItem* item, QRect pos,
                                                       Qt::Alignment align, const QString &text) const
{
    QPixmap pixmap(pos.size());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    paintText(&painter, QRect(0, 0, pos.width(), pos.height()), align, text);
    item->setPixmap(pixmap);
    item->setPos(pos.x(), pos.y());
}

void IQSanComponentSkin::QSanShadowTextFont::paintText(QPainter* painter, QRect pos,
                                                       Qt::Alignment align, const QString &text) const
{
    QImage image(pos.width(), pos.height(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter imagePainter(&image);
    // @todo: currently, we have not considered _m_sahdowOffset yet
    QSanSimpleTextFont::paintText(&imagePainter, QRect(m_shadowRadius, m_shadowRadius,
        pos.width() - m_shadowRadius * 2, pos.height() - m_shadowRadius * 2), align, text);
    if (m_shadowRadius < 0 || (m_shadowRadius == 0 && m_shadowOffset.x() == 0 && m_shadowOffset.y() == 0))
    {
        painter->drawImage(0, 0, image);
        return;
    }
    QImage shadow = QSanUiUtils::produceShadow(image, m_shadowColor, m_shadowRadius, m_shadowDecadeFactor);
    // now, overlay foreground on shadow
    painter->drawImage(pos.topLeft(), shadow);
    painter->drawImage(pos.topLeft(), image); //pos, image);
}

void IQSanComponentSkin::QSanShadowTextFont::paintText(QGraphicsPixmapItem* pixmapItem,
                                                       QRect pos,
                                                       Qt::Alignment align,
                                                       const QString &text) const
{
    QImage image(pos.width(), pos.height(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter imagePainter(&image);
    // @todo: currently, we have not considered _m_sahdowOffset yet
    QSanSimpleTextFont::paintText(&imagePainter, QRect(m_shadowRadius, m_shadowRadius,
        pos.width() - m_shadowRadius * 2, pos.height() - m_shadowRadius * 2), align, text);
    QImage shadow = QSanUiUtils::produceShadow(image, m_shadowColor, m_shadowRadius, m_shadowDecadeFactor);
    // now, overlay foreground on shadow
    QPixmap pixmap = QPixmap::fromImage(shadow);
    QPainter shadowPainter(&pixmap);
    shadowPainter.drawImage(0, 0, image);
    pixmapItem->setPixmap(pixmap);
    pixmapItem->setPos(pos.x(), pos.y());
}

QString QSanRoomSkin::getButtonPixmapPath(const QString &groupName,
                                          const QString &buttonName,
                                          QSanButton::ButtonState state) const
{
    const char* key;
    QString qkey = QString(QSanRoomSkin::S_SKIN_KEY_BUTTON).arg(groupName);
    QByteArray arr = qkey.toAscii();
    key = arr.constData();
    if (!isImageKeyDefined(key)) return QString();
    QString path = toQString(_m_imageConfig[key]);
    QString stateKey;
    if (state == QSanButton::S_STATE_DISABLED)
        stateKey = "disabled";
    else if (state == QSanButton::S_STATE_DOWN)
        stateKey = "down";
    else if (state == QSanButton::S_STATE_HOVER)
        stateKey = "hover";
    else if (state == QSanButton::S_STATE_UP)
        stateKey = "normal";
    else return QString(); // older Qt version cries for non-zero QPixmap...
    return path.arg(buttonName).arg(stateKey);
}

QPixmap QSanRoomSkin::getSkillButtonPixmap(QSanButton::ButtonState state,
                                           QSanInvokeSkillButton::SkillType type, 
                                           QSanInvokeSkillButton::SkillButtonWidth width) const
{
    QString path = getButtonPixmapPath(S_SKIN_KEY_BUTTON_SKILL,
        QSanInvokeSkillButton::getSkillTypeString(type), state);
    if (path.isNull())
        return QPixmap(1, 1); // older Qt version cries for non-zero QPixmap...
    else {
        QString arg2;
        if (width == QSanInvokeSkillButton::S_WIDTH_NARROW) arg2 = "3";
        else if (width == QSanInvokeSkillButton::S_WIDTH_MED) arg2 = "2";
        else if (width == QSanInvokeSkillButton::S_WIDTH_WIDE) arg2 = "1";
        return getPixmapFromFileName(path.arg(arg2));        
    }
}

QPixmap QSanRoomSkin::getButtonPixmap(const QString &groupName,
                const QString &buttonName, QSanButton::ButtonState state) const
{
    QString path = getButtonPixmapPath(groupName, buttonName, state);
    if (path.isNull())
        return QPixmap(1, 1); // older Qt version cries for non-zero QPixmap...
    else return getPixmapFromFileName(path);
}

QPixmap QSanRoomSkin::getCardFramePixmap(const QString &frameType) const
{
    return getPixmap(QString(QSanRoomSkin::S_SKIN_KEY_HAND_CARD_FRAME).arg(frameType));
}

QPixmap QSanRoomSkin::getProgressBarPixmap(int percentile) const
{
    Json::Value allMaps = _m_imageConfig[S_SKIN_KEY_PROGRESS_BAR_IMAGE];
    if (!allMaps.isArray()) return QPixmap();
    for (unsigned int i = 0; i < allMaps.size(); i++)
    {
        Json::Value progMap = allMaps[i];
        if (!allMaps[i][0].isInt()) continue;
        int thred = allMaps[i][0].asInt();
        if (thred >= percentile) {
            if (!allMaps[i][1].isString()) continue;
            return getPixmapFromFileName(toQString(allMaps[i][1]));
        }
    }
    return QPixmap();
}

QString QSanRoomSkin::getCardMainPixmapPath(const QString &cardName) const
{
    QString key = QString(QSanRoomSkin::S_SKIN_KEY_HAND_CARD_MAIN_PHOTO).arg(cardName);
    if (isImageKeyDefined(key))
        return toQString(_m_imageConfig[key.toAscii().constData()]);
    else
    {
        QString fileName = toQString(_m_imageConfig[QString(S_SKIN_KEY_HAND_CARD_MAIN_PHOTO)
                                     .arg("default").toAscii().constData()]).arg(cardName);
        return fileName;
    }
}

QPixmap QSanRoomSkin::getCardMainPixmap(const QString &cardName) const
{
    return getPixmapFromFileName(getCardMainPixmapPath(cardName));
}

QPixmap QSanRoomSkin::getCardSuitPixmap(Card::Suit suit) const{
    return getPixmap(QSanRoomSkin::S_SKIN_KEY_HAND_CARD_SUIT, Card::Suit2String(suit));
}

QPixmap QSanRoomSkin::getCardNumberPixmap(int point, bool isBlack) const{
    QString pathKey = isBlack ? S_SKIN_KEY_HAND_CARD_NUMBER_BLACK : S_SKIN_KEY_HAND_CARD_NUMBER_RED;
    return getPixmap(pathKey, QString::number(point));
}

QPixmap QSanRoomSkin::getCardJudgeIconPixmap(const QString &judgeName) const{
    return getPixmap(S_SKIN_KEY_JUDGE_CARD_ICON, judgeName);
}

QPixmap QSanRoomSkin::getCardAvatarPixmap(const QString &generalName) const{
    return getGeneralPixmap(generalName, S_GENERAL_ICON_SIZE_TINY);
}

QString QSanRoomSkin::getGeneralPixmapPath(const QString &generalName, GeneralIconSize size) const{
    if (size == S_GENERAL_ICON_SIZE_CARD)
        return getCardMainPixmapPath(generalName);
    else
    {
        QString key = QString(S_SKIN_KEY_PLAYER_GENERAL_ICON).arg(generalName).arg(size);
        if (isImageKeyDefined(key))
            return toQString(_m_imageConfig[key.toAscii().constData()]);
        else
        {
            QString fileName = toQString(_m_imageConfig[QString(S_SKIN_KEY_PLAYER_GENERAL_ICON)
                               .arg(S_SKIN_KEY_DEFAULT).arg(size).toAscii().constData()]).arg(generalName);
            return fileName;
        }
    }
}

QPixmap QSanRoomSkin::getGeneralPixmap(const QString &generalName, GeneralIconSize size) const{
    return getPixmapFromFileName(getGeneralPixmapPath(generalName, size));
}

QString QSanRoomSkin::getPlayerAudioEffectPath(const QString &eventName, bool isMale, int index) const{
    QString gender = isMale ? "male" : "female";
    QString fileName;
    QString key = QString(QSanRoomSkin::S_SKIN_KEY_PLAYER_AUDIO_EFFECT).arg(gender).arg(eventName);

    if (index == -1)
        fileName = getRandomAudioFileName(key);
    else
    {
        QStringList fileNames = getAudioFileNames(key);
        if(!fileNames.isEmpty())
        {
            if (fileNames.length() >= index) return fileNames[index - 1];
            else return fileNames[qrand() % fileNames.length()];
        }
    }

    if(fileName.isEmpty())
    {
        const Skill *skill = Sanguosha->getSkill(eventName);
        QStringList fileNames;
        if(skill) fileNames = skill->getSources();
        if(!fileNames.isEmpty())
        {
            if (index == -1)
                fileName = fileNames.at(qrand() % fileNames.length());
            else
            {
                if (fileNames.length() >= index) return fileNames[index - 1];
                else return fileNames[qrand() % fileNames.length()];
            }
        }
    }

    if(fileName.isEmpty())
    {
        fileName = toQString(_m_audioConfig[QString(S_SKIN_KEY_PLAYER_AUDIO_EFFECT)
            .arg(gender).arg("default").toAscii().constData()]).arg(eventName);
    }
    return fileName;
}

QRect IQSanComponentSkin::AnchoredRect::getTranslatedRect(QRect parentRect, QSize size) const
{
    QPoint parentAnchor;
    Qt::Alignment hAlign = m_anchorParent & Qt::AlignHorizontal_Mask;
    if (hAlign == Qt::AlignRight) parentAnchor.setX(parentRect.right());
    else if (hAlign == Qt::AlignHCenter) parentAnchor.setX(parentRect.center().x());
    else parentAnchor.setX(parentRect.left());
    Qt::Alignment vAlign = m_anchorParent & Qt::AlignVertical_Mask;
    if (vAlign == Qt::AlignBottom) parentAnchor.setY(parentRect.bottom());
    else if (vAlign == Qt::AlignCenter) parentAnchor.setY(parentRect.center().y() );
    else parentAnchor.setY(parentRect.top());

    QPoint childAnchor;
    hAlign = m_anchorChild & Qt::AlignHorizontal_Mask;
    if (hAlign == Qt::AlignRight) childAnchor.setX(size.width());
    else if (hAlign == Qt::AlignHCenter) childAnchor.setX(size.width() / 2);
    else childAnchor.setX(0);
    vAlign = m_anchorChild & Qt::AlignVertical_Mask;
    if (vAlign == Qt::AlignBottom) childAnchor.setY(size.height());
    else if (vAlign == Qt::AlignCenter) childAnchor.setY(size.height() / 2);
    else childAnchor.setY(0);
    
    QPoint pos = parentAnchor - childAnchor + m_offset;
    QRect rect(pos, size);
    return rect;
}

QRect IQSanComponentSkin::AnchoredRect::getTranslatedRect(QRect parentRect) const
{
    Q_ASSERT(m_useFixedSize);
    return getTranslatedRect(parentRect, m_fixedSize);
}

bool IQSanComponentSkin::AnchoredRect::tryParse(Json::Value value)
{
    // must be in one of the following format:
    // [offsetX, offestY, sizeX, sizeY]
    // [childAnchor, parentAnchor, [offsetX, offsetY]]
    // [childAnchor, parentAnchor, [offsetX, offsetY], [sizeX, sizeY]]
    m_useFixedSize = false;
    m_anchorChild = m_anchorParent = Qt::AlignLeft | Qt::AlignTop;
    if (isIntArray(value, 0, 3))
    {
        QRect rect;
        bool success = QSanProtocol::Utils::tryParse(value, rect);
        if (!success) return false;
        m_useFixedSize = true;
        m_fixedSize = rect.size();
        m_offset = rect.topLeft();
    }
    else if (isStringArray(value, 0, 0) && value.size() >= 3 &&
             isIntArray(value[2], 0, 1))
    {
        if (_tryParse(value[0].asCString(), m_anchorChild) &&
            _tryParse(value[1].asCString(), m_anchorParent) &&
            QSanProtocol::Utils::tryParse(value[2], m_offset))
        {
            if (value.size() >= 4 && isIntArray(value[3], 0, 1) &&
                QSanProtocol::Utils::tryParse(value[3], m_fixedSize))
                m_useFixedSize = true;
            return true;
        }
    }
    return false;
}

// Load pixmap from a file and map it to the given key.
const QPixmap& QSanPixmapCache::getPixmap(const QString &key, const QString &fileName)
{
    if (!_m_pixmapBank.contains(key))
    {
        bool success = !fileName.isEmpty() && _m_pixmapBank[key].load(fileName);
        if (!success)
        {
            qWarning("Unable to open resource file \"%s\" for key \"%s\"\n",
                fileName.toAscii().constData(),
                key.toAscii().constData());
            _m_pixmapBank[key] = QPixmap(1, 1); // make Qt happy
        }
    }
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

bool IQSanComponentSkin::load(const QString &layoutConfigName, const QString &imageConfigName,
                              const QString &audioConfigName)
{
    Json::Reader layoutReader, imageReader, audioReader;
    ifstream layoutFile(layoutConfigName.toAscii());
    ifstream imageFile(imageConfigName.toAscii());
    ifstream audioFile(audioConfigName.toAscii());
    bool success = (!layoutFile.bad() && !imageFile.bad() && !audioFile.bad());
    QString errorMsg;
    if (!layoutReader.parse(layoutFile, this->_m_layoutConfig))
    {
        errorMsg = QString("Error when reading layout config file \"%1\": \n%2")
                   .arg(layoutConfigName).arg(layoutReader.getFormattedErrorMessages().c_str());
        QMessageBox::warning(NULL, "Config Error", errorMsg);
        success = false;
    }
    if (!imageReader.parse(imageFile, this->_m_imageConfig))
    {
        errorMsg = QString("Error when reading image config file \"%1\": \n%2")
                   .arg(imageConfigName).arg(imageReader.getFormattedErrorMessages().c_str());
        QMessageBox::warning(NULL, "Config Error", errorMsg);
        success = false;
    }
    if (!audioReader.parse(audioFile, this->_m_audioConfig))
    {
        errorMsg = QString("Error when reading audio config file \"%1\": \n%2")
                   .arg(audioConfigName).arg(audioReader.getFormattedErrorMessages().c_str());
        QMessageBox::warning(NULL, "Config Error", errorMsg);
        success = false;
    }
    layoutFile.close(); imageFile.close(); audioFile.close();
    success &= (_m_layoutConfig.isObject() && _m_imageConfig.isObject() && _m_audioConfig.isObject());
    if (_m_layoutConfig.isObject()) success = _loadLayoutConfig();
    return success;
}

QStringList IQSanComponentSkin::getAudioFileNames(const QString &key) const
{
    Json::Value result = _m_audioConfig[key.toAscii().constData()];
    if (result.isNull()) return QStringList();
    else if (result.isString()) return QStringList(result.asCString());
    else if (result.isArray())
    {
        QStringList audios;
        tryParse(result, audios);
        return audios;
    }
    return QStringList();
}

QString IQSanComponentSkin::getRandomAudioFileName(const QString &key) const
{
    QStringList audios = getAudioFileNames(key);
    if (audios.isEmpty()) return QString();
    int r = qrand() % audios.length();
    return audios[r];
}

QString IQSanComponentSkin::_readConfig(const Json::Value &dict, const QString &key,
                                        const QString &defaultValue) const
{
    if (!dict.isObject()) return defaultValue;
    Json::Value val = dict[key.toAscii().constData()];
    if (!val.isString())
    {
        qWarning("Unable to read configuration: %s", key.toAscii().constData());
        return defaultValue;
    }
    else return val.asCString();
}

QString IQSanComponentSkin::_readImageConfig(const QString &key, QRect &rect,
    bool& clipping, QSize &newScale, bool scaled, const QString &defaultValue) const
{
    clipping = false;
    scaled = false;
    if (!_m_imageConfig.isObject()) return defaultValue;
    Json::Value val = _m_imageConfig[key.toAscii().constData()];
    QString result;
    if (val.isString())
    {
        result = val.asCString();
    }
    else if (val.isArray() && val.size() >= 2 && val[0].isString() &&
             tryParse(val[1], rect))
    {
        clipping = true;
        result = val[0].asCString();
        if (val.size() >= 3 && tryParse(val[3], newScale))
        {
            scaled = true;
        }
    }
    else
    {
        qWarning("Unable to read configuration: %s", key.toAscii().constData());
        return defaultValue;
    }
    return result;
}
 QPixmap IQSanComponentSkin::getPixmap(const QString &key, const QString &arg) const
 {
     QString totalkey;
     if (arg.isNull()) totalkey = key;
     else totalkey = key.arg(arg);
     static QHash<QString, QPixmap> _pixmapCache;
     if (!_pixmapCache.contains(totalkey))
     {
         QRect clipRegion;
         QSize scaleRegion;
         bool clipping = false;
         bool scaled = false;

         bool useDefault = false;
         QString keyFile;
         if (arg.isNull()) keyFile = key;
         else keyFile = key.arg(arg);
         if (!isImageKeyDefined(keyFile))
         {
             useDefault = true;
             Q_ASSERT(!arg.isNull());
             keyFile = key.arg(S_SKIN_KEY_DEFAULT);
         }

         QString fileName = _readImageConfig(keyFile,
                             clipRegion, clipping, scaleRegion, scaled);
         if (useDefault) fileName = fileName.arg(arg);
         QPixmap pixmap = QSanPixmapCache::getPixmap(totalkey, fileName);
         if (clipping)
         {
             _pixmapCache[totalkey] = pixmap.copy(clipRegion);
             if (scaled)
             {
                 _pixmapCache[totalkey] = _pixmapCache[totalkey].scaled(
                     scaleRegion, Qt::IgnoreAspectRatio,
                     Qt::SmoothTransformation);
             }
         }
         else
             _pixmapCache[totalkey] = pixmap;
     }
     return _pixmapCache[totalkey];
 }

 QPixmap IQSanComponentSkin::getPixmapFileName(const QString &key) const
 {
     return _readConfig(_m_imageConfig, key);
 }

 QPixmap IQSanComponentSkin::getPixmapFromFileName(const QString &fileName) const
{
    return QSanPixmapCache::getPixmap(fileName, fileName);
}

const QSanRoomSkin::RoomLayout& QSanRoomSkin::getRoomLayout() const
{
    return this->_m_roomLayout;
}

const QSanRoomSkin::PhotoLayout& QSanRoomSkin::getPhotoLayout() const
{
    return _m_photoLayout;
}

const QSanRoomSkin::DashboardLayout& QSanRoomSkin::getDashboardLayout() const
{
    return _m_dashboardLayout;
}

const QSanRoomSkin::CommonLayout& QSanRoomSkin::getCommonLayout() const
{
    return _m_commonLayout;
}

bool IQSanComponentSkin::_tryParse(const QString &str, Qt::Alignment &align)
{
    QString alignStr = str.toLower();
    if (alignStr.contains("left")) align = Qt::AlignLeft;
    else if (alignStr.contains("right")) align = Qt::AlignRight;
    else if (alignStr.contains("center")) align = Qt::AlignHCenter;

    if (alignStr.contains("top")) align |= Qt::AlignTop;
    else if (alignStr.contains("bottom")) align |= Qt::AlignBottom;
    else if (alignStr.contains("center")) align |= Qt::AlignVCenter;
    
    return true;
}

QSanRoomSkin::QSanShadowTextFont 
QSanRoomSkin::DashboardLayout::getSkillTextFont(QSanButton::ButtonState state,
                                                QSanInvokeSkillButton::SkillType type,
                                                QSanInvokeSkillButton::SkillButtonWidth width) const
{ 
    int i = QSanButton::S_NUM_BUTTON_STATES * (int)type + (int)state;
    QSanShadowTextFont font = m_skillTextFonts[width];
    font.m_color = m_skillTextColors[i];
    font.m_shadowColor = m_skillTextShadowColors[i];
    return font;
}
    
bool QSanRoomSkin::_loadLayoutConfig()
{
    Json::Value config = _m_layoutConfig[S_SKIN_KEY_COMMON];
    _m_commonLayout.m_cardNormalHeight = config["cardNormalHeight"].asInt();
    _m_commonLayout.m_cardNormalWidth = config["cardNormalWidth"].asInt();
    _m_commonLayout.m_hpExtraSpaceHolder = config["hpExtraSpaceHolder"].asInt();
    tryParse(config["cardMainArea"], _m_commonLayout.m_cardMainArea);
    tryParse(config["cardSuitArea"], _m_commonLayout.m_cardSuitArea);
    tryParse(config["cardNumberArea"], _m_commonLayout.m_cardNumberArea);
    tryParse(config["cardFrameArea"], _m_commonLayout.m_cardFrameArea);
    tryParse(config["cardFootnoteArea"], _m_commonLayout.m_cardFootnoteArea);
    tryParse(config["cardAvatarArea"], _m_commonLayout.m_cardAvatarArea);
    _m_commonLayout.m_chooseGeneralBoxSwitchIconSizeThreshold =
        config["chooseGeneralBoxSwitchIconSizeThreshold"].asInt();
    tryParse(config["chooseGeneralBoxDenseIconSize"],
                                  _m_commonLayout.m_chooseGeneralBoxDenseIconSize);
    tryParse(config["chooseGeneralBoxSparseIconSize"],
                                  _m_commonLayout.m_chooseGeneralBoxSparseIconSize);
    _m_commonLayout.m_cardFootnoteFont.tryParse(config["cardFootnoteFont"]);
    for (int i = 0; i < 6; i++)
    {
        _m_commonLayout.m_hpFont[i].tryParse(config["magatamaFont"][i]);
    }

    config = _m_layoutConfig[S_SKIN_KEY_ROOM];
    _m_roomLayout.m_chatBoxHeightPercentage = config["chatBoxHeightPercentage"].asDouble();
    _m_roomLayout.m_chatTextBoxHeight = config["chatTextBoxHeight"].asInt();
    _m_roomLayout.m_discardPileMinWidth = config["discardPileMinWidth"].asInt();
    _m_roomLayout.m_discardPilePadding = config["discardPilePadding"].asInt();
    _m_roomLayout.m_infoPlaneWidthPercentage = config["infoPlaneWidthPercentage"].asDouble();
    _m_roomLayout.m_logBoxHeightPercentage = config["logBoxHeightPercentage"].asDouble();
    tryParse(config["minimumSceneSize"], _m_roomLayout.m_minimumSceneSize);
    _m_roomLayout.m_photoHDistance = config["photoHDistance"].asInt();
    _m_roomLayout.m_photoVDistance = config["photoVDistance"].asInt();
    _m_roomLayout.m_photoDashboardPadding = config["photoDashboardPadding"].asInt();
    _m_roomLayout.m_roleBoxHeight = config["roleBoxHeight"].asInt();
    _m_roomLayout.m_scenePadding = config["scenePadding"].asInt();

    for (int i = 0; i < 2; i++)
    {
        Json::Value playerConfig;
        PlayerCardContainerLayout* layout;
        if (i == 0)
        {
            layout = &_m_photoLayout;
            playerConfig = _m_layoutConfig[S_SKIN_KEY_PHOTO];
        }
        else
        {
            layout = &_m_dashboardLayout;
            playerConfig = _m_layoutConfig[S_SKIN_KEY_DASHBOARD];
        }

        layout->m_normalHeight = playerConfig["normalHeight"].asInt();
        // @todo: focusFrameArea has not been parsed for dashboard
        // @todo: rename this to "handCardNumIconArea"
        tryParse(playerConfig["handCardNumIconArea"], layout->m_handCardArea);
        for (int j = 0; j < 4; j++)
        {
            tryParse(playerConfig["equipAreas"][j], layout->m_equipAreas[j]);
        }
        tryParse(playerConfig["equipImageArea"], layout->m_equipImageArea);
        tryParse(playerConfig["equipTextArea"], layout->m_equipTextArea);
        tryParse(playerConfig["equipSuitArea"], layout->m_equipSuitArea);
        tryParse(playerConfig["equipDistanceArea"], layout->m_equipDistanceArea);
        tryParse(playerConfig["equipPointArea"], layout->m_equipPointArea);
        layout->m_equipFont.tryParse(playerConfig["equipFont"]);
        layout->m_equipPointFont.tryParse(playerConfig["equipPointFont"]);

        tryParse(playerConfig["delayedTrickFirstRegion"],
                                      layout->m_delayedTrickFirstRegion);
        tryParse(playerConfig["delayedTrickStep"], layout->m_delayedTrickStep);
        
        layout->m_markTextArea.tryParse(playerConfig["markTextArea"]);
        tryParse(playerConfig["roleComboBoxPos"], layout->m_roleComboBoxPos);

        tryParse(playerConfig["avatarArea"], layout->m_avatarArea);
        tryParse(playerConfig["smallAvatarArea"], layout->m_smallAvatarArea);
        layout->m_avatarSize = playerConfig["avatarImageType"].asInt();
        layout->m_smallAvatarSize = playerConfig["smallAvatarImageType"].asInt();
        tryParse(playerConfig["avatarNameArea"], layout->m_avatarNameArea);
        layout->m_avatarNameFont.tryParse(playerConfig["avatarNameFont"]);
        tryParse(playerConfig["smallAvatarNameArea"], layout->m_smallAvatarNameArea);
        layout->m_smallAvatarNameFont.tryParse(playerConfig["smallAvatarNameFont"]);
        tryParse(playerConfig["kingdomMaskArea"], layout->m_kingdomMaskArea);
        tryParse(playerConfig["kingdomIconArea"], layout->m_kingdomIconArea);

        layout->m_handCardFont.tryParse(playerConfig["handCardFont"]);
        tryParse(playerConfig["screenNameArea"], layout->m_screenNameArea);
        layout->m_screenNameFont.tryParse(playerConfig["screenNameFont"]);

        layout->m_progressBarArea.tryParse(playerConfig["progressBarArea"]);
        layout->m_isProgressBarHorizontal = playerConfig["progressBarHorizontal"].asBool();
        tryParse(playerConfig["magatamaSize"], layout->m_magatamaSize);
        layout->m_magatamasHorizontal = playerConfig["magatamasHorizontal"].asBool();
        layout->m_magatamasBgVisible = playerConfig["magatamasBgVisible"].asBool();
        tryParse(playerConfig["magatamasAnchor"][1], layout->m_magatamasAnchor);
        if (playerConfig["magatamasAnchor"][0].isString())
            _tryParse(playerConfig["magatamasAnchor"][0].asCString(), layout->m_magatamasAlign);

        layout->m_phaseArea.tryParse(playerConfig["phaseArea"]);
        tryParse(playerConfig["privatePileStartPos"], layout->m_privatePileStartPos);
        tryParse(playerConfig["privatePileStep"], layout->m_privatePileStep);
        tryParse(playerConfig["privatePileButtonSize"], layout->m_privatePileButtonSize);
        tryParse(playerConfig["actionedIconRegion"], layout->m_actionedIconRegion);
        tryParse(playerConfig["saveMeIconRegion"], layout->m_saveMeIconRegion);
        tryParse(playerConfig["chainedIconRegion"], layout->m_chainedIconRegion);
        tryParse(playerConfig["readyIconRegion"], layout->m_readyIconRegion);
        tryParse(playerConfig["deathIconRegion"], layout->m_deathIconRegion);
        tryParse(playerConfig["votesIconRegion"], layout->m_votesIconRegion);
        tryParse(playerConfig["drankMaskColor"], layout->m_drankMaskColor);
    }


    config = _m_layoutConfig[S_SKIN_KEY_PHOTO];
    
    _m_photoLayout.m_normalWidth = config["normalWidth"].asInt();
    if (!tryParse(config["focusFrameArea"], _m_photoLayout.m_focusFrameArea)
        && config["borderWidth"].isInt())
    {
        int borderWidth = config["borderWidth"].asInt();
        _m_photoLayout.m_focusFrameArea = QRect(
                    -borderWidth, -borderWidth,
            _m_photoLayout.m_normalWidth + 2 * borderWidth,
            _m_photoLayout.m_normalHeight + 2 * borderWidth);
    }
    tryParse(config["mainFrameArea"], _m_photoLayout.m_mainFrameArea);
    tryParse(config["onlineStatusArea"], _m_photoLayout.m_onlineStatusArea);
    tryParse(config["onlineStatusBgColor"], _m_photoLayout.m_onlineStatusBgColor);
    _m_photoLayout.m_onlineStatusFont.tryParse(config["onlineStatusFont"]);
    tryParse(config["cardMoveArea"], _m_photoLayout.m_cardMoveRegion);
    tryParse(config["skillNameArea"], _m_photoLayout.m_skillNameArea);
    _m_photoLayout.m_skillNameFont.tryParse(config["skillNameFont"]);
    tryParse(config["canvasArea"], _m_photoLayout.m_boundingRect);

    config = _m_layoutConfig[S_SKIN_KEY_DASHBOARD];
    _m_dashboardLayout.m_leftWidth = config["leftWidth"].asInt();
    _m_dashboardLayout.m_rightWidth = config["rightWidth"].asInt();
    _m_dashboardLayout.m_floatingAreaHeight = config["floatingAreaHeight"].asInt();
    tryParse(config["focusFrameArea"], _m_dashboardLayout.m_focusFrameArea);
    tryParse(config["buttonSetSize"], _m_dashboardLayout.m_buttonSetSize);
    tryParse(config["confirmButtonArea"], _m_dashboardLayout.m_confirmButtonArea);
    tryParse(config["cancelButtonArea"], _m_dashboardLayout.m_cancelButtonArea);
    tryParse(config["discardButtonArea"], _m_dashboardLayout.m_discardButtonArea);
    tryParse(config["trustButtonArea"], _m_dashboardLayout.m_trustButtonArea);
    config = _m_layoutConfig["skillButton"];
    for (int i = 0; i < 3; i++)
    {
        _m_dashboardLayout.m_skillButtonsSize[i].setHeight(config["height"].asInt());
        _m_dashboardLayout.m_skillButtonsSize[i].setWidth(config["width"][i].asInt());
        tryParse(config["textArea"][i], _m_dashboardLayout.m_skillTextArea[i]);
        _m_dashboardLayout.m_skillTextFonts[i].tryParse(config["textFont"][i]);
    }
    for (int i = 0; i < 5; i++)
    {
        QString key;
        switch((QSanInvokeSkillButton::SkillType)i)
        {
        case QSanInvokeSkillButton::S_SKILL_AWAKEN:
            key = "awakenFontColor";
            break;
        case QSanInvokeSkillButton::S_SKILL_COMPULSORY:
            key = "compulsoryFontColor";
            break;
        case QSanInvokeSkillButton::S_SKILL_FREQUENT:
            key = "frequentFontColor";
            break;
        case QSanInvokeSkillButton::S_SKILL_ONEOFF_SPELL:
            key = "oneoffFontColor";
            break;
        case QSanInvokeSkillButton::S_SKILL_PROACTIVE:
            key = "proactiveFontColor";
            break;
        default:
            Q_ASSERT(false);
            break;
        }
        for (int j = 0; j < 4; j++)
        {
            int index = i * 4 + j;
            QByteArray arr = key.toAscii();;
            const char* sKey = arr.constData();
            tryParse(config[sKey][j][0], _m_dashboardLayout.m_skillTextColors[index]);
            tryParse(config[sKey][j][1], _m_dashboardLayout.m_skillTextShadowColors[index]);
        }
    }
    
    return true;
}


bool QSanSkinScheme::load(Json::Value configs)
{
    if (!configs.isObject()) return false;
    return _m_roomSkin.load(configs["roomLayoutConfigFile"].asCString(),
                            configs["roomImageConfigFile"].asCString(),
                            configs["roomAudioConfigFile"].asCString());
}

const QSanRoomSkin& QSanSkinScheme::getRoomSkin() const
{
    return _m_roomSkin;
}

QSanSkinFactory& QSanSkinFactory::getInstance()
{
    if (_sm_singleton == NULL)
        _sm_singleton = new QSanSkinFactory("skins/skinList.json");
    return *_sm_singleton;
}

const QSanSkinScheme& QSanSkinFactory::getCurrentSkinScheme()
{
    if (!_m_isSkinSet)
        switchSkin("default");
    return this->_sm_currentSkin;
}

bool QSanSkinFactory::switchSkin(QString skinName)
{
    bool success = _sm_currentSkin.load(_m_skinList[skinName.toAscii().constData()]);
    if (success) _m_isSkinSet = true;
    return success;
}

QSanSkinFactory::QSanSkinFactory(const char* fileName)
{
    _m_isSkinSet = false;
    Json::Reader reader;
    ifstream file(fileName);
    reader.parse(file, this->_m_skinList, false);
    file.close();
}
