#include "global.h"
#include "rtc.h"
#include "overworld.h"
#include "random.h"
#include "event_data.h"
#include "fieldmap.h"
#include "field_camera.h"
#include "field_specials.h"
#include "fldeff.h"
#include "strings.h"
#include "string_util.h"
#include "international_string_util.h"
#include "pokemon_storage_system.h"
#include "field_message_box.h"
#include "easy_chat.h"
#include "battle.h"
#include "battle_tower.h"
#include "contest.h"
#include "item.h"
#include "link.h"
#include "main.h"
#include "event_scripts.h"
#include "shop.h"
#include "lilycove_lady.h"
#include "pokedex.h"
#include "event_object_movement.h"
#include "text.h"
#include "script_menu.h"
#include "naming_screen.h"
#include "malloc.h"
#include "region_map.h"
#include "decoration.h"
#include "secret_base.h"
#include "tv.h"
#include "pokeball.h"
#include "data.h"
#include "constants/battle_frontier.h"
#include "constants/contest.h"
#include "constants/decorations.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/layouts.h"
#include "constants/lilycove_lady.h"
#include "constants/metatile_behaviors.h"
#include "constants/metatile_labels.h"
#include "constants/moves.h"
#include "constants/region_map_sections.h"
#include "constants/script_menu.h"

#define LAST_TVSHOW_IDX (TV_SHOWS_COUNT - 1)

#define rbernoulli(num, den) BernoulliTrial(0xFFFF * (num) / (den))

enum {
    TVGROUP_NONE,
    TVGROUP_UNUSED,
    TVGROUP_NORMAL,
    TVGROUP_RECORD_MIX,
    TVGROUP_OUTBREAK,
};

enum {
    SLOT_MACHINE,
    ROULETTE,
};

s8 sCurTVShowSlot;
u16 sTV_SecretBaseVisitMovesTemp[8];
u8 sTV_DecorationsBuffer[DECOR_MAX_SECRET_BASE];
struct {
    u8 level;
    u16 species;
    u16 move;
} sTV_SecretBaseVisitMonsTemp[10];

static u8 sTVShowMixingNumPlayers;
static u8 sTVShowNewsMixingNumPlayers;
static s8 sTVShowMixingCurSlot;

static EWRAM_DATA u16 sPokemonAnglerSpecies = 0;
static EWRAM_DATA u16 sPokemonAnglerAttemptCounters = 0;
static EWRAM_DATA u16 sFindThatGamerCoinsSpent = 0;
static EWRAM_DATA u8 sFindThatGamerWhichGame = SLOT_MACHINE;
static EWRAM_DATA ALIGNED(4) u8 sRecordMixingPartnersWithoutShowsToShare = 0;
static EWRAM_DATA ALIGNED(4) u8 sTVShowState = 0;
static EWRAM_DATA u8 sTVSecretBaseSecretsRandomValues[3] = {};

static void ClearPokeNews(void);
static u8 GetTVGroupByShowId(u8);
static u8 FindFirstActiveTVShowThatIsNotAMassOutbreak(void);
static void SetTVMetatilesOnMap(int, int, u16);
static u8 FindAnyPokeNewsOnTheAir(void);
static void TakeGabbyAndTyOffTheAir(void);
static bool8 BernoulliTrial(u16 ratio);
static s8 FindFirstEmptyRecordMixTVShowSlot(TVShow *);
static bool8 IsRecordMixShowAlreadySpawned(u8, bool8);
static void StorePlayerIdInRecordMixShow(TVShow *);
static void DeleteTVShowInArrayByIdx(TVShow *, u8);
static s8 FindFirstEmptyNormalTVShowSlot(TVShow *);
static void TryReplaceOldTVShowOfKind(u8);
static void InterviewBefore_BravoTrainerPkmnProfile(void);
static void InterviewBefore_NameRater(void);
static u16 GetRandomDifferentSpeciesSeenByPlayer(u16);
static void Script_FindFirstEmptyNormalTVShowSlot(void);
static void CompactTVShowArray(TVShow *);
static s8 GetFirstEmptyPokeNewsSlot(PokeNews *);
static bool8 IsAddingPokeNewsDisallowed(u8);
static void ClearPokeNewsBySlot(u8);
static void TranslateRubyShows(TVShow *);
static void TranslateJapaneseEmeraldShows(TVShow *);
static void SetMixedTVShows(TVShow *, TVShow *, TVShow *, TVShow *);
static void DeleteExcessMixedShows(void);
static void DeactivateShowsWithUnseenSpecies(void);
static void DeactivateGameCompleteShowsIfNotUnlocked(void);
static s8 FindInactiveShowInArray(TVShow *);
static bool8 TryMixTVShow(TVShow *[], TVShow *[], u8);
static bool8 TryMixNormalTVShow(TVShow *, TVShow *, u8);
static bool8 TryMixRecordMixTVShow(TVShow *, TVShow *, u8);
static bool8 TryMixOutbreakTVShow(TVShow *, TVShow *, u8);
static void DeactivateShow(u8 showIdx);
static void DeactivateShowIfNotSeenSpecies(u16, u8);
static void SetMixedPokeNews(PokeNews *, PokeNews *, PokeNews *, PokeNews *);
static void ClearInvalidPokeNews(void);
static void ClearPokeNewsIfGameNotComplete(void);
static s8 GetPokeNewsSlotIfActive(PokeNews *, u8);
static void InitTryMixPokeNewsShow(PokeNews *[], PokeNews *[]);
static bool8 TryMixPokeNewsShow(PokeNews *, PokeNews *, s8);
static void TVShowDone(void);
static void InterviewAfter_FanClubLetter(void);
static void InterviewAfter_RecentHappenings(void);
static void InterviewAfter_PkmnFanClubOpinions(void);
static void InterviewAfter_Dummy(void);
static void InterviewAfter_BravoTrainerPokemonProfile(void);
static void InterviewAfter_BravoTrainerBattleTowerProfile(void);
static void InterviewAfter_ContestLiveUpdates(void);
static void InitWorldOfMastersShowAttempt(void);
static void TryPutPokemonTodayFailedOnTheAir(void);
static void TryStartRandomMassOutbreak(void);
static void TryPutRandomPokeNewsOnAir(void);
static void SortPurchasesByQuantity(void);
static void UpdateMassOutbreakTimeLeft(u16);
static void TryEndMassOutbreak(u16);
static void UpdatePokeNewsCountdown(u16);
static void ResolveWorldOfMastersShow(u16);
static void ResolveNumberOneShow(u16);
static void TryPutFishingAdviceOnAir(void);
static u8 MonDataIdxToRibbon(u8);
static void TryPutNumberOneOnAir(u8);
static bool8 ShouldApplyPokeNewsEffect(u8);
static void TryPutWorldOfMastersOnAir(void);
static void InterviewBefore_FanClubLetter(void);
static void InterviewBefore_RecentHappenings(void);
static void InterviewBefore_PkmnFanClubOpinions(void);
static void InterviewBefore_Dummy(void);
static void InterviewBefore_BravoTrainerBTProfile(void);
static void InterviewBefore_ContestLiveUpdates(void);
static void InterviewBefore_3CheersForPokeblocks(void);
static void InterviewBefore_FanClubSpecial(void);
static void ChangeBoxPokemonNickname_CB(void);
static void DoTVShowPokemonFanClubLetter(void);
static void DoTVShowRecentHappenings(void);
static void DoTVShowPokemonFanClubOpinions(void);
static void DoTVShowDummiedOut(void);
static void DoTVShowPokemonNewsMassOutbreak(void);
static void DoTVShowBravoTrainerPokemonProfile(void);
static void DoTVShowBravoTrainerBattleTower(void);
static void DoTVShowPokemonTodaySuccessfulCapture(void);
static void DoTVShowTodaysSmartShopper(void);
static void DoTVShowTheNameRaterShow(void);
static void DoTVShowPokemonContestLiveUpdates(void);
static void DoTVShowPokemonBattleUpdate(void);
static void DoTVShow3CheersForPokeblocks(void);
static void DoTVShowPokemonTodayFailedCapture(void);
static void DoTVShowPokemonAngler(void);
static void DoTVShowTheWorldOfMasters(void);
static void DoTVShowTodaysRivalTrainer(void);
static void DoTVShowDewfordTrendWatcherNetwork(void);
static void DoTVShowHoennTreasureInvestigators(void);
static void DoTVShowFindThatGamer(void);
static void DoTVShowBreakingNewsTV(void);
static void DoTVShowSecretBaseVisit(void);
static void DoTVShowPokemonLotteryWinnerFlashReport(void);
static void DoTVShowThePokemonBattleSeminar(void);
static void DoTVShowTrainerFanClubSpecial(void);
static void DoTVShowTrainerFanClub(void);
static void DoTVShowSpotTheCuties(void);
static void DoTVShowPokemonNewsBattleFrontier(void);
static void DoTVShowWhatsNo1InHoennToday(void);
static void DoTVShowSecretBaseSecrets(void);
static void DoTVShowSafariFanClub(void);
static void DoTVShowLilycoveContestLady(void);

static const struct {
    u16 species;
    u16 moves[MAX_MON_MOVES];
    u8 level;
    u8 location;
} sPokeOutbreakSpeciesList[] = {
    {
        .species = SPECIES_SEEDOT,
        .moves = {MOVE_BIDE, MOVE_HARDEN, MOVE_LEECH_SEED},
        .level = 3,
        .location = MAP_NUM(ROUTE102)
    },
    {
        .species = SPECIES_NUZLEAF,
        .moves = {MOVE_HARDEN, MOVE_GROWTH, MOVE_NATURE_POWER, MOVE_LEECH_SEED},
        .level = 15,
        .location = MAP_NUM(ROUTE114),
    },
    {
        .species = SPECIES_SEEDOT,
        .moves = {MOVE_HARDEN, MOVE_GROWTH, MOVE_NATURE_POWER, MOVE_LEECH_SEED},
        .level = 13,
        .location = MAP_NUM(ROUTE117),
    },
    {
        .species = SPECIES_SEEDOT,
        .moves = {MOVE_GIGA_DRAIN, MOVE_FRUSTRATION, MOVE_SOLAR_BEAM, MOVE_LEECH_SEED},
        .level = 25,
        .location = MAP_NUM(ROUTE120),
    },
    {
        .species = SPECIES_SKITTY,
        .moves = {MOVE_GROWL, MOVE_TACKLE, MOVE_TAIL_WHIP, MOVE_ATTRACT},
        .level = 8,
        .location = MAP_NUM(ROUTE116),
    }
};

static const u16 sGoldSymbolFlags[NUM_FRONTIER_FACILITIES] = {
    [FRONTIER_FACILITY_TOWER]   = FLAG_SYS_TOWER_GOLD,
    [FRONTIER_FACILITY_DOME]    = FLAG_SYS_DOME_GOLD,
    [FRONTIER_FACILITY_PALACE]  = FLAG_SYS_PALACE_GOLD,
    [FRONTIER_FACILITY_ARENA]   = FLAG_SYS_ARENA_GOLD,
    [FRONTIER_FACILITY_FACTORY] = FLAG_SYS_FACTORY_GOLD,
    [FRONTIER_FACILITY_PIKE]    = FLAG_SYS_PIKE_GOLD,
    [FRONTIER_FACILITY_PYRAMID] = FLAG_SYS_PYRAMID_GOLD
};

static const u16 sSilverSymbolFlags[NUM_FRONTIER_FACILITIES] = {
    [FRONTIER_FACILITY_TOWER]   = FLAG_SYS_TOWER_SILVER,
    [FRONTIER_FACILITY_DOME]    = FLAG_SYS_DOME_SILVER,
    [FRONTIER_FACILITY_PALACE]  = FLAG_SYS_PALACE_SILVER,
    [FRONTIER_FACILITY_ARENA]   = FLAG_SYS_ARENA_SILVER,
    [FRONTIER_FACILITY_FACTORY] = FLAG_SYS_FACTORY_SILVER,
    [FRONTIER_FACILITY_PIKE]    = FLAG_SYS_PIKE_SILVER,
    [FRONTIER_FACILITY_PYRAMID] = FLAG_SYS_PYRAMID_SILVER
};

static const u16 sNumberOneVarsAndThresholds[][2] = {
    {VAR_DAILY_SLOTS, 100},
    {VAR_DAILY_ROULETTE,  50},
    {VAR_DAILY_WILDS, 100},
    {VAR_DAILY_BLENDER,  20},
    {VAR_DAILY_PLANTED_BERRIES,  20},
    {VAR_DAILY_PICKED_BERRIES,  20},
    {VAR_DAILY_BP,  30}
};

static const u8 *const sPokeNewsTextGroup_Upcoming[NUM_POKENEWS_TYPES + 1] = {
    [POKENEWS_NONE]        = NULL,
    [POKENEWS_SLATEPORT]   = gPokeNewsTextSlateport_Upcoming,
    [POKENEWS_GAME_CORNER] = gPokeNewsTextGameCorner_Upcoming,
    [POKENEWS_LILYCOVE]    = gPokeNewsTextLilycove_Upcoming,
    [POKENEWS_BLENDMASTER] = gPokeNewsTextBlendMaster_Upcoming
};

static const u8 *const sPokeNewsTextGroup_Ongoing[NUM_POKENEWS_TYPES + 1] = {
    [POKENEWS_NONE]        = NULL,
    [POKENEWS_SLATEPORT]   = gPokeNewsTextSlateport_Ongoing,
    [POKENEWS_GAME_CORNER] = gPokeNewsTextGameCorner_Ongoing,
    [POKENEWS_LILYCOVE]    = gPokeNewsTextLilycove_Ongoing,
    [POKENEWS_BLENDMASTER] = gPokeNewsTextBlendMaster_Ongoing
};

