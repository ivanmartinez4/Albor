#include "global.h"
#include "battle_tower.h"
#include "event_data.h"
#include "battle_setup.h"
#include "overworld.h"
#include "random.h"
#include "text.h"
#include "main.h"
#include "international_string_util.h"
#include "battle.h"
#include "frontier_util.h"
#include "strings.h"
#include "recorded_battle.h"
#include "easy_chat.h"
#include "gym_leader_rematch.h"
#include "battle_transition.h"
#include "trainer_see.h"
#include "new_game.h"
#include "string_util.h"
#include "data.h"
#include "link.h"
#include "field_message_box.h"
#include "tv.h"
#include "battle_factory.h"
#include "constants/battle_dome.h"
#include "constants/battle_frontier.h"
#include "constants/battle_frontier_mons.h"
#include "constants/battle_tent.h"
#include "constants/battle_tent_mons.h"
#include "constants/battle_tent_trainers.h"
#include "constants/battle_tower.h"
#include "constants/frontier_util.h"
#include "constants/items.h"
#include "constants/trainers.h"
#include "constants/event_objects.h"
#include "constants/moves.h"

extern const u8 MossdeepCity_SpaceCenter_2F_EventScript_MaxieTrainer[];
extern const u8 MossdeepCity_SpaceCenter_2F_EventScript_TabithaTrainer[];

// EWRAM vars.
EWRAM_DATA const struct BattleFrontierTrainer *gFacilityTrainers = NULL;
EWRAM_DATA const struct FacilityMon *gFacilityTrainerMons = NULL;

// IWRAM common
u16 gFrontierTempParty[MAX_FRONTIER_PARTY_SIZE];

// This file's functions.
static void InitTowerChallenge(void);
static void GetTowerData(void);
static void SetTowerData(void);
static void SetNextFacilityOpponent(void);
static void SetTowerBattleWon(void);
static void AwardBattleTowerRibbons(void);
static void SaveTowerChallenge(void);
static void GetOpponentIntroSpeech(void);
static void BattleTowerNop1(void);
static void BattleTowerNop2(void);
static void LoadMultiPartnerCandidatesData(void);
static void ShowPartnerCandidateMessage(void);
static void LoadLinkMultiOpponentsData(void);
static void TowerTryCloseLink(void);
static void SetMultiPartnerGfx(void);
static void SetTowerInterviewData(void);
static void ValidateBattleTowerRecordChecksums(void);
static void SaveCurrentWinStreak(void);
static void SetNextBattleTentOpponent(void);
static void CopyEReaderTrainerFarewellMessage(void);
static void ClearBattleTowerRecord(struct EmeraldBattleTowerRecord *record);
static void FillTrainerParty(u16 trainerId, u8 firstMonId, u8 monCount);
static void FillTentTrainerParty_(u16 trainerId, u8 firstMonId, u8 monCount);
static void FillFactoryFrontierTrainerParty(u16 trainerId, u8 firstMonId);
static void FillFactoryTentTrainerParty(u16 trainerId, u8 firstMonId);
static u8 GetFrontierTrainerFixedIvs(u16 trainerId);
static void FillPartnerParty(u16 trainerId);
static void SetEReaderTrainerChecksum(struct BattleTowerEReaderTrainer *ereaderTrainer);
static u8 SetTentPtrsGetLevel(void);

// Const rom data.
const u16 gBattleFrontierHeldItems[] =
{
    ITEM_NONE,
    ITEM_KINGS_ROCK,
    ITEM_SITRUS_BERRY,
    ITEM_ORAN_BERRY,
    ITEM_CHESTO_BERRY,
    ITEM_HARD_STONE,
    ITEM_FOCUS_BAND,
    ITEM_PERSIM_BERRY,
    ITEM_MIRACLE_SEED,
    ITEM_BERRY_JUICE,
    ITEM_MACHO_BRACE,
    ITEM_SILVER_POWDER,
    ITEM_CHERI_BERRY,
    ITEM_BLACK_GLASSES,
    ITEM_BLACK_BELT,
    ITEM_SOUL_DEW,
    ITEM_CHOICE_BAND,
    ITEM_MAGNET,
    ITEM_SILK_SCARF,
    ITEM_WHITE_HERB,
    ITEM_DEEP_SEA_SCALE,
    ITEM_DEEP_SEA_TOOTH,
    ITEM_MYSTIC_WATER,
    ITEM_SHARP_BEAK,
    ITEM_QUICK_CLAW,
    ITEM_LEFTOVERS,
    ITEM_RAWST_BERRY,
    ITEM_LIGHT_BALL,
    ITEM_POISON_BARB,
    ITEM_NEVER_MELT_ICE,
    ITEM_ASPEAR_BERRY,
    ITEM_SPELL_TAG,
    ITEM_BRIGHT_POWDER,
    ITEM_LEPPA_BERRY,
    ITEM_SCOPE_LENS,
    ITEM_TWISTED_SPOON,
    ITEM_METAL_COAT,
    ITEM_MENTAL_HERB,
    ITEM_CHARCOAL,
    ITEM_PECHA_BERRY,
    ITEM_SOFT_SAND,
    ITEM_LUM_BERRY,
    ITEM_DRAGON_SCALE,
    ITEM_DRAGON_FANG,
    ITEM_IAPAPA_BERRY,
    ITEM_WIKI_BERRY,
    ITEM_SEA_INCENSE,
    ITEM_SHELL_BELL,
    ITEM_SALAC_BERRY,
    ITEM_LANSAT_BERRY,
    ITEM_APICOT_BERRY,
    ITEM_STARF_BERRY,
    ITEM_LIECHI_BERRY,
    ITEM_LEEK,
    ITEM_LAX_INCENSE,
    ITEM_AGUAV_BERRY,
    ITEM_FIGY_BERRY,
    ITEM_THICK_CLUB,
    ITEM_MAGO_BERRY,
    ITEM_METAL_POWDER,
    ITEM_PETAYA_BERRY,
    ITEM_LUCKY_PUNCH,
    ITEM_GANLON_BERRY
};

#include "data/battle_frontier/battle_frontier_trainer_mons.h"
#include "data/battle_frontier/battle_frontier_trainers.h"
#include "data/battle_frontier/battle_frontier_mons.h"

const u8 gTowerMaleFacilityClasses[30] =
{
    FACILITY_CLASS_RUIN_MANIAC,
    FACILITY_CLASS_TUBER_M,
    FACILITY_CLASS_COOLTRAINER_M,
    FACILITY_CLASS_RICH_BOY,
    FACILITY_CLASS_POKEMANIAC,
    FACILITY_CLASS_SWIMMER_M,
    FACILITY_CLASS_BLACK_BELT,
    FACILITY_CLASS_GUITARIST,
    FACILITY_CLASS_KINDLER,
    FACILITY_CLASS_CAMPER,
    FACILITY_CLASS_BUG_MANIAC,
    FACILITY_CLASS_PSYCHIC_M,
    FACILITY_CLASS_GENTLEMAN,
    FACILITY_CLASS_SCHOOL_KID_M,
    FACILITY_CLASS_POKEFAN_M,
    FACILITY_CLASS_EXPERT_M,
    FACILITY_CLASS_YOUNGSTER,
    FACILITY_CLASS_FISHERMAN,
    FACILITY_CLASS_CYCLING_TRIATHLETE_M,
    FACILITY_CLASS_RUNNING_TRIATHLETE_M,
    FACILITY_CLASS_SWIMMING_TRIATHLETE_M,
    FACILITY_CLASS_DRAGON_TAMER,
    FACILITY_CLASS_BIRD_KEEPER,
    FACILITY_CLASS_NINJA_BOY,
    FACILITY_CLASS_SAILOR,
    FACILITY_CLASS_COLLECTOR,
    FACILITY_CLASS_PKMN_BREEDER_M,
    FACILITY_CLASS_PKMN_RANGER_M,
    FACILITY_CLASS_BUG_CATCHER,
    FACILITY_CLASS_HIKER
};

