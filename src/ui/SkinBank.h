/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Rara
    *********************************************************************/

#ifndef _SKIN_BANK_H
#define _SKIN_BANK_H

#define QSAN_UI_LIBRARY_AVAILABLE

#include "card.h"
#include "qsanbutton.h"
#include "util.h"
#include "json.h"

#include <QString>
#include <QPixmap>
#include <QHash>
#include <QFont>
#include <QPen>
#include <QPainter>
#include <QGraphicsPixmapItem>
#include <QAbstractAnimation>

class QSanPixmapCache {
public:
    // Load pixmap from a file and map it to the given key.
    static QPixmap getPixmap(const QString &key, const QString &fileName);
    // Load pixmap from a existing key.
    static QPixmap getPixmap(const QString &key);
    static bool contains(const QString &key);
};

class IQSanComponentSkin { // interface class
public:
    class QSanSimpleTextFont {
    public:
        int *m_fontFace;
        QSize m_fontSize;
        int m_spacing;
        int m_weight;
        QColor m_color;
        bool m_vertical;
        QSanSimpleTextFont();
        bool tryParse(const QVariant &arg);
        void paintText(QPainter *painter, const QRect &pos, Qt::Alignment align, const QString &text) const;
        // this function's prototype is confusing. It will CLEAR ALL contents on the
        // QGraphicsPixmapItem passed in and then start drawing.
        void paintText(QGraphicsPixmapItem *item, const QRect &pos, Qt::Alignment align, const QString &text) const;

    protected:
        static QHash<QString, int *> _m_fontBank;
    };

    class QSanShadowTextFont : public QSanSimpleTextFont {
    public:
        int m_shadowRadius;
        double m_shadowDecadeFactor;
        QPoint m_shadowOffset;
        QColor m_shadowColor;
        bool tryParse(const QVariant &arg);
        void paintText(QPainter *painter, const QRect &pos, Qt::Alignment align, const QString &text) const;
        // this function's prototype is confusing. It will CLEAR ALL contents on the
        // QGraphicsPixmapItem passed in and then start drawing.
        void paintText(QGraphicsPixmapItem *item, const QRect &pos, Qt::Alignment align, const QString &text) const;
    };

    class AnchoredRect {
    public:
        QRect getTranslatedRect(const QRect &parentRect) const;
        QRect getTranslatedRect(const QRect &parentRect, const QSize &childSize) const;
        bool tryParse(const QVariant &value);

    protected:
        Qt::Alignment m_anchorChild;
        Qt::Alignment m_anchorParent;
        QPoint m_offset;
        QSize m_fixedSize;
        bool m_useFixedSize;
    };

    static const char *S_SKIN_KEY_DEFAULT;
    static const char *S_SKIN_KEY_DEFAULT_SECOND;
    bool load(const QString &layoutConfigFileName, const QString &imageConfigFileName,
        const QString &audioConfigFileName, const QString &animationConfigFileName);
    QPixmap getPixmap(const QString &key, const QString &arg = QString(), const QString &arg2 = QString()) const;
    QPixmap getPixmapFileName(const QString &key) const;
    QPixmap getPixmapFromFileName(const QString &fileName) const;
    QStringList getAudioFileNames(const QString &key) const;
    QString getRandomAudioFileName(const QString &key) const;
    bool isImageKeyDefined(const QString &key) const;
    QStringList getAnimationFileNames() const;

protected:
    virtual bool _loadLayoutConfig(const QVariant &config) = 0;
    virtual bool _loadImageConfig(const QVariant &config);
    virtual bool _loadAnimationConfig(const QVariant &config) = 0;
    QString _readConfig(const QVariant &dictionary, const QString &key,
        const QString &defaultValue = QString()) const;
    QString _readImageConfig(const QString &key, QRect &clipRegion, bool &clipping,
        QSize &newScale, bool &scaled,
        const QString &defaultValue = QString()) const;

    JsonObject _m_imageConfig;
    JsonObject _m_audioConfig;
    JsonObject _m_animationConfig;
    // image key -> image file name
    static QHash<QString, QString> S_IMAGE_KEY2FILE;
    static QHash<QString, QPixmap> S_IMAGE_KEY2PIXMAP;
    // image group key -> image keys
    static QHash<QString, QList<QString> > S_IMAGE_GROUP_KEYS;
    static QHash<QString, int> S_HERO_SKIN_INDEX;
};