static const u8 *const sPokeNewsTextGroup_Ending[NUM_POKENEWS_TYPES + 1] = {
    [POKENEWS_NONE]        = NULL,
    [POKENEWS_SLATEPORT]   = gPokeNewsTextSlateport_Ending,
    [POKENEWS_GAME_CORNER] = gPokeNewsTextGameCorner_Ending,
    [POKENEWS_LILYCOVE]    = gPokeNewsTextLilycove_Ending,
    [POKENEWS_BLENDMASTER] = gPokeNewsTextBlendMaster_Ending
};

u8 *const gTVStringVarPtrs[] = {
    gStringVar1,
    gStringVar2,
    gStringVar3
};

static const u8 *const sTVFanClubTextGroup[] = {
    gTVFanClubText00,
    gTVFanClubText01,
    gTVFanClubText02,
    gTVFanClubText03,
    gTVFanClubText04,
    gTVFanClubText05,
    gTVFanClubText06,
    gTVFanClubText07
};

static const u8 *const sTVRecentHappeninssTextGroup[] = {
    gTVRecentHappeningsText00,
    gTVRecentHappeningsText01,
    gTVRecentHappeningsText02,
    gTVRecentHappeningsText03,
    gTVRecentHappeningsText04,
    gTVRecentHappeningsText05
};

static const u8 *const sTVFanClubOpinionsTextGroup[] = {
    gTVFanClubOpinionsText00,
    gTVFanClubOpinionsText01,
    gTVFanClubOpinionsText02,
    gTVFanClubOpinionsText03,
    gTVFanClubOpinionsText04
};

static const u8 *const sTVMassOutbreakTextGroup[] = {
    gTVMassOutbreakText00
};

static const u8 *const sTVPokemonTodaySuccessfulTextGroup[] = {
    gTVPokemonTodaySuccessfulText00,
    gTVPokemonTodaySuccessfulText01,
    gTVPokemonTodaySuccessfulText02,
    gTVPokemonTodaySuccessfulText03,
    gTVPokemonTodaySuccessfulText04,
    gTVPokemonTodaySuccessfulText05,
    gTVPokemonTodaySuccessfulText06,
    gTVPokemonTodaySuccessfulText07,
    gTVPokemonTodaySuccessfulText08,
    gTVPokemonTodaySuccessfulText09,
    gTVPokemonTodaySuccessfulText10,
    gTVPokemonTodaySuccessfulText11
};

static const u8 *const sTVTodaysSmartShopperTextGroup[] = {
    [SMARTSHOPPER_STATE_INTRO]          = SmartShopper_Text_Intro,
    [SMARTSHOPPER_STATE_CLERK_NORMAL]   = SmartShopper_Text_ClerkNormal,
    [SMARTSHOPPER_STATE_RAND_COMMENT_1] = SmartShopper_Text_RandomComment1,
    [SMARTSHOPPER_STATE_RAND_COMMENT_2] = SmartShopper_Text_RandomComment2,
    [SMARTSHOPPER_STATE_RAND_COMMENT_3] = SmartShopper_Text_RandomComment3,
    [SMARTSHOPPER_STATE_RAND_COMMENT_4] = SmartShopper_Text_RandomComment4,
    [SMARTSHOPPER_STATE_SECOND_ITEM]    = SmartShopper_Text_SecondItem,
    [SMARTSHOPPER_STATE_THIRD_ITEM]     = SmartShopper_Text_ThirdItem,
    [SMARTSHOPPER_STATE_DURING_SALE]    = SmartShopper_Text_DuringSale,
    [SMARTSHOPPER_STATE_OUTRO_NORMAL]   = SmartShopper_Text_OutroNormal,
    [SMARTSHOPPER_STATE_IS_VIP]         = SmartShopper_Text_IsVIP,
    [SMARTSHOPPER_STATE_CLERK_MAX]      = SmartShopper_Text_ClerkMax,
    [SMARTSHOPPER_STATE_OUTRO_MAX]      = SmartShopper_Text_OutroMax
};

static const u8 *const sTVBravoTrainerTextGroup[] = {
    gTVBravoTrainerText00,
    gTVBravoTrainerText01,
    gTVBravoTrainerText02,
    gTVBravoTrainerText03,
    gTVBravoTrainerText04,
    gTVBravoTrainerText05,
    gTVBravoTrainerText06,
    gTVBravoTrainerText07,
    gTVBravoTrainerText08
};

static const u8 *const sTV3CheersForPokeblocksTextGroup[] = {
    gTV3CheersForPokeblocksText00,
    gTV3CheersForPokeblocksText01,
    gTV3CheersForPokeblocksText02,
    gTV3CheersForPokeblocksText03,
    gTV3CheersForPokeblocksText04,
    gTV3CheersForPokeblocksText05
};

static const u8 *const sTVBravoTrainerBattleTowerTextGroup[] = {
    [BRAVOTOWER_STATE_INTRO]                = BravoTrainerBattleTower_Text_Intro,
    [BRAVOTOWER_STATE_NEW_RECORD]           = BravoTrainerBattleTower_Text_NewRecord,
    [BRAVOTOWER_STATE_LOST]                 = BravoTrainerBattleTower_Text_Lost,
    [BRAVOTOWER_STATE_WON]                  = BravoTrainerBattleTower_Text_Won,
    [BRAVOTOWER_STATE_LOST_FINAL]           = BravoTrainerBattleTower_Text_LostFinal,
    [BRAVOTOWER_STATE_SATISFIED]            = BravoTrainerBattleTower_Text_Satisfied,
    [BRAVOTOWER_STATE_UNSATISFIED]          = BravoTrainerBattleTower_Text_Unsatisfied,
    [BRAVOTOWER_STATE_RESPONSE]             = BravoTrainerBattleTower_Text_Response,
    [BRAVOTOWER_STATE_RESPONSE_SATISFIED]   = BravoTrainerBattleTower_Text_ResponseSatisfied,
    [BRAVOTOWER_STATE_RESPONSE_UNSATISFIED] = BravoTrainerBattleTower_Text_ResponseUnsatisfied,
    [BRAVOTOWER_STATE_OUTRO]                = BravoTrainerBattleTower_Text_Outro
};

static const u8 *const sTVContestLiveUpdatesTextGroup[] = {
    [CONTESTLIVE_STATE_INTRO]                 = ContestLiveUpdates_Text_Intro,
    [CONTESTLIVE_STATE_WON_BOTH_ROUNDS]       = ContestLiveUpdates_Text_WonBothRounds,
    [CONTESTLIVE_STATE_BETTER_ROUND2]         = ContestLiveUpdates_Text_BetterRound2,
    [CONTESTLIVE_STATE_EQUAL_ROUNDS]          = ContestLiveUpdates_Text_EqualRounds,
    [CONTESTLIVE_STATE_BETTER_ROUND1]         = ContestLiveUpdates_Text_BetterRound1,
    [CONTESTLIVE_STATE_GOT_NERVOUS]           = ContestLiveUpdates_Text_GotNervous,
    [CONTESTLIVE_STATE_STARTLED_OTHER]        = ContestLiveUpdates_Text_StartledFoes,
    [CONTESTLIVE_STATE_USED_COMBO]            = ContestLiveUpdates_Text_UsedCombo,
    [CONTESTLIVE_STATE_EXCITING_APPEAL]       = ContestLiveUpdates_Text_ExcitingAppeal,
    [CONTESTLIVE_STATE_COOL]                  = ContestLiveUpdates_Text_WasCool,
    [CONTESTLIVE_STATE_BEAUTIFUL]             = ContestLiveUpdates_Text_WasBeautiful,
    [CONTESTLIVE_STATE_CUTE]                  = ContestLiveUpdates_Text_WasCute,
    [CONTESTLIVE_STATE_SMART]                 = ContestLiveUpdates_Text_WasSmart,
    [CONTESTLIVE_STATE_TOUGH]                 = ContestLiveUpdates_Text_WasTough,
    [CONTESTLIVE_STATE_VERY_EXCITING_APPEAL]  = ContestLiveUpdates_Text_VeryExcitingAppeal,
    [CONTESTLIVE_STATE_VERY_COOL]             = ContestLiveUpdates_Text_VeryCool,
    [CONTESTLIVE_STATE_VERY_BEAUTIFUL]        = ContestLiveUpdates_Text_VeryBeautiful,
    [CONTESTLIVE_STATE_VERY_CUTE]             = ContestLiveUpdates_Text_VeryCute,
    [CONTESTLIVE_STATE_VERY_SMART]            = ContestLiveUpdates_Text_VerySmart,
    [CONTESTLIVE_STATE_VERY_TOUGH]            = ContestLiveUpdates_Text_VeryTough,
    [CONTESTLIVE_STATE_TOOK_BREAK]            = ContestLiveUpdates_Text_TookBreak,
    [CONTESTLIVE_STATE_GOT_STARTLED]          = ContestLiveUpdates_Text_GotStartled,
    [CONTESTLIVE_STATE_USED_MOVE]             = ContestLiveUpdates_Text_MoveWonderful,
    [CONTESTLIVE_STATE_TALK_ABOUT_LOSER]      = ContestLiveUpdates_Text_TalkAboutAnotherMon,
    [CONTESTLIVE_STATE_NO_APPEALS]            = ContestLiveUpdates_Text_FailedToAppeal,
    [CONTESTLIVE_STATE_LAST_BOTH]             = ContestLiveUpdates_Text_LastInBothRounds,
    [CONTESTLIVE_STATE_NOT_EXCITING_ENOUGH]   = ContestLiveUpdates_Text_NotExcitingEnough,
    [CONTESTLIVE_STATE_LOST_AFTER_ROUND1_WIN] = ContestLiveUpdates_Text_LostAfterWinningRound1,
    [CONTESTLIVE_STATE_NO_EXCITING_APPEALS]   = ContestLiveUpdates_Text_NeverExciting,
    [CONTESTLIVE_STATE_LOST_SMALL_MARGIN]     = ContestLiveUpdates_Text_LostBySmallMargin,
    [CONTESTLIVE_STATE_REPEATED_APPEALS]      = ContestLiveUpdates_Text_RepeatedAppeals,
    [CONTESTLIVE_STATE_LOST]                  = ContestLiveUpdates_Text_ValiantEffortButLost,
    [CONTESTLIVE_STATE_OUTRO]                 = ContestLiveUpdates_Text_Outro
};

static const u8 *const sTVPokemonBattleUpdateTextGroup[] = {
    gTVPokemonBattleUpdateText00,
    gTVPokemonBattleUpdateText01,
    gTVPokemonBattleUpdateText02,
    gTVPokemonBattleUpdateText03,
    gTVPokemonBattleUpdateText04,
    gTVPokemonBattleUpdateText05,
    gTVPokemonBattleUpdateText06,
    gTVPokemonBattleUpdateText07
};

static const u8 *const sTVTrainerFanClubSpecialTextGroup[] = {
    gTVTrainerFanClubSpecialText00,
    gTVTrainerFanClubSpecialText01,
    gTVTrainerFanClubSpecialText02,
    gTVTrainerFanClubSpecialText03,
    gTVTrainerFanClubSpecialText04,
    gTVTrainerFanClubSpecialText05
};

static const u8 *const sTVNameRaterTextGroup[] = {
    gTVNameRaterText00,
    gTVNameRaterText01,
    gTVNameRaterText02,
    gTVNameRaterText03,
    gTVNameRaterText04,
    gTVNameRaterText05,
    gTVNameRaterText06,
    gTVNameRaterText07,
    gTVNameRaterText08,
    gTVNameRaterText09,
    gTVNameRaterText10,
    gTVNameRaterText11,
    gTVNameRaterText12,
    gTVNameRaterText13,
    gTVNameRaterText14,
    gTVNameRaterText15,
    gTVNameRaterText16,
    gTVNameRaterText17,
    gTVNameRaterText18
};

static const u8 *const sTVLilycoveContestLadyTextGroup[] = {
    [CONTESTLADYLIVE_STATE_INTRO]      = ContestLadyShow_Text_Intro,
    [CONTESTLADYLIVE_STATE_WON]        = ContestLadyShow_Text_Won,
    [CONTESTLADYLIVE_STATE_LOST]       = ContestLadyShow_Text_Lost,
    [CONTESTLADYLIVE_STATE_LOST_BADLY] = ContestLadyShow_Text_LostBadly
};

static const u8 *const sTVPokemonTodayFailedTextGroup[] = {
    gTVPokemonTodayFailedText00,
    gTVPokemonTodayFailedText01,
    gTVPokemonTodayFailedText02,
    gTVPokemonTodayFailedText03,
    gTVPokemonTodayFailedText04,
    gTVPokemonTodayFailedText05,
    gTVPokemonTodayFailedText06
};

static const u8 *const sTVPokemonAnslerTextGroup[] = {
    gTVPokemonAnglerText00,
    gTVPokemonAnglerText01
};

static const u8 *const sTVWorldOfMastersTextGroup[] = {
    gTVWorldOfMastersText00,
    gTVWorldOfMastersText01,
    gTVWorldOfMastersText02
};

static const u8 *const sTVTodaysRivalTrainerTextGroup[] = {
    gTVTodaysRivalTrainerText00,
    gTVTodaysRivalTrainerText01,
    gTVTodaysRivalTrainerText02,
    gTVTodaysRivalTrainerText03,
    gTVTodaysRivalTrainerText04,
    gTVTodaysRivalTrainerText05,
    gTVTodaysRivalTrainerText06,
    gTVTodaysRivalTrainerText07,
    gTVTodaysRivalTrainerText08,
    gTVTodaysRivalTrainerText09,
    gTVTodaysRivalTrainerText10
};