const u8 gTowerFemaleFacilityClasses[20] =
{
    FACILITY_CLASS_AROMA_LADY,
    FACILITY_CLASS_TUBER_F,
    FACILITY_CLASS_COOLTRAINER_F,
    FACILITY_CLASS_HEX_MANIAC,
    FACILITY_CLASS_LADY,
    FACILITY_CLASS_BEAUTY,
    FACILITY_CLASS_PSYCHIC_F,
    FACILITY_CLASS_SCHOOL_KID_F,
    FACILITY_CLASS_POKEFAN_F,
    FACILITY_CLASS_EXPERT_F,
    FACILITY_CLASS_CYCLING_TRIATHLETE_F,
    FACILITY_CLASS_RUNNING_TRIATHLETE_F,
    FACILITY_CLASS_SWIMMING_TRIATHLETE_F,
    FACILITY_CLASS_BATTLE_GIRL,
    FACILITY_CLASS_PARASOL_LADY,
    FACILITY_CLASS_SWIMMER_F,
    FACILITY_CLASS_PICNICKER,
    FACILITY_CLASS_PKMN_BREEDER_F,
    FACILITY_CLASS_PKMN_RANGER_F,
    FACILITY_CLASS_LASS
};

const u16 gTowerMaleTrainerGfxIds[30] =
{
    OBJ_EVENT_GFX_HIKER,
    OBJ_EVENT_GFX_TUBER_M,
    OBJ_EVENT_GFX_MAN_3,
    OBJ_EVENT_GFX_RICH_BOY,
    OBJ_EVENT_GFX_MANIAC,
    OBJ_EVENT_GFX_RUNNING_TRIATHLETE_M,
    OBJ_EVENT_GFX_BLACK_BELT,
    OBJ_EVENT_GFX_MAN_5,
    OBJ_EVENT_GFX_MAN_5,
    OBJ_EVENT_GFX_CAMPER,
    OBJ_EVENT_GFX_MANIAC,
    OBJ_EVENT_GFX_PSYCHIC_M,
    OBJ_EVENT_GFX_GENTLEMAN,
    OBJ_EVENT_GFX_SCHOOL_KID_M,
    OBJ_EVENT_GFX_POKEFAN_M,
    OBJ_EVENT_GFX_EXPERT_M,
    OBJ_EVENT_GFX_YOUNGSTER,
    OBJ_EVENT_GFX_FISHERMAN,
    OBJ_EVENT_GFX_CYCLING_TRIATHLETE_M,
    OBJ_EVENT_GFX_RUNNING_TRIATHLETE_M,
    OBJ_EVENT_GFX_RUNNING_TRIATHLETE_M,
    OBJ_EVENT_GFX_MAN_3,
    OBJ_EVENT_GFX_MAN_5,
    OBJ_EVENT_GFX_NINJA_BOY,
    OBJ_EVENT_GFX_SAILOR,
    OBJ_EVENT_GFX_MANIAC,
    OBJ_EVENT_GFX_MAN_4,
    OBJ_EVENT_GFX_CAMPER,
    OBJ_EVENT_GFX_BUG_CATCHER,
    OBJ_EVENT_GFX_HIKER
};

const u16 gTowerFemaleTrainerGfxIds[20] =
{
    OBJ_EVENT_GFX_WOMAN_2,
    OBJ_EVENT_GFX_TUBER_F,
    OBJ_EVENT_GFX_WOMAN_5,
    OBJ_EVENT_GFX_HEX_MANIAC,
    OBJ_EVENT_GFX_WOMAN_2,
    OBJ_EVENT_GFX_BEAUTY,
    OBJ_EVENT_GFX_LASS,
    OBJ_EVENT_GFX_GIRL_3,
    OBJ_EVENT_GFX_POKEFAN_F,
    OBJ_EVENT_GFX_EXPERT_F,
    OBJ_EVENT_GFX_CYCLING_TRIATHLETE_F,
    OBJ_EVENT_GFX_RUNNING_TRIATHLETE_F,
    OBJ_EVENT_GFX_RUNNING_TRIATHLETE_F,
    OBJ_EVENT_GFX_GIRL_3,
    OBJ_EVENT_GFX_WOMAN_5,
    OBJ_EVENT_GFX_RUNNING_TRIATHLETE_F,
    OBJ_EVENT_GFX_PICNICKER,
    OBJ_EVENT_GFX_WOMAN_2,
    OBJ_EVENT_GFX_PICNICKER,
    OBJ_EVENT_GFX_LASS
};