class QSanRoomSkin : public IQSanComponentSkin {
public:
    struct RoomLayout {
        int m_scenePadding;
        int m_roleBoxHeight;
        int m_chatTextBoxHeight;
        int m_discardPileMinWidth;
        int m_discardPilePadding;
        double m_logBoxHeightPercentage;
        double m_chatBoxHeightPercentage;
        double m_infoPlaneWidthPercentage;
        double m_photoDashboardPadding;
        double m_photoRoomPadding;
        int m_photoHDistance;
        int m_photoVDistance;
        QSize m_minimumSceneSize;
        QSize m_maximumSceneSize;
        QSize m_minimumSceneSize10Player;
        QSize m_maximumSceneSize10Player;
    };

    struct PlayerCardContainerLayout {
        int m_normalHeight;
        QRect m_boundingRect;
        QRect m_focusFrameArea;
        QRect m_focusFrameArea2;
        QRect m_handCardArea;

        // equips
        QRect m_equipAreas[S_EQUIP_AREA_LENGTH];
        QRect m_equipImageArea;
        QRect m_equipSuitArea;
        QRect m_equipPointArea;
        QRect m_horseImageArea;
        QRect m_horseSuitArea;
        QRect m_horsePointArea;
        QSanShadowTextFont m_equipPointFont;

        // delayed trick area
        QRect m_delayedTrickFirstRegion;
        QPoint m_delayedTrickStep;

        AnchoredRect m_markTextArea;
        QPoint m_roleComboBoxPos;

        // photo area
        QRect m_avatarArea;
        int m_avatarSize;
        QRect m_secondaryAvatarArea;
        int m_smallAvatarSize;
        int m_primaryAvatarSize;
        QRect m_circleArea;
        int m_circleImageSize;
        QRect m_avatarNameArea;
        QRect m_secondaryAvatarNameArea;
        QSanShadowTextFont m_avatarNameFont;
        QSanShadowTextFont m_smallAvatarNameFont;
        QRect m_kingdomIconArea;
        QRect m_kingdomMaskArea;
        QRect m_kingdomMaskArea2;
        QSanShadowTextFont m_handCardFont;
        QRect m_screenNameArea;
        QSanShadowTextFont m_screenNameFont;
        QRect leftDisableShowLockArea;
        QRect rightDisableShowLockArea;

        // progress bar and other controls
        bool m_isProgressBarHorizontal;
        AnchoredRect m_progressBarArea;
        QSize m_magatamaSize;
        QRect m_magatamaImageArea;
        bool m_magatamasHorizontal;
        bool m_magatamasBgVisible;
        QPoint m_magatamasAnchor;
        Qt::Alignment m_magatamasAlign;

        AnchoredRect m_phaseArea;

        // private pile (e.g. 7 stars, buqu)
        QPoint m_privatePileStartPos;
        QPoint m_privatePileStep;
        QSize m_privatePileButtonSize;

        // various icons
        QRect m_actionedIconRegion;
        QRect m_saveMeIconRegion;
        QRect m_chainedIconRegion;
        QRect m_duanchangMaskRegion, m_duanchangMaskRegion2;
        QRect m_hiddenMarkRegion1, m_hiddenMarkRegion2;
        QRect m_headIconRegion, m_deputyIconRegion;
        AnchoredRect m_deathIconRegion;
        QRect m_votesIconRegion;
        QRect m_seatIconRegion;
        QColor m_drankMaskColor;
        QColor m_duanchangMaskColor;
        QColor m_deathEffectColor;
        QColor m_generalShadowColor;

        QRect m_extraSkillArea;
        QSanShadowTextFont m_extraSkillFont;
        QRect m_extraSkillTextArea;
    };

    struct PhotoLayout : public PlayerCardContainerLayout {
        int m_normalWidth;
        QRect m_mainFrameArea;
        QRect m_cardMoveRegion;
        QRect m_onlineStatusArea;
        QSanShadowTextFont m_onlineStatusFont;
        QColor m_onlineStatusBgColor;
        QRect m_skillNameArea;
        QSanShadowTextFont m_skillNameFont;
    };