static const u8 *const sTVDewfordTrendWatcherNetworkTextGroup[] = {
    [TRENDWATCHER_STATE_INTRO]           = TrendWatcher_Text_Intro,
    [TRENDWATCHER_STATE_TAUGHT_MALE]     = TrendWatcher_Text_MaleTaughtMePhrase,
    [TRENDWATCHER_STATE_TAUGHT_FEMALE]   = TrendWatcher_Text_FemaleTaughtMePhrase,
    [TRENDWATCHER_STATE_PHRASE_HOPELESS] = TrendWatcher_Text_PhraseWasHopeless,
    [TRENDWATCHER_STATE_BIGGER_MALE]     = TrendWatcher_Text_MaleTellMeBigger,
    [TRENDWATCHER_STATE_BIGGER_FEMALE]   = TrendWatcher_Text_FemaleTellMeBigger,
    [TRENDWATCHER_STATE_OUTRO]           = TrendWatcher_Text_Outro
};

static const u8 *const sTVHoennTreasureInvestisatorsTextGroup[] = {
    gTVHoennTreasureInvestigatorsText00,
    gTVHoennTreasureInvestigatorsText01,
    gTVHoennTreasureInvestigatorsText02
};

static const u8 *const sTVFindThatGamerTextGroup[] = {
    gTVFindThatGamerText00,
    gTVFindThatGamerText01,
    gTVFindThatGamerText02,
    gTVFindThatGamerText03
};

static const u8 *const sTVBreakingNewsTextGroup[] = {
    gTVBreakingNewsText00,
    gTVBreakingNewsText01,
    gTVBreakingNewsText02,
    gTVBreakingNewsText03,
    gTVBreakingNewsText04,
    gTVBreakingNewsText05,
    gTVBreakingNewsText06,
    gTVBreakingNewsText07,
    gTVBreakingNewsText08,
    gTVBreakingNewsText09,
    gTVBreakingNewsText10,
    gTVBreakingNewsText11,
    gTVBreakingNewsText12
};

static const u8 *const sTVSecretBaseVisitTextGroup[] = {
    gTVSecretBaseVisitText00,
    gTVSecretBaseVisitText01,
    gTVSecretBaseVisitText02,
    gTVSecretBaseVisitText03,
    gTVSecretBaseVisitText04,
    gTVSecretBaseVisitText05,
    gTVSecretBaseVisitText06,
    gTVSecretBaseVisitText07,
    gTVSecretBaseVisitText08,
    gTVSecretBaseVisitText09,
    gTVSecretBaseVisitText10,
    gTVSecretBaseVisitText11,
    gTVSecretBaseVisitText12,
    gTVSecretBaseVisitText13
};

static const u8 *const sTVPokemonLotteryWinnerFlashReportTextGroup[] = {
    gTVPokemonLotteryWinnerFlashReportText00
};

static const u8 *const sTVThePokemonBattleSeminarTextGroup[] = {
    gTVThePokemonBattleSeminarText00,
    gTVThePokemonBattleSeminarText01,
    gTVThePokemonBattleSeminarText02,
    gTVThePokemonBattleSeminarText03,
    gTVThePokemonBattleSeminarText04,
    gTVThePokemonBattleSeminarText05,
    gTVThePokemonBattleSeminarText06
};

static const u8 *const sTVTrainerFanClubTextGroup[] = {
    gTVTrainerFanClubText00,
    gTVTrainerFanClubText01,
    gTVTrainerFanClubText02,
    gTVTrainerFanClubText03,
    gTVTrainerFanClubText04,
    gTVTrainerFanClubText05,
    gTVTrainerFanClubText06,
    gTVTrainerFanClubText07,
    gTVTrainerFanClubText08,
    gTVTrainerFanClubText09,
    gTVTrainerFanClubText10,
    gTVTrainerFanClubText11
};

static const u8 *const sTVCutiesTextGroup[] = {
    [SPOTCUTIES_STATE_INTRO]           = TVSpotTheCuties_Text_Intro,
    [SPOTCUTIES_STATE_RIBBONS_LOW]     = TVSpotTheCuties_Text_RibbonsLow,
    [SPOTCUTIES_STATE_RIBBONS_MID]     = TVSpotTheCuties_Text_RibbonsMid,
    [SPOTCUTIES_STATE_RIBBONS_HIGH]    = TVSpotTheCuties_Text_RibbonsHigh,
    [SPOTCUTIES_STATE_RIBBON_INTRO]    = TVSpotTheCuties_Text_RibbonIntro,
    [SPOTCUTIES_STATE_RIBBON_CHAMPION] = TVSpotTheCuties_Text_RibbonChampion,
    [SPOTCUTIES_STATE_RIBBON_COOL]     = TVSpotTheCuties_Text_RibbonCool,
    [SPOTCUTIES_STATE_RIBBON_BEAUTY]   = TVSpotTheCuties_Text_RibbonBeauty,
    [SPOTCUTIES_STATE_RIBBON_CUTE]     = TVSpotTheCuties_Text_RibbonCute,
    [SPOTCUTIES_STATE_RIBBON_SMART]    = TVSpotTheCuties_Text_RibbonSmart,
    [SPOTCUTIES_STATE_RIBBON_TOUGH]    = TVSpotTheCuties_Text_RibbonTough,
    [SPOTCUTIES_STATE_RIBBON_WINNING]  = TVSpotTheCuties_Text_RibbonWinning,
    [SPOTCUTIES_STATE_RIBBON_VICTORY]  = TVSpotTheCuties_Text_RibbonVictory,
    [SPOTCUTIES_STATE_RIBBON_ARTIST]   = TVSpotTheCuties_Text_RibbonArtist,
    [SPOTCUTIES_STATE_RIBBON_EFFORT]   = TVSpotTheCuties_Text_RibbonEffort,
    [SPOTCUTIES_STATE_OUTRO]           = TVSpotTheCuties_Text_Outro
};

static const u8 *const sTVPokemonNewsBattleFrontierTextGroup[] = {
    gTVPokemonNewsBattleFrontierText00,
    gTVPokemonNewsBattleFrontierText01,
    gTVPokemonNewsBattleFrontierText02,
    gTVPokemonNewsBattleFrontierText03,
    gTVPokemonNewsBattleFrontierText04,
    gTVPokemonNewsBattleFrontierText05,
    gTVPokemonNewsBattleFrontierText06,
    gTVPokemonNewsBattleFrontierText07,
    gTVPokemonNewsBattleFrontierText08,
    gTVPokemonNewsBattleFrontierText09,
    gTVPokemonNewsBattleFrontierText10,
    gTVPokemonNewsBattleFrontierText11,
    gTVPokemonNewsBattleFrontierText12,
    gTVPokemonNewsBattleFrontierText13,
    gTVPokemonNewsBattleFrontierText14,
    gTVPokemonNewsBattleFrontierText15,
    gTVPokemonNewsBattleFrontierText16,
    gTVPokemonNewsBattleFrontierText17,
    gTVPokemonNewsBattleFrontierText18
};

static const u8 *const sTVWhatsNo1InHoennTodayTextGroup[] = {
    gTVWhatsNo1InHoennTodayText00,
    gTVWhatsNo1InHoennTodayText01,
    gTVWhatsNo1InHoennTodayText02,
    gTVWhatsNo1InHoennTodayText03,
    gTVWhatsNo1InHoennTodayText04,
    gTVWhatsNo1InHoennTodayText05,
    gTVWhatsNo1InHoennTodayText06,
    gTVWhatsNo1InHoennTodayText07,
    gTVWhatsNo1InHoennTodayText08
};

static const u8 *const sTVSecretBaseSecretsTextGroup[SBSECRETS_NUM_STATES] =
{
    [SBSECRETS_STATE_INTRO]               = TVSecretBaseSecrets_Text_Intro,
    [SBSECRETS_STATE_DO_NEXT1]            = TVSecretBaseSecrets_Text_WhatWillPlayerDoNext1,
    [SBSECRETS_STATE_DO_NEXT2]            = TVSecretBaseSecrets_Text_WhatWillPlayerDoNext2,
    [SBSECRETS_STATE_TOOK_X_STEPS]        = TVSecretBaseSecrets_Text_TookXStepsBeforeLeaving,
    [SBSECRETS_STATE_BASE_INTEREST_LOW]   = TVSecretBaseSecrets_Text_BaseFailedToInterestPlayer,
    [SBSECRETS_STATE_BASE_INTEREST_MED]   = TVSecretBaseSecrets_Text_PlayerEnjoyedBase,
    [SBSECRETS_STATE_BASE_INTEREST_HIGH]  = TVSecretBaseSecrets_Text_PlayerHugeFanOfBase,
    [SBSECRETS_STATE_OUTRO]               = TVSecretBaseSecrets_Text_Outro,
    [SBSECRETS_STATE_NOTHING_USED1]       = TVSecretBaseSecrets_Text_StoppedMoving1,
    [SBSECRETS_STATE_NOTHING_USED2]       = TVSecretBaseSecrets_Text_StoppedMoving2,
    [SBSECRETS_STATE_USED_CHAIR]          = TVSecretBaseSecrets_Text_UsedChair,
    [SBSECRETS_STATE_USED_BALLOON]        = TVSecretBaseSecrets_Text_UsedBalloon,
    [SBSECRETS_STATE_USED_TENT]           = TVSecretBaseSecrets_Text_UsedTent,
    [SBSECRETS_STATE_USED_PLANT]          = TVSecretBaseSecrets_Text_UsedPlant,
    [SBSECRETS_STATE_USED_GOLD_SHIELD]    = TVSecretBaseSecrets_Text_UsedGoldShield,
    [SBSECRETS_STATE_USED_SILVER_SHIELD]  = TVSecretBaseSecrets_Text_UsedSilverShield,
    [SBSECRETS_STATE_USED_GLASS_ORNAMENT] = TVSecretBaseSecrets_Text_UsedGlassOrnament,
    [SBSECRETS_STATE_USED_TV]             = TVSecretBaseSecrets_Text_UsedTV,
    [SBSECRETS_STATE_USED_MUD_BALL]       = TVSecretBaseSecrets_Text_UsedMudBall,
    [SBSECRETS_STATE_USED_BAG]            = TVSecretBaseSecrets_Text_UsedBag,
    [SBSECRETS_STATE_USED_CUSHION]        = TVSecretBaseSecrets_Text_UsedCushion,
    [SBSECRETS_STATE_HIT_CUSHION]         = TVSecretBaseSecrets_Text_HitCushion,
    [SBSECRETS_STATE_HUGGED_CUSHION]      = TVSecretBaseSecrets_Text_HuggedCushion,
    [SBSECRETS_STATE_BATTLED_WON]         = TVSecretBaseSecrets_Text_BattledWon,
    [SBSECRETS_STATE_BATTLED_LOST]        = TVSecretBaseSecrets_Text_BattledLost,
    [SBSECRETS_STATE_DECLINED_BATTLE]     = TVSecretBaseSecrets_Text_DeclinedBattle,
    [SBSECRETS_STATE_USED_POSTER]         = TVSecretBaseSecrets_Text_UsedPoster,
    [SBSECRETS_STATE_USED_NOTE_MAT]       = TVSecretBaseSecrets_Text_UsedNoteMat,
    [SBSECRETS_STATE_BATTLED_DRAW]        = TVSecretBaseSecrets_Text_BattledDraw,
    [SBSECRETS_STATE_USED_SPIN_MAT]       = TVSecretBaseSecrets_Text_UsedSpinMat,
    [SBSECRETS_STATE_USED_SAND_ORNAMENT]  = TVSecretBaseSecrets_Text_UsedSandOrnament,
    [SBSECRETS_STATE_USED_DESK]           = TVSecretBaseSecrets_Text_UsedDesk,
    [SBSECRETS_STATE_USED_BRICK]          = TVSecretBaseSecrets_Text_UsedBrick,
    [SBSECRETS_STATE_USED_SOLID_BOARD]    = TVSecretBaseSecrets_Text_UsedSolidBoard,
    [SBSECRETS_STATE_USED_FENCE]          = TVSecretBaseSecrets_Text_UsedFence,
    [SBSECRETS_STATE_USED_GLITTER_MAT]    = TVSecretBaseSecrets_Text_UsedGlitterMat,
    [SBSECRETS_STATE_USED_TIRE]           = TVSecretBaseSecrets_Text_UsedTire,
    [SBSECRETS_STATE_USED_STAND]          = TVSecretBaseSecrets_Text_UsedStand,
    [SBSECRETS_STATE_USED_BREAKABLE_DOOR] = TVSecretBaseSecrets_Text_BrokeDoor,
    [SBSECRETS_STATE_USED_DOLL]           = TVSecretBaseSecrets_Text_UsedDoll,
    [SBSECRETS_STATE_USED_SLIDE]          = TVSecretBaseSecrets_Text_UsedSlide,
    [SBSECRETS_STATE_DECLINED_SLIDE]      = TVSecretBaseSecrets_Text_UsedSlideButDidntGoDown,
    [SBSECRETS_STATE_USED_JUMP_MAT]       = TVSecretBaseSecrets_Text_UsedJumpMat
};