// Excludes the unused RS_FACILITY_CLASS_BOARDER_1 and _2
static const u8 sRubyFacilityClassToEmerald[RS_FACILITY_CLASSES_COUNT - 2][2] =
{
    {RS_FACILITY_CLASS_AQUA_LEADER_ARCHIE, FACILITY_CLASS_AQUA_LEADER_ARCHIE},
    {RS_FACILITY_CLASS_AQUA_GRUNT_M, FACILITY_CLASS_AQUA_GRUNT_M},
    {RS_FACILITY_CLASS_AQUA_GRUNT_F, FACILITY_CLASS_AQUA_GRUNT_F},
    {RS_FACILITY_CLASS_AROMA_LADY, FACILITY_CLASS_AROMA_LADY},
    {RS_FACILITY_CLASS_RUIN_MANIAC, FACILITY_CLASS_RUIN_MANIAC},
    {RS_FACILITY_CLASS_INTERVIEWER, FACILITY_CLASS_INTERVIEWER},
    {RS_FACILITY_CLASS_TUBER_F, FACILITY_CLASS_TUBER_F},
    {RS_FACILITY_CLASS_TUBER_M, FACILITY_CLASS_TUBER_M},
    {RS_FACILITY_CLASS_COOLTRAINER_M, FACILITY_CLASS_COOLTRAINER_M},
    {RS_FACILITY_CLASS_COOLTRAINER_F, FACILITY_CLASS_COOLTRAINER_F},
    {RS_FACILITY_CLASS_HEX_MANIAC, FACILITY_CLASS_HEX_MANIAC},
    {RS_FACILITY_CLASS_LADY, FACILITY_CLASS_LADY},
    {RS_FACILITY_CLASS_BEAUTY, FACILITY_CLASS_BEAUTY},
    {RS_FACILITY_CLASS_RICH_BOY, FACILITY_CLASS_RICH_BOY},
    {RS_FACILITY_CLASS_POKEMANIAC, FACILITY_CLASS_POKEMANIAC},
    {RS_FACILITY_CLASS_SWIMMER_M, FACILITY_CLASS_SWIMMER_M},
    {RS_FACILITY_CLASS_BLACK_BELT, FACILITY_CLASS_BLACK_BELT},
    {RS_FACILITY_CLASS_GUITARIST, FACILITY_CLASS_GUITARIST},
    {RS_FACILITY_CLASS_KINDLER, FACILITY_CLASS_KINDLER},
    {RS_FACILITY_CLASS_CAMPER, FACILITY_CLASS_CAMPER},
    {RS_FACILITY_CLASS_BUG_MANIAC, FACILITY_CLASS_BUG_MANIAC},
    {RS_FACILITY_CLASS_PSYCHIC_M, FACILITY_CLASS_PSYCHIC_M},
    {RS_FACILITY_CLASS_PSYCHIC_F, FACILITY_CLASS_PSYCHIC_F},
    {RS_FACILITY_CLASS_GENTLEMAN, FACILITY_CLASS_GENTLEMAN},
    {RS_FACILITY_CLASS_ELITE_FOUR_M, FACILITY_CLASS_ELITE_FOUR_SIDNEY},
    {RS_FACILITY_CLASS_ELITE_FOUR_F, FACILITY_CLASS_ELITE_FOUR_PHOEBE},
    {RS_FACILITY_CLASS_LEADER_F, FACILITY_CLASS_LEADER_ROXANNE},
    {RS_FACILITY_CLASS_LEADER_M, FACILITY_CLASS_LEADER_BRAWLY},
    {RS_FACILITY_CLASS_LEADER_MF, FACILITY_CLASS_LEADER_TATE_AND_LIZA},
    {RS_FACILITY_CLASS_SCHOOL_KID_M, FACILITY_CLASS_SCHOOL_KID_M},
    {RS_FACILITY_CLASS_SCHOOL_KID_F, FACILITY_CLASS_SCHOOL_KID_F},
    {RS_FACILITY_CLASS_SR_AND_JR, FACILITY_CLASS_SR_AND_JR},
    {RS_FACILITY_CLASS_POKEFAN_M, FACILITY_CLASS_POKEFAN_M},
    {RS_FACILITY_CLASS_POKEFAN_F, FACILITY_CLASS_POKEFAN_F},
    {RS_FACILITY_CLASS_EXPERT_M, FACILITY_CLASS_EXPERT_M},
    {RS_FACILITY_CLASS_EXPERT_F, FACILITY_CLASS_EXPERT_F},
    {RS_FACILITY_CLASS_YOUNGSTER, FACILITY_CLASS_YOUNGSTER},
    {RS_FACILITY_CLASS_CHAMPION, FACILITY_CLASS_CHAMPION_WALLACE},
    {RS_FACILITY_CLASS_FISHERMAN, FACILITY_CLASS_FISHERMAN},
    {RS_FACILITY_CLASS_CYCLING_TRIATHLETE_M, FACILITY_CLASS_CYCLING_TRIATHLETE_M},
    {RS_FACILITY_CLASS_CYCLING_TRIATHLETE_F, FACILITY_CLASS_CYCLING_TRIATHLETE_F},
    {RS_FACILITY_CLASS_RUNNING_TRIATHLETE_M, FACILITY_CLASS_RUNNING_TRIATHLETE_M},
    {RS_FACILITY_CLASS_RUNNING_TRIATHLETE_F, FACILITY_CLASS_RUNNING_TRIATHLETE_F},
    {RS_FACILITY_CLASS_SWIMMING_TRIATHLETE_M, FACILITY_CLASS_SWIMMING_TRIATHLETE_M},
    {RS_FACILITY_CLASS_SWIMMING_TRIATHLETE_F, FACILITY_CLASS_SWIMMING_TRIATHLETE_F},
    {RS_FACILITY_CLASS_DRAGON_TAMER, FACILITY_CLASS_DRAGON_TAMER},
    {RS_FACILITY_CLASS_BIRD_KEEPER, FACILITY_CLASS_BIRD_KEEPER},
    {RS_FACILITY_CLASS_NINJA_BOY, FACILITY_CLASS_NINJA_BOY},
    {RS_FACILITY_CLASS_BATTLE_GIRL, FACILITY_CLASS_BATTLE_GIRL},
    {RS_FACILITY_CLASS_PARASOL_LADY, FACILITY_CLASS_PARASOL_LADY},
    {RS_FACILITY_CLASS_SWIMMER_F, FACILITY_CLASS_SWIMMER_F},
    {RS_FACILITY_CLASS_PICNICKER, FACILITY_CLASS_PICNICKER},
    {RS_FACILITY_CLASS_TWINS, FACILITY_CLASS_TWINS},
    {RS_FACILITY_CLASS_SAILOR, FACILITY_CLASS_SAILOR},
    {RS_FACILITY_CLASS_COLLECTOR, FACILITY_CLASS_COLLECTOR},
    {RS_FACILITY_CLASS_WALLY, FACILITY_CLASS_WALLY},
    {RS_FACILITY_CLASS_BRENDAN_1, FACILITY_CLASS_BRENDAN},
    {RS_FACILITY_CLASS_BRENDAN_2, FACILITY_CLASS_BRENDAN_2},
    {RS_FACILITY_CLASS_BRENDAN_3, FACILITY_CLASS_BRENDAN_3},
    {RS_FACILITY_CLASS_MAY_1, FACILITY_CLASS_MAY},
    {RS_FACILITY_CLASS_MAY_2, FACILITY_CLASS_MAY_2},
    {RS_FACILITY_CLASS_MAY_3, FACILITY_CLASS_MAY_3},
    {RS_FACILITY_CLASS_PKMN_BREEDER_M, FACILITY_CLASS_PKMN_BREEDER_M},
    {RS_FACILITY_CLASS_PKMN_BREEDER_F, FACILITY_CLASS_PKMN_BREEDER_F},
    {RS_FACILITY_CLASS_PKMN_RANGER_M, FACILITY_CLASS_PKMN_RANGER_M},
    {RS_FACILITY_CLASS_PKMN_RANGER_F, FACILITY_CLASS_PKMN_RANGER_F},
    {RS_FACILITY_CLASS_MAGMA_LEADER, FACILITY_CLASS_MAGMA_LEADER_MAXIE},
    {RS_FACILITY_CLASS_MAGMA_GRUNT_M, FACILITY_CLASS_MAGMA_GRUNT_M},
    {RS_FACILITY_CLASS_MAGMA_GRUNT_F, FACILITY_CLASS_MAGMA_GRUNT_F},
    {RS_FACILITY_CLASS_LASS, FACILITY_CLASS_LASS},
    {RS_FACILITY_CLASS_BUG_CATCHER, FACILITY_CLASS_BUG_CATCHER},
    {RS_FACILITY_CLASS_HIKER, FACILITY_CLASS_HIKER},
    {RS_FACILITY_CLASS_YOUNG_COUPLE, FACILITY_CLASS_YOUNG_COUPLE},
    {RS_FACILITY_CLASS_OLD_COUPLE, FACILITY_CLASS_OLD_COUPLE},
    {RS_FACILITY_CLASS_SIS_AND_BRO, FACILITY_CLASS_SIS_AND_BRO},
};

#define PARTNER_TEXTS(name)                                             \
    BattleFrontier_BattleTowerMultiPartnerRoom_Text_##name##Intro,      \
    BattleFrontier_BattleTowerMultiPartnerRoom_Text_##name##Mon1,       \
    BattleFrontier_BattleTowerMultiPartnerRoom_Text_##name##Mon2Ask,    \
    BattleFrontier_BattleTowerMultiPartnerRoom_Text_##name##Accept,     \
    BattleFrontier_BattleTowerMultiPartnerRoom_Text_##name##Reject

static const u8 *const sPartnerTextsLass[] =
{
    PARTNER_TEXTS(Lass)
};

static const u8 *const sPartnerTextsYoungster[] =
{
    PARTNER_TEXTS(Youngster)
};

static const u8 *const sPartnerTextsHiker[] =
{
    PARTNER_TEXTS(Hiker)
};

static const u8 *const sPartnerTextsBeauty[] =
{
    PARTNER_TEXTS(Beauty)
};

static const u8 *const sPartnerTextsFisherman[] =
{
    PARTNER_TEXTS(Fisherman)
};

static const u8 *const sPartnerTextsLady[] =
{
    PARTNER_TEXTS(Lady)
};

static const u8 *const sPartnerTextsCyclingTriathleteF[] =
{
    PARTNER_TEXTS(CyclingTriathleteF)
};

static const u8 *const sPartnerTextsBugCatcher[] =
{
    PARTNER_TEXTS(BugCatcher)
};

static const u8 *const sPartnerTextsSchoolKidM[] =
{
    PARTNER_TEXTS(SchoolKidM)
};

static const u8 *const sPartnerTextsRichBoy[] =
{
    PARTNER_TEXTS(RichBoy)
};

static const u8 *const sPartnerTextsBlackBelt[] =
{
    PARTNER_TEXTS(BlackBelt)
};

static const u8 *const sPartnerTextsTuberF[] =
{
    PARTNER_TEXTS(TuberF)
};

static const u8 *const sPartnerTextsHexManiac[] =
{
    PARTNER_TEXTS(HexManiac)
};

static const u8 *const sPartnerTextsPkmnBreederM[] =
{
    PARTNER_TEXTS(PkmnBreederM)
};

static const u8 *const sPartnerTextsRunningTriathleteF[] =
{
    PARTNER_TEXTS(RunningTriathleteF)
};

static const u8 *const sPartnerTextsRunningTriathleteM[] =
{
    PARTNER_TEXTS(RunningTriathleteM)
};

static const u8 *const sPartnerTextsBattleGirl[] =
{
    PARTNER_TEXTS(BattleGirl)
};

static const u8 *const sPartnerTextsCyclingTriathleteM[] =
{
    PARTNER_TEXTS(CyclingTriathleteM)
};

static const u8 *const sPartnerTextsTuberM[] =
{
    PARTNER_TEXTS(TuberM)
};