    struct DashboardLayout : public PlayerCardContainerLayout {
        int m_leftWidth, m_rightWidth, m_magatamasBaseWidth;
        int m_floatingAreaHeight;
        int m_rswidth;
        QSize m_buttonSetSize;
        QRect m_confirmButtonArea;
        QRect m_cancelButtonArea;
        QRect m_discardButtonArea;
        QRect m_trustButtonArea;
        QSize m_skillButtonsSize[3];
        QRect m_skillTextArea[3];
        QRect m_skillTextAreaDown[3];
        QPoint m_equipBorderPos;
        QPoint m_equipSelectedOffset;
        int m_disperseWidth;
        QColor m_trustEffectColor;
        QSanShadowTextFont m_skillTextFonts[3];
        QColor m_skillTextColors[QSanButton::S_NUM_BUTTON_STATES * QSanInvokeSkillButton::S_NUM_SKILL_TYPES];
        QColor m_skillTextShadowColors[QSanButton::S_NUM_BUTTON_STATES * QSanInvokeSkillButton::S_NUM_SKILL_TYPES];

        QPoint m_changeHeadHeroSkinButtonPos;
        QPoint m_changeDeputyHeroSkinButtonPos;

        QSanShadowTextFont getSkillTextFont(QSanButton::ButtonState state,
            QSanInvokeSkillButton::SkillType type,
            QSanInvokeSkillButton::SkillButtonWidth width) const;
    };

    struct CommonLayout {
        // card related
        int m_cardNormalWidth;
        int m_cardNormalHeight;
        QRect m_cardMainArea;
        QRect m_cardSuitArea;
        QRect m_cardNumberArea;
        QRect m_cardTransferableIconArea;
        QRect m_cardFootnoteArea;
        QRect m_cardAvatarArea;
        QRect m_cardFrameArea;
        QSanShadowTextFont m_cardFootnoteFont;
        QSanShadowTextFont m_hpFont[6];
        int m_hpExtraSpaceHolder;

        // dialogs
        // when # of generals <= switchIconSizeThreadshold
        QSize m_chooseGeneralBoxSparseIconSize;
        // when # of generals > switchIconSizeThreadshold
        QSize m_chooseGeneralBoxDenseIconSize;
        int m_chooseGeneralBoxSwitchIconSizeThreshold;
        int m_chooseGeneralBoxSwitchIconEachRow;
        int m_chooseGeneralBoxSwitchIconEachRowForTooManyGenerals;
        int m_chooseGeneralBoxNoIconThreshold;

        // avatar size
        QSize m_tinyAvatarSize;

        //role combo box
        QSize m_roleNormalBgSize;
        QHash<QString, QRect> m_rolesRect;
        QHash<QString, QColor> m_rolesColor;
        QColor m_roleDarkColor;

        //Graphics Box
        QColor graphicsBoxBackgroundColor;
        QColor graphicsBoxBorderColor;
        QSanSimpleTextFont graphicsBoxTitleFont;

        //Choose General Box
        QSanSimpleTextFont m_chooseGeneralBoxDestSeatFont;

        //General Card Item
        QRect m_generalCardItemCompanionPromptRegion;

        //Option Button
        QSanShadowTextFont optionButtonText;

        //General Button
        QRect generalButtonPositionIconRegion;
        QRect generalButtonNameRegion;

        //Player Card Box
        QSanSimpleTextFont playerCardBoxPlaceNameText;

        //Skin Item
        QSanShadowTextFont skinItemTitleText;
    };

    enum GeneralIconSize {
        S_GENERAL_ICON_SIZE_TINY,
        S_GENERAL_ICON_SIZE_SMALL,
        S_GENERAL_ICON_SIZE_LARGE,
        S_GENERAL_ICON_SIZE_CARD,
        S_GENERAL_ICON_SIZE_PHOTO_SECONDARY,
        S_GENERAL_ICON_SIZE_DASHBOARD_SECONDARY,
        S_GENERAL_ICON_SIZE_PHOTO_PRIMARY,
        S_GENERAL_ICON_SIZE_DASHBOARD_PRIMARY,
        S_GENERAL_ICON_SIZE_KOF,
        S_GENERAL_ICON_SIZE_HERO_SKIN
    };

    const RoomLayout &getRoomLayout() const;
    const PhotoLayout &getPhotoLayout() const;
    const CommonLayout &getCommonLayout() const;
    const DashboardLayout &getDashboardLayout() const;