static const u8 *const sTVSafariFanClubTextGroup[] = {
    gTVSafariFanClubText00,
    gTVSafariFanClubText01,
    gTVSafariFanClubText02,
    gTVSafariFanClubText03,
    gTVSafariFanClubText04,
    gTVSafariFanClubText05,
    gTVSafariFanClubText06,
    gTVSafariFanClubText07,
    gTVSafariFanClubText08,
    gTVSafariFanClubText09,
    gTVSafariFanClubText10
};

static const u8 *const sTVInSearchOfTrainersTextGroup[] = {
    gTVInSearchOfTrainersText00,
    gTVInSearchOfTrainersText01,
    gTVInSearchOfTrainersText02,
    gTVInSearchOfTrainersText03,
    gTVInSearchOfTrainersText04,
    gTVInSearchOfTrainersText05,
    gTVInSearchOfTrainersText06,
    gTVInSearchOfTrainersText07,
    gTVInSearchOfTrainersText08
};

// Secret Base Secrets TV Show states for actions that can be taken in a secret base
// The flags that determine whether or not the action was taken are commented
static const u8 sTVSecretBaseSecretsActions[NUM_SECRET_BASE_FLAGS] =
{
    SBSECRETS_STATE_USED_CHAIR,             // SECRET_BASE_USED_CHAIR
    SBSECRETS_STATE_USED_BALLOON,           // SECRET_BASE_USED_BALLOON
    SBSECRETS_STATE_USED_TENT,              // SECRET_BASE_USED_TENT
    SBSECRETS_STATE_USED_PLANT,             // SECRET_BASE_USED_PLANT
    SBSECRETS_STATE_USED_GOLD_SHIELD,       // SECRET_BASE_USED_GOLD_SHIELD
    SBSECRETS_STATE_USED_SILVER_SHIELD,     // SECRET_BASE_USED_SILVER_SHIELD
    SBSECRETS_STATE_USED_GLASS_ORNAMENT,    // SECRET_BASE_USED_GLASS_ORNAMENT
    SBSECRETS_STATE_USED_TV,                // SECRET_BASE_USED_TV
    SBSECRETS_STATE_USED_MUD_BALL,          // SECRET_BASE_USED_MUD_BALL
    SBSECRETS_STATE_USED_BAG,               // SECRET_BASE_USED_BAG
    SBSECRETS_STATE_USED_CUSHION,           // SECRET_BASE_USED_CUSHION
    SBSECRETS_STATE_BATTLED_WON,            // SECRET_BASE_BATTLED_WON
    SBSECRETS_STATE_BATTLED_LOST,           // SECRET_BASE_BATTLED_LOST
    SBSECRETS_STATE_DECLINED_BATTLE,        // SECRET_BASE_DECLINED_BATTLE
    SBSECRETS_STATE_USED_POSTER,            // SECRET_BASE_USED_POSTER
    SBSECRETS_STATE_USED_NOTE_MAT,          // SECRET_BASE_USED_NOTE_MAT
    SBSECRETS_STATE_BATTLED_DRAW,           // SECRET_BASE_BATTLED_DRAW
    SBSECRETS_STATE_USED_SPIN_MAT,          // SECRET_BASE_USED_SPIN_MAT
    SBSECRETS_STATE_USED_SAND_ORNAMENT,     // SECRET_BASE_USED_SAND_ORNAMENT
    SBSECRETS_STATE_USED_DESK,              // SECRET_BASE_USED_DESK
    SBSECRETS_STATE_USED_BRICK,             // SECRET_BASE_USED_BRICK
    SBSECRETS_STATE_USED_SOLID_BOARD,       // SECRET_BASE_USED_SOLID_BOARD
    SBSECRETS_STATE_USED_FENCE,             // SECRET_BASE_USED_FENCE
    SBSECRETS_STATE_USED_GLITTER_MAT,       // SECRET_BASE_USED_GLITTER_MAT
    SBSECRETS_STATE_USED_TIRE,              // SECRET_BASE_USED_TIRE
    SBSECRETS_STATE_USED_STAND,             // SECRET_BASE_USED_STAND
    SBSECRETS_STATE_USED_BREAKABLE_DOOR,    // SECRET_BASE_USED_BREAKABLE_DOOR
    SBSECRETS_STATE_USED_DOLL,              // SECRET_BASE_USED_DOLL
    SBSECRETS_STATE_USED_SLIDE,             // SECRET_BASE_USED_SLIDE
    SBSECRETS_STATE_DECLINED_SLIDE,         // SECRET_BASE_DECLINED_SLIDE
    SBSECRETS_STATE_USED_JUMP_MAT,          // SECRET_BASE_USED_JUMP_MAT
    SBSECRETS_NUM_STATES                    // SECRET_BASE_UNUSED_FLAG. Odd that this is included, if it were used it would overflow sTVSecretBaseSecretsTextGroup
};

void ClearTVShowData(void)
{
}

u8 GetRandomActiveShowIdx(void)
{
}

u8 FindAnyTVShowOnTheAir(void)
{
}

void UpdateTVScreensOnMap(int width, int height)
{
}

static void SetTVMetatilesOnMap(int width, int height, u16 metatileId)
{
    int x;
    int y;

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            if (MapGridGetMetatileBehaviorAt(x, y) == MB_TELEVISION)
                MapGridSetMetatileIdAt(x, y, metatileId | MAPGRID_COLLISION_MASK);
        }
    }
}

void TurnOffTVScreen(void)
{
    SetTVMetatilesOnMap(gBackupMapLayout.width, gBackupMapLayout.height, METATILE_Building_TV_Off);
    DrawWholeMapView();
}

void TurnOnTVScreen(void)
{
    SetTVMetatilesOnMap(gBackupMapLayout.width, gBackupMapLayout.height, METATILE_Building_TV_On);
    DrawWholeMapView();
}

// gSpecialVar_0x8004 here is set from GetRandomActiveShowIdx in EventScript_TryDoTVShow
u8 GetSelectedTVShow(void)
{
}

static u8 FindFirstActiveTVShowThatIsNotAMassOutbreak(void)
{
}

u8 GetNextActiveShowIfMassOutbreak(void)
{
}

// IN SEARCH OF TRAINERS

void ResetGabbyAndTy(void)
{
}

void GabbyAndTyBeforeInterview(void)
{
}

void GabbyAndTyAfterInterview(void)
{
}

static void TakeGabbyAndTyOffTheAir(void)
{
}

u8 GabbyAndTyGetBattleNum(void)
{
}

bool8 IsGabbyAndTyShowOnTheAir(void)
{
}

bool8 GabbyAndTyGetLastQuote(void)
{
}

u8 GabbyAndTyGetLastBattleTrivia(void)
{
}

void GetGabbyAndTyLocalIds(void)
{
    switch (GabbyAndTyGetBattleNum())
    {
    case 1:
        gSpecialVar_0x8004 = 14;
        gSpecialVar_0x8005 = 13;
        break;
    case 2:
        gSpecialVar_0x8004 = 5;
        gSpecialVar_0x8005 = 6;
        break;
    case 3:
        gSpecialVar_0x8004 = 18;
        gSpecialVar_0x8005 = 17;
        break;
    case 4:
        gSpecialVar_0x8004 = 21;
        gSpecialVar_0x8005 = 22;
        break;
    case 5:
        gSpecialVar_0x8004 = 8;
        gSpecialVar_0x8005 = 9;
        break;
    case 6:
        gSpecialVar_0x8004 = 19;
        gSpecialVar_0x8005 = 20;
        break;
    case 7:
        gSpecialVar_0x8004 = 23;
        gSpecialVar_0x8005 = 24;
        break;
    case 8:
        gSpecialVar_0x8004 = 10;
        gSpecialVar_0x8005 = 11;
        break;
    }
}

void InterviewAfter(void)
{
    switch (gSpecialVar_0x8005)
    {
    case TVSHOW_FAN_CLUB_LETTER:
        InterviewAfter_FanClubLetter();
        break;
    case TVSHOW_RECENT_HAPPENINGS:
        InterviewAfter_RecentHappenings();
        break;
    case TVSHOW_PKMN_FAN_CLUB_OPINIONS:
        InterviewAfter_PkmnFanClubOpinions();
        break;
    case TVSHOW_DUMMY:
        InterviewAfter_Dummy();
        break;
    case TVSHOW_BRAVO_TRAINER_POKEMON_PROFILE:
        InterviewAfter_BravoTrainerPokemonProfile();
        break;
    case TVSHOW_BRAVO_TRAINER_BATTLE_TOWER_PROFILE:
        InterviewAfter_BravoTrainerBattleTowerProfile();
        break;
    case TVSHOW_CONTEST_LIVE_UPDATES:
        InterviewAfter_ContestLiveUpdates();
        break;
    }
}

void TryPutPokemonTodayOnAir(void)
{
}

// Show is initialized in last slot and updated there until it's
// either triggered or deleted at the end of the day by ResolveWorldOfMastersShow
static void InitWorldOfMastersShowAttempt(void)
{
}

static void TryPutPokemonTodayFailedOnTheAir(void)
{
}

static void StorePlayerIdInRecordMixShow(TVShow *show)
{
    u32 id = GetPlayerIDAsU32();
    show->common.srcTrainerId2Lo = id;
    show->common.srcTrainerId2Hi = id >> 8;
    show->common.srcTrainerIdLo = id;
    show->common.srcTrainerIdHi = id >> 8;
    show->common.trainerIdLo = id;
    show->common.trainerIdHi = id >> 8;
}

static void StorePlayerIdInNormalShow(TVShow *show)
{
    u32 id = GetPlayerIDAsU32();
    show->common.srcTrainerIdLo = id;
    show->common.srcTrainerIdHi = id >> 8;
    show->common.trainerIdLo = id;
    show->common.trainerIdHi = id >> 8;
}

static void InterviewAfter_ContestLiveUpdates(void)
{
}

void PutBattleUpdateOnTheAir(u8 opponentLinkPlayerId, u16 move, u16 speciesPlayer, u16 speciesOpponent)
{
}

bool8 Put3CheersForPokeblocksOnTheAir(const u8 *partnersName, u8 flavor, u8 color, u8 sheen, u8 language)
{
}

void PutFanClubSpecialOnTheAir(void)
{
}

void ContestLiveUpdates_Init(u8 round1Placing)
{
}

void ContestLiveUpdates_SetRound2Placing(u8 round2Placing)
{
}

void ContestLiveUpdates_SetWinnerAppealFlag(u8 flag)
{
}

void ContestLiveUpdates_SetWinnerMoveUsed(u16 move)
{
}

void ContestLiveUpdates_SetLoserData(u8 flag, u8 loser)
{
}

static void InterviewAfter_BravoTrainerPokemonProfile(void)
{
}

void BravoTrainerPokemonProfile_BeforeInterview1(u16 move)
{
}

void BravoTrainerPokemonProfile_BeforeInterview2(u8 contestStandingPlace)
{
}

static void InterviewAfter_BravoTrainerBattleTowerProfile(void)
{
}

void TryPutSmartShopperOnAir(void)
{
}

void PutNameRaterShowOnTheAir(void)
{
}

void StartMassOutbreak(void)
{
}

void PutLilycoveContestLadyShowOnTheAir(void)
{
}

static void InterviewAfter_FanClubLetter(void)
{
}

static void InterviewAfter_RecentHappenings(void)
{
}

static void InterviewAfter_PkmnFanClubOpinions(void)
{
}

static void InterviewAfter_Dummy(void)
{
}

static void TryStartRandomMassOutbreak(void)
{
}

void EndMassOutbreak(void)
{
}

void UpdateTVShowsPerDay(u16 days)
{
    UpdateMassOutbreakTimeLeft(days);
    TryEndMassOutbreak(days);
    UpdatePokeNewsCountdown(days);
    ResolveWorldOfMastersShow(days);
    ResolveNumberOneShow(days);
}

static void UpdateMassOutbreakTimeLeft(u16 days)
{
}

static void TryEndMassOutbreak(u16 days)
{
}

void RecordFishingAttemptForTV(bool8 caughtFish)
{
    if (caughtFish)
    {
        if (sPokemonAnglerAttemptCounters >> 8 > 4)
            TryPutFishingAdviceOnAir();

        sPokemonAnglerAttemptCounters &= 0xFF;
        if (sPokemonAnglerAttemptCounters != 0xFF)
            sPokemonAnglerAttemptCounters += 0x01;
    }
    else
    {
        if ((u8)sPokemonAnglerAttemptCounters > 4)
            TryPutFishingAdviceOnAir();

        sPokemonAnglerAttemptCounters &= 0xFF00;
        if (sPokemonAnglerAttemptCounters >> 8 != 0xFF)
            sPokemonAnglerAttemptCounters += 0x0100;
    }
}

static void TryPutFishingAdviceOnAir(void)
{
}

void SetPokemonAnglerSpecies(u16 species)
{
    sPokemonAnglerSpecies = species;
}

// World of Masters is initialized in the last slot by InitWorldOfMastersShowAttempt
// If enough Pokémon were caught during the day the show can be put on air (and will
// be moved out of the last slot).
// Either way the temporary version of the show in the last slot is deleted.
static void ResolveWorldOfMastersShow(u16 days)
{
}

static void TryPutWorldOfMastersOnAir(void)
{
}

void TryPutTodaysRivalTrainerOnAir(void)
{
}

void TryPutTrendWatcherOnAir(const u16 *words)
{
}

void TryPutTreasureInvestigatorsOnAir(void)
{
}

void TryPutFindThatGamerOnAir(u16 nCoinsPaidOut)
{
}

void AlertTVThatPlayerPlayedSlotMachine(u16 nCoinsSpent)
{
    sFindThatGamerWhichGame = SLOT_MACHINE;
    sFindThatGamerCoinsSpent = nCoinsSpent;
}