static const u8 *const sPartnerTextsGuitarist[] =
{
    PARTNER_TEXTS(Guitarist)
};

static const u8 *const sPartnerTextsGentleman[] =
{
    PARTNER_TEXTS(Gentleman)
};

static const u8 *const sPartnerTextsPokefanM[] =
{
    PARTNER_TEXTS(PokefanM)
};

static const u8 *const sPartnerTextsExpertM[] =
{
    PARTNER_TEXTS(ExpertM)
};

static const u8 *const sPartnerTextsExpertF[] =
{
    PARTNER_TEXTS(ExpertF)
};

static const u8 *const sPartnerTextsDragonTamer[] =
{
    PARTNER_TEXTS(DragonTamer)
};

static const u8 *const sPartnerTextsBirdKeeper[] =
{
    PARTNER_TEXTS(BirdKeeper)
};

static const u8 *const sPartnerTextsNinjaBoy[] =
{
    PARTNER_TEXTS(NinjaBoy)
};

static const u8 *const sPartnerTextsParasolLady[] =
{
    PARTNER_TEXTS(ParasolLady)
};

static const u8 *const sPartnerTextsBugManiac[] =
{
    PARTNER_TEXTS(BugManiac)
};

static const u8 *const sPartnerTextsSailor[] =
{
    PARTNER_TEXTS(Sailor)
};

static const u8 *const sPartnerTextsCollector[] =
{
    PARTNER_TEXTS(Collector)
};

static const u8 *const sPartnerTextsPkmnRangerM[] =
{
    PARTNER_TEXTS(PkmnRangerM)
};

static const u8 *const sPartnerTextsPkmnRangerF[] =
{
    PARTNER_TEXTS(PkmnRangerF)
};

static const u8 *const sPartnerTextsAromaLady[] =
{
    PARTNER_TEXTS(AromaLady)
};

static const u8 *const sPartnerTextsRuinManiac[] =
{
    PARTNER_TEXTS(RuinManiac)
};

static const u8 *const sPartnerTextsCoolTrainerM[] =
{
    PARTNER_TEXTS(CoolTrainerM)
};

static const u8 *const sPartnerTextsCoolTrainerF[] =
{
    PARTNER_TEXTS(CoolTrainerF)
};

static const u8 *const sPartnerTextsPokemaniac[] =
{
    PARTNER_TEXTS(Pokemaniac)
};

static const u8 *const sPartnerTextsKindler[] =
{
    PARTNER_TEXTS(Kindler)
};

static const u8 *const sPartnerTextsCamper[] =
{
    PARTNER_TEXTS(Camper)
};

static const u8 *const sPartnerTextsPicnicker[] =
{
    PARTNER_TEXTS(Picnicker)
};

static const u8 *const sPartnerTextsPsychicM[] =
{
    PARTNER_TEXTS(PsychicM)
};

static const u8 *const sPartnerTextsPsychicF[] =
{
    PARTNER_TEXTS(PsychicF)
};

static const u8 *const sPartnerTextsSchoolKidF[] =
{
    PARTNER_TEXTS(SchoolKidF)
};

static const u8 *const sPartnerTextsPkmnBreederF[] =
{
    PARTNER_TEXTS(PkmnBreederF)
};

static const u8 *const sPartnerTextsPokefanF[] =
{
    PARTNER_TEXTS(PokefanF)
};

static const u8 *const sPartnerTextsSwimmerF[] =
{
    PARTNER_TEXTS(SwimmerF)
};

static const u8 *const sPartnerTextsSwimmingTriathleteM[] =
{
    PARTNER_TEXTS(SwimmingTriathleteM)
};

static const u8 *const sPartnerTextsSwimmingTriathleteF[] =
{
    PARTNER_TEXTS(SwimmingTriathleteF)
};

static const u8 *const sPartnerTextsSwimmerM[] =
{
    PARTNER_TEXTS(SwimmerM)
};

struct
{
    u32 facilityClass;
    const u8 *const *strings;
} static const sPartnerTrainerTextTables[] =
{
    {FACILITY_CLASS_LASS,                  sPartnerTextsLass},
    {FACILITY_CLASS_YOUNGSTER,             sPartnerTextsYoungster},
    {FACILITY_CLASS_HIKER,                 sPartnerTextsHiker},
    {FACILITY_CLASS_BEAUTY,                sPartnerTextsBeauty},
    {FACILITY_CLASS_FISHERMAN,             sPartnerTextsFisherman},
    {FACILITY_CLASS_LADY,                  sPartnerTextsLady},
    {FACILITY_CLASS_CYCLING_TRIATHLETE_F,  sPartnerTextsCyclingTriathleteF},
    {FACILITY_CLASS_BUG_CATCHER,           sPartnerTextsBugCatcher},
    {FACILITY_CLASS_SCHOOL_KID_M,          sPartnerTextsSchoolKidM},
    {FACILITY_CLASS_RICH_BOY,              sPartnerTextsRichBoy},
    {FACILITY_CLASS_BLACK_BELT,            sPartnerTextsBlackBelt},
    {FACILITY_CLASS_TUBER_F,               sPartnerTextsTuberF},
    {FACILITY_CLASS_HEX_MANIAC,            sPartnerTextsHexManiac},
    {FACILITY_CLASS_PKMN_BREEDER_M,        sPartnerTextsPkmnBreederM},
    {FACILITY_CLASS_RUNNING_TRIATHLETE_F,  sPartnerTextsRunningTriathleteF},
    {FACILITY_CLASS_RUNNING_TRIATHLETE_M,  sPartnerTextsRunningTriathleteM},
    {FACILITY_CLASS_BATTLE_GIRL,           sPartnerTextsBattleGirl},
    {FACILITY_CLASS_CYCLING_TRIATHLETE_M,  sPartnerTextsCyclingTriathleteM},
    {FACILITY_CLASS_TUBER_M,               sPartnerTextsTuberM},
    {FACILITY_CLASS_GUITARIST,             sPartnerTextsGuitarist},
    {FACILITY_CLASS_GENTLEMAN,             sPartnerTextsGentleman},
    {FACILITY_CLASS_POKEFAN_M,             sPartnerTextsPokefanM},
    {FACILITY_CLASS_EXPERT_M,              sPartnerTextsExpertM},
    {FACILITY_CLASS_EXPERT_F,              sPartnerTextsExpertF},
    {FACILITY_CLASS_DRAGON_TAMER,          sPartnerTextsDragonTamer},
    {FACILITY_CLASS_BIRD_KEEPER,           sPartnerTextsBirdKeeper},
    {FACILITY_CLASS_NINJA_BOY,             sPartnerTextsNinjaBoy},
    {FACILITY_CLASS_PARASOL_LADY,          sPartnerTextsParasolLady},
    {FACILITY_CLASS_BUG_MANIAC,            sPartnerTextsBugManiac},
    {FACILITY_CLASS_SAILOR,                sPartnerTextsSailor},
    {FACILITY_CLASS_COLLECTOR,             sPartnerTextsCollector},
    {FACILITY_CLASS_PKMN_RANGER_M,         sPartnerTextsPkmnRangerM},
    {FACILITY_CLASS_PKMN_RANGER_F,         sPartnerTextsPkmnRangerF},
    {FACILITY_CLASS_AROMA_LADY,            sPartnerTextsAromaLady},
    {FACILITY_CLASS_RUIN_MANIAC,           sPartnerTextsRuinManiac},
    {FACILITY_CLASS_COOLTRAINER_M,         sPartnerTextsCoolTrainerM},
    {FACILITY_CLASS_COOLTRAINER_F,         sPartnerTextsCoolTrainerF},
    {FACILITY_CLASS_POKEMANIAC,            sPartnerTextsPokemaniac},
    {FACILITY_CLASS_KINDLER,               sPartnerTextsKindler},
    {FACILITY_CLASS_CAMPER,                sPartnerTextsCamper},
    {FACILITY_CLASS_PICNICKER,             sPartnerTextsPicnicker},
    {FACILITY_CLASS_PSYCHIC_M,             sPartnerTextsPsychicM},
    {FACILITY_CLASS_PSYCHIC_F,             sPartnerTextsPsychicF},
    {FACILITY_CLASS_SCHOOL_KID_F,          sPartnerTextsSchoolKidF},
    {FACILITY_CLASS_PKMN_BREEDER_F,        sPartnerTextsPkmnBreederF},
    {FACILITY_CLASS_POKEFAN_F,             sPartnerTextsPokefanF},
    {FACILITY_CLASS_SWIMMER_F,             sPartnerTextsSwimmerF},
    {FACILITY_CLASS_SWIMMING_TRIATHLETE_M, sPartnerTextsSwimmingTriathleteM},
    {FACILITY_CLASS_SWIMMING_TRIATHLETE_F, sPartnerTextsSwimmingTriathleteF},
    {FACILITY_CLASS_SWIMMER_M,             sPartnerTextsSwimmerM}
};