    QString getButtonPixmapPath(const QString &groupName, const QString &buttonName, QSanButton::ButtonState state) const;
    QPixmap getButtonPixmap(const QString &groupName, const QString &buttonName, QSanButton::ButtonState state, const bool &first_state = true) const;
    QPixmap getSkillButtonPixmap(QSanButton::ButtonState state,
        QSanInvokeSkillButton::SkillType type,
        QSanInvokeSkillButton::SkillButtonWidth width) const;
    QPixmap getCardMainPixmap(const QString &cardName) const;
    QPixmap getGeneralCardPixmap(const QString generalName, const int skinId = 0) const;
    QPixmap getCardSuitPixmap(Card::Suit suit) const;
    QPixmap getCardNumberPixmap(int point, bool isBlack) const;
    QPixmap getCardJudgeIconPixmap(const QString &judgeName) const;
    QPixmap getCardFramePixmap(const QString &frameType) const;
    QPixmap getCardAvatarPixmap(const QString &generalName) const;
    QPixmap getGeneralPixmap(const QString &generalName, GeneralIconSize size, const int skinId = 0) const;
    QString getPlayerAudioEffectPath(const QString &eventName, bool isMale, int index = -1) const;
    QString getPlayerAudioEffectPath(const QString &eventName, const QString &category, int index = -1, const Player *player = NULL) const;
    QPixmap getProgressBarPixmap(int percentile) const;

    bool doesGeneralHaveSkin(const QString &general, const int skinId = 1, const bool isCard = false) const;

    // static consts
    // main keys
    static const char *S_SKIN_KEY_DASHBOARD;
    static const char *S_SKIN_KEY_PHOTO;
    static const char *S_SKIN_KEY_COMMON;
    static const char *S_SKIN_KEY_ROOM;

    // role box
    static const char *S_SKIN_KEY_ROLE_BOX_RECT;
    static const char *S_SKIN_KEY_ROLE_BOX_COLOR;

    //bg
    static const char *S_SKIN_KEY_TABLE_BG;

    // button
    static const char *S_SKIN_KEY_BUTTON;
    static const char *S_SKIN_KEY_DASHBOARD_BUTTON_SET_BG;
    static const char *S_SKIN_KEY_BUTTON_DASHBOARD_CONFIRM;
    static const char *S_SKIN_KEY_BUTTON_DASHBOARD_CANCEL;
    static const char *S_SKIN_KEY_BUTTON_DASHBOARD_DISCARD;
    static const char *S_SKIN_KEY_BUTTON_DASHBOARD_TRUST;
    static const char *S_SKIN_KEY_PLATTER;
    static const char *S_SKIN_KEY_BUTTON_SKILL;

    // player container
    static const char *S_SKIN_KEY_MAINFRAME;
    static const char *S_SKIN_KEY_LEFTFRAME;
    static const char *S_SKIN_KEY_RIGHTFRAME;
    static const char *S_SKIN_KEY_RIGHTBASE;
    static const char *S_SKIN_KEY_MAGATAMAS_BASE;
    static const char *S_SKIN_KEY_AVATAR_FRAME;
    static const char *S_SKIN_KEY_MIDDLEFRAME;
    static const char *S_SKIN_KEY_HANDCARDNUM;
    static const char *S_SKIN_KEY_FACETURNEDMASK;
    static const char *S_SKIN_KEY_BLANK_GENERAL;
    static const char *S_SKIN_KEY_CHAIN;
    static const char *S_SKIN_KEY_DUANCHANG;
    static const char *S_SKIN_KEY_PHASE;
    static const char *S_SKIN_KEY_SELECTED_FRAME;
    static const char *S_SKIN_KEY_FOCUS_FRAME;
    static const char *S_SKIN_KEY_SAVE_ME_ICON;
    static const char *S_SKIN_KEY_ACTIONED_ICON;
    static const char *S_SKIN_KEY_KINGDOM_ICON;
    static const char *S_SKIN_KEY_KINGDOM_COLOR_MASK;
    static const char *S_SKIN_KEY_VOTES_NUMBER;
    static const char *S_SKIN_KEY_SEAT_NUMBER;
    static const char *S_SKIN_KEY_HAND_CARD_BACK;
    static const char *S_SKIN_KEY_HAND_CARD_SUIT;
    static const char *S_SKIN_KEY_JUDGE_CARD_ICON;
    static const char *S_SKIN_KEY_HAND_CARD_MAIN_PHOTO;
    static const char *S_SKIN_KEY_HAND_CARD_NUMBER_BLACK;
    static const char *S_SKIN_KEY_HAND_CARD_NUMBER_RED;
    static const char *S_SKIN_KEY_HAND_CARD_FRAME;
    static const char *S_SKIN_KEY_GENERAL_CARD;
    static const char *S_SKIN_KEY_PLAYER_GENERAL_ICON;
    static const char *S_SKIN_KEY_EXTRA_SKILL_BG;
    static const char *S_SKIN_KEY_MAGATAMAS_BG;
    static const char *S_SKIN_KEY_MAGATAMAS;
    static const char *S_SKIN_KEY_PLAYER_AUDIO_EFFECT;
    static const char *S_SKIN_KEY_SYSTEM_AUDIO_EFFECT;
    static const char *S_SKIN_KEY_EQUIP_ICON;
    static const char *S_SKIN_KEY_PROGRESS_BAR_IMAGE;
    static const char *S_SKIN_KEY_GENERAL_CIRCLE_IMAGE;
    static const char *S_SKIN_KEY_GENERAL_CIRCLE_MASK;
    static const char *S_SKIN_KEY_HIDDEN_MARK;
    static const char *S_SKIN_KEY_HEAD_ICON;
    static const char *S_SKIN_KEY_DEPUTY_ICON;
    static const char *S_SKIN_KEY_DISABLE_SHOW_LOCK;