void AlertTVThatPlayerPlayedRoulette(u16 nCoinsSpent)
{
    sFindThatGamerWhichGame = ROULETTE;
    sFindThatGamerCoinsSpent = nCoinsSpent;
}

static void SecretBaseVisit_CalculateDecorationData(TVShow *show)
{
}

static void SecretBaseVisit_CalculatePartyData(TVShow *show)
{
    u8 i;
    u16 move;
    u16 j;
    u8 numMoves;
    u8 numPokemon;
    u16 sum;

    for (i = 0, numPokemon = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
        {
            sTV_SecretBaseVisitMonsTemp[numPokemon].level = GetMonData(&gPlayerParty[i], MON_DATA_LEVEL);
            sTV_SecretBaseVisitMonsTemp[numPokemon].species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES);

            // Check all the Pokémon's moves, then randomly select one to save
            numMoves = 0;
            move = GetMonData(&gPlayerParty[i], MON_DATA_MOVE1);
            if (move != MOVE_NONE)
            {
                sTV_SecretBaseVisitMovesTemp[numMoves] = move;
                numMoves++;
            }
            move = GetMonData(&gPlayerParty[i], MON_DATA_MOVE2);
            if (move != MOVE_NONE)
            {
                sTV_SecretBaseVisitMovesTemp[numMoves] = move;
                numMoves++;
            }
            move = GetMonData(&gPlayerParty[i], MON_DATA_MOVE3);
            if (move != MOVE_NONE)
            {
                sTV_SecretBaseVisitMovesTemp[numMoves] = move;
                numMoves++;
            }
            move = GetMonData(&gPlayerParty[i], MON_DATA_MOVE4);
            if (move != MOVE_NONE)
            {
                sTV_SecretBaseVisitMovesTemp[numMoves] = move;
                numMoves++;
            }
            sTV_SecretBaseVisitMonsTemp[numPokemon].move = sTV_SecretBaseVisitMovesTemp[Random() % numMoves];
            numPokemon++;
        }
    }

    for (i = 0, sum = 0; i < numPokemon; i++)
        sum += sTV_SecretBaseVisitMonsTemp[i].level;

    // Using the data calculated above, save the data to talk about on the show
    // (average level, and one randomly selected species / move)
    show->secretBaseVisit.avgLevel = sum / numPokemon;
    j = Random() % numPokemon;
    show->secretBaseVisit.species = sTV_SecretBaseVisitMonsTemp[j].species;
    show->secretBaseVisit.move = sTV_SecretBaseVisitMonsTemp[j].move;
}

void TryPutSecretBaseVisitOnAir(void)
{
}

void TryPutBreakingNewsOnAir(void)
{
}

void TryPutLotteryWinnerReportOnAir(void)
{
}

void TryPutBattleSeminarOnAir(u16 foeSpecies, u16 species, u8 moveIdx, const u16 *movePtr, u16 betterMove)
{
}

void TryPutSafariFanClubOnAir(u8 monsCaught, u8 pokeblocksUsed)
{
}

void TryPutSpotTheCutiesOnAir(struct Pokemon *pokemon, u8 ribbonMonDataIdx)
{
}

u8 GetRibbonCount(struct Pokemon *pokemon)
{
    u8 nRibbons;

    nRibbons = 0;
    nRibbons += GetMonData(pokemon, MON_DATA_COOL_RIBBON);
    nRibbons += GetMonData(pokemon, MON_DATA_BEAUTY_RIBBON);
    nRibbons += GetMonData(pokemon, MON_DATA_CUTE_RIBBON);
    nRibbons += GetMonData(pokemon, MON_DATA_SMART_RIBBON);
    nRibbons += GetMonData(pokemon, MON_DATA_TOUGH_RIBBON);
    nRibbons += GetMonData(pokemon, MON_DATA_CHAMPION_RIBBON);
    nRibbons += GetMonData(pokemon, MON_DATA_WINNING_RIBBON);
    nRibbons += GetMonData(pokemon, MON_DATA_VICTORY_RIBBON);
    nRibbons += GetMonData(pokemon, MON_DATA_ARTIST_RIBBON);
    nRibbons += GetMonData(pokemon, MON_DATA_EFFORT_RIBBON);
    nRibbons += GetMonData(pokemon, MON_DATA_MARINE_RIBBON);
    nRibbons += GetMonData(pokemon, MON_DATA_LAND_RIBBON);
    nRibbons += GetMonData(pokemon, MON_DATA_SKY_RIBBON);
    nRibbons += GetMonData(pokemon, MON_DATA_COUNTRY_RIBBON);
    nRibbons += GetMonData(pokemon, MON_DATA_NATIONAL_RIBBON);
    nRibbons += GetMonData(pokemon, MON_DATA_EARTH_RIBBON);
    nRibbons += GetMonData(pokemon, MON_DATA_WORLD_RIBBON);
    return nRibbons;
}

static u8 MonDataIdxToRibbon(u8 monDataIdx)
{
    if (monDataIdx == MON_DATA_CHAMPION_RIBBON) return CHAMPION_RIBBON;
    if (monDataIdx == MON_DATA_COOL_RIBBON)     return COOL_RIBBON_NORMAL;
    if (monDataIdx == MON_DATA_BEAUTY_RIBBON)   return BEAUTY_RIBBON_NORMAL;
    if (monDataIdx == MON_DATA_CUTE_RIBBON)     return CUTE_RIBBON_NORMAL;
    if (monDataIdx == MON_DATA_SMART_RIBBON)    return SMART_RIBBON_NORMAL;
    if (monDataIdx == MON_DATA_TOUGH_RIBBON)    return TOUGH_RIBBON_NORMAL;
    if (monDataIdx == MON_DATA_WINNING_RIBBON)  return WINNING_RIBBON;
    if (monDataIdx == MON_DATA_VICTORY_RIBBON)  return VICTORY_RIBBON;
    if (monDataIdx == MON_DATA_ARTIST_RIBBON)   return ARTIST_RIBBON;
    if (monDataIdx == MON_DATA_EFFORT_RIBBON)   return EFFORT_RIBBON;
    if (monDataIdx == MON_DATA_MARINE_RIBBON)   return MARINE_RIBBON;
    if (monDataIdx == MON_DATA_LAND_RIBBON)     return LAND_RIBBON;
    if (monDataIdx == MON_DATA_SKY_RIBBON)      return SKY_RIBBON;
    if (monDataIdx == MON_DATA_COUNTRY_RIBBON)  return COUNTRY_RIBBON;
    if (monDataIdx == MON_DATA_NATIONAL_RIBBON) return NATIONAL_RIBBON;
    if (monDataIdx == MON_DATA_EARTH_RIBBON)    return EARTH_RIBBON;
    if (monDataIdx == MON_DATA_WORLD_RIBBON)    return WORLD_RIBBON;
    return CHAMPION_RIBBON;
}

void TryPutTrainerFanClubOnAir(void)
{
}

bool8 ShouldHideFanClubInterviewer(void)
{
}

bool8 ShouldAirFrontierTVShow(void)
{
}

void TryPutFrontierTVShowOnAir(u16 winStreak, u8 facilityAndMode)
{
}

void TryPutSecretBaseSecretsOnAir(void)
{
}

// Check var thresholds required to trigger the Number One show
// The vars are reset afterwards regardless
static void ResolveNumberOneShow(u16 days)
{
    u8 i;

    for (i = 0; i < ARRAY_COUNT(sNumberOneVarsAndThresholds); i++)
    {
        if (VarGet(sNumberOneVarsAndThresholds[i][0]) >= sNumberOneVarsAndThresholds[i][1])
        {
            TryPutNumberOneOnAir(i);
            break;
        }
    }

    for (i = 0; i < ARRAY_COUNT(sNumberOneVarsAndThresholds); i++)
        VarSet(sNumberOneVarsAndThresholds[i][0], 0);
}

static void TryPutNumberOneOnAir(u8 actionIdx)
{
}

void IncrementDailySlotsUses(void)
{
    VarSet(VAR_DAILY_SLOTS, VarGet(VAR_DAILY_SLOTS) + 1);
}

void IncrementDailyRouletteUses(void)
{
    VarSet(VAR_DAILY_ROULETTE, VarGet(VAR_DAILY_ROULETTE) + 1);
}

void IncrementDailyWildBattles(void)
{
    VarSet(VAR_DAILY_WILDS, VarGet(VAR_DAILY_WILDS) + 1);
}

void IncrementDailyBerryBlender(void)
{
    VarSet(VAR_DAILY_BLENDER, VarGet(VAR_DAILY_BLENDER) + 1);
}

void IncrementDailyPlantedBerries(void)
{
    VarSet(VAR_DAILY_PLANTED_BERRIES, VarGet(VAR_DAILY_PLANTED_BERRIES) + 1);
}

void IncrementDailyPickedBerries(void)
{
    VarSet(VAR_DAILY_PICKED_BERRIES, VarGet(VAR_DAILY_PICKED_BERRIES) + gSpecialVar_0x8006);
}

void IncrementDailyBattlePoints(u16 delta)
{
    VarSet(VAR_DAILY_BP, VarGet(VAR_DAILY_BP) + delta);
}

// PokeNews

static void TryPutRandomPokeNewsOnAir(void)
{
}

static s8 GetFirstEmptyPokeNewsSlot(PokeNews *pokeNews)
{
    s8 i;

    for (i = 0; i < POKE_NEWS_COUNT; i++)
    {
        if (pokeNews[i].kind == POKENEWS_NONE)
            return i;
    }
    return -1;
}

static void ClearPokeNews(void)
{
    u8 i;

    for (i = 0; i < POKE_NEWS_COUNT; i++)
        ClearPokeNewsBySlot(i);
}

static void ClearPokeNewsBySlot(u8 i)
{
}

static void CompactPokeNews(void)
{
}

static u8 FindAnyPokeNewsOnTheAir(void)
{
}

void DoPokeNews(void)
{
}

bool8 IsPokeNewsActive(u8 newsKind)
{
}

// Returns TRUE if the effects of the given PokeNews should be applied.
// For POKENEWS_SLATEPORT / POKENEWS_LILYCOVE, only apply the effect if
// the player is talking to the Energy Guru / at the Dept Store Rooftop.
// For any other type of PokeNews this is always TRUE.
static bool8 ShouldApplyPokeNewsEffect(u8 newsKind)
{
    switch (newsKind)
    {
    case POKENEWS_SLATEPORT:
        if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(SLATEPORT_CITY)
         && gSaveBlock1Ptr->location.mapNum == MAP_NUM(SLATEPORT_CITY)
         && gSpecialVar_LastTalked == LOCALID_SLATEPORT_ENERGY_GURU)
            return TRUE;
        return FALSE;
    case POKENEWS_LILYCOVE:
        if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(LILYCOVE_CITY_DEPARTMENT_STORE_ROOFTOP)
         && gSaveBlock1Ptr->location.mapNum == MAP_NUM(LILYCOVE_CITY_DEPARTMENT_STORE_ROOFTOP))
            return TRUE;
        return FALSE;
    }
    return TRUE;
}

static bool8 IsAddingPokeNewsDisallowed(u8 newsKind)
{
}

static void UpdatePokeNewsCountdown(u16 days)
{
}

void CopyContestRankToStringVar(u8 varIdx, u8 rank)
{
    switch (rank)
    {
    case CONTEST_RANK_NORMAL:
        StringCopy(gTVStringVarPtrs[varIdx], gStdStrings[STDSTRING_NORMAL]);
        break;
    case CONTEST_RANK_SUPER:
        StringCopy(gTVStringVarPtrs[varIdx], gStdStrings[STDSTRING_SUPER]);
        break;
    case CONTEST_RANK_HYPER:
        StringCopy(gTVStringVarPtrs[varIdx], gStdStrings[STDSTRING_HYPER]);
        break;
    case CONTEST_RANK_MASTER:
        StringCopy(gTVStringVarPtrs[varIdx], gStdStrings[STDSTRING_MASTER]);
        break;
    }
}

void CopyContestCategoryToStringVar(u8 varIdx, u8 category)
{
    switch (category)
    {
    case CONTEST_CATEGORY_COOL:
        StringCopy(gTVStringVarPtrs[varIdx], gStdStrings[STDSTRING_COOL]);
        break;
    case CONTEST_CATEGORY_BEAUTY:
        StringCopy(gTVStringVarPtrs[varIdx], gStdStrings[STDSTRING_BEAUTY]);
        break;
    case CONTEST_CATEGORY_CUTE:
        StringCopy(gTVStringVarPtrs[varIdx], gStdStrings[STDSTRING_CUTE]);
        break;
    case CONTEST_CATEGORY_SMART:
        StringCopy(gTVStringVarPtrs[varIdx], gStdStrings[STDSTRING_SMART]);
        break;
    case CONTEST_CATEGORY_TOUGH:
        StringCopy(gTVStringVarPtrs[varIdx], gStdStrings[STDSTRING_TOUGH]);
        break;
    }
}

void SetContestCategoryStringVarForInterview(void)
{
}

void ConvertIntToDecimalString(u8 varIdx, int value)
{
    int nDigits = CountDigits(value);
    ConvertIntToDecimalStringN(gTVStringVarPtrs[varIdx], value, STR_CONV_MODE_LEFT_ALIGN, nDigits);
}

size_t CountDigits(int value)
{
    if (value / 10 == 0)        return 1;
    if (value / 100 == 0)       return 2;
    if (value / 1000 == 0)      return 3;
    if (value / 10000 == 0)     return 4;
    if (value / 100000 == 0)    return 5;
    if (value / 1000000 == 0)   return 6;
    if (value / 10000000 == 0)  return 7;
    if (value / 100000000 == 0) return 8;

    return 1;
}

