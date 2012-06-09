#include "SkinBank.h"
#include "jsonutils.h"
#include "uiUtils.h"
#include <fstream>
#include <QGraphicsPixmapItem>
#include <QTextItem>
#include <QStyleOptionGraphicsItem>

using namespace std;

const char* QSanRoomSkin::S_SKIN_KEY_PHOTO = "photo";
const char* QSanRoomSkin::S_SKIN_KEY_ROOM = "room";
const char* QSanRoomSkin::S_SKIN_KEY_COMMON = "common";
const char* QSanRoomSkin::S_SKIN_KEY_PHOTO_MAINFRAME = "photoMainFrame";
const char* QSanRoomSkin::S_SKIN_KEY_PHOTO_HANDCARDNUM = "photoHandCardNum";
const char* QSanRoomSkin::S_SKIN_KEY_PHOTO_FACETURNEDMASK = "photoFaceTurnedMask";
const char* QSanRoomSkin::S_SKIN_KEY_PHOTO_BLANK_GENERAL = "photoBlankGeneral";
const char* QSanRoomSkin::S_SKIN_KEY_PHOTO_CHAIN = "photoChain";
const char* QSanRoomSkin::S_SKIN_KEY_PHOTO_PHASE = "photoPhase%1";
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

QSanSkinFactory* QSanSkinFactory::_sm_singleton = NULL;
QHash<QString, QPixmap> QSanPixmapCache::_m_pixmapBank;

bool QSanRoomSkin::QSanSimpleTextFont::tryParse(Json::Value arg)
{
	if (!arg.isArray() || arg.size() < 4) return false;
	m_font = QFont(arg[0].asCString(), arg[1].asInt(), arg[2].asInt());
	m_foregroundPen = QPen(QColor(arg[3][0].asInt(), arg[3][1].asInt(), arg[3][2].asInt(), arg[3][3].asInt()));
	
	return true;
}

bool QSanRoomSkin::QSanShadowTextFont::tryParse(Json::Value arg)
{
	if (!arg.isArray() || arg.size() < 8) return false;
	if (!QSanSimpleTextFont::tryParse(arg)) return false;
	m_shadowRadius = arg[4].asInt();
	m_shadowDecadeFactor = arg[5].asDouble();
	QSanProtocol::Utils::tryParse(arg[6], m_shadowOffset);
	m_shadowColor = QColor(arg[7][0].asInt(), arg[7][1].asInt(), arg[7][2].asInt(), arg[7][3].asInt());
	return true;
}

void QSanRoomSkin::QSanSimpleTextFont::paintText(QPainter* painter, QRect pos, Qt::AlignmentFlag align, const QString &text) const
{
	painter->setPen(m_foregroundPen);
	painter->drawText(pos, align, text);
}

void QSanRoomSkin::QSanShadowTextFont::paintText(QGraphicsPixmapItem* pixmapItem,
	                                             QRect pos,
												 Qt::AlignmentFlag align,
												 const QString &text) const
{
	QImage image(pos.width() + m_shadowRadius * 2, pos.height() + m_shadowRadius * 2, QImage::Format_ARGB32);
	image.fill(qRgba(0, 0, 0, 0));
	QPainter imagePainter(&image);
	imagePainter.setFont(m_font);
	imagePainter.setPen(m_foregroundPen);
	imagePainter.drawText(QRectF(m_shadowRadius, m_shadowRadius, pos.width(), pos.height()), align, text);
	QImage shadow = QSanUiUtils::produceShadow(image, m_shadowColor, m_shadowRadius, m_shadowDecadeFactor);
	// now, overlay foreground on shadow
	QPainter shadowPainter(&shadow);
	shadowPainter.drawImage(0, 0, image);
	QPixmap pixmap = QPixmap::fromImage(shadow);
	pixmapItem->setPixmap(pixmap);
	pixmapItem->setPos(pos.x() - m_shadowRadius, pos.y() - m_shadowRadius);
}

QPixmap QSanRoomSkin::getCardFramePixmap(const QString &frameType) const
{
	return getPixmap(QString(QSanRoomSkin::S_SKIN_KEY_HAND_CARD_FRAME).arg(frameType));
}

QString QSanRoomSkin::getCardMainPixmapPath(const QString &cardName) const
{
	return QString(QSanRoomSkin::S_SKIN_KEY_HAND_CARD_MAIN_PHOTO).arg(cardName);
}

QPixmap QSanRoomSkin::getCardMainPixmap(const QString &cardName) const
{
	return getPixmap(QString(QSanRoomSkin::S_SKIN_KEY_HAND_CARD_MAIN_PHOTO).arg(cardName));
}

QPixmap QSanRoomSkin::getCardSuitPixmap(Card::Suit suit) const{
	return getPixmap(QString(QSanRoomSkin::S_SKIN_KEY_HAND_CARD_SUIT).arg(Card::Suit2String(suit)));
}

QPixmap QSanRoomSkin::getCardNumberPixmap(int point, bool isBlack) const{
	QString pathKey = isBlack ? S_SKIN_KEY_HAND_CARD_NUMBER_BLACK : S_SKIN_KEY_HAND_CARD_NUMBER_RED; 
	return getPixmap(pathKey.arg(point));
}

QPixmap QSanRoomSkin::getCardJudgeIconPixmap(const QString &judgeName) const{
	return getPixmap(QString(S_SKIN_KEY_JUDGE_CARD_ICON).arg(judgeName));
}

QPixmap QSanRoomSkin::getCardAvatarPixmap(const QString &generalName) const{
	return getPixmap(QString(S_SKIN_KEY_HAND_CARD_AVATAR).arg(generalName));
}