    //CardContainer

    static const char *S_SKIN_KEY_CARD_CONTAINER_TOP;
    static const char *S_SKIN_KEY_CARD_CONTAINER_MIDDLE;
    static const char *S_SKIN_KEY_CARD_CONTAINER_BOTTOM;
    static const char *S_SKIN_KEY_CARD_CONTAINER_FRAME;

    // Animations
    static const char *S_SKIN_KEY_ANIMATIONS;

    // RoleComboBox
    static const char *S_SKIN_KEY_EXPANDING_ROLE_BOX;
    static const char *S_SKIN_KEY_ROLE_BOX_KINGDOM_MASK;

    //ChooseGeneralBox
    static const char *S_SKIN_KEY_CHOOSE_GENERAL_BOX_SPLIT_LINE;
    static const char *S_SKIN_KEY_CHOOSE_GENERAL_BOX_DEST_SEAT;

    //CardItem
    static const char *S_SKIN_KEY_CARD_TRANSFERABLE_ICON;

    //GeneralCardItem
    static const char *S_SKIN_KEY_GENERAL_CARD_ITEM_COMPANION_FONT;
    static const char *S_SKIN_KEY_GENERAL_CARD_ITEM_COMPANION_ICON;

protected:
    RoomLayout _m_roomLayout;
    PhotoLayout _m_photoLayout;
    CommonLayout _m_commonLayout;
    DashboardLayout _m_dashboardLayout;
    virtual bool _loadLayoutConfig(const QVariant &layout);
    virtual bool _loadAnimationConfig(const QVariant &animationConfig);
};

class QSanSkinScheme {
    // Why do we need another layer above room skin? Because we may add lobby, login interface
    // in the future; and we may need to assemble a set of different skins into a scheme.
public:
    bool load(const QVariant &configs);
    const QSanRoomSkin& getRoomSkin() const;

protected:
    QSanRoomSkin _m_roomSkin;
};

class QSanSkinFactory {
public:
    static QSanSkinFactory &getInstance();
    static void destroyInstance();
    const QString &getCurrentSkinName() const;
    const QSanSkinScheme &getCurrentSkinScheme();
    bool switchSkin(QString skinName);

    QString S_DEFAULT_SKIN_NAME;
    QString S_COMPACT_SKIN_NAME;

protected:
    QSanSkinFactory(const char *fileName);
    static QSanSkinFactory* _sm_singleton;
    QSanSkinScheme _sm_currentSkin;
    JsonObject _m_skinList;
    QString _m_skinName;
};

#define G_ROOM_SKIN (QSanSkinFactory::getInstance().getCurrentSkinScheme().getRoomSkin())
#define G_DASHBOARD_LAYOUT (G_ROOM_SKIN.getDashboardLayout())
#define G_ROOM_LAYOUT (G_ROOM_SKIN.getRoomLayout())
#define G_PHOTO_LAYOUT (G_ROOM_SKIN.getPhotoLayout())
#define G_COMMON_LAYOUT (G_ROOM_SKIN.getCommonLayout())

#endif