static void SmartShopper_BufferPurchaseTotal(u8 varIdx, TVShow *show)
{
    u8 i;
    int price = 0;
    for (i = 0; i < SMARTSHOPPER_NUM_ITEMS; i++)
    {
        if (show->smartshopperShow.itemIds[i] != ITEM_NONE)
            price += ItemId_GetPrice(show->smartshopperShow.itemIds[i]) * show->smartshopperShow.itemAmounts[i];
    }

    if (show->smartshopperShow.priceReduced == TRUE)
        ConvertIntToDecimalString(varIdx, price >> 1);
    else
        ConvertIntToDecimalString(varIdx, price);
}

static bool8 IsRecordMixShowAlreadySpawned(u8 kind, bool8 delete)
{
}

static void SortPurchasesByQuantity(void)
{
    u8 i, j;

    for (i = 0; i < SMARTSHOPPER_NUM_ITEMS - 1; i++)
    {
        for (j = i + 1; j < SMARTSHOPPER_NUM_ITEMS; j++)
        {
            if (gMartPurchaseHistory[i].quantity < gMartPurchaseHistory[j].quantity)
            {
                u16 tempItemId = gMartPurchaseHistory[i].itemId;
                u16 tempQuantity = gMartPurchaseHistory[i].quantity;
                gMartPurchaseHistory[i].itemId = gMartPurchaseHistory[j].itemId;
                gMartPurchaseHistory[i].quantity = gMartPurchaseHistory[j].quantity;
                gMartPurchaseHistory[j].itemId = tempItemId;
                gMartPurchaseHistory[j].quantity = tempQuantity;
            }
        }
    }
}

static void TryReplaceOldTVShowOfKind(u8 kind)
{
}

void InterviewBefore(void)
{
    gSpecialVar_Result = FALSE;
    switch (gSpecialVar_0x8005)
    {
    case TVSHOW_FAN_CLUB_LETTER:
        InterviewBefore_FanClubLetter();
        break;
    case TVSHOW_RECENT_HAPPENINGS:
        InterviewBefore_RecentHappenings();
        break;
    case TVSHOW_PKMN_FAN_CLUB_OPINIONS:
        InterviewBefore_PkmnFanClubOpinions();
        break;
    case TVSHOW_DUMMY:
        InterviewBefore_Dummy();
        break;
    case TVSHOW_NAME_RATER_SHOW:
        InterviewBefore_NameRater();
        break;
    case TVSHOW_BRAVO_TRAINER_POKEMON_PROFILE:
        InterviewBefore_BravoTrainerPkmnProfile();
        break;
    case TVSHOW_BRAVO_TRAINER_BATTLE_TOWER_PROFILE:
        InterviewBefore_BravoTrainerBTProfile();
        break;
    case TVSHOW_CONTEST_LIVE_UPDATES:
        InterviewBefore_ContestLiveUpdates();
        break;
    case TVSHOW_3_CHEERS_FOR_POKEBLOCKS:
        InterviewBefore_3CheersForPokeblocks();
        break;
    case TVSHOW_FAN_CLUB_SPECIAL:
        InterviewBefore_FanClubSpecial();
        break;
    }
}

static void InterviewBefore_FanClubLetter(void)
{
}

static void InterviewBefore_RecentHappenings(void)
{
}

static void InterviewBefore_PkmnFanClubOpinions(void)
{
}

static void InterviewBefore_Dummy(void)
{
    gSpecialVar_Result = TRUE;
}

static void InterviewBefore_NameRater(void)
{
    TryReplaceOldTVShowOfKind(TVSHOW_NAME_RATER_SHOW);
}

static void InterviewBefore_BravoTrainerPkmnProfile(void)
{
}

static void InterviewBefore_ContestLiveUpdates(void)
{
    TryReplaceOldTVShowOfKind(TVSHOW_CONTEST_LIVE_UPDATES);
}

static void InterviewBefore_3CheersForPokeblocks(void)
{
    TryReplaceOldTVShowOfKind(TVSHOW_3_CHEERS_FOR_POKEBLOCKS);
}

static void InterviewBefore_BravoTrainerBTProfile(void)
{
}

static void InterviewBefore_FanClubSpecial(void)
{
}

static bool8 IsPartyMonNicknamedOrNotEnglish(u8 monIdx)
{
    struct Pokemon *pokemon;
    u8 language;

    pokemon = &gPlayerParty[monIdx];
    GetMonData(pokemon, MON_DATA_NICKNAME, gStringVar1);
    language = GetMonData(pokemon, MON_DATA_LANGUAGE, &language);
    if (language == GAME_LANGUAGE && !StringCompare(gSpeciesNames[GetMonData(pokemon, MON_DATA_SPECIES, NULL)], gStringVar1))
        return FALSE;

    return TRUE;
}

bool8 IsLeadMonNicknamedOrNotEnglish(void)
{
    return IsPartyMonNicknamedOrNotEnglish(GetLeadMonIndex());
}

static void DeleteTVShowInArrayByIdx(TVShow *shows, u8 idx)
{
    u8 i;

    shows[idx].commonInit.kind = TVSHOW_OFF_AIR;
    shows[idx].commonInit.active = FALSE;
    for (i = 0; i < ARRAY_COUNT(shows[idx].commonInit.data); i++)
        shows[idx].commonInit.data[i] = 0;
}

static void CompactTVShowArray(TVShow *shows)
{
    u8 i;
    u8 j;

    // Compact normal TV shows
    for (i = 0; i < NUM_NORMAL_TVSHOW_SLOTS - 1; i++)
    {
        if (shows[i].common.kind == TVSHOW_OFF_AIR)
        {
            for (j = i + 1; j < NUM_NORMAL_TVSHOW_SLOTS; j++)
            {
                if (shows[j].common.kind != TVSHOW_OFF_AIR)
                {
                    shows[i] = shows[j];
                    DeleteTVShowInArrayByIdx(shows, j);
                    break;
                }
            }
        }
    }

    // Compact Record Mix TV shows
    for (i = NUM_NORMAL_TVSHOW_SLOTS; i < LAST_TVSHOW_IDX; i++)
    {
        if (shows[i].common.kind == TVSHOW_OFF_AIR)
        {
            for (j = i + 1; j < LAST_TVSHOW_IDX; j++)
            {
                if (shows[j].common.kind != TVSHOW_OFF_AIR)
                {
                    shows[i] = shows[j];
                    DeleteTVShowInArrayByIdx(shows, j);
                    break;
                }
            }
        }
    }
}

static u16 GetRandomDifferentSpeciesAndNameSeenByPlayer(u8 varIdx, u16 excludedSpecies)
{
    u16 species = GetRandomDifferentSpeciesSeenByPlayer(excludedSpecies);
    StringCopy(gTVStringVarPtrs[varIdx], gSpeciesNames[species]);
    return species;
}

static u16 GetRandomDifferentSpeciesSeenByPlayer(u16 excludedSpecies)
{
    u16 species = Random() % (NUM_SPECIES - 1) + 1;
    u16 initSpecies = species;

    while (GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_SEEN) != TRUE || species == excludedSpecies)
    {
        if (species == SPECIES_NONE + 1)
            species = NUM_SPECIES - 1;
        else
            species--;

        if (species == initSpecies)
        {
            // Looped back to initial species (only Pokémon seen), must choose excluded species
            species = excludedSpecies;
            return species;
        }
    };
    return species;
}

static void Script_FindFirstEmptyNormalTVShowSlot(void)
{
}

static s8 FindFirstEmptyNormalTVShowSlot(TVShow *shows)
{
    u8 i;

    for (i = 0; i < NUM_NORMAL_TVSHOW_SLOTS; i++)
    {
        if (shows[i].common.kind == TVSHOW_OFF_AIR)
            return i;
    }
    return -1;
}

static s8 FindFirstEmptyRecordMixTVShowSlot(TVShow *shows)
{
    s8 i;

    for (i = NUM_NORMAL_TVSHOW_SLOTS; i < LAST_TVSHOW_IDX; i++)
    {
        if (shows[i].common.kind == TVSHOW_OFF_AIR)
            return i;
    }
    return -1;
}

static bool8 BernoulliTrial(u16 ratio)
{
    if (Random() <= ratio)
        return FALSE;

    return TRUE;
}

// For TVSHOW_FAN_CLUB_LETTER / TVSHOW_RECENT_HAPPENINGS
// Both are assumed to have the same struct layout
static void GetRandomWordFromShow(TVShow *show)
{
    u8 i;

    i = Random() % ARRAY_COUNT(show->fanclubLetter.words);

    // From random point, get first non-empty word
    while (TRUE)
    {
        if (i == ARRAY_COUNT(show->fanclubLetter.words))
            i = 0;

        if (show->fanclubLetter.words[i] != EC_EMPTY_WORD)
            break;
        i++;
    }
    CopyEasyChatWord(gStringVar3, show->fanclubLetter.words[i]);
}

static u8 GetRandomNameRaterStateFromName(TVShow *show)
{
    u8 i;
    u16 nameSum;

    nameSum = 0;
    for (i = 0; i < POKEMON_NAME_LENGTH + 1; i++)
    {
        if (show->nameRaterShow.pokemonName[i] == EOS)
            break;

        nameSum += show->nameRaterShow.pokemonName[i];
    }
    return nameSum & 7;
}

static void GetNicknameSubstring(u8 varIdx, u8 whichPosition, u8 charParam, u16 whichString, u16 species, TVShow *show)
{
    u8 buff[16];
    u8 i;
    u16 strlen;

    for (i = 0; i < 3; i++)
        buff[i] = EOS;

    if (whichString == 0)
    {
        strlen = StringLength(show->nameRaterShow.trainerName);
        if (charParam == 0)
        {
            buff[0] = show->nameRaterShow.trainerName[whichPosition];
        }
        else if (charParam == 1)
        {
            buff[0] = show->nameRaterShow.trainerName[strlen - whichPosition];
        }
        else if (charParam == 2)
        {
            buff[0] = show->nameRaterShow.trainerName[whichPosition];
            buff[1] = show->nameRaterShow.trainerName[whichPosition + 1];
        }
        else
        {
            buff[0] = show->nameRaterShow.trainerName[strlen - (whichPosition + 2)];
            buff[1] = show->nameRaterShow.trainerName[strlen - (whichPosition + 1)];
        }
        ConvertInternationalString(buff, show->nameRaterShow.language);
    }
    else if (whichString == 1)
    {
        strlen = StringLength(show->nameRaterShow.pokemonName);
        if (charParam == 0)
        {
            buff[0] = show->nameRaterShow.pokemonName[whichPosition];
        }
        else if (charParam == 1)
        {
            buff[0] = show->nameRaterShow.pokemonName[strlen - whichPosition];
        }
        else if (charParam == 2)
        {
            buff[0] = show->nameRaterShow.pokemonName[whichPosition];
            buff[1] = show->nameRaterShow.pokemonName[whichPosition + 1];
        }
        else
        {
            buff[0] = show->nameRaterShow.pokemonName[strlen - (whichPosition + 2)];
            buff[1] = show->nameRaterShow.pokemonName[strlen - (whichPosition + 1)];
        }
        ConvertInternationalString(buff, show->nameRaterShow.pokemonNameLanguage);
    }
    else
    {
        strlen = StringLength(gSpeciesNames[species]);
        if (charParam == 0)
        {
            buff[0] = gSpeciesNames[species][whichPosition];
        }
        else if (charParam == 1)
        {
            buff[0] = gSpeciesNames[species][strlen - whichPosition];
        }
        else if (charParam == 2)
        {
            buff[0] = gSpeciesNames[species][whichPosition];
            buff[1] = gSpeciesNames[species][whichPosition + 1];
        }
        else
        {
            buff[0] = gSpeciesNames[species][strlen - (whichPosition + 2)];
            buff[1] = gSpeciesNames[species][strlen - (whichPosition + 1)];
        }
    }
    StringCopy(gTVStringVarPtrs[varIdx], buff);
}

bool8 TryPutNameRaterShowOnTheAir(void)
{
    GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_NICKNAME, gStringVar1);

    // Nickname wasnt changed
    if (!StringCompare(gStringVar3, gStringVar1))
        return FALSE;

    PutNameRaterShowOnTheAir();
    return TRUE;
}

void ChangePokemonNickname(void)
{
    void ChangePokemonNickname_CB(void);

    GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_NICKNAME, gStringVar3);
    GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_NICKNAME, gStringVar2);
    DoNamingScreen(NAMING_SCREEN_NICKNAME, gStringVar2, GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_SPECIES, NULL), GetMonGender(&gPlayerParty[gSpecialVar_0x8004]), GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_PERSONALITY, NULL), ChangePokemonNickname_CB);
}

void ChangePokemonNickname_CB(void)
{
    SetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_NICKNAME, gStringVar2);
    CB2_ReturnToFieldContinueScriptPlayMapMusic();
}

void ChangeBoxPokemonNickname(void)
{
    struct BoxPokemon *boxMon;

    boxMon = GetBoxedMonPtr(gSpecialVar_MonBoxId, gSpecialVar_MonBoxPos);
    GetBoxMonData(boxMon, MON_DATA_NICKNAME, gStringVar3);
    GetBoxMonData(boxMon, MON_DATA_NICKNAME, gStringVar2);
    DoNamingScreen(NAMING_SCREEN_NICKNAME, gStringVar2, GetBoxMonData(boxMon, MON_DATA_SPECIES, NULL), GetBoxMonGender(boxMon), GetBoxMonData(boxMon, MON_DATA_PERSONALITY, NULL), ChangeBoxPokemonNickname_CB);
}