struct
{
    u16 species;
    u8 fixedIV;
    u8 level;
    u8 nature;
    u8 evs[NUM_STATS];
    u16 moves[MAX_MON_MOVES];
} static const sStevenMons[MULTI_PARTY_SIZE] =
{
    {
        .species = SPECIES_METANG,
        .fixedIV = MAX_PER_STAT_IVS,
        .level = 42,
        .nature = NATURE_BRAVE,
        .evs = {0, 252, 252, 0, 6, 0},
        .moves = {MOVE_LIGHT_SCREEN, MOVE_PSYCHIC, MOVE_REFLECT, MOVE_METAL_CLAW}
    },
    {
        .species = SPECIES_SKARMORY,
        .fixedIV = MAX_PER_STAT_IVS,
        .level = 43,
        .nature = NATURE_IMPISH,
        .evs = {252, 0, 0, 0, 6, 252},
        .moves = {MOVE_TOXIC, MOVE_AERIAL_ACE, MOVE_PROTECT, MOVE_STEEL_WING}
    },
    {
        .species = SPECIES_AGGRON,
        .fixedIV = MAX_PER_STAT_IVS,
        .level = 44,
        .nature = NATURE_ADAMANT,
        .evs = {0, 252, 0, 0, 252, 6},
        .moves = {MOVE_THUNDER, MOVE_PROTECT, MOVE_SOLAR_BEAM, MOVE_DRAGON_CLAW}
    }
};

#include "data/battle_frontier/battle_tent.h"

static void (* const sBattleTowerFuncs[])(void) =
{
    [BATTLE_TOWER_FUNC_INIT]                = InitTowerChallenge,
    [BATTLE_TOWER_FUNC_GET_DATA]            = GetTowerData,
    [BATTLE_TOWER_FUNC_SET_DATA]            = SetTowerData,
    [BATTLE_TOWER_FUNC_SET_OPPONENT]        = SetNextFacilityOpponent,
    [BATTLE_TOWER_FUNC_SET_BATTLE_WON]      = SetTowerBattleWon,
    [BATTLE_TOWER_FUNC_GIVE_RIBBONS]        = AwardBattleTowerRibbons,
    [BATTLE_TOWER_FUNC_SAVE]                = SaveTowerChallenge,
    [BATTLE_TOWER_FUNC_GET_OPPONENT_INTRO]  = GetOpponentIntroSpeech,
    [BATTLE_TOWER_FUNC_NOP]                 = BattleTowerNop1,
    [BATTLE_TOWER_FUNC_NOP2]                = BattleTowerNop2,
    [BATTLE_TOWER_FUNC_LOAD_PARTNERS]       = LoadMultiPartnerCandidatesData,
    [BATTLE_TOWER_FUNC_PARTNER_MSG]         = ShowPartnerCandidateMessage,
    [BATTLE_TOWER_FUNC_LOAD_LINK_OPPONENTS] = LoadLinkMultiOpponentsData,
    [BATTLE_TOWER_FUNC_TRY_CLOSE_LINK]      = TowerTryCloseLink,
    [BATTLE_TOWER_FUNC_SET_PARTNER_GFX]     = SetMultiPartnerGfx,
    [BATTLE_TOWER_FUNC_SET_INTERVIEW_DATA]  = SetTowerInterviewData,
};

static const u32 sWinStreakFlags[][2] =
{
    {STREAK_TOWER_SINGLES_50,     STREAK_TOWER_SINGLES_OPEN},
    {STREAK_TOWER_DOUBLES_50,     STREAK_TOWER_DOUBLES_OPEN},
    {STREAK_TOWER_MULTIS_50,      STREAK_TOWER_MULTIS_OPEN},
    {STREAK_TOWER_LINK_MULTIS_50, STREAK_TOWER_LINK_MULTIS_OPEN},
};

static const u32 sWinStreakMasks[][2] =
{
    {~(STREAK_TOWER_SINGLES_50),     ~(STREAK_TOWER_SINGLES_OPEN)},
    {~(STREAK_TOWER_DOUBLES_50),     ~(STREAK_TOWER_DOUBLES_OPEN)},
    {~(STREAK_TOWER_MULTIS_50),      ~(STREAK_TOWER_MULTIS_OPEN)},
    {~(STREAK_TOWER_LINK_MULTIS_50), ~(STREAK_TOWER_LINK_MULTIS_OPEN)},
};

// Unclear why this was duplicated
static const u8 sBattleTowerPartySizes2[] =
{
    [FRONTIER_MODE_SINGLES]     = FRONTIER_PARTY_SIZE,
    [FRONTIER_MODE_DOUBLES]     = FRONTIER_DOUBLES_PARTY_SIZE,
    [FRONTIER_MODE_MULTIS]      = FRONTIER_MULTI_PARTY_SIZE,
    [FRONTIER_MODE_LINK_MULTIS] = FRONTIER_MULTI_PARTY_SIZE,
};

// Trainer ID ranges for possible frontier trainers to encounter on particular challenges
// Trainers are scaled by difficulty, so higher trainer IDs have better teams
static const u16 sFrontierTrainerIdRanges[][2] =
{
    {FRONTIER_TRAINER_BRADY,   FRONTIER_TRAINER_JILL},   //   0 -  99
    {FRONTIER_TRAINER_TREVIN,  FRONTIER_TRAINER_CHLOE},  //  80 - 119
    {FRONTIER_TRAINER_ERIK,    FRONTIER_TRAINER_SOFIA},  // 100 - 139
    {FRONTIER_TRAINER_NORTON,  FRONTIER_TRAINER_JAZLYN}, // 120 - 159
    {FRONTIER_TRAINER_BRADEN,  FRONTIER_TRAINER_ALISON}, // 140 - 179
    {FRONTIER_TRAINER_ZACHERY, FRONTIER_TRAINER_LAMAR},  // 160 - 199
    {FRONTIER_TRAINER_HANK,    FRONTIER_TRAINER_TESS},   // 180 - 219
    {FRONTIER_TRAINER_JAXON,   FRONTIER_TRAINER_GRETEL}, // 200 - 299
};

static const u16 sFrontierTrainerIdRangesHard[][2] =
{
    {FRONTIER_TRAINER_ERIK,    FRONTIER_TRAINER_CHLOE},  // 100 - 119
    {FRONTIER_TRAINER_NORTON,  FRONTIER_TRAINER_SOFIA},  // 120 - 139
    {FRONTIER_TRAINER_BRADEN,  FRONTIER_TRAINER_JAZLYN}, // 140 - 159
    {FRONTIER_TRAINER_ZACHERY, FRONTIER_TRAINER_ALISON}, // 160 - 179
    {FRONTIER_TRAINER_HANK,    FRONTIER_TRAINER_LAMAR},  // 180 - 199
    {FRONTIER_TRAINER_JAXON,   FRONTIER_TRAINER_TESS},   // 200 - 219
    {FRONTIER_TRAINER_LEON,    FRONTIER_TRAINER_RAUL},   // 220 - 239
    {FRONTIER_TRAINER_JAXON,   FRONTIER_TRAINER_GRETEL}, // 200 - 299
};

static const u8 sBattleTowerPartySizes[FRONTIER_MODE_COUNT] =
{
    [FRONTIER_MODE_SINGLES]     = FRONTIER_PARTY_SIZE,
    [FRONTIER_MODE_DOUBLES]     = FRONTIER_DOUBLES_PARTY_SIZE,
    [FRONTIER_MODE_MULTIS]      = FRONTIER_MULTI_PARTY_SIZE,
    [FRONTIER_MODE_LINK_MULTIS] = FRONTIER_MULTI_PARTY_SIZE,
};