QString QSanRoomSkin::getGeneralPixmapPath(const QString &generalName, GeneralIconSize size) const{
	if (size == S_GENERAL_ICON_SIZE_CARD)
		return getCardMainPixmapPath(generalName);
	else
		return QString(S_SKIN_KEY_PLAYER_GENERAL_ICON).arg(generalName).arg(size);
}

QPixmap QSanRoomSkin::getGeneralPixmap(const QString &generalName, GeneralIconSize size) const{
	if (size == S_GENERAL_ICON_SIZE_CARD)
		return getCardMainPixmap(generalName);
	else
		return getPixmap(QString(S_SKIN_KEY_PLAYER_GENERAL_ICON).arg(generalName).arg(size));
}

QString QSanRoomSkin::getPlayerAudioEffectPath(const QString &eventName, bool isMale, int index) const{
	QString gender = isMale ? "male" : "female";
	QString key = QString(QSanRoomSkin::S_SKIN_KEY_PLAYER_AUDIO_EFFECT).arg(gender).arg(eventName);
	QString fileName;
	if (index == -1) 
		fileName = getRandomAudioFileName(key);
	else
	{
		QStringList fileNames = getAudioFileNames(key);
		if (fileNames.length() > index) return fileNames[index];
	}
	if (fileName.isEmpty())
	{
		key = QString(QSanRoomSkin::S_SKIN_KEY_PLAYER_AUDIO_EFFECT).arg("common").arg(eventName); 
		if (index == -1) 
		fileName = getRandomAudioFileName(key);
		else
		{
			QStringList fileNames = getAudioFileNames(key);
			if (fileNames.length() > index) return fileNames[index];
		}
	}
	return fileName;
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

bool IQSanComponentSkin::load(const QString &layoutConfigName, const QString &imageConfigName, const QString &audioConfigName)
{
	Json::Reader layoutReader, imageReader, audioReader;
	ifstream layoutFile(layoutConfigName.toAscii()), imageFile(imageConfigName.toAscii()), audioFile(audioConfigName.toAscii());
	bool success = (!layoutFile.bad() && !imageFile.bad() && !audioFile.bad());
	if (!layoutReader.parse(layoutFile, this->_m_layoutConfig))
	{
		qWarning("Error when reading layout config file \"%s\": ", layoutConfigName.toAscii().constData());
		qWarning(layoutReader.getFormattedErrorMessages().c_str());
		success = false;
	}
	if (!imageReader.parse(imageFile, this->_m_imageConfig))
	{
		qWarning("Error when reading image config file \"%s\": ", imageConfigName.toAscii().constData());
		qWarning(imageReader.getFormattedErrorMessages().c_str());
		success = false;
	}
	if (!audioReader.parse(audioFile, this->_m_audioConfig))
	{
		qWarning("Error when reading audio config file \"%s\": ", audioConfigName.toAscii().constData());
		qWarning(audioReader.getFormattedErrorMessages().c_str());
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
	if (result == Json::nullValue) return QStringList();
	else if (result == Json::stringValue) return QStringList(result.asCString());
	else if (result == Json::arrayValue)
	{
		QStringList audios;
		QSanProtocol::Utils::tryParse(result, audios);
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

QString IQSanComponentSkin::_readConfig(const Json::Value &dict, const QString &key, const QString &defaultValue) const
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

QPixmap IQSanComponentSkin::getPixmap(const QString &key) const
{
	return QSanPixmapCache::getPixmap(key, _readConfig(_m_imageConfig, key));
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
	QSanProtocol::Utils::tryParse(config["delayedTrickStartPos"], _m_photoLayout.m_delayedTrickStartPos);
	QSanProtocol::Utils::tryParse(config["delayedTrickStep"], _m_photoLayout.m_delayedTrickStep);
	QSanProtocol::Utils::tryParse(config["privatePileStartPos"], _m_photoLayout.m_privatePileStartPos);
	QSanProtocol::Utils::tryParse(config["privatePileStep"], _m_photoLayout.m_privatePileStep);
	QSanProtocol::Utils::tryParse(config["privatePileButtonSize"], _m_photoLayout.m_privatePileButtonSize);
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
	QSanProtocol::Utils::tryParse(config["cardMainArea"], _m_commonLayout.m_cardMainArea);
	QSanProtocol::Utils::tryParse(config["cardSuitArea"], _m_commonLayout.m_cardSuitArea);
	QSanProtocol::Utils::tryParse(config["cardNumberArea"], _m_commonLayout.m_cardNumberArea);
	QSanProtocol::Utils::tryParse(config["cardFootnoteArea"], _m_commonLayout.m_cardFootnoteArea);
	QSanProtocol::Utils::tryParse(config["cardAvatarArea"], _m_commonLayout.m_cardAvatarArea);
	_m_commonLayout.m_chooseGeneralBoxSwitchIconSizeThreshold = config["chooseGeneralBoxSwitchIconSizeThreshold"].asInt();
	QSanProtocol::Utils::tryParse(config["chooseGeneralBoxDenseIconSize"], _m_commonLayout.m_chooseGeneralBoxDenseIconSize);
	QSanProtocol::Utils::tryParse(config["chooseGeneralBoxSparseIconSize"], _m_commonLayout.m_chooseGeneralBoxSparseIconSize);
	_m_commonLayout.m_cardFootnoteFont.tryParse(config["cardFootnoteFont"]);
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
		_sm_singleton = new QSanSkinFactory("skins\\skinList.json");
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