static void ChangeBoxPokemonNickname_CB(void)
{
    SetBoxMonNickAt(gSpecialVar_MonBoxId, gSpecialVar_MonBoxPos, gStringVar2);
    CB2_ReturnToFieldContinueScriptPlayMapMusic();
}

void BufferMonNickname(void)
{
    GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
}

void IsMonOTIDNotPlayers(void)
{
    if (GetPlayerIDAsU32() == GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_OT_ID, NULL))
        gSpecialVar_Result = FALSE;
    else
        gSpecialVar_Result = TRUE;
}

static u8 GetTVGroupByShowId(u8 kind)
{
    if (kind == TVSHOW_OFF_AIR)
        return TVGROUP_NONE;

    if (kind >= TVGROUP_NORMAL_START && kind <= TVGROUP_NORMAL_END)
        return TVGROUP_NORMAL;

    if (kind >= TVGROUP_RECORD_MIX_START && kind <= TVGROUP_RECORD_MIX_END)
        return TVGROUP_RECORD_MIX;

    if (kind >= TVGROUP_OUTBREAK_START && kind <= TVGROUP_OUTBREAK_END)
        return TVGROUP_OUTBREAK;

    return TVGROUP_NONE;
}

u32 GetPlayerIDAsU32(void)
{
    return (gSaveBlock2Ptr->playerTrainerId[3] << 24) | (gSaveBlock2Ptr->playerTrainerId[2] << 16) | (gSaveBlock2Ptr->playerTrainerId[1] << 8) | gSaveBlock2Ptr->playerTrainerId[0];
}

u8 CheckForPlayersHouseNews(void)
{
    // Check if not in Littleroot house map group
    if (gSaveBlock1Ptr->location.mapGroup != MAP_GROUP(LITTLEROOT_TOWN_BRENDANS_HOUSE_1F))
        return PLAYERS_HOUSE_TV_NONE;

    // Check if not in player's house (dependent on gender)
    if (gSaveBlock2Ptr->playerGender == MALE)
    {
        if (gSaveBlock1Ptr->location.mapNum != MAP_NUM(LITTLEROOT_TOWN_BRENDANS_HOUSE_1F))
            return PLAYERS_HOUSE_TV_NONE;
    }
    else
    {
        if (gSaveBlock1Ptr->location.mapNum != MAP_NUM(LITTLEROOT_TOWN_MAYS_HOUSE_1F))
            return PLAYERS_HOUSE_TV_NONE;
    }

    if (FlagGet(FLAG_SYS_TV_LATIAS_LATIOS) == TRUE)
        return PLAYERS_HOUSE_TV_LATI;

    if (FlagGet(FLAG_SYS_TV_HOME) == TRUE)
        return PLAYERS_HOUSE_TV_MOVIE;

    return PLAYERS_HOUSE_TV_LATI;
}

void GetMomOrDadStringForTVMessage(void)
{
    if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(LITTLEROOT_TOWN_BRENDANS_HOUSE_1F))
    {
        if (gSaveBlock2Ptr->playerGender == MALE)
        {
            if (gSaveBlock1Ptr->location.mapNum == MAP_NUM(LITTLEROOT_TOWN_BRENDANS_HOUSE_1F))
            {
                StringCopy(gStringVar1, gText_Mom);
                VarSet(VAR_TEMP_3, 1);
            }
        }
        else
        {
            if (gSaveBlock1Ptr->location.mapNum == MAP_NUM(LITTLEROOT_TOWN_MAYS_HOUSE_1F))
            {
                StringCopy(gStringVar1, gText_Mom);
                VarSet(VAR_TEMP_3, 1);
            }
        }
    }
    if (VarGet(VAR_TEMP_3) == 1)
    {
        StringCopy(gStringVar1, gText_Mom);
    }
    else if (VarGet(VAR_TEMP_3) == 2)
    {
        StringCopy(gStringVar1, gText_Dad);
    }
    else if (VarGet(VAR_TEMP_3) > 2)
    {
        if (VarGet(VAR_TEMP_3) % 2 == 0)
            StringCopy(gStringVar1, gText_Mom);
        else
            StringCopy(gStringVar1, gText_Dad);
    }
    else
    {
        if (Random() % 2 != 0)
        {
            StringCopy(gStringVar1, gText_Mom);
            VarSet(VAR_TEMP_3, 1);
        }
        else
        {
            StringCopy(gStringVar1, gText_Dad);
            VarSet(VAR_TEMP_3, 2);
        }
    }
}