static const u16 sRecordTrainerSpeechWon[] =
{
    EC_WORD_YAY, EC_WORD_YAY, EC_WORD_EXCL_EXCL, EC_WORD_I_VE, EC_WORD_WON, EC_WORD_EXCL_EXCL
};

static const u16 sRecordTrainerSpeechLost[] =
{
    EC_WORD_TOO, EC_WORD_BAD, EC_WORD_ELLIPSIS, EC_WORD_WE, EC_WORD_LOST, EC_WORD_ELLIPSIS
};

// code
void CallBattleTowerFunc(void)
{
    sBattleTowerFuncs[gSpecialVar_0x8004]();
}

static void InitTowerChallenge(void)
{
}

static void GetTowerData(void)
{
}

static void SetTowerData(void)
{
}

static void SetTowerBattleWon(void)
{
}

static bool8 ChooseSpecialBattleTowerTrainer(void)
{
}

static void SetNextFacilityOpponent(void)
{
}

u16 GetRandomScaledFrontierTrainerId(u8 challengeNum, u8 battleNum)
{
    u16 trainerId;

    if (challengeNum <= 7)
    {
        if (battleNum == FRONTIER_STAGES_PER_CHALLENGE - 1)
        {
            // The last battle in each challenge has a jump in difficulty, pulls from a table with higher ranges
            trainerId = (sFrontierTrainerIdRangesHard[challengeNum][1] - sFrontierTrainerIdRangesHard[challengeNum][0]) + 1;
            trainerId = sFrontierTrainerIdRangesHard[challengeNum][0] + (Random() % trainerId);
        }
        else
        {
            trainerId = (sFrontierTrainerIdRanges[challengeNum][1] - sFrontierTrainerIdRanges[challengeNum][0]) + 1;
            trainerId = sFrontierTrainerIdRanges[challengeNum][0] + (Random() % trainerId);
        }
    }
    else
    {
        // After challenge 7, trainer IDs always come from the last, hardest range, which is the same for both trainer ID tables
        trainerId = (sFrontierTrainerIdRanges[7][1] - sFrontierTrainerIdRanges[7][0]) + 1;
        trainerId = sFrontierTrainerIdRanges[7][0] + (Random() % trainerId);
    }

    return trainerId;
}

void SetBattleFacilityTrainerGfxId(u16 trainerId, u8 tempVarId)
{
}

void SetEReaderTrainerGfxId(void)
{
    SetBattleFacilityTrainerGfxId(TRAINER_EREADER, 0);
}

u16 GetBattleFacilityTrainerGfxId(u16 trainerId)
{
}

void PutNewBattleTowerRecord(struct EmeraldBattleTowerRecord *newRecordEm)
{
}

u8 GetFrontierTrainerFrontSpriteId(u16 trainerId)
{
}

u8 GetFrontierOpponentClass(u16 trainerId)
{
}

static u8 GetFrontierTrainerFacilityClass(u16 trainerId)
{
}

void GetFrontierTrainerName(u8 *dst, u16 trainerId)
{
}

static bool8 IsFrontierTrainerFemale(u16 trainerId)
{
}

void FillFrontierTrainerParty(u8 monsCount)
{
    ZeroEnemyPartyMons();
    FillTrainerParty(gTrainerBattleOpponent_A, 0, monsCount);
}

void FillFrontierTrainersParties(u8 monsCount)
{
    ZeroEnemyPartyMons();
    FillTrainerParty(gTrainerBattleOpponent_A, 0, monsCount);
    FillTrainerParty(gTrainerBattleOpponent_B, 3, monsCount);
}

static void FillTentTrainerParty(u8 monsCount)
{
    ZeroEnemyPartyMons();
    FillTentTrainerParty_(gTrainerBattleOpponent_A, 0, monsCount);
}

static void FillTrainerParty(u16 trainerId, u8 firstMonId, u8 monCount)
{
}

u16 GetRandomFrontierMonFromSet(u16 trainerId)
{
    u8 level = SetFacilityPtrsGetLevel();
    const u16 *monSet = gFacilityTrainers[trainerId].monSet;
    u8 numMons = 0;
    u32 monId = monSet[numMons];

    while (monId != 0xFFFF)
    {
        numMons++;
        monId = monSet[numMons];
        if (monId == 0xFFFF)
            break;
    }

    do
    {
        // "High tier" pokemon are only allowed on open level mode
        // 20 is not a possible value for level here
        monId = monSet[Random() % numMons];
    } while((level == FRONTIER_MAX_LEVEL_50 || level == 20) && monId > FRONTIER_MONS_HIGH_TIER);

    return monId;
}

static void FillFactoryTrainerParty(void)
{
}

static void FillFactoryFrontierTrainerParty(u16 trainerId, u8 firstMonId)
{
}

static void FillFactoryTentTrainerParty(u16 trainerId, u8 firstMonId)
{
}

void FrontierSpeechToString(const u16 *words)
{
    ConvertEasyChatWordsToString(gStringVar4, words, 3, 2);
    if (GetStringWidth(FONT_NORMAL, gStringVar4, -1) > 204u)
    {
        s32 i = 0;

        ConvertEasyChatWordsToString(gStringVar4, words, 2, 3);
        while (gStringVar4[i++] != CHAR_NEWLINE)
            ;
        while (gStringVar4[i] != CHAR_NEWLINE)
            i++;

        gStringVar4[i] = CHAR_PROMPT_SCROLL;
    }
}

static void GetOpponentIntroSpeech(void)
{
}

static void HandleSpecialTrainerBattleEnd(void)
{
}
static void Task_StartBattleAfterTransition(u8 taskId)
{
    if (IsBattleTransitionDone() == TRUE)
    {
        gMain.savedCallback = HandleSpecialTrainerBattleEnd;
        SetMainCallback2(CB2_InitBattle);
        DestroyTask(taskId);
    }
}

void DoSpecialTrainerBattle(void)
{
}

static void SaveCurrentWinStreak(void)
{
}

static void SaveBattleTowerRecord(void)
{
}

static void SaveTowerChallenge(void)
{
}

static void BattleTowerNop1(void)
{

}

static void BattleTowerNop2(void)
{

}

static void GetRecordMixFriendMultiPartnerParty(u16 trainerId)
{
}

static void LoadMultiPartnerCandidatesData(void)
{
}

static void GetPotentialPartnerMoveAndSpecies(u16 trainerId, u16 monId)
{
    u16 move = 0;
    u16 species = 0;
    SetFacilityPtrsGetLevel();

    if (trainerId != TRAINER_EREADER)
    {
        if (trainerId < FRONTIER_TRAINERS_COUNT)
        {
            move = gFacilityTrainerMons[monId].moves[0];
            species = gFacilityTrainerMons[monId].species;
        }
    }

    StringCopy(gStringVar1, gMoveNames[move]);
    StringCopy(gStringVar2, gSpeciesNames[species]);
}

// For multi battles in the Battle Tower, the player may choose a partner by talking to them
// These partners can be an NPC or a former/record-mixed Apprentice
// When talked to, their response consists of:
// PARTNER_MSGID_INTRO - A greeting
// PARTNER_MSGID_MON1 - Naming one pokemon on their team, and a move it has
// PARTNER_MSGID_MON2_ASK - Naming a second pokemon on their team, a move it has, and asking if they'd like to be their partner
// PARTNER_MSGID_ACCEPT - If the player agrees to be their partner
// PARTNER_MSGID_REJECT - If the player declines to be their partner
static void ShowPartnerCandidateMessage(void)
{
}

static void LoadLinkMultiOpponentsData(void)
{
}

static void TowerTryCloseLink(void)
{
    if (gWirelessCommType != 0)
        SetCloseLinkCallback();
}

static void SetMultiPartnerGfx(void)
{
}

static void SetTowerInterviewData(void)
{
}

static void ValidateBattleTowerRecordChecksums(void)
{
}

