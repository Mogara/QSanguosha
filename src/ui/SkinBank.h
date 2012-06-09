#ifndef _SKIN_BANK_H
#define _SKIN_BANK_H

#define QSAN_UI_LIBRARY_AVAILABLE

#include "card.h"

#include <json/json.h>
#include <QString>
#include <QPixmap>
#include <QHash>
#include <QFont>
#include <QPen>
#include <QPainter>
#include <QGraphicsPixmapItem>

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
	bool load(const QString &layoutConfigFileName, const QString &imageConfigFileName, const QString &audioConfigFileName);
	QPixmap getPixmap(const QString &key) const;
	QPixmap getPixmapFileName(const QString &key) const;
	QPixmap getPixmapFromFileName(const QString &fileName) const;
	QStringList getAudioFileNames(const QString &key) const;
	QString getRandomAudioFileName(const QString &key) const;
protected:
	virtual bool _loadLayoutConfig() = 0;
	QString _readConfig(const Json::Value &dictionary, const QString &key, const QString &defaultValue = QString()) const;
	Json::Value _m_layoutConfig;
	Json::Value _m_imageConfig;
	Json::Value _m_audioConfig;
};

class QSanRoomSkin : public IQSanComponentSkin
{
public:
	class QSanSimpleTextFont {
	public:
		QFont m_font;
		QPen m_foregroundPen;
		bool tryParse(Json::Value arg);
		void paintText(QPainter* painter, QRect pos, Qt::AlignmentFlag align, const QString &text) const;
	};

	class QSanShadowTextFont : QSanSimpleTextFont{
	public:
		int m_shadowRadius;
		double m_shadowDecadeFactor;
		QPoint m_shadowOffset;
		QColor m_shadowColor;
		bool tryParse(Json::Value arg);
		void paintText(QGraphicsPixmapItem* item, QRect pos, Qt::AlignmentFlag align, const QString &text) const;
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
		
		// delayed trick area
		QPoint m_delayedTrickStartPos;
		QPoint m_delayedTrickStep;
		// private pile (e.g. 7 stars, buqu)
		QPoint m_privatePileStartPos;
		QPoint m_privatePileStep;
		QSize m_privatePileButtonSize;
		// progress bar
		bool m_isProgressBarHorizontal;
		QRect m_progressBarArea;
	};
	struct CommonLayout
	{
		// card related
		int m_cardNormalWidth;
		int m_cardNormalHeight;
		QRect m_cardMainArea;
		QRect m_cardSuitArea;
		QRect m_cardNumberArea;
		QRect m_cardFootnoteArea;
		QRect m_cardAvatarArea;
		QRect m_cardFrameArea;
		QSanShadowTextFont m_cardFootnoteFont;

		// dialogs
		QSize m_chooseGeneralBoxSparseIconSize; // when # of generals < switchIconSizeThreadshold
		QSize m_chooseGeneralBoxDenseIconSize; // when # of generals < switchIconSizeThreadshold
		int m_chooseGeneralBoxSwitchIconSizeThreshold;
	};
	enum GeneralIconSize
	{
		S_GENERAL_ICON_SIZE_TINY,
		S_GENERAL_ICON_SIZE_SMALL,
		S_GENERAL_ICON_SIZE_LARGE,
		S_GENERAL_ICON_SIZE_CARD
	};

	const RoomLayout& getRoomLayout() const;
	const PhotoLayout& getPhotoLayout() const;
	const CommonLayout& getCommonLayout() const;
	
	QString getCardMainPixmapPath(const QString &cardName) const;
	QPixmap getCardMainPixmap(const QString &cardName) const;
	QPixmap getCardSuitPixmap(Card::Suit suit) const;
	QPixmap getCardNumberPixmap(int point, bool isBlack) const;
	QPixmap getCardJudgeIconPixmap(const QString &judgeName) const;
	QPixmap getCardFramePixmap(const QString &frameType) const;
	QPixmap getCardAvatarPixmap(const QString &generalName) const;
	QString getGeneralPixmapPath(const QString &generalName, GeneralIconSize size) const;
	QPixmap getGeneralPixmap(const QString &generalName, GeneralIconSize size) const;
	QString getPlayerAudioEffectPath(const QString &eventName, bool isMale, int index = -1) const;

	// static consts
	static const char* S_SKIN_KEY_PHOTO;
	static const char* S_SKIN_KEY_COMMON;
	static const char* S_SKIN_KEY_ROOM;
	static const char* S_SKIN_KEY_PHOTO_MAINFRAME;
	static const char* S_SKIN_KEY_PHOTO_HANDCARDNUM;
	static const char* S_SKIN_KEY_PHOTO_FACETURNEDMASK;
	static const char* S_SKIN_KEY_PHOTO_BLANK_GENERAL;
	static const char* S_SKIN_KEY_PHOTO_CHAIN;
	static const char* S_SKIN_KEY_PHOTO_PHASE;
	static const char* S_SKIN_KEY_HAND_CARD_BACK;
	static const char* S_SKIN_KEY_HAND_CARD_SUIT;
	static const char* S_SKIN_KEY_JUDGE_CARD_ICON;
	static const char* S_SKIN_KEY_HAND_CARD_MAIN_PHOTO;
	static const char* S_SKIN_KEY_HAND_CARD_NUMBER_BLACK;
	static const char* S_SKIN_KEY_HAND_CARD_NUMBER_RED;
	static const char* S_SKIN_KEY_HAND_CARD_FRAME;
	static const char* S_SKIN_KEY_HAND_CARD_AVATAR;
	static const char* S_SKIN_KEY_PLAYER_GENERAL_ICON;

	static const char* S_SKIN_KEY_PLAYER_AUDIO_EFFECT;
	static const char* S_SKIN_KEY_SYSTEM_AUDIO_EFFECT;

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
	const QSanSkinScheme& getCurrentSkinScheme();
	bool switchSkin(QString skinName);
protected:
	QSanSkinFactory(const char* fileName);
	static QSanSkinFactory* _sm_singleton;
	QSanSkinScheme _sm_currentSkin;
	Json::Value _m_skinList;
	bool _m_isSkinSet;
};

#define G_ROOM_SKIN (QSanSkinFactory::getInstance().getCurrentSkinScheme().getRoomSkin())
#define G_ROOM_LAYOUT (G_ROOM_SKIN.getRoomLayout())
#define G_PHOTO_LAYOUT (G_ROOM_SKIN.getPhotoLayout())
#define G_COMMON_LAYOUT (G_ROOM_SKIN.getCommonLayout())

#endif