void HideBattleTowerReporter(void)
{
    VarSet(VAR_BRAVO_TRAINER_BATTLE_TOWER_ON, 0);
    RemoveObjectEventByLocalIdAndMap(LOCALID_BATTLE_TOWER_LOBBY_REPORTER, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    FlagSet(FLAG_HIDE_BATTLE_TOWER_REPORTER);
}

void ReceiveTvShowsData(void *src, u32 size, u8 playersLinkId)
{
}

static void SetMixedTVShows(TVShow player1[TV_SHOWS_COUNT], TVShow player2[TV_SHOWS_COUNT], TVShow player3[TV_SHOWS_COUNT], TVShow player4[TV_SHOWS_COUNT])
{
    u8 i;
    u8 j;
    TVShow **tvShows[MAX_LINK_PLAYERS];

    tvShows[0] = &player1;
    tvShows[1] = &player2;
    tvShows[2] = &player3;
    tvShows[3] = &player4;
    sTVShowMixingNumPlayers = GetLinkPlayerCount();
    while (1)
    {
        for (i = 0; i < sTVShowMixingNumPlayers; i++)
        {
            if (i == 0)
                sRecordMixingPartnersWithoutShowsToShare = 0;

            sTVShowMixingCurSlot = FindInactiveShowInArray(tvShows[i][0]);
            if (sTVShowMixingCurSlot == -1)
            {
                sRecordMixingPartnersWithoutShowsToShare++;
                if (sRecordMixingPartnersWithoutShowsToShare == sTVShowMixingNumPlayers)
                    return;
            }
            else
            {
                for (j = 0; j < sTVShowMixingNumPlayers - 1; j++)
                {
                    sCurTVShowSlot = FindFirstEmptyRecordMixTVShowSlot(tvShows[(i + j + 1) % sTVShowMixingNumPlayers][0]);
                    if (sCurTVShowSlot != -1
                        && TryMixTVShow(&tvShows[(i + j + 1) % sTVShowMixingNumPlayers][0], &tvShows[i][0], (i + j + 1) % sTVShowMixingNumPlayers) == 1)
                        break;
                }
                if (j == sTVShowMixingNumPlayers - 1)
                    DeleteTVShowInArrayByIdx(tvShows[i][0], sTVShowMixingCurSlot);
            }
        }
    }
}

static bool8 TryMixTVShow(TVShow *dest[TV_SHOWS_COUNT], TVShow *src[TV_SHOWS_COUNT], u8 idx)
{
    bool8 success;
    u8 type;
    TVShow *tv1 = *dest;
    TVShow *tv2 = *src;

    success = FALSE;
    type = GetTVGroupByShowId(tv2[sTVShowMixingCurSlot].common.kind);
    switch (type)
    {
    case TVGROUP_NORMAL:
        success = TryMixNormalTVShow(&tv1[sCurTVShowSlot], &tv2[sTVShowMixingCurSlot], idx);
        break;
    case TVGROUP_RECORD_MIX:
        success = TryMixRecordMixTVShow(&tv1[sCurTVShowSlot], &tv2[sTVShowMixingCurSlot], idx);
        break;
    case TVGROUP_OUTBREAK:
        success = TryMixOutbreakTVShow(&tv1[sCurTVShowSlot], &tv2[sTVShowMixingCurSlot], idx);
        break;
    }

    // Show was mixed, delete from array
    if (success == TRUE)
    {
        DeleteTVShowInArrayByIdx(tv2, sTVShowMixingCurSlot);
        return TRUE;
    }
    return FALSE;
}

static bool8 TryMixNormalTVShow(TVShow *dest, TVShow *src, u8 idx)
{
    u32 linkTrainerId = GetLinkPlayerTrainerId(idx);

    if ((linkTrainerId & 0xFF) == src->common.trainerIdLo
    && ((linkTrainerId >> 8) & 0xFF) == src->common.trainerIdHi)
        return FALSE;

    src->common.trainerIdLo = src->common.srcTrainerIdLo;
    src->common.trainerIdHi = src->common.srcTrainerIdHi;
    src->common.srcTrainerIdLo = linkTrainerId & 0xFF;
    src->common.srcTrainerIdHi = linkTrainerId >> 8;
    *dest = *src;
    dest->common.active = TRUE;
    return TRUE;
}

static bool8 TryMixRecordMixTVShow(TVShow *dest, TVShow *src, u8 idx)
{
    u32 linkTrainerId = GetLinkPlayerTrainerId(idx);

    if ((linkTrainerId & 0xFF) == src->common.srcTrainerIdLo
    && ((linkTrainerId >> 8) & 0xFF) == src->common.srcTrainerIdHi)
        return FALSE;

    if ((linkTrainerId & 0xFF) == src->common.trainerIdLo
    && ((linkTrainerId >> 8) & 0xFF) == src->common.trainerIdHi)
        return FALSE;

    src->common.srcTrainerIdLo = src->common.srcTrainerId2Lo;
    src->common.srcTrainerIdHi = src->common.srcTrainerId2Hi;
    src->common.srcTrainerId2Lo = linkTrainerId & 0xFF;
    src->common.srcTrainerId2Hi = linkTrainerId >> 8;
    *dest = *src;
    dest->common.active = TRUE;
    return TRUE;
}

static bool8 TryMixOutbreakTVShow(TVShow *dest, TVShow *src, u8 idx)
{
    u32 linkTrainerId = GetLinkPlayerTrainerId(idx);

    if ((linkTrainerId & 0xFF) == src->common.trainerIdLo
    && ((linkTrainerId >> 8) & 0xFF) == src->common.trainerIdHi)
        return FALSE;

    src->common.trainerIdLo = src->common.srcTrainerIdLo;
    src->common.trainerIdHi = src->common.srcTrainerIdHi;
    src->common.srcTrainerIdLo = linkTrainerId & 0xFF;
    src->common.srcTrainerIdHi = linkTrainerId >> 8;
    *dest = *src;
    dest->common.active = TRUE;
    dest->massOutbreak.daysLeft = 1;
    return TRUE;
}

static s8 FindInactiveShowInArray(TVShow *tvShows)
{
    u8 i;

    for (i = 0; i < LAST_TVSHOW_IDX; i++)
    {
        // Second check is to make sure its a valid show (not too high, not TVSHOW_OFF_AIR)
        if (tvShows[i].common.active == FALSE && (u8)(tvShows[i].common.kind - 1) < TVGROUP_OUTBREAK_END)
            return i;
    }
    return -1;
}

static void DeactivateShowsWithUnseenSpecies(void)
{
}

static void DeactivateShow(u8 showIdx)
{
}

static void DeactivateShowIfNotSeenSpecies(u16 species, u8 showIdx)
{
}

static void DeactivateGameCompleteShowsIfNotUnlocked(void)
{
}

void DeactivateAllNormalTVShows(void)
{
}

// Ensures a minimum of 5 empty record mixed show slots
static void DeleteExcessMixedShows(void)
{
}

void ReceivePokeNewsData(void *src, u32 size, u8 playersLinkId)
{
}

static void SetMixedPokeNews(PokeNews player1[POKE_NEWS_COUNT], PokeNews player2[POKE_NEWS_COUNT], PokeNews player3[POKE_NEWS_COUNT], PokeNews player4[POKE_NEWS_COUNT])
{
    u8 i, j, k;
    PokeNews **pokeNews[MAX_LINK_PLAYERS];

    pokeNews[0] = &player1;
    pokeNews[1] = &player2;
    pokeNews[2] = &player3;
    pokeNews[3] = &player4;
    sTVShowNewsMixingNumPlayers = GetLinkPlayerCount();
    for (i = 0; i < POKE_NEWS_COUNT; i++)
    {
        for (j = 0; j < sTVShowNewsMixingNumPlayers; j++)
        {
            sTVShowMixingCurSlot = GetPokeNewsSlotIfActive(*pokeNews[j], i);
            if (sTVShowMixingCurSlot != -1)
            {
                for (k = 0; k < sTVShowNewsMixingNumPlayers - 1; k++)
                {
                    sCurTVShowSlot = GetFirstEmptyPokeNewsSlot(*pokeNews[(j + k + 1) % sTVShowNewsMixingNumPlayers]);
                    if (sCurTVShowSlot != -1)
                        InitTryMixPokeNewsShow(pokeNews[(j + k + 1) % sTVShowNewsMixingNumPlayers], pokeNews[j]);
                }
            }
        }
    }
}

static void InitTryMixPokeNewsShow(PokeNews *dest[POKE_NEWS_COUNT], PokeNews *src[POKE_NEWS_COUNT])
{
    PokeNews *ptr1 = *dest;
    PokeNews *ptr2 = *src;
    ptr2 += sTVShowMixingCurSlot;
    TryMixPokeNewsShow(ptr1, ptr2, sCurTVShowSlot);
}

static bool8 TryMixPokeNewsShow(PokeNews *dest, PokeNews *src, s8 slot)
{
    u8 i;

    if (src->kind == POKENEWS_NONE)
        return FALSE;

    for (i = 0; i < POKE_NEWS_COUNT; i++)
    {
        if (dest[i].kind == src->kind)
            return FALSE;
    }
    dest[slot].kind = src->kind;
    dest[slot].state = POKENEWS_STATE_UPCOMING;
    dest[slot].dayCountdown = src->dayCountdown;
    return TRUE;
}

static s8 GetPokeNewsSlotIfActive(PokeNews *pokeNews, u8 idx)
{
    if (pokeNews[idx].kind == POKENEWS_NONE)
        return -1;

    return idx;
}

static void ClearInvalidPokeNews(void)
{
}

static void ClearPokeNewsIfGameNotComplete(void)
{
}

#define SetStrLanguage(strptr, langptr, langfix) \
if (IsStringJapanese(strptr)) \
{   \
    (langptr) = LANGUAGE_JAPANESE; \
} \
else \
{ \
    (langptr) = langfix; \
}

void SanitizeTVShowsForRuby(TVShow *shows)
{
    TVShow *curShow;

    SanitizeTVShowLocationsForRuby(shows);
    for (curShow = shows; curShow < shows + LAST_TVSHOW_IDX; curShow++)
    {
        if (curShow->bravoTrainerTower.kind == TVSHOW_BRAVO_TRAINER_BATTLE_TOWER_PROFILE)
        {
            if ((curShow->bravoTrainerTower.language == LANGUAGE_JAPANESE && curShow->bravoTrainerTower.pokemonNameLanguage != LANGUAGE_JAPANESE)
             || (curShow->bravoTrainerTower.language != LANGUAGE_JAPANESE && curShow->bravoTrainerTower.pokemonNameLanguage == LANGUAGE_JAPANESE))
                memset(curShow, 0, sizeof(TVShow));
        }
    }
}

static void TranslateRubyShows(TVShow *shows)
{
    TVShow *curShow;

    for (curShow = shows; curShow < shows + LAST_TVSHOW_IDX; curShow++)
    {
        if (curShow->bravoTrainerTower.kind == TVSHOW_BRAVO_TRAINER_BATTLE_TOWER_PROFILE)
        {
            if (IsStringJapanese(curShow->bravoTrainerTower.pokemonName))
                curShow->bravoTrainerTower.pokemonNameLanguage = LANGUAGE_JAPANESE;
            else
                curShow->bravoTrainerTower.pokemonNameLanguage = GAME_LANGUAGE;
        }
    }
}

static u8 GetStringLanguage(u8 *str)
{
    return IsStringJapanese(str) ? LANGUAGE_JAPANESE : GAME_LANGUAGE;
}

static void TranslateJapaneseEmeraldShows(TVShow *shows)
{
    TVShow *curShow;

    for (curShow = shows; curShow < shows + LAST_TVSHOW_IDX; curShow++)
    {
        switch(curShow->common.kind)
        {
        case TVSHOW_FAN_CLUB_LETTER:
            curShow->fanclubLetter.language = GetStringLanguage(curShow->fanclubLetter.playerName);
            break;
        case TVSHOW_RECENT_HAPPENINGS:
            curShow->recentHappenings.language = GetStringLanguage(curShow->recentHappenings.playerName);
            break;
        case TVSHOW_PKMN_FAN_CLUB_OPINIONS:
            curShow->fanclubOpinions.language = GetStringLanguage(curShow->fanclubOpinions.playerName);
            curShow->fanclubOpinions.pokemonNameLanguage = GetStringLanguage(curShow->fanclubOpinions.nickname);
            break;
        case TVSHOW_DUMMY:
            curShow->dummy.language = GetStringLanguage(curShow->dummy.name);
            break;
        case TVSHOW_NAME_RATER_SHOW:
            curShow->nameRaterShow.language = GetStringLanguage(curShow->nameRaterShow.trainerName);
            curShow->nameRaterShow.pokemonNameLanguage = GetStringLanguage(curShow->nameRaterShow.pokemonName);
            break;
        case TVSHOW_BRAVO_TRAINER_POKEMON_PROFILE:
            curShow->bravoTrainer.language = GetStringLanguage(curShow->bravoTrainer.playerName);
            curShow->bravoTrainer.pokemonNameLanguage = GetStringLanguage(curShow->bravoTrainer.pokemonNickname);
            break;
        case TVSHOW_BRAVO_TRAINER_BATTLE_TOWER_PROFILE:
            curShow->bravoTrainerTower.language = GetStringLanguage(curShow->bravoTrainerTower.trainerName);
            curShow->bravoTrainerTower.pokemonNameLanguage = GetStringLanguage(curShow->bravoTrainerTower.pokemonName);
            break;
        case TVSHOW_CONTEST_LIVE_UPDATES:
            curShow->contestLiveUpdates.winningTrainerLanguage = GetStringLanguage(curShow->contestLiveUpdates.winningTrainerName);
            curShow->contestLiveUpdates.losingTrainerLanguage = GetStringLanguage(curShow->contestLiveUpdates.losingTrainerName);
            break;
        case TVSHOW_3_CHEERS_FOR_POKEBLOCKS:
            curShow->threeCheers.language = GetStringLanguage(curShow->threeCheers.playerName);
            curShow->threeCheers.worstBlenderLanguage = GetStringLanguage(curShow->threeCheers.worstBlenderName);
            break;
        case TVSHOW_BATTLE_UPDATE:
            curShow->battleUpdate.language = GetStringLanguage(curShow->battleUpdate.playerName);
            curShow->battleUpdate.linkOpponentLanguage = GetStringLanguage(curShow->battleUpdate.linkOpponentName);
            break;
        case TVSHOW_FAN_CLUB_SPECIAL:
            curShow->fanClubSpecial.language = GetStringLanguage(curShow->fanClubSpecial.playerName);
            curShow->fanClubSpecial.idolNameLanguage = GetStringLanguage(curShow->fanClubSpecial.idolName);
            break;
        case TVSHOW_LILYCOVE_CONTEST_LADY:
            curShow->contestLady.language = GetStringLanguage(curShow->contestLady.playerName);
            curShow->contestLady.pokemonNameLanguage = GetStringLanguage(curShow->contestLady.nickname);
            break;
        case TVSHOW_POKEMON_TODAY_CAUGHT:
            curShow->pokemonToday.language = GetStringLanguage(curShow->pokemonToday.playerName);
            curShow->pokemonToday.language2 = GetStringLanguage(curShow->pokemonToday.nickname);
            break;
        case TVSHOW_SMART_SHOPPER:
            curShow->smartshopperShow.language = GetStringLanguage(curShow->smartshopperShow.playerName);
            break;
        case TVSHOW_POKEMON_TODAY_FAILED:
            curShow->pokemonTodayFailed.language = GetStringLanguage(curShow->pokemonTodayFailed.playerName);
            break;
        case TVSHOW_FISHING_ADVICE:
            curShow->pokemonAngler.language = GetStringLanguage(curShow->pokemonAngler.playerName);
            break;
        case TVSHOW_WORLD_OF_MASTERS:
            curShow->worldOfMasters.language = GetStringLanguage(curShow->worldOfMasters.playerName);
            break;
        case TVSHOW_TREND_WATCHER:
            curShow->trendWatcher.language = GetStringLanguage(curShow->trendWatcher.playerName);
            break;
        case TVSHOW_BREAKING_NEWS:
            curShow->breakingNews.language = GetStringLanguage(curShow->breakingNews.playerName);
            break;
        case TVSHOW_BATTLE_SEMINAR:
            curShow->battleSeminar.language = GetStringLanguage(curShow->battleSeminar.playerName);
            break;
        case TVSHOW_FIND_THAT_GAMER:
        case TVSHOW_TRAINER_FAN_CLUB:
            curShow->trainerFanClub.language = GetStringLanguage(curShow->trainerFanClub.playerName);
            break;
        case TVSHOW_CUTIES:
            curShow->cuties.language = GetStringLanguage(curShow->cuties.playerName);
            curShow->cuties.pokemonNameLanguage = GetStringLanguage(curShow->cuties.nickname);
            break;
        case TVSHOW_TODAYS_RIVAL_TRAINER:
        case TVSHOW_SECRET_BASE_VISIT:
        case TVSHOW_FRONTIER:
            curShow->rivalTrainer.language = GetStringLanguage(curShow->rivalTrainer.playerName);
            break;
        case TVSHOW_TREASURE_INVESTIGATORS:
        case TVSHOW_LOTTO_WINNER:
        case TVSHOW_NUMBER_ONE:
            curShow->treasureInvestigators.language = GetStringLanguage(curShow->treasureInvestigators.playerName);
            break;
        case TVSHOW_SECRET_BASE_SECRETS:
            curShow->secretBaseSecrets.language = GetStringLanguage(curShow->secretBaseSecrets.playerName);
            curShow->secretBaseSecrets.baseOwnersNameLanguage = GetStringLanguage(curShow->secretBaseSecrets.baseOwnersName);
            break;
        case TVSHOW_SAFARI_FAN_CLUB:
            curShow->safariFanClub.language = GetStringLanguage(curShow->safariFanClub.playerName);
            break;
        case TVSHOW_MASS_OUTBREAK:
            break;
        }
    }
}

void SanitizeTVShowLocationsForRuby(TVShow *shows)
{
    int i;

    for (i = 0; i < LAST_TVSHOW_IDX; i++)
    {
        switch (shows[i].common.kind)
        {
        case TVSHOW_WORLD_OF_MASTERS:
            if (shows[i].worldOfMasters.location > KANTO_MAPSEC_START)
                memset(&shows[i], 0, sizeof(TVShow));
            break;
        case TVSHOW_POKEMON_TODAY_FAILED:
            if (shows[i].pokemonTodayFailed.location > KANTO_MAPSEC_START)
                memset(&shows[i], 0, sizeof(TVShow));
            break;
        }
    }
}

// gSpecialVar_0x8004 here is set from GetRandomActiveShowIdx in EventScript_TryDoTVShow
void DoTVShow(void)
{
}

static void DoTVShowBravoTrainerPokemonProfile(void)
{
}

// This is the TV show triggered by accepting the reporter's interview in the lobby of Battle Tower.
// The reporter had asked the player if they were satisfied or not with the challenge, and then asked
// for a one word Easy Chat description of their feelings about the challenge.
static void DoTVShowBravoTrainerBattleTower(void)
{
}

static void DoTVShowTodaysSmartShopper(void)
{
}

static void DoTVShowTheNameRaterShow(void)
{
}

static void DoTVShowPokemonTodaySuccessfulCapture(void)
{
}

static void DoTVShowPokemonTodayFailedCapture(void)
{
}

static void DoTVShowPokemonFanClubLetter(void)
{
}

static void DoTVShowRecentHappenings(void)
{
}

static void DoTVShowPokemonFanClubOpinions(void)
{
}

static void DoTVShowDummiedOut(void)
{

}

static void DoTVShowPokemonNewsMassOutbreak(void)
{
}

// TV Show that plays after a Link Contest.
// First talks about the winner and something they did, then about a losing player and something they did
// The show is only generated when the player wins, but can be record mixed to other games
// Each state buffers any needed data for a message to print from sTVContestLiveUpdatesTextGroup
// Many cases in this function are identical, and its size can be reduced a good deal by collapsing them
// Can't get this to match while collapsing them though
static void DoTVShowPokemonContestLiveUpdates(void)
{
}

static void DoTVShowPokemonBattleUpdate(void)
{
}

static void DoTVShow3CheersForPokeblocks(void)
{
}

void DoTVShowInSearchOfTrainers(void)
{
}

static void DoTVShowPokemonAngler(void)
{
}

static void DoTVShowTheWorldOfMasters(void)
{
}

static void DoTVShowTodaysRivalTrainer(void)
{
}

static void DoTVShowDewfordTrendWatcherNetwork(void)
{
}

static void DoTVShowHoennTreasureInvestigators(void)
{
}

static void DoTVShowFindThatGamer(void)
{
}

static void DoTVShowBreakingNewsTV(void)
{
}

static void DoTVShowSecretBaseVisit(void)
{
}

static void DoTVShowPokemonLotteryWinnerFlashReport(void)
{
}

static void DoTVShowThePokemonBattleSeminar(void)
{
}

static void DoTVShowTrainerFanClubSpecial(void)
{
}

static void DoTVShowTrainerFanClub(void)
{
}

static void DoTVShowSpotTheCuties(void)
{
}

static void DoTVShowPokemonNewsBattleFrontier(void)
{
}

static void DoTVShowWhatsNo1InHoennToday(void)
{
}

u8 SecretBaseSecrets_GetNumActionsTaken(TVShow *show)
{
    u8 i;
    u8 flagsSet;

    for (i = 0, flagsSet = 0; i < NUM_SECRET_BASE_FLAGS; i++)
    {
        if ((show->secretBaseSecrets.flags >> i) & 1)
            flagsSet++;
    }
    return flagsSet;
}

static u8 SecretBaseSecrets_GetStateByFlagNumber(TVShow *show, u8 flagId)
{
    u8 i;
    u8 flagsSet;

    for (i = 0, flagsSet = 0; i < NUM_SECRET_BASE_FLAGS; i++)
    {
        if ((show->secretBaseSecrets.flags >> i) & 1)
        {
            if (flagsSet == flagId)
                return sTVSecretBaseSecretsActions[i];

            flagsSet++;
        }
    }
    return 0;
}

static void DoTVShowSecretBaseSecrets(void)
{
}

static void DoTVShowSafariFanClub(void)
{
}

// This show is a version of Contest Live Updates for the Lilycove Contest Lady
static void DoTVShowLilycoveContestLady(void)
{
}

static void TVShowDone(void)
{
}

void ResetTVShowState(void)
{
    sTVShowState = 0;
}