void CalcEmeraldBattleTowerChecksum(struct EmeraldBattleTowerRecord *record)
{
    u32 i;

    record->checksum = 0;
    for (i = 0; i < (sizeof(struct EmeraldBattleTowerRecord) - 4) / 4; i++) // - 4, because of the last field being the checksum itself.
        record->checksum += ((u32 *)record)[i];
}

void CalcRubyBattleTowerChecksum(struct RSBattleTowerRecord *record)
{
    u32 i;

    record->checksum = 0;
    for (i = 0; i < (sizeof(struct RSBattleTowerRecord) - 4) / 4; i++) // - 4, because of the last field being the checksum itself.
        record->checksum += ((u32 *)record)[i];
}

static void ClearBattleTowerRecord(struct EmeraldBattleTowerRecord *record)
{
    u32 i;

    for (i = 0; i < sizeof(struct EmeraldBattleTowerRecord) / 4; i++)
        ((u32 *)record)[i] = 0;
}

u16 GetCurrentBattleTowerWinStreak(u8 lvlMode, u8 battleMode)
{
}

static u8 GetMonCountForBattleMode(u8 battleMode)
{
    u8 partySizes[ARRAY_COUNT(sBattleTowerPartySizes)];
    memcpy(partySizes, sBattleTowerPartySizes, sizeof(sBattleTowerPartySizes));

    if (battleMode < ARRAY_COUNT(sBattleTowerPartySizes))
        return partySizes[battleMode];
    else
        return FRONTIER_PARTY_SIZE;
}

struct RibbonCounter
{
    u8 partyIndex;
    u8 count;
};

static void AwardBattleTowerRibbons(void)
{
}

// This is a leftover debugging function that is used to populate the E-Reader
// trainer with the player's current data.
static void FillEReaderTrainerWithPlayerData(void)
{
}

u8 GetEreaderTrainerFrontSpriteId(void)
{
}

u8 GetEreaderTrainerClassId(void)
{
}

void GetEreaderTrainerName(u8 *dst)
{
}

// Checks if the saved E-Reader trainer is valid.
void ValidateEReaderTrainer(void)
{
}

static void SetEReaderTrainerChecksum(struct BattleTowerEReaderTrainer *ereaderTrainer)
{
    s32 i;

    ereaderTrainer->checksum = 0;
    for (i = 0; i < (sizeof(struct BattleTowerEReaderTrainer) - 4) / 4; i++) // - 4, because of the last field being the checksum itself.
        ereaderTrainer->checksum += ((u32 *)ereaderTrainer)[i];
}

void ClearEReaderTrainer(struct BattleTowerEReaderTrainer *ereaderTrainer)
{
    u32 i;

    for (i = 0; i < (sizeof(struct BattleTowerEReaderTrainer)) / 4; i++)
        ((u32 *)ereaderTrainer)[i] = 0;
}

void CopyEReaderTrainerGreeting(void)
{
}

static void CopyEReaderTrainerFarewellMessage(void)
{
}

void TryHideBattleTowerReporter(void)
{
}

#define STEVEN_OTID 61226

static void FillPartnerParty(u16 trainerId)
{
}

bool32 RubyBattleTowerRecordToEmerald(struct RSBattleTowerRecord *src, struct EmeraldBattleTowerRecord *dst)
{
    s32 i, validMons = 0;

    for (i = 0; i < FRONTIER_PARTY_SIZE; i++)
    {
        if (src->party[i].species)
            validMons++;
    }

    if (validMons != FRONTIER_PARTY_SIZE)
    {
        memset(dst, 0, sizeof(*dst));
        return FALSE;
    }
    else
    {
        dst->lvlMode = src->lvlMode;
        dst->winStreak = src->winStreak;
        // UB: Reading outside the array. sRubyFacilityClassToEmerald has less than FACILITY_CLASSES_COUNT entries.
        #ifdef UBFIX
        for (i = 0; i < ARRAY_COUNT(sRubyFacilityClassToEmerald); i++)
        #else
        for (i = 0; i < FACILITY_CLASSES_COUNT; i++)
        #endif
        {
            if (sRubyFacilityClassToEmerald[i][0] == src->facilityClass)
                break;
        }
        if (i != FACILITY_CLASSES_COUNT)
            dst->facilityClass = sRubyFacilityClassToEmerald[i][1];
        else
            dst->facilityClass = FACILITY_CLASS_YOUNGSTER;

        for (i = 0; i < PLAYER_NAME_LENGTH + 1; i++)
            dst->name[i] = src->name[i];
        for (i = 0; i < TRAINER_ID_LENGTH; i++)
            dst->trainerId[i] = src->trainerId[i];
        for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; i++)
            dst->greeting[i] = src->greeting[i];
        for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; i++)
            dst->speechWon[i] = sRecordTrainerSpeechWon[i];
        for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; i++)
            dst->speechLost[i] = sRecordTrainerSpeechLost[i];
        for (i = 0; i < FRONTIER_PARTY_SIZE; i++)
            dst->party[i] = src->party[i];

        CpuFill32(0, &dst->party[FRONTIER_PARTY_SIZE], sizeof(dst->party[FRONTIER_PARTY_SIZE]));
        CalcEmeraldBattleTowerChecksum(dst);
        return TRUE;
    }
}

bool32 EmeraldBattleTowerRecordToRuby(struct EmeraldBattleTowerRecord *src, struct RSBattleTowerRecord *dst)
{
    s32 i, validMons = 0;

    for (i = 0; i < FRONTIER_PARTY_SIZE; i++)
    {
        if (src->party[i].species)
            validMons++;
    }

    if (validMons != FRONTIER_PARTY_SIZE)
    {
        memset(dst, 0, sizeof(*dst));
        return FALSE;
    }
    else
    {
        dst->lvlMode = src->lvlMode;
        dst->winStreak = src->winStreak;
        // UB: Reading outside the array. sRubyFacilityClassToEmerald has less than FACILITY_CLASSES_COUNT entries.
        #ifdef UBFIX
        for (i = 0; i < ARRAY_COUNT(sRubyFacilityClassToEmerald); i++)
        #else
        for (i = 0; i < FACILITY_CLASSES_COUNT; i++)
        #endif
        {
            if (sRubyFacilityClassToEmerald[i][1] == src->facilityClass)
                break;
        }
        if (i != FACILITY_CLASSES_COUNT)
            dst->facilityClass = sRubyFacilityClassToEmerald[i][0];
        else
            dst->facilityClass = RS_FACILITY_CLASS_YOUNGSTER;

        for (i = 0; i < PLAYER_NAME_LENGTH + 1; i++)
            dst->name[i] = src->name[i];
        for (i = 0; i < TRAINER_ID_LENGTH; i++)
            dst->trainerId[i] = src->trainerId[i];
        for (i = 0; i < EASY_CHAT_BATTLE_WORDS_COUNT; i++)
            dst->greeting[i] = src->greeting[i];
        for (i = 0; i < FRONTIER_PARTY_SIZE; i++)
            dst->party[i] = src->party[i];

        CalcRubyBattleTowerChecksum(dst);
        return TRUE;
    }
}

void GetBattleTowerTrainerLanguage(u8 *dst, u16 trainerId)
{
}

u8 SetFacilityPtrsGetLevel(void)
{
}

u8 GetFrontierEnemyMonLevel(u8 lvlMode)
{
    u8 level;

    switch (lvlMode)
    {
    default:
    case FRONTIER_LVL_50:
        level = FRONTIER_MAX_LEVEL_50;
        break;
    case FRONTIER_LVL_OPEN:
        level = GetHighestLevelInPlayerParty();
        if (level < FRONTIER_MIN_LEVEL_OPEN)
            level = FRONTIER_MIN_LEVEL_OPEN;
        break;
    }

    return level;
}

s32 GetHighestLevelInPlayerParty(void)
{
    s32 highestLevel = 0;
    s32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES, NULL)
            && GetMonData(&gPlayerParty[i], MON_DATA_SPECIES2, NULL) != SPECIES_EGG)
        {
            s32 level = GetMonData(&gPlayerParty[i], MON_DATA_LEVEL, NULL);
            if (level > highestLevel)
                highestLevel = level;
        }
    }

    return highestLevel;
}

// Frontier Trainer parties are roughly scaled in difficulty with higher trainer IDs, so scale IVs as well
// Duplicated in Battle Dome as GetDomeTrainerMonIvs
static u8 GetFrontierTrainerFixedIvs(u16 trainerId)
{
    u8 fixedIv;

    if (trainerId <= FRONTIER_TRAINER_JILL)         // 0 - 99
        fixedIv = 3;
    else if (trainerId <= FRONTIER_TRAINER_CHLOE)   // 100 - 119
        fixedIv = 6;
    else if (trainerId <= FRONTIER_TRAINER_SOFIA)   // 120 - 139
        fixedIv = 9;
    else if (trainerId <= FRONTIER_TRAINER_JAZLYN)  // 140 - 159
        fixedIv = 12;
    else if (trainerId <= FRONTIER_TRAINER_ALISON)  // 160 - 179
        fixedIv = 15;
    else if (trainerId <= FRONTIER_TRAINER_LAMAR)   // 180 - 199
        fixedIv = 18;
    else if (trainerId <= FRONTIER_TRAINER_TESS)    // 200 - 219
        fixedIv = 21;
    else                                            // 220+ (- 299)
        fixedIv = MAX_PER_STAT_IVS;

    return fixedIv;
}

static u16 GetBattleTentTrainerId(void)
{
    u32 facility = VarGet(VAR_FRONTIER_FACILITY);

    if (facility == FRONTIER_FACILITY_PALACE)       // Verdanturf Tent; uses Palace mechanics
        return Random() % NUM_BATTLE_TENT_TRAINERS;
    else if (facility == FRONTIER_FACILITY_ARENA)   // Fallarbor Tent; uses Arena mechanics
        return Random() % NUM_BATTLE_TENT_TRAINERS;
    else if (facility == FRONTIER_FACILITY_FACTORY) // Slateport Tent; uses Factory mechanics
        return Random() % NUM_BATTLE_TENT_TRAINERS;
    else if (facility == FRONTIER_FACILITY_TOWER)
        return 0;
    else
        return 0;
}

static u8 SetTentPtrsGetLevel(void)
{
    u8 level = TENT_MIN_LEVEL;
    u32 facility = VarGet(VAR_FRONTIER_FACILITY);

    if (facility == FRONTIER_FACILITY_FACTORY)
    {
        gFacilityTrainers = gSlateportBattleTentTrainers;
        gFacilityTrainerMons = gSlateportBattleTentMons;
    }
    else if (facility == FRONTIER_FACILITY_PALACE)
    {
        gFacilityTrainers = gVerdanturfBattleTentTrainers;
        gFacilityTrainerMons = gVerdanturfBattleTentMons;
    }
    else if (facility == FRONTIER_FACILITY_ARENA)
    {
        gFacilityTrainers = gFallarborBattleTentTrainers;
        gFacilityTrainerMons = gFallarborBattleTentMons;
    }
    else
    {
        gFacilityTrainers = gBattleFrontierTrainers;
        gFacilityTrainerMons = gBattleFrontierMons;
    }

    level = GetHighestLevelInPlayerParty();
    if (level < TENT_MIN_LEVEL)
        level = TENT_MIN_LEVEL;

    return level;
}

static void SetNextBattleTentOpponent(void)
{
}

static void FillTentTrainerParty_(u16 trainerId, u8 firstMonId, u8 monCount)
{
    s32 i, j;
    u16 chosenMonIndices[MAX_FRONTIER_PARTY_SIZE];
    u8 friendship;
    u8 level = SetTentPtrsGetLevel();
    u8 fixedIV = 0;
    u8 bfMonCount;
    const u16 *monSet = NULL;
    u32 otID = 0;
    u16 monId;

    monSet = gFacilityTrainers[gTrainerBattleOpponent_A].monSet;

    bfMonCount = 0;
    monId = monSet[bfMonCount];
    while (monId != 0xFFFF)
    {
        bfMonCount++;
        monId = monSet[bfMonCount];
        if (monId == 0xFFFF)
            break;
    }

    i = 0;
    otID = Random32();
    while (i != monCount)
    {
        u16 monId = monSet[Random() % bfMonCount];

        // Ensure this pokemon species isn't a duplicate.
        for (j = 0; j < i + firstMonId; j++)
        {
            if (GetMonData(&gEnemyParty[j], MON_DATA_SPECIES, NULL) == gFacilityTrainerMons[monId].species)
                break;
        }
        if (j != i + firstMonId)
            continue;

        // Ensure this Pokemon's held item isn't a duplicate.
        for (j = 0; j < i + firstMonId; j++)
        {
            if (GetMonData(&gEnemyParty[j], MON_DATA_HELD_ITEM, NULL) != ITEM_NONE
             && GetMonData(&gEnemyParty[j], MON_DATA_HELD_ITEM, NULL) == gBattleFrontierHeldItems[gFacilityTrainerMons[monId].itemTableId])
                break;
        }
        if (j != i + firstMonId)
            continue;

        // Ensure this exact pokemon index isn't a duplicate. This check doesn't seem necessary
        // because the species and held items were already checked directly above.
        for (j = 0; j < i; j++)
        {
            if (chosenMonIndices[j] == monId)
                break;
        }
        if (j != i)
            continue;

        chosenMonIndices[i] = monId;

        // Place the chosen pokemon into the trainer's party.
        CreateMonWithEVSpreadNatureOTID(&gEnemyParty[i + firstMonId],
                                             gFacilityTrainerMons[monId].species,
                                             level,
                                             gFacilityTrainerMons[monId].nature,
                                             fixedIV,
                                             gFacilityTrainerMons[monId].evSpread,
                                             otID);

        friendship = MAX_FRIENDSHIP;
        // Give the chosen pokemon its specified moves.
        for (j = 0; j < MAX_MON_MOVES; j++)
        {
            SetMonMoveSlot(&gEnemyParty[i + firstMonId], gFacilityTrainerMons[monId].moves[j], j);
            if (gFacilityTrainerMons[monId].moves[j] == MOVE_FRUSTRATION)
                friendship = 0;  // Frustration is more powerful the lower the pokemon's friendship is.
        }

        SetMonData(&gEnemyParty[i + firstMonId], MON_DATA_FRIENDSHIP, &friendship);
        SetMonData(&gEnemyParty[i + firstMonId], MON_DATA_HELD_ITEM, &gBattleFrontierHeldItems[gFacilityTrainerMons[monId].itemTableId]);

        // The pokemon was successfully added to the trainer's party, so it's safe to move on to
        // the next party slot.
        i++;
    }
}

u16 FacilityClassToGraphicsId(u8 facilityClass)
{
    u16 trainerObjectGfxId;
    u8 i;

    // Search male classes.
    for (i = 0; i < ARRAY_COUNT(gTowerMaleFacilityClasses); i++)
    {
        if (gTowerMaleFacilityClasses[i] == facilityClass)
            break;
    }
    if (i != ARRAY_COUNT(gTowerMaleFacilityClasses))
    {
        trainerObjectGfxId = gTowerMaleTrainerGfxIds[i];
        return trainerObjectGfxId;
    }

    // Search female classes.
    for (i = 0; i < ARRAY_COUNT(gTowerFemaleFacilityClasses); i++)
    {
        if (gTowerFemaleFacilityClasses[i] == facilityClass)
            break;
    }
    if (i != ARRAY_COUNT(gTowerFemaleFacilityClasses))
    {
        trainerObjectGfxId = gTowerFemaleTrainerGfxIds[i];
        return trainerObjectGfxId;
    }
    else
    {
        return OBJ_EVENT_GFX_BOY_1;
    }
}

void TrySetLinkBattleTowerEnemyPartyLevel(void)
{
    if (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK))
    {
        s32 i;
        u8 enemyLevel = SetFacilityPtrsGetLevel();

        for (i = 0; i < PARTY_SIZE; i++)
        {
            u32 species = GetMonData(&gEnemyParty[i], MON_DATA_SPECIES, NULL);
            if (species)
            {
                SetMonData(&gEnemyParty[i], MON_DATA_EXP, &gExperienceTables[gBaseStats[species].growthRate][enemyLevel]);
                CalculateMonStats(&gEnemyParty[i]);
            }
        }
    }
}
