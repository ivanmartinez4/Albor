#include "global.h"
#include "battle_dome.h"
#include "battle.h"
#include "battle_main.h"
#include "battle_setup.h"
#include "battle_tower.h"
#include "frontier_util.h"
#include "battle_message.h"
#include "event_data.h"
#include "overworld.h"
#include "util.h"
#include "malloc.h"
#include "string_util.h"
#include "random.h"
#include "task.h"
#include "main.h"
#include "gpu_regs.h"
#include "text.h"
#include "bg.h"
#include "window.h"
#include "strings.h"
#include "palette.h"
#include "decompress.h"
#include "party_menu.h"
#include "menu.h"
#include "sound.h"
#include "pokemon_icon.h"
#include "data.h"
#include "international_string_util.h"
#include "trainer_pokemon_sprites.h"
#include "scanline_effect.h"
#include "script_pokemon_util.h"
#include "graphics.h"
#include "constants/battle_dome.h"
#include "constants/frontier_util.h"
#include "constants/moves.h"
#include "constants/trainers.h"
#include "constants/abilities.h"
#include "constants/songs.h"
#include "constants/battle_frontier.h"
#include "constants/rgb.h"

// Enough space to hold 2 match info cards worth of trainers and their parties
#define NUM_INFOCARD_SPRITES ((FRONTIER_PARTY_SIZE + 1) * 4)
#define NUM_INFOCARD_TRAINERS 2

// An 'Info Card' is a trainer or match information page that can be viewed on the Tourney Tree
struct TourneyTreeInfoCard
{
    u8 spriteIds[NUM_INFOCARD_SPRITES];
    u8 pos;
    u8 tournamentIds[NUM_INFOCARD_TRAINERS];
};

struct TourneyTreeLineSection
{
    u8 x;
    u8 y;
    u16 src;
};

#define tState              data[0]

// Task data for Task_ShowTourneyTree
#define tNotInteractive     data[1]
#define tIsPrevTourneyTree  data[4]

// Task data for Task_ShowTourneyInfoCard
#define tTournamentId       data[1]
#define tMode               data[2]
#define tPrevTaskId         data[3]

#define EFFECTIVENESS_MODE_GOOD     0
#define EFFECTIVENESS_MODE_BAD      1
#define EFFECTIVENESS_MODE_AI_VS_AI 2

static u8 GetDomeTrainerMonIvs(u16);
static void SwapDomeTrainers(int, int, u16 *);
static void CalcDomeMonStats(u16, int, int, u8, u8, int *);
static void CreateDomeOpponentMons(u16);
static int SelectOpponentMons_Good(u16, bool8);
static int SelectOpponentMons_Bad(u16, bool8);
static int GetTypeEffectivenessPoints(int, int, int);
static int SelectOpponentMonsFromParty(int *, bool8);
static void Task_ShowTourneyInfoCard(u8);
static void Task_HandleInfoCardInput(u8);
static u8 Task_GetInfoCardInput(u8);
static void SetFacilityTrainerAndMonPtrs(void);
static int TrainerIdToTournamentId(u16);
static u16 TrainerIdOfPlayerOpponent(void);
static void Task_ShowTourneyTree(u8);
static void Task_HandleStaticTourneyTreeInput(u8);
static void CB2_TourneyTree(void);
static void VblankCb_TourneyInfoCard(void);
static void DisplayMatchInfoOnCard(u8, u8);
static void DisplayTrainerInfoOnCard(u8, u8);
static int BufferDomeWinString(u8, u8 *);
static u8 GetDomeBrainTrainerPicId(void);
static u8 GetDomeBrainTrainerClass(void);
static void CopyDomeBrainTrainerName(u8 *);
static void CopyDomeTrainerName(u8 *, u16);
static void HblankCb_TourneyTree(void);
static void VblankCb_TourneyTree(void);
static u8 UpdateTourneyTreeCursor(u8);
static void DecideRoundWinners(u8);
static u8 GetOpposingNPCTournamentIdByRound(u8, u8);
static void DrawTourneyAdvancementLine(u8, u8);
static void SpriteCB_HorizontalScrollArrow(struct Sprite *);
static void SpriteCB_VerticalScrollArrow(struct Sprite *);
static void InitDomeChallenge(void);
static void GetDomeData(void);
static void SetDomeData(void);
static void BufferDomeRoundText(void);
static void BufferDomeOpponentName(void);
static void InitDomeOpponentParty(void);
static void ShowDomeOpponentInfo(void);
static void ShowDomeTourneyTree(void);
static void ShowPreviousDomeTourneyTree(void);
static void SetDomeOpponentId(void);
static void SetDomeOpponentGraphicsId(void);
static void ShowNonInteractiveDomeTourneyTree(void);
static void ResolveDomeRoundWinners(void);
static void SaveDomeChallenge(void);
static void IncrementDomeStreaks(void);
static void ResetSketchedMoves(void);
static void RestoreDomePlayerPartyHeldItems(void);
static void ReduceDomePlayerPartyToSelectedMons(void);
static void GetPlayerSeededBeforeOpponent(void);
static void BufferLastDomeWinnerName(void);
static void InitRandomTourneyTreeResults(void);
static void InitDomeTrainers(void);

EWRAM_DATA u32 gPlayerPartyLostHP = 0; // never read
static EWRAM_DATA u32 sPlayerPartyMaxHP = 0; // never read
static EWRAM_DATA struct TourneyTreeInfoCard *sInfoCard = {0};
static EWRAM_DATA u8 *sTilemapBuffer = NULL;

// Each move has an array of points for different move characteristics which contribute to a tourney trainers listed battle style (see sBattleStyleThresholds)
// All move points are either 1 or 0, so theyre essentially flags saying whether or not the move has that characteristic
static const u8 sBattleStyleMovePoints[MOVES_COUNT][NUM_MOVE_POINT_TYPES] =
{
    [MOVE_NONE]          = {0},
    [MOVE_POUND]         = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_KARATE_CHOP]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_DOUBLE_SLAP]   = {[MOVE_POINTS_DMG] = 1},
    [MOVE_COMET_PUNCH]   = {[MOVE_POINTS_DMG] = 1},
    [MOVE_MEGA_PUNCH]    = {[MOVE_POINTS_DMG] = 1},
    [MOVE_PAY_DAY]       = {[MOVE_POINTS_RARE] = 1, [MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_FIRE_PUNCH]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_ICE_PUNCH]     = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_THUNDER_PUNCH] = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_SCRATCH]       = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_VISE_GRIP]     = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_GUILLOTINE]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_RAZOR_WIND]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_SWORDS_DANCE]  = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_STAT_RAISE] = 1, [MOVE_POINTS_POPULAR] = 1},
    [MOVE_CUT]           = {[MOVE_POINTS_DMG] = 1},
    [MOVE_GUST]          = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_WING_ATTACK]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_WHIRLWIND]     = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_FLY]           = {[MOVE_POINTS_DMG] = 1},
    [MOVE_BIND]          = {[MOVE_POINTS_STATUS] = 1, [MOVE_POINTS_DMG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_SLAM]          = {[MOVE_POINTS_DMG] = 1},
    [MOVE_VINE_WHIP]     = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_STOMP]         = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_DOUBLE_KICK]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_MEGA_KICK]     = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_JUMP_KICK]     = {[MOVE_POINTS_DMG] = 1},
    [MOVE_ROLLING_KICK]  = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_SAND_ATTACK]   = {[MOVE_POINTS_STAT_LOWER] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_HEADBUTT]      = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_HORN_ATTACK]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_FURY_ATTACK]   = {[MOVE_POINTS_DMG] = 1},
    [MOVE_HORN_DRILL]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_TACKLE]        = {[MOVE_POINTS_DMG] = 1},
    [MOVE_BODY_SLAM]     = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_WRAP]          = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_TAKE_DOWN]     = {[MOVE_POINTS_DMG] = 1},
    [MOVE_THRASH]        = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_DOUBLE_EDGE]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_TAIL_WHIP]     = {[MOVE_POINTS_STAT_LOWER] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_POISON_STING]  = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_TWINEEDLE]     = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_PIN_MISSILE]   = {[MOVE_POINTS_DMG] = 1},
    [MOVE_LEER]          = {[MOVE_POINTS_STAT_LOWER] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_BITE]          = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_GROWL]         = {[MOVE_POINTS_STAT_LOWER] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_ROAR]          = {[MOVE_POINTS_ACCURATE] = 1},
    [MOVE_SING]          = {[MOVE_POINTS_STATUS] = 1},
    [MOVE_SUPERSONIC]    = {[MOVE_POINTS_STATUS] = 1},
    [MOVE_SONIC_BOOM]    = {[MOVE_POINTS_DMG] = 1},
    [MOVE_DISABLE]       = {[MOVE_POINTS_STATUS] = 1},
    [MOVE_ACID]          = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_EMBER]         = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_FLAMETHROWER]  = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_POPULAR] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_MIST]          = {0},
    [MOVE_WATER_GUN]     = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_HYDRO_PUMP]    = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_DMG] = 1, [MOVE_POINTS_POWERFUL] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_SURF]          = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_POPULAR] = 1, [MOVE_POINTS_STRONG] = 1},
    [MOVE_ICE_BEAM]      = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_POPULAR] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_BLIZZARD]      = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_POWERFUL] = 1, [MOVE_POINTS_LOW_PP] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_PSYBEAM]       = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_BUBBLE_BEAM]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_AURORA_BEAM]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_HYPER_BEAM]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_POWERFUL] = 1, [MOVE_POINTS_POPULAR] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_PECK]          = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_DRILL_PECK]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_SUBMISSION]    = {[MOVE_POINTS_DMG] = 1},
    [MOVE_LOW_KICK]      = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_COUNTER]       = {[MOVE_POINTS_DEF] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_LUCK] = 1},
    [MOVE_SEISMIC_TOSS]  = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_STRENGTH]      = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_ABSORB]        = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_MEGA_DRAIN]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_LEECH_SEED]    = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_STATUS] = 1},
    [MOVE_GROWTH]        = {[MOVE_POINTS_STAT_RAISE] = 1},
    [MOVE_RAZOR_LEAF]    = {[MOVE_POINTS_DMG] = 1},
    [MOVE_SOLAR_BEAM]    = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_POWERFUL] = 1, [MOVE_POINTS_POPULAR] = 1, [MOVE_POINTS_STRONG] = 1},
    [MOVE_POISON_POWDER] = {[MOVE_POINTS_STATUS] = 1},
    [MOVE_STUN_SPORE]    = {[MOVE_POINTS_STATUS] = 1},
    [MOVE_SLEEP_POWDER]  = {[MOVE_POINTS_STATUS] = 1},
    [MOVE_PETAL_DANCE]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_STRING_SHOT]   = {[MOVE_POINTS_STAT_LOWER] = 1},
    [MOVE_DRAGON_RAGE]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_FIRE_SPIN]     = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_THUNDER_SHOCK] = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_THUNDERBOLT]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_POPULAR] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_THUNDER_WAVE]  = {[MOVE_POINTS_STATUS] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_THUNDER]       = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_POWERFUL] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_ROCK_THROW]    = {[MOVE_POINTS_DMG] = 1},
    [MOVE_EARTHQUAKE]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_POWERFUL] = 1, [MOVE_POINTS_POPULAR] = 1, [MOVE_POINTS_STRONG] = 1},
    [MOVE_FISSURE]       = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_LUCK] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_DIG]           = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_TOXIC]         = {[MOVE_POINTS_STATUS] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_CONFUSION]     = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_PSYCHIC]       = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_POPULAR] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_HYPNOSIS]      = {[MOVE_POINTS_COMBO] = 1},
    [MOVE_MEDITATE]      = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_STAT_RAISE] = 1},
    [MOVE_AGILITY]       = {[MOVE_POINTS_STAT_RAISE] = 1},
    [MOVE_QUICK_ATTACK]  = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_RAGE]          = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_TELEPORT]      = {0},
    [MOVE_NIGHT_SHADE]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_MIMIC]         = {[MOVE_POINTS_RARE] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_SCREECH]       = {[MOVE_POINTS_STAT_LOWER] = 1},
    [MOVE_DOUBLE_TEAM]   = {[MOVE_POINTS_STAT_RAISE] = 1, [MOVE_POINTS_DEF] = 1},
    [MOVE_RECOVER]       = {0},
    [MOVE_HARDEN]        = {[MOVE_POINTS_STAT_RAISE] = 1, [MOVE_POINTS_DEF] = 1},
    [MOVE_MINIMIZE]      = {[MOVE_POINTS_STAT_RAISE] = 1, [MOVE_POINTS_DEF] = 1},
    [MOVE_SMOKESCREEN]   = {[MOVE_POINTS_STAT_LOWER] = 1, [MOVE_POINTS_DEF] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_CONFUSE_RAY]   = {[MOVE_POINTS_ACCURATE] = 1},
    [MOVE_WITHDRAW]      = {[MOVE_POINTS_STAT_RAISE] = 1, [MOVE_POINTS_DEF] = 1},
    [MOVE_DEFENSE_CURL]  = {[MOVE_POINTS_STAT_RAISE] = 1, [MOVE_POINTS_DEF] = 1},
    [MOVE_BARRIER]       = {[MOVE_POINTS_DEF] = 1},
    [MOVE_LIGHT_SCREEN]  = {[MOVE_POINTS_DEF] = 1},
    [MOVE_HAZE]          = {0},
    [MOVE_REFLECT]       = {[MOVE_POINTS_DEF] = 1},
    [MOVE_FOCUS_ENERGY]  = {[MOVE_POINTS_COMBO] = 1},
    [MOVE_BIDE]          = {[MOVE_POINTS_ACCURATE] = 1},
    [MOVE_METRONOME]     = {[MOVE_POINTS_RARE] = 1, [MOVE_POINTS_LUCK] = 1},
    [MOVE_MIRROR_MOVE]   = {[MOVE_POINTS_RARE] = 1, [MOVE_POINTS_LUCK] = 1},
    [MOVE_SELF_DESTRUCT] = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_POWERFUL] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_EGG_BOMB]      = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_POWERFUL] = 1, [MOVE_POINTS_STRONG] = 1},
    [MOVE_LICK]          = {[MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_SMOG]          = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_SLUDGE]        = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_BONE_CLUB]     = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_FIRE_BLAST]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_POWERFUL] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_LOW_PP] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_WATERFALL]     = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_CLAMP]         = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_SWIFT]         = {[MOVE_POINTS_DMG] = 1},
    [MOVE_SKULL_BASH]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_STRONG] = 1},
    [MOVE_SPIKE_CANNON]  = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_CONSTRICT]     = {[MOVE_POINTS_STATUS] = 1, [MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_AMNESIA]       = {[MOVE_POINTS_STAT_RAISE] = 1, [MOVE_POINTS_DEF] = 1},
    [MOVE_KINESIS]       = {[MOVE_POINTS_STAT_LOWER] = 1},
    [MOVE_SOFT_BOILED]   = {[MOVE_POINTS_HEAL] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_HIGH_JUMP_KICK]  = {[MOVE_POINTS_DMG] = 1},
    [MOVE_GLARE]         = {[MOVE_POINTS_STAT_LOWER] = 1},
    [MOVE_DREAM_EATER]   = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_RARE] = 1, [MOVE_POINTS_HEAL] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_STRONG] = 1},
    [MOVE_POISON_GAS]    = {[MOVE_POINTS_STATUS] = 1},
    [MOVE_BARRAGE]       = {[MOVE_POINTS_DMG] = 1},
    [MOVE_LEECH_LIFE]    = {[MOVE_POINTS_HEAL] = 1, [MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_LOVELY_KISS]   = {[MOVE_POINTS_STATUS] = 1},
    [MOVE_SKY_ATTACK]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_POWERFUL] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_LOW_PP] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_TRANSFORM]     = {[MOVE_POINTS_RARE] = 1},
    [MOVE_BUBBLE]        = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_DIZZY_PUNCH]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_SPORE]         = {[MOVE_POINTS_STATUS] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_FLASH]         = {0},
    [MOVE_PSYWAVE]       = {[MOVE_POINTS_DMG] = 1},
    [MOVE_SPLASH]        = {[MOVE_POINTS_RARE] = 1},
    [MOVE_ACID_ARMOR]    = {[MOVE_POINTS_STAT_RAISE] = 1, [MOVE_POINTS_DEF] = 1},
    [MOVE_CRABHAMMER]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_STRONG] = 1},
    [MOVE_EXPLOSION]     = {[MOVE_POINTS_RISKY] = 1, [MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_POWERFUL] = 1, [MOVE_POINTS_POPULAR] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_FURY_SWIPES]   = {[MOVE_POINTS_DMG] = 1},
    [MOVE_BONEMERANG]    = {[MOVE_POINTS_DMG] = 1},
    [MOVE_REST]          = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_HEAL] = 1},
    [MOVE_ROCK_SLIDE]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_HYPER_FANG]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_SHARPEN]       = {[MOVE_POINTS_STAT_RAISE] = 1, [MOVE_POINTS_DEF] = 1},
    [MOVE_CONVERSION]    = {[MOVE_POINTS_DEF] = 1},
    [MOVE_TRI_ATTACK]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_SUPER_FANG]    = {[MOVE_POINTS_DMG] = 1},
    [MOVE_SLASH]         = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_SUBSTITUTE]    = {[MOVE_POINTS_RARE] = 1, [MOVE_POINTS_DEF] = 1},
    [MOVE_STRUGGLE]      = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_LOW_PP] = 1}, // Odd that this is assigned qualities
    [MOVE_SKETCH]        = {[MOVE_POINTS_RARE] = 1, [MOVE_POINTS_LUCK] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_TRIPLE_KICK]   = {[MOVE_POINTS_DMG] = 1},
    [MOVE_THIEF]         = {[MOVE_POINTS_RARE] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_SPIDER_WEB]    = {[MOVE_POINTS_STAT_LOWER] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_MIND_READER]   = {[MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_NIGHTMARE]     = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_STATUS] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_FLAME_WHEEL]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_SNORE]         = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_CURSE]         = {[MOVE_POINTS_STATUS] = 1},
    [MOVE_FLAIL]         = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_CONVERSION_2]  = {[MOVE_POINTS_ACCURATE] = 1},
    [MOVE_AEROBLAST]     = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_POWERFUL] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_COTTON_SPORE]  = {[MOVE_POINTS_STAT_LOWER] = 1},
    [MOVE_REVERSAL]      = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_SPITE]         = {[MOVE_POINTS_RARE] = 1, [MOVE_POINTS_RISKY] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_POWDER_SNOW]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_PROTECT]       = {[MOVE_POINTS_DEF] = 1, [MOVE_POINTS_POPULAR] = 1},
    [MOVE_MACH_PUNCH]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_SCARY_FACE]    = {0},
    [MOVE_FEINT_ATTACK]  = {[MOVE_POINTS_DMG] = 1},
    [MOVE_SWEET_KISS]    = {0},
    [MOVE_BELLY_DRUM]    = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_STAT_RAISE] = 1},
    [MOVE_SLUDGE_BOMB]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_MUD_SLAP]      = {[MOVE_POINTS_STAT_LOWER] = 1, [MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_OCTAZOOKA]     = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_SPIKES]        = {[MOVE_POINTS_COMBO] = 1},
    [MOVE_ZAP_CANNON]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_POWERFUL] = 1, [MOVE_POINTS_LUCK] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_LOW_PP] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_FORESIGHT]     = {[MOVE_POINTS_ACCURATE] = 1},
    [MOVE_DESTINY_BOND]  = {[MOVE_POINTS_RISKY] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_PERISH_SONG]   = {[MOVE_POINTS_RISKY] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_ICY_WIND]      = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_DETECT]        = {[MOVE_POINTS_DEF] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_BONE_RUSH]     = {[MOVE_POINTS_DMG] = 1},
    [MOVE_LOCK_ON]       = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_OUTRAGE]       = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_SANDSTORM]     = {0},
    [MOVE_GIGA_DRAIN]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_ENDURE]        = {[MOVE_POINTS_DEF] = 1},
    [MOVE_CHARM]         = {[MOVE_POINTS_ACCURATE] = 1},
    [MOVE_ROLLOUT]       = {[MOVE_POINTS_DMG] = 1},
    [MOVE_FALSE_SWIPE]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_SWAGGER]       = {[MOVE_POINTS_EFFECT] = 1},
    [MOVE_MILK_DRINK]    = {[MOVE_POINTS_HEAL] = 1},
    [MOVE_SPARK]         = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_FURY_CUTTER]   = {[MOVE_POINTS_DMG] = 1},
    [MOVE_STEEL_WING]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_MEAN_LOOK]     = {[MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_ATTRACT]       = {[MOVE_POINTS_STATUS] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_SLEEP_TALK]    = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_LUCK] = 1},
    [MOVE_HEAL_BELL]     = {[MOVE_POINTS_LOW_PP] = 1},
    [MOVE_RETURN]        = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_PRESENT]       = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_LUCK] = 1},
    [MOVE_FRUSTRATION]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_SAFEGUARD]     = {[MOVE_POINTS_DEF] = 1},
    [MOVE_PAIN_SPLIT]    = {[MOVE_POINTS_RARE] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_SACRED_FIRE]   = {[MOVE_POINTS_POWERFUL] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_LOW_PP] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_MAGNITUDE]     = {[MOVE_POINTS_ACCURATE] = 1},
    [MOVE_DYNAMIC_PUNCH] = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_LUCK] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_LOW_PP] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_MEGAHORN]      = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_STRONG] = 1},
    [MOVE_DRAGON_BREATH] = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_BATON_PASS]    = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_RARE] = 1},
    [MOVE_ENCORE]        = {[MOVE_POINTS_STATUS] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_PURSUIT]       = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_RAPID_SPIN]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_SWEET_SCENT]   = {[MOVE_POINTS_STAT_LOWER] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_IRON_TAIL]     = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_METAL_CLAW]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_VITAL_THROW]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_MORNING_SUN]   = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_HEAL] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_SYNTHESIS]     = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_HEAL] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_MOONLIGHT]     = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_HEAL] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_HIDDEN_POWER]  = {[MOVE_POINTS_RARE] = 1, [MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_CROSS_CHOP]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_TWISTER]       = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_RAIN_DANCE]    = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_SUNNY_DAY]     = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_CRUNCH]        = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_MIRROR_COAT]   = {[MOVE_POINTS_DEF] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_PSYCH_UP]      = {[MOVE_POINTS_STAT_RAISE] = 1},
    [MOVE_EXTREME_SPEED] = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_ANCIENT_POWER] = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_LOW_PP] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_SHADOW_BALL]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_FUTURE_SIGHT]  = {[MOVE_POINTS_RARE] = 1, [MOVE_POINTS_DMG] = 1},
    [MOVE_ROCK_SMASH]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_WHIRLPOOL]     = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_BEAT_UP]       = {[MOVE_POINTS_RARE] = 1, [MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_FAKE_OUT]      = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_UPROAR]        = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_STOCKPILE]     = {[MOVE_POINTS_COMBO] = 1},
    [MOVE_SPIT_UP]       = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_STRONG] = 1},
    [MOVE_SWALLOW]       = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_HEAL] = 1},
    [MOVE_HEAT_WAVE]     = {[MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_HAIL]          = {0},
    [MOVE_TORMENT]       = {[MOVE_POINTS_STATUS] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_FLATTER]       = {[MOVE_POINTS_ACCURATE] = 1},
    [MOVE_WILL_O_WISP]   = {[MOVE_POINTS_STATUS] = 1},
    [MOVE_MEMENTO]       = {[MOVE_POINTS_RARE] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_FACADE]        = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_FOCUS_PUNCH]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_STRONG] = 1},
    [MOVE_SMELLING_SALTS] = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_FOLLOW_ME]     = {[MOVE_POINTS_RARE] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_NATURE_POWER]  = {[MOVE_POINTS_DMG] = 1},
    [MOVE_CHARGE]        = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_TAUNT]         = {[MOVE_POINTS_STATUS] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_HELPING_HAND]  = {[MOVE_POINTS_RARE] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_TRICK]         = {[MOVE_POINTS_RARE] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_ROLE_PLAY]     = {[MOVE_POINTS_ACCURATE] = 1},
    [MOVE_WISH]          = {[MOVE_POINTS_HEAL] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_ASSIST]        = {[MOVE_POINTS_RARE] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_LUCK] = 1},
    [MOVE_INGRAIN]       = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_HEAL] = 1, [MOVE_POINTS_DEF] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_SUPERPOWER]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_MAGIC_COAT]    = {[MOVE_POINTS_DEF] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_LUCK] = 1},
    [MOVE_RECYCLE]       = {[MOVE_POINTS_ACCURATE] = 1},
    [MOVE_REVENGE]       = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_LUCK] = 1},
    [MOVE_BRICK_BREAK]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_YAWN]          = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_STATUS] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_KNOCK_OFF]     = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_ENDEAVOR]      = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_ERUPTION]      = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_POWERFUL] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_SKILL_SWAP]    = {[MOVE_POINTS_RARE] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_IMPRISON]      = {[MOVE_POINTS_RARE] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_LUCK] = 1},
    [MOVE_REFRESH]       = {[MOVE_POINTS_HEAL] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_GRUDGE]        = {[MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_SNATCH]        = {[MOVE_POINTS_RARE] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_LUCK] = 1},
    [MOVE_SECRET_POWER]  = {[MOVE_POINTS_RARE] = 1, [MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_DIVE]          = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_ARM_THRUST]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_CAMOUFLAGE]    = {[MOVE_POINTS_RARE] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_TAIL_GLOW]     = {[MOVE_POINTS_ACCURATE] = 1},
    [MOVE_LUSTER_PURGE]  = {[MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_LOW_PP] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_MIST_BALL]     = {[MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_LOW_PP] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_FEATHER_DANCE] = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_TEETER_DANCE]  = {[MOVE_POINTS_ACCURATE] = 1},
    [MOVE_BLAZE_KICK]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_MUD_SPORT]     = {[MOVE_POINTS_ACCURATE] = 1},
    [MOVE_ICE_BALL]      = {[MOVE_POINTS_DMG] = 1},
    [MOVE_NEEDLE_ARM]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_SLACK_OFF]     = {[MOVE_POINTS_HEAL] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_HYPER_VOICE]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_STRONG] = 1},
    [MOVE_POISON_FANG]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_CRUSH_CLAW]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_BLAST_BURN]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_POWERFUL] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_HYDRO_CANNON]  = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_POWERFUL] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_METEOR_MASH]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_POWERFUL] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_ASTONISH]      = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_WEATHER_BALL]  = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_AROMATHERAPY]  = {[MOVE_POINTS_LOW_PP] = 1},
    [MOVE_FAKE_TEARS]    = {[MOVE_POINTS_ACCURATE] = 1},
    [MOVE_AIR_CUTTER]    = {[MOVE_POINTS_DMG] = 1},
    [MOVE_OVERHEAT]      = {[MOVE_POINTS_POWERFUL] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_LOW_PP] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_ODOR_SLEUTH]   = {[MOVE_POINTS_ACCURATE] = 1},
    [MOVE_ROCK_TOMB]     = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_SILVER_WIND]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_LOW_PP] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_METAL_SOUND]   = {0},
    [MOVE_GRASS_WHISTLE] = {0},
    [MOVE_TICKLE]        = {[MOVE_POINTS_ACCURATE] = 1},
    [MOVE_COSMIC_POWER]  = {0},
    [MOVE_WATER_SPOUT]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_POWERFUL] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_SIGNAL_BEAM]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_SHADOW_PUNCH]  = {[MOVE_POINTS_DMG] = 1},
    [MOVE_EXTRASENSORY]  = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_SKY_UPPERCUT]  = {[MOVE_POINTS_DMG] = 1},
    [MOVE_SAND_TOMB]     = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_SHEER_COLD]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_LUCK] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_MUDDY_WATER]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_BULLET_SEED]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_AERIAL_ACE]    = {[MOVE_POINTS_DMG] = 1},
    [MOVE_ICICLE_SPEAR]  = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_IRON_DEFENSE]  = {[MOVE_POINTS_DEF] = 1},
    [MOVE_BLOCK]         = {[MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_HOWL]          = {0},
    [MOVE_DRAGON_CLAW]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_FRENZY_PLANT]  = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_POWERFUL] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_BULK_UP]       = {[MOVE_POINTS_COMBO] = 1},
    [MOVE_BOUNCE]        = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_LOW_PP] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_MUD_SHOT]      = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_POISON_TAIL]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_COVET]         = {[MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_VOLT_TACKLE]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_POWERFUL] = 1, [MOVE_POINTS_STRONG] = 1},
    [MOVE_MAGICAL_LEAF]  = {[MOVE_POINTS_DMG] = 1},
    [MOVE_WATER_SPORT]   = {[MOVE_POINTS_ACCURATE] = 1},
    [MOVE_CALM_MIND]     = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_STAT_RAISE] = 1},
    [MOVE_LEAF_BLADE]    = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1},
    [MOVE_DRAGON_DANCE]  = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_STAT_RAISE] = 1},
    [MOVE_ROCK_BLAST]    = {[MOVE_POINTS_DMG] = 1},
    [MOVE_SHOCK_WAVE]    = {[MOVE_POINTS_DMG] = 1},
    [MOVE_WATER_PULSE]   = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_ACCURATE] = 1, [MOVE_POINTS_EFFECT] = 1},
    [MOVE_DOOM_DESIRE]   = {[MOVE_POINTS_RARE] = 1, [MOVE_POINTS_DMG] = 1, [MOVE_POINTS_POWERFUL] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_LOW_PP] = 1},
    [MOVE_PSYCHO_BOOST]  = {[MOVE_POINTS_DMG] = 1, [MOVE_POINTS_POWERFUL] = 1, [MOVE_POINTS_STRONG] = 1, [MOVE_POINTS_LOW_PP] = 1, [MOVE_POINTS_EFFECT] = 1},
};

// This array is searched in-order to determine what battle style a tourney trainer uses.
// If the sum of the points for the party's moves meets/exceeds all the point totals of an element, then they use that battle style
static const u8 sBattleStyleThresholds[NUM_BATTLE_STYLES][NUM_MOVE_POINT_TYPES] =
{
    [DOME_BATTLE_STYLE_RISKY]           = {[MOVE_POINTS_RISKY] = 1},
    [DOME_BATTLE_STYLE_STALL]           = {[MOVE_POINTS_HEAL] = 2, [MOVE_POINTS_STATUS] = 1, [MOVE_POINTS_DEF] = 2},
    [DOME_BATTLE_STYLE_VARIED]          = {[MOVE_POINTS_COMBO] = 1, [MOVE_POINTS_STAT_RAISE] = 1, [MOVE_POINTS_STAT_LOWER] = 1, [MOVE_POINTS_HEAL] = 1, [MOVE_POINTS_STATUS] = 1, [MOVE_POINTS_DEF] = 1},
    [DOME_BATTLE_STYLE_COMBO_HIGH]      = {[MOVE_POINTS_COMBO] = 3},
    [DOME_BATTLE_STYLE_RARE_MOVES]      = {[MOVE_POINTS_RARE] = 2},
    [DOME_BATTLE_STYLE_RARE_MOVE]       = {[MOVE_POINTS_RARE] = 1},
    [DOME_BATTLE_STYLE_HP]              = {[MOVE_POINTS_HEAL] = 3},
    [DOME_BATTLE_STYLE_STORE_POWER]     = {[MOVE_POINTS_STAT_RAISE] = 1, [MOVE_POINTS_HEAL] = 1},
    [DOME_BATTLE_STYLE_ENFEEBLE_LOW]    = {[MOVE_POINTS_STAT_LOWER] = 1, [MOVE_POINTS_STATUS] = 1},
    [DOME_BATTLE_STYLE_LUCK]            = {[MOVE_POINTS_LUCK] = 2},
    [DOME_BATTLE_STYLE_REGAL]           = {[MOVE_POINTS_STAT_RAISE] = 1, [MOVE_POINTS_HEAL] = 1, [MOVE_POINTS_DEF] = 1, [MOVE_POINTS_POPULAR] = 1, [MOVE_POINTS_STRONG] = 1},
    [DOME_BATTLE_STYLE_LOW_PP]          = {[MOVE_POINTS_LOW_PP] = 3},
    [DOME_BATTLE_STYLE_STATUS_ATK]      = {[MOVE_POINTS_STAT_RAISE] = 1, [MOVE_POINTS_STATUS] = 1},
    [DOME_BATTLE_STYLE_ENDURE]          = {[MOVE_POINTS_HEAL] = 2, [MOVE_POINTS_DEF] = 2},
    [DOME_BATTLE_STYLE_STATUS]          = {[MOVE_POINTS_STATUS] = 2},
    [DOME_BATTLE_STYLE_STRAIGHTFORWARD] = {[MOVE_POINTS_ACCURATE] = 3, [MOVE_POINTS_STRONG] = 3},
    [DOME_BATTLE_STYLE_AGGRESSIVE]      = {[MOVE_POINTS_STRONG] = 4},
    [DOME_BATTLE_STYLE_DEF]             = {[MOVE_POINTS_DEF] = 3},
    [DOME_BATTLE_STYLE_ENFEEBLE_HIGH]   = {[MOVE_POINTS_STAT_LOWER] = 2, [MOVE_POINTS_STATUS] = 2}, // BUG: This battle style is unobtainable; DOME_BATTLE_STYLE_ENFEEBLE_LOW will always succeed before it
    [DOME_BATTLE_STYLE_POPULAR_POWER]   = {[MOVE_POINTS_POWERFUL] = 3, [MOVE_POINTS_POPULAR] = 3},
    [DOME_BATTLE_STYLE_COMBO_LOW]       = {[MOVE_POINTS_COMBO] = 2},
    [DOME_BATTLE_STYLE_ACCURATE]        = {[MOVE_POINTS_HEAL] = 1, [MOVE_POINTS_ACCURATE] = 3},
    [DOME_BATTLE_STYLE_POWERFUL]        = {[MOVE_POINTS_POWERFUL] = 4},
    [DOME_BATTLE_STYLE_ATK_OVER_DEF]    = {[MOVE_POINTS_DMG] = 7},
    [DOME_BATTLE_STYLE_DEF_OVER_ATK]    = {[MOVE_POINTS_DEF] = 4}, // BUG: This battle style is unobtainable; DOME_BATTLE_STYLE_DEF will always succeed before it
    [DOME_BATTLE_STYLE_POPULAR_STRONG]  = {[MOVE_POINTS_POPULAR] = 2, [MOVE_POINTS_STRONG] = 4},
    [DOME_BATTLE_STYLE_EFFECTS]         = {[MOVE_POINTS_EFFECT] = 4},
    [DOME_BATTLE_STYLE_BALANCED]        = {0}, // If no other thresholds are met, this battle style is used
};

// 1st array is for cursor position (sprite id): cursor can be on a trainer info button, a match info button, or the exit/cancel button
// 2nd array is for round count. For some reason this array contains an inaccessible Round 5 which is identical to Round 4
// 3rd array is movement direction (see the MOVE_DIR_* constants in UpdateTourneyTreeCursor)
// The values are sprite IDs for the cursor position to move to, with 0xFF being an invalid move
static const u8 sTourneyTreeCursorMovementMap[DOME_TOURNAMENT_TRAINERS_COUNT + DOME_TOURNAMENT_MATCHES_COUNT + 1][DOME_ROUNDS_COUNT + 1][4]=
{
    [0]  = {{   7,    1,    8,   16}, {   7,    1,    8,   16}, {   7,    1,    8,   16}, {   7,    1,    8,   16}, {   7,    1,    8,   16}},
    [1]  = {{   0,    2,    9,   16}, {   0,    2,    9,   16}, {   0,    2,    9,   16}, {   0,    2,    9,   16}, {   0,    2,    9,   16}},
    [2]  = {{   1,    3,   10,   17}, {   1,    3,   10,   17}, {   1,    3,   10,   17}, {   1,    3,   10,   17}, {   1,    3,   10,   17}},
    [3]  = {{   2,    4,   11,   17}, {   2,    4,   11,   17}, {   2,    4,   11,   17}, {   2,    4,   11,   17}, {   2,    4,   11,   17}},
    [4]  = {{   3,    5,   12,   18}, {   3,    5,   12,   18}, {   3,    5,   12,   18}, {   3,    5,   12,   18}, {   3,    5,   12,   18}},
    [5]  = {{   4,    6,   13,   18}, {   4,    6,   13,   18}, {   4,    6,   13,   18}, {   4,    6,   13,   18}, {   4,    6,   13,   18}},
    [6]  = {{   5,    7,   14,   19}, {   5,    7,   14,   19}, {   5,    7,   14,   19}, {   5,    7,   14,   19}, {   5,    7,   14,   19}},
    [7]  = {{   6,    0,   15,   19}, {   6,    0,   15,   19}, {   6,    0,   15,   19}, {   6,    0,   15,   19}, {   6,    0,   15,   19}},
    [8]  = {{  31,    9,   20,   31}, {  31,    9,   20,   31}, {  31,    9,   20,   31}, {  31,    9,   20,   31}, {  31,    9,   20,   31}},
    [9]  = {{   8,   10,   20,    1}, {   8,   10,   20,    1}, {   8,   10,   20,    1}, {   8,   10,   20,    1}, {   8,   10,   20,    1}},
    [10] = {{   9,   11,   21,    2}, {   9,   11,   21,    2}, {   9,   11,   21,    2}, {   9,   11,   21,    2}, {   9,   11,   21,    2}},
    [11] = {{  10,   12,   21,    3}, {  10,   12,   21,    3}, {  10,   12,   21,    3}, {  10,   12,   21,    3}, {  10,   12,   21,    3}},
    [12] = {{  11,   13,   22,    4}, {  11,   13,   22,    4}, {  11,   13,   22,    4}, {  11,   13,   22,    4}, {  11,   13,   22,    4}},
    [13] = {{  12,   14,   22,    5}, {  12,   14,   22,    5}, {  12,   14,   22,    5}, {  12,   14,   22,    5}, {  12,   14,   22,    5}},
    [14] = {{  13,   15,   23,    6}, {  13,   15,   23,    6}, {  13,   15,   23,    6}, {  13,   15,   23,    6}, {  13,   15,   23,    6}},
    [15] = {{  14,   31,   23,    7}, {  14,   31,   23,    7}, {  14,   31,   23,    7}, {  14,   31,   23,    7}, {  14,   31,   23,    7}},
    [16] = {{  19,   17,    0,   20}, {  19,   17,    0,   24}, {  19,   17,    0,   24}, {  19,   17,    0,   24}, {  19,   17,    0,   24}},
    [17] = {{  16,   18,    2,   21}, {  16,   18,    2,   24}, {  16,   18,    2,   24}, {  16,   18,    2,   24}, {  16,   18,    2,   24}},
    [18] = {{  17,   19,    4,   22}, {  17,   19,    4,   25}, {  17,   19,    4,   25}, {  17,   19,    4,   25}, {  17,   19,    4,   25}},
    [19] = {{  18,   16,    6,   23}, {  18,   16,    6,   25}, {  18,   16,    6,   25}, {  18,   16,    6,   25}, {  18,   16,    6,   25}},
    [20] = {{  23,   21,   16,    8}, {  23,   21,   26,    8}, {  23,   21,   26,    8}, {  23,   21,   26,    8}, {  23,   21,   26,    8}},
    [21] = {{  20,   22,   17,   10}, {  20,   22,   26,   10}, {  20,   22,   26,   10}, {  20,   22,   26,   10}, {  20,   22,   26,   10}},
    [22] = {{  21,   23,   18,   12}, {  21,   23,   27,   12}, {  21,   23,   27,   12}, {  21,   23,   27,   12}, {  21,   23,   27,   12}},
    [23] = {{  22,   20,   19,   14}, {  22,   20,   27,   14}, {  22,   20,   27,   14}, {  22,   20,   27,   14}, {  22,   20,   27,   14}},
    [24] = {{0xFF, 0xFF, 0xFF, 0xFF}, {  25,   25,   16,   26}, {  25,   25,   16,   28}, {  25,   25,   16,   28}, {  25,   25,   16,   28}},
    [25] = {{0xFF, 0xFF, 0xFF, 0xFF}, {  24,   24,   18,   27}, {  24,   24,   18,   28}, {  24,   24,   18,   28}, {  24,   24,   18,   28}},
    [26] = {{0xFF, 0xFF, 0xFF, 0xFF}, {  27,   27,   24,   20}, {  27,   27,   29,   20}, {  27,   27,   29,   20}, {  27,   27,   29,   20}},
    [27] = {{0xFF, 0xFF, 0xFF, 0xFF}, {  26,   26,   25,   22}, {  26,   26,   29,   22}, {  26,   26,   29,   22}, {  26,   26,   29,   22}},
    [28] = {{0xFF, 0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF, 0xFF}, {0xFF, 0xFF,   24,   29}, {0xFF, 0xFF,   24,   30}, {0xFF, 0xFF,   24,   30}},
    [29] = {{0xFF, 0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF, 0xFF}, {0xFF, 0xFF,   28,   26}, {0xFF, 0xFF,   30,   26}, {0xFF, 0xFF,   30,   26}},
    [30] = {{0xFF, 0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF, 0xFF}, {0xFF, 0xFF,   28,   29}, {0xFF, 0xFF,   28,   29}},
    [31] = {{  15,    8,    8,    0}, {  15,    8,    8,    0}, {  15,    8,    8,    0}, {  15,    8,    8,    0}, {  15,    8,    8,    0}}, // TOURNEY_TREE_CLOSE_BUTTON
};

static const struct BgTemplate sTourneyTreeBgTemplates[4] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 28,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 1,
        .charBaseIndex = 1,
        .mapBaseIndex = 29,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    },
    {
        .bg = 2,
        .charBaseIndex = 2,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0
    },
    {
        .bg = 3,
        .charBaseIndex = 2,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0
    },
};

static const struct BgTemplate sInfoCardBgTemplates[4] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 20,
        .screenSize = 3,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 1,
        .charBaseIndex = 1,
        .mapBaseIndex = 24,
        .screenSize = 3,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 2,
        .charBaseIndex = 2,
        .mapBaseIndex = 28,
        .screenSize = 3,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    },
    {
        .bg = 3,
        .charBaseIndex = 2,
        .mapBaseIndex = 7,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    },
};

static const struct WindowTemplate sTourneyTreeWindowTemplates[] =
{
    {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 3,
        .width = 8,
        .height = 16,
        .paletteNum = 15,
        .baseBlock = 16,
    },
    {
        .bg = 0,
        .tilemapLeft = 22,
        .tilemapTop = 3,
        .width = 8,
        .height = 16,
        .paletteNum = 15,
        .baseBlock = 144,
    },
    {
        .bg = 0,
        .tilemapLeft = 8,
        .tilemapTop = 1,
        .width = 14,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 272,
    },
    DUMMY_WIN_TEMPLATE,
};

static const struct WindowTemplate sInfoCardWindowTemplates[] =
{
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 2,
        .width = 26,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 1,
    },
    {
        .bg = 0,
        .tilemapLeft = 16,
        .tilemapTop = 5,
        .width = 8,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 53,
    },
    {
        .bg = 0,
        .tilemapLeft = 19,
        .tilemapTop = 7,
        .width = 9,
        .height = 3,
        .paletteNum = 15,
        .baseBlock = 69,
    },
    {
        .bg = 0,
        .tilemapLeft = 16,
        .tilemapTop = 10,
        .width = 8,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 96,
    },
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 12,
        .width = 26,
        .height = 7,
        .paletteNum = 15,
        .baseBlock = 112,
    },
    {
        .bg = 0,
        .tilemapLeft = 5,
        .tilemapTop = 2,
        .width = 23,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 294,
    },
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 5,
        .width = 8,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 340,
    },
    {
        .bg = 0,
        .tilemapLeft = 20,
        .tilemapTop = 5,
        .width = 8,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 356,
    },
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 16,
        .width = 26,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 372,
    },
    {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 2,
        .width = 26,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 1,
    },
    {
        .bg = 1,
        .tilemapLeft = 16,
        .tilemapTop = 5,
        .width = 8,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 53,
    },
    {
        .bg = 1,
        .tilemapLeft = 19,
        .tilemapTop = 7,
        .width = 9,
        .height = 3,
        .paletteNum = 15,
        .baseBlock = 69,
    },
    {
        .bg = 1,
        .tilemapLeft = 16,
        .tilemapTop = 10,
        .width = 8,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 96,
    },
    {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 12,
        .width = 26,
        .height = 7,
        .paletteNum = 15,
        .baseBlock = 112,
    },
    {
        .bg = 1,
        .tilemapLeft = 5,
        .tilemapTop = 2,
        .width = 23,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 294,
    },
    {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 5,
        .width = 8,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 340,
    },
    {
        .bg = 1,
        .tilemapLeft = 20,
        .tilemapTop = 5,
        .width = 8,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 356,
    },
    {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 16,
        .width = 26,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 372,
    },
    #ifdef UBFIX
    DUMMY_WIN_TEMPLATE,
    #endif
};

static const struct ScanlineEffectParams sTourneyTreeScanlineEffectParams =
{
    .dmaDest = &REG_BG3CNT,
    .dmaControl = SCANLINE_EFFECT_DMACNT_16BIT,
    .initState = 1,
};

static const struct CompressedSpriteSheet sTourneyTreeButtonsSpriteSheet[] =
{
    {gDomeTourneyTreeButtons_Gfx, 0x0600, 0x0000},
    {},
};

static const struct OamData sOamData_TourneyTreePokeball =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(16x16),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(16x16),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0,
};

// For Exit/Cancel buttons
static const struct OamData sOamData_TourneyTreeCloseButton =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(32x16),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(32x16),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 1,
    .affineParam = 0,
};

static const struct OamData sOamData_VerticalScrollArrow =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(16x8),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(16x8),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 2,
    .affineParam = 0,
};

static const struct OamData sOamData_HorizontalScrollArrow =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(8x16),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(8x16),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 2,
    .affineParam = 0,
};

static const union AnimCmd sSpriteAnim_TourneyTreePokeballNormal[] =
{
    ANIMCMD_FRAME(20, 1),
    ANIMCMD_END,
};
static const union AnimCmd sSpriteAnim_TourneyTreePokeballSelected[] =
{
    ANIMCMD_FRAME(24, 1),
    ANIMCMD_END,
};

static const union AnimCmd * const sSpriteAnimTable_TourneyTreePokeball[] =
{
    sSpriteAnim_TourneyTreePokeballNormal,
    sSpriteAnim_TourneyTreePokeballSelected,
};

// Sprite template for the pokeballs on the tourney tree that act as buttons to view a trainer/match info card
static const struct SpriteTemplate sTourneyTreePokeballSpriteTemplate =
{
    .tileTag = 0x0000,
    .paletteTag = TAG_NONE,
    .oam = &sOamData_TourneyTreePokeball,
    .anims = sSpriteAnimTable_TourneyTreePokeball,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy
};

static const union AnimCmd sSpriteAnim_TourneyTreeCancelButtonNormal[] =
{
    ANIMCMD_FRAME(8, 1),
    ANIMCMD_END,
};

static const union AnimCmd sSpriteAnim_TourneyTreeCancelButtonSelected[] =
{
    ANIMCMD_FRAME(0, 1),
    ANIMCMD_END,
};

static const union AnimCmd * const sSpriteAnimTable_TourneyTreeCancelButton[] =
{
    sSpriteAnim_TourneyTreeCancelButtonNormal,
    sSpriteAnim_TourneyTreeCancelButtonSelected,
};

static const struct SpriteTemplate sCancelButtonSpriteTemplate =
{
    .tileTag = 0x0000,
    .paletteTag = TAG_NONE,
    .oam = &sOamData_TourneyTreeCloseButton,
    .anims = sSpriteAnimTable_TourneyTreeCancelButton,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy
};

static const union AnimCmd sSpriteAnim_TourneyTreeExitButtonNormal[] =
{
    ANIMCMD_FRAME(40, 1),
    ANIMCMD_END,
};

static const union AnimCmd sSpriteAnim_TourneyTreeExitButtonSelected[] =
{
    ANIMCMD_FRAME(32, 1),
    ANIMCMD_END,
 };

static const union AnimCmd * const sSpriteAnimTable_TourneyTreeExitButton[] =
{
    sSpriteAnim_TourneyTreeExitButtonNormal,
    sSpriteAnim_TourneyTreeExitButtonSelected,
};

static const struct SpriteTemplate sExitButtonSpriteTemplate =
{
    .tileTag = 0x0000,
    .paletteTag = TAG_NONE,
    .oam = &sOamData_TourneyTreeCloseButton,
    .anims = sSpriteAnimTable_TourneyTreeExitButton,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy
};

static const union AnimCmd sSpriteAnim_UpArrow[] =
{
    ANIMCMD_FRAME(18, 1),
    ANIMCMD_END,
};

static const union AnimCmd sSpriteAnim_DownArrow[] =
{
    ANIMCMD_FRAME(18, 1, .vFlip = TRUE),
    ANIMCMD_END,
 };

static const union AnimCmd sSpriteAnim_LeftArrow[] =
{
    ANIMCMD_FRAME(16, 1, .hFlip = TRUE),
    ANIMCMD_END,
};

static const union AnimCmd sSpriteAnim_RightArrow[] =
{
    ANIMCMD_FRAME(16, 1),
    ANIMCMD_END,
};

static const union AnimCmd * const sSpriteAnimTable_VerticalScrollArrow[] =
{
    sSpriteAnim_UpArrow,
    sSpriteAnim_DownArrow,
};

static const union AnimCmd * const sSpriteAnimTable_HorizontalScrollArrow[] =
{
    sSpriteAnim_LeftArrow,
    sSpriteAnim_RightArrow,
};

static const struct SpriteTemplate sHorizontalScrollArrowSpriteTemplate =
{
    .tileTag = 0x0000,
    .paletteTag = TAG_NONE,
    .oam = &sOamData_HorizontalScrollArrow,
    .anims = sSpriteAnimTable_HorizontalScrollArrow,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_HorizontalScrollArrow
};

static const struct SpriteTemplate sVerticalScrollArrowSpriteTemplate =
{
    .tileTag = 0x0000,
    .paletteTag = TAG_NONE,
    .oam = &sOamData_VerticalScrollArrow,
    .anims = sSpriteAnimTable_VerticalScrollArrow,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_VerticalScrollArrow
};

// Organized by seed starting position, i.e. seed 0 battles seed 8 first
static const u8 sTourneyTreeTrainerIds[DOME_TOURNAMENT_TRAINERS_COUNT] = {0, 8, 12, 4, 7, 15, 11, 3, 2, 10, 14, 6, 5, 13, 9, 1};

static void (* const sBattleDomeFunctions[])(void) =
{
    [BATTLE_DOME_FUNC_INIT]                     = InitDomeChallenge,
    [BATTLE_DOME_FUNC_GET_DATA]                 = GetDomeData,
    [BATTLE_DOME_FUNC_SET_DATA]                 = SetDomeData,
    [BATTLE_DOME_FUNC_GET_ROUND_TEXT]           = BufferDomeRoundText,
    [BATTLE_DOME_FUNC_GET_OPPONENT_NAME]        = BufferDomeOpponentName,
    [BATTLE_DOME_FUNC_INIT_OPPONENT_PARTY]      = InitDomeOpponentParty,
    [BATTLE_DOME_FUNC_SHOW_OPPONENT_INFO]       = ShowDomeOpponentInfo,
    [BATTLE_DOME_FUNC_SHOW_TOURNEY_TREE]        = ShowDomeTourneyTree,
    [BATTLE_DOME_FUNC_SHOW_PREV_TOURNEY_TREE]   = ShowPreviousDomeTourneyTree,
    [BATTLE_DOME_FUNC_SET_OPPONENT_ID]          = SetDomeOpponentId,
    [BATTLE_DOME_FUNC_SET_OPPONENT_GFX]         = SetDomeOpponentGraphicsId,
    [BATTLE_DOME_FUNC_SHOW_STATIC_TOURNEY_TREE] = ShowNonInteractiveDomeTourneyTree,
    [BATTLE_DOME_FUNC_RESOLVE_WINNERS]          = ResolveDomeRoundWinners,
    [BATTLE_DOME_FUNC_SAVE]                     = SaveDomeChallenge,
    [BATTLE_DOME_FUNC_INCREMENT_STREAK]         = IncrementDomeStreaks,
    [BATTLE_DOME_FUNC_SET_TRAINERS]             = SetFacilityTrainerAndMonPtrs,
    [BATTLE_DOME_FUNC_RESET_SKETCH]             = ResetSketchedMoves,
    [BATTLE_DOME_FUNC_RESTORE_HELD_ITEMS]       = RestoreDomePlayerPartyHeldItems,
    [BATTLE_DOME_FUNC_REDUCE_PARTY]             = ReduceDomePlayerPartyToSelectedMons,
    [BATTLE_DOME_FUNC_COMPARE_SEEDS]            = GetPlayerSeededBeforeOpponent,
    [BATTLE_DOME_FUNC_GET_WINNER_NAME]          = BufferLastDomeWinnerName,
    [BATTLE_DOME_FUNC_INIT_RESULTS_TREE]        = InitRandomTourneyTreeResults,
    [BATTLE_DOME_FUNC_INIT_TRAINERS]            = InitDomeTrainers,
};

static const u32 sWinStreakFlags[][2] =
{
    {STREAK_DOME_SINGLES_50, STREAK_DOME_SINGLES_OPEN},
    {STREAK_DOME_DOUBLES_50, STREAK_DOME_DOUBLES_OPEN},
};

static const u32 sWinStreakMasks[][2] =
{
    {~(STREAK_DOME_SINGLES_50), ~(STREAK_DOME_SINGLES_OPEN)},
    {~(STREAK_DOME_DOUBLES_50), ~(STREAK_DOME_DOUBLES_OPEN)},
};

// TODO: The below two arrays probably need better names. The one below for example is only true of sIdToOpponentId[i][0]
static const u8 sIdToOpponentId[DOME_TOURNAMENT_TRAINERS_COUNT][DOME_ROUNDS_COUNT] =
{
    [0]  = { 8,  0,  4,  8},
    [1]  = { 9, 12,  8,  0},
    [2]  = {10,  8, 12,  0},
    [3]  = {11,  4,  0,  8},
    [4]  = {12,  0,  4,  8},
    [5]  = {13, 12,  8,  0},
    [6]  = {14,  8, 12,  0},
    [7]  = {15,  4,  0,  8},
    [8]  = { 0,  0,  4,  8},
    [9]  = { 1, 12,  8,  0},
    [10] = { 2,  8, 12,  0},
    [11] = { 3,  4,  0,  8},
    [12] = { 4,  0,  4,  8},
    [13] = { 5, 12,  8,  0},
    [14] = { 6,  8, 12,  0},
    [15] = { 7,  4,  0,  8},
};

// sTourneyTreeTrainerIds with every other pair swapped
static const u8 sTourneyTreeTrainerOpponentIds[DOME_TOURNAMENT_TRAINERS_COUNT] = { 0, 8, 4, 12, 7, 15, 3, 11, 2, 10, 6, 14, 5, 13, 1, 9 };

// The match number - 1 that a given tournament trainer will participate in for a given round
static const u8 sIdToMatchNumber[DOME_TOURNAMENT_TRAINERS_COUNT][DOME_ROUNDS_COUNT] =
{
    { 0,  8, 12, 14},
    { 0,  8, 12, 14},
    { 1,  8, 12, 14},
    { 1,  8, 12, 14},
    { 2,  9, 12, 14},
    { 2,  9, 12, 14},
    { 3,  9, 12, 14},
    { 3,  9, 12, 14},
    { 4, 10, 13, 14},
    { 4, 10, 13, 14},
    { 5, 10, 13, 14},
    { 5, 10, 13, 14},
    { 6, 11, 13, 14},
    { 6, 11, 13, 14},
    { 7, 11, 13, 14},
    { 7, 11, 13, 14},
};

static const u8 sLastMatchCardNum[DOME_ROUNDS_COUNT] =
{
    [DOME_ROUND1]    = 23,
    [DOME_ROUND2]    = 27,
    [DOME_SEMIFINAL] = 29,
    [DOME_FINAL]     = 30
};

static const u8 sTrainerAndRoundToLastMatchCardNum[DOME_TOURNAMENT_TRAINERS_COUNT / 2][DOME_ROUNDS_COUNT] =
{
    {16, 24, 28, 30},
    {17, 24, 28, 30},
    {18, 25, 28, 30},
    {19, 25, 28, 30},
    {20, 26, 29, 30},
    {21, 26, 29, 30},
    {22, 27, 29, 30},
    {23, 27, 29, 30},
};

static const u8 sTournamentIdToPairedTrainerIds[DOME_TOURNAMENT_TRAINERS_COUNT] = {0, 15, 8, 7, 3, 12, 11, 4, 1, 14, 9, 6, 2, 13, 10, 5};

// The first line of text on a trainers info card. It describes their potential to win, based on their seed in the tournament tree.
// Dome Ace Tucker has their own separate potential text.
static const u8 *const sBattleDomePotentialTexts[DOME_TOURNAMENT_TRAINERS_COUNT + 1] =
{
    BattleDome_Text_Potential1, // Highest potential
    BattleDome_Text_Potential2,
    BattleDome_Text_Potential3,
    BattleDome_Text_Potential4,
    BattleDome_Text_Potential5,
    BattleDome_Text_Potential6,
    BattleDome_Text_Potential7,
    BattleDome_Text_Potential8,
    BattleDome_Text_Potential9,
    BattleDome_Text_Potential10,
    BattleDome_Text_Potential11,
    BattleDome_Text_Potential12,
    BattleDome_Text_Potential13,
    BattleDome_Text_Potential14,
    BattleDome_Text_Potential15,
    BattleDome_Text_Potential16, // Lowest potential
    BattleDome_Text_PotentialDomeAceTucker,
};

// The second line of text on a trainers info card. It gives information about their battle style (dependent on their party's moves).
static const u8 *const sBattleDomeOpponentStyleTexts[NUM_BATTLE_STYLES] =
{
    [DOME_BATTLE_STYLE_RISKY]           = BattleDome_Text_StyleRiskDisaster,
    [DOME_BATTLE_STYLE_STALL]           = BattleDome_Text_StyleEndureLongBattles,
    [DOME_BATTLE_STYLE_VARIED]          = BattleDome_Text_StyleVariesTactics,
    [DOME_BATTLE_STYLE_COMBO_HIGH]      = BattleDome_Text_StyleToughWinningPattern,
    [DOME_BATTLE_STYLE_RARE_MOVES]      = BattleDome_Text_StyleUsesVeryRareMove,   // Seems like the text for these two was swapped
    [DOME_BATTLE_STYLE_RARE_MOVE]       = BattleDome_Text_StyleUsesStartlingMoves, //
    [DOME_BATTLE_STYLE_HP]              = BattleDome_Text_StyleConstantlyWatchesHP,
    [DOME_BATTLE_STYLE_STORE_POWER]     = BattleDome_Text_StyleStoresAndLoosesPower,
    [DOME_BATTLE_STYLE_ENFEEBLE_LOW]    = BattleDome_Text_StyleEnfeeblesFoes,
    [DOME_BATTLE_STYLE_LUCK]            = BattleDome_Text_StylePrefersLuckTactics,
    [DOME_BATTLE_STYLE_REGAL]           = BattleDome_Text_StyleRegalAtmosphere,
    [DOME_BATTLE_STYLE_LOW_PP]          = BattleDome_Text_StylePowerfulLowPPMoves,
    [DOME_BATTLE_STYLE_STATUS_ATK]      = BattleDome_Text_StyleEnfeebleThenAttack,
    [DOME_BATTLE_STYLE_ENDURE]          = BattleDome_Text_StyleBattlesWhileEnduring,
    [DOME_BATTLE_STYLE_STATUS]          = BattleDome_Text_StyleUpsetsFoesEmotionally,
    [DOME_BATTLE_STYLE_STRAIGHTFORWARD] = BattleDome_Text_StyleStrongAndStraightforward,
    [DOME_BATTLE_STYLE_AGGRESSIVE]      = BattleDome_Text_StyleAggressivelyStrongMoves,
    [DOME_BATTLE_STYLE_DEF]             = BattleDome_Text_StyleCleverlyDodgesAttacks,
    [DOME_BATTLE_STYLE_ENFEEBLE_HIGH]   = BattleDome_Text_StyleUsesUpsettingMoves,
    [DOME_BATTLE_STYLE_POPULAR_POWER]   = BattleDome_Text_StyleUsesPopularMoves,
    [DOME_BATTLE_STYLE_COMBO_LOW]       = BattleDome_Text_StyleHasPowerfulComboMoves,
    [DOME_BATTLE_STYLE_ACCURATE]        = BattleDome_Text_StyleUsesHighProbabilityMoves,
    [DOME_BATTLE_STYLE_POWERFUL]        = BattleDome_Text_StyleAggressivelySpectacularMoves,
    [DOME_BATTLE_STYLE_ATK_OVER_DEF]    = BattleDome_Text_StyleEmphasizesOffenseOverDefense,
    [DOME_BATTLE_STYLE_DEF_OVER_ATK]    = BattleDome_Text_StyleEmphasizesDefenseOverOffense,
    [DOME_BATTLE_STYLE_POPULAR_STRONG]  = BattleDome_Text_StyleAttacksQuicklyStrongMoves,
    [DOME_BATTLE_STYLE_EFFECTS]         = BattleDome_Text_StyleUsesAddedEffectMoves,
    [DOME_BATTLE_STYLE_BALANCED]        = BattleDome_Text_StyleUsesBalancedMixOfMoves,
};

// The third line of text on a trainers info card. It that gives information about their party's stat spread (based on their Pokémon's effort values and Nature).
static const u8 *const sBattleDomeOpponentStatsTexts[] =
{
    BattleDome_Text_EmphasizesHPAndAtk,      // DOME_TEXT_TWO_GOOD_STATS and DOME_TEXT_HP start here
    BattleDome_Text_EmphasizesHPAndDef,
    BattleDome_Text_EmphasizesHPAndSpeed,
    BattleDome_Text_EmphasizesHPAndSpAtk,
    BattleDome_Text_EmphasizesHPAndSpDef,
    BattleDome_Text_EmphasizesAtkAndDef,     // DOME_TEXT_ATK starts here
    BattleDome_Text_EmphasizesAtkAndSpeed,
    BattleDome_Text_EmphasizesAtkAndSpAtk,
    BattleDome_Text_EmphasizesAtkAndSpDef,
    BattleDome_Text_EmphasizesDefAndSpeed,   // DOME_TEXT_DEF starts here
    BattleDome_Text_EmphasizesDefAndSpAtk,
    BattleDome_Text_EmphasizesDefAndSpDef,
    BattleDome_Text_EmphasizesSpeedAndSpAtk, // DOME_TEXT_SPEED starts here
    BattleDome_Text_EmphasizesSpeedAndSpDef,
    BattleDome_Text_EmphasizesSpAtkAndSpDef, // DOME_TEXT_SPATK starts here
    BattleDome_Text_EmphasizesHP,            // DOME_TEXT_ONE_GOOD_STAT starts here
    BattleDome_Text_EmphasizesAtk,
    BattleDome_Text_EmphasizesDef,
    BattleDome_Text_EmphasizesSpeed,
    BattleDome_Text_EmphasizesSpAtk,
    BattleDome_Text_EmphasizesSpDef,
    BattleDome_Text_NeglectsHPAndAtk,        // DOME_TEXT_TWO_BAD_STATS starts here
    BattleDome_Text_NeglectsHPAndDef,
    BattleDome_Text_NeglectsHPAndSpeed,
    BattleDome_Text_NeglectsHPAndSpAtk,
    BattleDome_Text_NeglectsHPAndSpDef,
    BattleDome_Text_NeglectsAtkAndDef,
    BattleDome_Text_NeglectsAtkAndSpeed,
    BattleDome_Text_NeglectsAtkAndSpAtk,
    BattleDome_Text_NeglectsAtkAndSpDef,
    BattleDome_Text_NeglectsDefAndSpeed,
    BattleDome_Text_NeglectsDefAndSpAtk,
    BattleDome_Text_NeglectsDefAndSpDef,
    BattleDome_Text_NeglectsSpeedAndSpAtk,
    BattleDome_Text_NeglectsSpeedAndSpDef,
    BattleDome_Text_NeglectsSpAtkAndSpDef,
    BattleDome_Text_NeglectsHP,              // DOME_TEXT_ONE_BAD_STAT starts here
    BattleDome_Text_NeglectsAtk,
    BattleDome_Text_NeglectsDef,
    BattleDome_Text_NeglectsSpeed,
    BattleDome_Text_NeglectsSpAtk,
    BattleDome_Text_NeglectsSpDef,
    [DOME_TEXT_WELL_BALANCED] = BattleDome_Text_RaisesMonsWellBalanced,
};

static const u8 sInfoTrainerMonX[FRONTIER_PARTY_SIZE] = {104, 136, 104};
static const u8 sInfoTrainerMonY[FRONTIER_PARTY_SIZE] = { 38,  62,  78};
static const u8 sSpeciesNameTextYCoords[] = {0, 4, 0};

// Offsets within sBattleDomeOpponentStatsTexts for stat combinations
// SPDEF has no offset because by then all stat combinations have been reached, so it has no combination texts
static const u8 sStatTextOffsets[NUM_STATS - 1] =
{
    DOME_TEXT_HP,
    DOME_TEXT_ATK,
    DOME_TEXT_DEF,
    DOME_TEXT_SPEED,
    DOME_TEXT_SPATK
};

static const u8 *const sBattleDomeMatchNumberTexts[DOME_TOURNAMENT_MATCHES_COUNT] =
{
    BattleDome_Text_Round1Match1,
    BattleDome_Text_Round1Match2,
    BattleDome_Text_Round1Match3,
    BattleDome_Text_Round1Match4,
    BattleDome_Text_Round1Match5,
    BattleDome_Text_Round1Match6,
    BattleDome_Text_Round1Match7,
    BattleDome_Text_Round1Match8,
    BattleDome_Text_Round2Match1,
    BattleDome_Text_Round2Match2,
    BattleDome_Text_Round2Match3,
    BattleDome_Text_Round2Match4,
    BattleDome_Text_SemifinalMatch1,
    BattleDome_Text_SemifinalMatch2,
    BattleDome_Text_FinalMatch,
};

static const u8 *const sBattleDomeWinTexts[] =
{
    [DOME_TEXT_NO_WINNER_YET]    = BattleDome_Text_LetTheBattleBegin,
    [DOME_TEXT_WON_USING_MOVE]   = BattleDome_Text_TrainerWonUsingMove,
    [DOME_TEXT_CHAMP_USING_MOVE] = BattleDome_Text_TrainerBecameChamp,
    [DOME_TEXT_WON_ON_FORFEIT]   = BattleDome_Text_TrainerWonByDefault,
    [DOME_TEXT_CHAMP_ON_FORFEIT] = BattleDome_Text_TrainerWonOutrightByDefault,
    [DOME_TEXT_WON_NO_MOVES]     = BattleDome_Text_TrainerWonNoMoves,
    [DOME_TEXT_CHAMP_NO_MOVES]   = BattleDome_Text_TrainerWonOutrightNoMoves,
};

static const u8 sLeftTrainerMonX[FRONTIER_PARTY_SIZE]  = { 96,  96,  96};
static const u8 sLeftTrainerMonY[FRONTIER_PARTY_SIZE]  = { 56,  80, 104};
static const u8 sRightTrainerMonX[FRONTIER_PARTY_SIZE] = {144, 144, 144};
static const u8 sRightTrainerMonY[FRONTIER_PARTY_SIZE] = { 56,  80, 104};

// Duplicate of sTourneyTreeTrainerIds
static const u8 sTourneyTreeTrainerIds2[DOME_TOURNAMENT_TRAINERS_COUNT] = {0, 8, 12, 4, 7, 15, 11, 3, 2, 10, 14, 6, 5, 13, 9, 1};

// The number of possible trainers that could be competing in a given match
#define NUM_POSSIBLE_MATCH_TRAINERS(round) (DOME_TOURNAMENT_TRAINERS_COUNT / (1 << (DOME_ROUNDS_COUNT - round - 1)))

// The range of tournament trainers to check as possible participants in a given match
// Given by the offset in sCompetitorRangeByMatch[][0], the number of trainers in sCompetitorRangeByMatch[][1], and the round
static const u8 sCompetitorRangeByMatch[DOME_TOURNAMENT_MATCHES_COUNT][3] =
{
    { NUM_POSSIBLE_MATCH_TRAINERS(DOME_ROUND1) * 0,    NUM_POSSIBLE_MATCH_TRAINERS(DOME_ROUND1),    DOME_ROUND1},
    { NUM_POSSIBLE_MATCH_TRAINERS(DOME_ROUND1) * 1,    NUM_POSSIBLE_MATCH_TRAINERS(DOME_ROUND1),    DOME_ROUND1},
    { NUM_POSSIBLE_MATCH_TRAINERS(DOME_ROUND1) * 2,    NUM_POSSIBLE_MATCH_TRAINERS(DOME_ROUND1),    DOME_ROUND1},
    { NUM_POSSIBLE_MATCH_TRAINERS(DOME_ROUND1) * 3,    NUM_POSSIBLE_MATCH_TRAINERS(DOME_ROUND1),    DOME_ROUND1},
    { NUM_POSSIBLE_MATCH_TRAINERS(DOME_ROUND1) * 4,    NUM_POSSIBLE_MATCH_TRAINERS(DOME_ROUND1),    DOME_ROUND1},
    { NUM_POSSIBLE_MATCH_TRAINERS(DOME_ROUND1) * 5,    NUM_POSSIBLE_MATCH_TRAINERS(DOME_ROUND1),    DOME_ROUND1},
    { NUM_POSSIBLE_MATCH_TRAINERS(DOME_ROUND1) * 6,    NUM_POSSIBLE_MATCH_TRAINERS(DOME_ROUND1),    DOME_ROUND1},
    { NUM_POSSIBLE_MATCH_TRAINERS(DOME_ROUND1) * 7,    NUM_POSSIBLE_MATCH_TRAINERS(DOME_ROUND1),    DOME_ROUND1},
    { NUM_POSSIBLE_MATCH_TRAINERS(DOME_ROUND2) * 0,    NUM_POSSIBLE_MATCH_TRAINERS(DOME_ROUND2),    DOME_ROUND2},
    { NUM_POSSIBLE_MATCH_TRAINERS(DOME_ROUND2) * 1,    NUM_POSSIBLE_MATCH_TRAINERS(DOME_ROUND2),    DOME_ROUND2},
    { NUM_POSSIBLE_MATCH_TRAINERS(DOME_ROUND2) * 2,    NUM_POSSIBLE_MATCH_TRAINERS(DOME_ROUND2),    DOME_ROUND2},
    { NUM_POSSIBLE_MATCH_TRAINERS(DOME_ROUND2) * 3,    NUM_POSSIBLE_MATCH_TRAINERS(DOME_ROUND2),    DOME_ROUND2},
    { NUM_POSSIBLE_MATCH_TRAINERS(DOME_SEMIFINAL) * 0, NUM_POSSIBLE_MATCH_TRAINERS(DOME_SEMIFINAL), DOME_SEMIFINAL},
    { NUM_POSSIBLE_MATCH_TRAINERS(DOME_SEMIFINAL) * 1, NUM_POSSIBLE_MATCH_TRAINERS(DOME_SEMIFINAL), DOME_SEMIFINAL},
    { NUM_POSSIBLE_MATCH_TRAINERS(DOME_FINAL) * 0,     NUM_POSSIBLE_MATCH_TRAINERS(DOME_FINAL),     DOME_FINAL},
};

// 1st value is the windowId (0 for left column, 1 for right column)
// 2nd value is the y coord
static const u8 sTrainerNamePositions[DOME_TOURNAMENT_TRAINERS_COUNT][2] =
{
    { 0,   0},
    { 1, 112},
    { 1,   0},
    { 0, 112},
    { 0,  48},
    { 1,  64},
    { 1,  48},
    { 0,  64},
    { 0,  16},
    { 1,  96},
    { 1,  16},
    { 0,  96},
    { 0,  32},
    { 1,  80},
    { 1,  32},
    { 0,  80},
};

// Coords for the pokeballs on the tourney tree that act as buttons to view trainer/match info
static const u8 sTourneyTreePokeballCoords[DOME_TOURNAMENT_TRAINERS_COUNT + DOME_TOURNAMENT_MATCHES_COUNT][2] =
{
    { 68,  33}, // Left side trainers
    { 68,  49},
    { 68,  65},
    { 68,  81},
    { 68,  97},
    { 68, 113},
    { 68, 129},
    { 68, 145},
    {172,  33}, // Right side trainers
    {172,  49},
    {172,  65},
    {172,  81},
    {172,  97},
    {172, 113},
    {172, 129},
    {172, 145},
    { 87,  41}, // Left side Round 1 matches
    { 87,  73},
    { 87, 105},
    { 87, 137},
    {153,  41}, // Right side Round 1 matches
    {153,  73},
    {153, 105},
    {153, 137},
    { 95,  57}, // Left side Round 2 matches
    { 95, 121},
    {145,  57}, // Right side Round 2 matches
    {145, 121},
    {103,  89}, // Left side semifinal match
    {137,  89}, // Right side semifinal match
    {120,  89}, // Final match
};

// Each of these line sections define the position of the advancement line on the tourney tree for the victor of that round
// The trainers here are numbered by tourney ID (rank/seed) and ordered according to where they start on the tourney tree
#define LINESECTION_ROUND1_TRAINER1(lastSrc) \
    {.src = 0x6021,  .y = 0x04, .x = 0x09},  \
    {.src = 0x6023,  .y = 0x04, .x = 0x0a},  \
    {.src = 0x6047,  .y = 0x05, .x = 0x0a},  \
    {.src = lastSrc, .y = 0x05, .x = 0x0b},

#define LINESECTION_ROUND1_TRAINER9(lastSrc) \
    {.src = 0x6021,  .y = 0x06, .x = 0x09},  \
    {.src = 0x6021,  .y = 0x06, .x = 0x0a},  \
    {.src = 0x6027,  .y = 0x05, .x = 0x0a},  \
    {.src = lastSrc, .y = 0x05, .x = 0x0b},

#define LINESECTION_ROUND1_TRAINER13(lastSrc) \
    {.src = 0x6021,  .y = 0x08, .x = 0x09},   \
    {.src = 0x6023,  .y = 0x08, .x = 0x0a},   \
    {.src = 0x6047,  .y = 0x09, .x = 0x0a},   \
    {.src = lastSrc, .y = 0x09, .x = 0x0b},

#define LINESECTION_ROUND1_TRAINER5(lastSrc) \
    {.src = 0x6021,  .y = 0x0a, .x = 0x09},  \
    {.src = 0x6021,  .y = 0x0a, .x = 0x0a},  \
    {.src = 0x6027,  .y = 0x09, .x = 0x0a},  \
    {.src = lastSrc, .y = 0x09, .x = 0x0b},

#define LINESECTION_ROUND1_TRAINER8(lastSrc) \
    {.src = 0x6021,  .y = 0x0c, .x = 0x09},  \
    {.src = 0x6023,  .y = 0x0c, .x = 0x0a},  \
    {.src = 0x6047,  .y = 0x0d, .x = 0x0a},  \
    {.src = lastSrc, .y = 0x0d, .x = 0x0b},

#define LINESECTION_ROUND1_TRAINER16(lastSrc) \
    {.src = 0x6021,  .y = 0x0e, .x = 0x09},   \
    {.src = 0x6021,  .y = 0x0e, .x = 0x0a},   \
    {.src = 0x6027,  .y = 0x0d, .x = 0x0a},   \
    {.src = lastSrc, .y = 0x0d, .x = 0x0b},

#define LINESECTION_ROUND1_TRAINER12(lastSrc) \
    {.src = 0x6021,  .y = 0x10, .x = 0x09},   \
    {.src = 0x6023,  .y = 0x10, .x = 0x0a},   \
    {.src = 0x6047,  .y = 0x11, .x = 0x0a},   \
    {.src = lastSrc, .y = 0x11, .x = 0x0b},

#define LINESECTION_ROUND1_TRAINER4(lastSrc) \
    {.src = 0x602b,  .y = 0x12, .x = 0x09},  \
    {.src = 0x602b,  .y = 0x12, .x = 0x0a},  \
    {.src = 0x6027,  .y = 0x11, .x = 0x0a},  \
    {.src = lastSrc, .y = 0x11, .x = 0x0b},

#define LINESECTION_ROUND1_TRAINER3(lastSrc) \
    {.src = 0x6021,  .y = 0x04, .x = 0x14},  \
    {.src = 0x6025,  .y = 0x04, .x = 0x13},  \
    {.src = 0x6049,  .y = 0x05, .x = 0x13},  \
    {.src = lastSrc, .y = 0x05, .x = 0x12},

#define LINESECTION_ROUND1_TRAINER11(lastSrc) \
    {.src = 0x6021,  .y = 0x06, .x = 0x14},   \
    {.src = 0x6021,  .y = 0x06, .x = 0x13},   \
    {.src = 0x6029,  .y = 0x05, .x = 0x13},   \
    {.src = lastSrc, .y = 0x05, .x = 0x12},

#define LINESECTION_ROUND1_TRAINER15(lastSrc) \
    {.src = 0x6021,  .y = 0x08, .x = 0x14},   \
    {.src = 0x6025,  .y = 0x08, .x = 0x13},   \
    {.src = 0x6049,  .y = 0x09, .x = 0x13},   \
    {.src = lastSrc, .y = 0x09, .x = 0x12},

#define LINESECTION_ROUND1_TRAINER7(lastSrc) \
    {.src = 0x6021,  .y = 0x0a, .x = 0x14},  \
    {.src = 0x6021,  .y = 0x0a, .x = 0x13},  \
    {.src = 0x6029,  .y = 0x09, .x = 0x13},  \
    {.src = lastSrc, .y = 0x09, .x = 0x12},

#define LINESECTION_ROUND1_TRAINER6(lastSrc) \
    {.src = 0x6021,  .y = 0x0c, .x = 0x14},  \
    {.src = 0x6025,  .y = 0x0c, .x = 0x13},  \
    {.src = 0x6049,  .y = 0x0d, .x = 0x13},  \
    {.src = lastSrc, .y = 0x0d, .x = 0x12},

#define LINESECTION_ROUND1_TRAINER14(lastSrc) \
    {.src = 0x6021,  .y = 0x0e, .x = 0x14},   \
    {.src = 0x6021,  .y = 0x0e, .x = 0x13},   \
    {.src = 0x6029,  .y = 0x0d, .x = 0x13},   \
    {.src = lastSrc, .y = 0x0d, .x = 0x12},

#define LINESECTION_ROUND1_TRAINER10(lastSrc) \
    {.src = 0x6021,  .y = 0x10, .x = 0x14},   \
    {.src = 0x6025,  .y = 0x10, .x = 0x13},   \
    {.src = 0x6049,  .y = 0x11, .x = 0x13},   \
    {.src = lastSrc, .y = 0x11, .x = 0x12},

#define LINESECTION_ROUND1_TRAINER2(lastSrc) \
    {.src = 0x602b,  .y = 0x12, .x = 0x14},  \
    {.src = 0x602b,  .y = 0x12, .x = 0x13},  \
    {.src = 0x6029,  .y = 0x11, .x = 0x13},  \
    {.src = lastSrc, .y = 0x11, .x = 0x12},

#define LINESECTION_ROUND2_MATCH1(lastSrc)  \
    {.src = 0x6027,  .y = 0x06, .x = 0x0b}, \
    {.src = 0x6047,  .y = 0x07, .x = 0x0b}, \
    {.src = lastSrc, .y = 0x07, .x = 0x0c},

#define LINESECTION_ROUND2_MATCH2(lastSrc)  \
    {.src = 0x6027,  .y = 0x08, .x = 0x0b}, \
    {.src = 0x6027,  .y = 0x07, .x = 0x0b}, \
    {.src = lastSrc, .y = 0x07, .x = 0x0c},

#define LINESECTION_ROUND2_MATCH3(lastSrc)  \
    {.src = 0x6027,  .y = 0x0e, .x = 0x0b}, \
    {.src = 0x6047,  .y = 0x0f, .x = 0x0b}, \
    {.src = lastSrc, .y = 0x0f, .x = 0x0c},

#define LINESECTION_ROUND2_MATCH4(lastSrc)  \
    {.src = 0x6027,  .y = 0x10, .x = 0x0b}, \
    {.src = 0x6027,  .y = 0x0f, .x = 0x0b}, \
    {.src = lastSrc, .y = 0x0f, .x = 0x0c},

#define LINESECTION_ROUND2_MATCH5(lastSrc)  \
    {.src = 0x6029,  .y = 0x06, .x = 0x12}, \
    {.src = 0x6049,  .y = 0x07, .x = 0x12}, \
    {.src = lastSrc, .y = 0x07, .x = 0x11},

#define LINESECTION_ROUND2_MATCH6(lastSrc)  \
    {.src = 0x6029,  .y = 0x08, .x = 0x12}, \
    {.src = 0x6029,  .y = 0x07, .x = 0x12}, \
    {.src = lastSrc, .y = 0x07, .x = 0x11},

#define LINESECTION_ROUND2_MATCH7(lastSrc)  \
    {.src = 0x6029,  .y = 0x0e, .x = 0x12}, \
    {.src = 0x6049,  .y = 0x0f, .x = 0x12}, \
    {.src = lastSrc, .y = 0x0f, .x = 0x11},

#define LINESECTION_ROUND2_MATCH8(lastSrc)  \
    {.src = 0x6029,  .y = 0x10, .x = 0x12}, \
    {.src = 0x6029,  .y = 0x0f, .x = 0x12}, \
    {.src = lastSrc, .y = 0x0f, .x = 0x11},

#define LINESECTION_SEMIFINAL_TOP_LEFT     \
    {.src = 0x6027, .y = 0x08, .x = 0x0c}, \
    {.src = 0x6027, .y = 0x09, .x = 0x0c}, \
    {.src = 0x6027, .y = 0x0a, .x = 0x0c}, \
    {.src = 0x603b, .y = 0x0b, .x = 0x0c},

#define LINESECTION_SEMIFINAL_BOTTOM_LEFT  \
    {.src = 0x6033, .y = 0x0e, .x = 0x0c}, \
    {.src = 0x6032, .y = 0x0d, .x = 0x0c}, \
    {.src = 0x6031, .y = 0x0c, .x = 0x0c}, \
    {.src = 0x6030, .y = 0x0b, .x = 0x0c},

#define LINESECTION_SEMIFINAL_TOP_RIGHT    \
    {.src = 0x6029, .y = 0x08, .x = 0x11}, \
    {.src = 0x6029, .y = 0x09, .x = 0x11}, \
    {.src = 0x6029, .y = 0x0a, .x = 0x11}, \
    {.src = 0x603c, .y = 0x0b, .x = 0x11},

#define LINESECTION_SEMIFINAL_BOTTOM_RIGHT \
    {.src = 0x6038, .y = 0x0e, .x = 0x11}, \
    {.src = 0x6037, .y = 0x0d, .x = 0x11}, \
    {.src = 0x6036, .y = 0x0c, .x = 0x11}, \
    {.src = 0x6035, .y = 0x0b, .x = 0x11},

#define LINESECTION_FINAL_LEFT             \
    {.src = 0x602c, .y = 0x0b, .x = 0x0d}, \
    {.src = 0x602d, .y = 0x0b, .x = 0x0e},

#define LINESECTION_FINAL_RIGHT            \
    {.src = 0x602f, .y = 0x0b, .x = 0x10}, \
    {.src = 0x602e, .y = 0x0b, .x = 0x0f},


static const struct TourneyTreeLineSection sLineSectionTrainer1Round1[] =
{
    LINESECTION_ROUND1_TRAINER1(0x6043)
};

static const struct TourneyTreeLineSection sLineSectionTrainer1Round2[] =
{
    LINESECTION_ROUND1_TRAINER1(0x6023)
    LINESECTION_ROUND2_MATCH1(0x6043)
};

static const struct TourneyTreeLineSection sLineSectionTrainer1Semifinal[] =
{
    LINESECTION_ROUND1_TRAINER1(0x6023)
    LINESECTION_ROUND2_MATCH1(0x6023)
    LINESECTION_SEMIFINAL_TOP_LEFT
};

static const struct TourneyTreeLineSection sLineSectionTrainer1Final[] =
{
    LINESECTION_ROUND1_TRAINER1(0x6023)
    LINESECTION_ROUND2_MATCH1(0x6023)
    LINESECTION_SEMIFINAL_TOP_LEFT
    LINESECTION_FINAL_LEFT
};

static const struct TourneyTreeLineSection sLineSectionTrainer9Round1[] =
{
    LINESECTION_ROUND1_TRAINER9(0x6043)
};

static const struct TourneyTreeLineSection sLineSectionTrainer9Round2[] =
{
    LINESECTION_ROUND1_TRAINER9(0x6023)
    LINESECTION_ROUND2_MATCH1(0x6043)
};

static const struct TourneyTreeLineSection sLineSectionTrainer9Semifinal[] =
{
    LINESECTION_ROUND1_TRAINER9(0x6023)
    LINESECTION_ROUND2_MATCH1(0x6023)
    LINESECTION_SEMIFINAL_TOP_LEFT
};

static const struct TourneyTreeLineSection sLineSectionTrainer9Final[] =
{
    LINESECTION_ROUND1_TRAINER9(0x6023)
    LINESECTION_ROUND2_MATCH1(0x6023)
    LINESECTION_SEMIFINAL_TOP_LEFT
    LINESECTION_FINAL_LEFT
};

static const struct TourneyTreeLineSection sLineSectionTrainer13Round1[] =
{
    LINESECTION_ROUND1_TRAINER13(0x6021)
};

static const struct TourneyTreeLineSection sLineSectionTrainer13Round2[] =
{
    LINESECTION_ROUND1_TRAINER13(0x6021)
    LINESECTION_ROUND2_MATCH2(0x6043)
};

static const struct TourneyTreeLineSection sLineSectionTrainer13Semifinal[] =
{
    LINESECTION_ROUND1_TRAINER13(0x6021)
    LINESECTION_ROUND2_MATCH2(0x6023)
    LINESECTION_SEMIFINAL_TOP_LEFT
};

static const struct TourneyTreeLineSection sLineSectionTrainer13Final[] =
{
    LINESECTION_ROUND1_TRAINER13(0x6021)
    LINESECTION_ROUND2_MATCH2(0x6023)
    LINESECTION_SEMIFINAL_TOP_LEFT
    LINESECTION_FINAL_LEFT
};

static const struct TourneyTreeLineSection sLineSectionTrainer5Round1[] =
{
    LINESECTION_ROUND1_TRAINER5(0x6021)
};

static const struct TourneyTreeLineSection sLineSectionTrainer5Round2[] =
{
    LINESECTION_ROUND1_TRAINER5(0x6021)
    LINESECTION_ROUND2_MATCH2(0x6043)
};

static const struct TourneyTreeLineSection sLineSectionTrainer5Semifinal[] =
{
    LINESECTION_ROUND1_TRAINER5(0x6021)
    LINESECTION_ROUND2_MATCH2(0x6023)
    LINESECTION_SEMIFINAL_TOP_LEFT
};

static const struct TourneyTreeLineSection sLineSectionTrainer5Final[] =
{
    LINESECTION_ROUND1_TRAINER5(0x6021)
    LINESECTION_ROUND2_MATCH2(0x6023)
    LINESECTION_SEMIFINAL_TOP_LEFT
    LINESECTION_FINAL_LEFT
};

static const struct TourneyTreeLineSection sLineSectionTrainer8Round1[] =
{
    LINESECTION_ROUND1_TRAINER8(0x6043)
};

static const struct TourneyTreeLineSection sLineSectionTrainer8Round2[] =
{
    LINESECTION_ROUND1_TRAINER8(0x6023)
    LINESECTION_ROUND2_MATCH3(0x6021)
};

static const struct TourneyTreeLineSection sLineSectionTrainer8Semifinal[] =
{
    LINESECTION_ROUND1_TRAINER8(0x6023)
    LINESECTION_ROUND2_MATCH3(0x6021)
    LINESECTION_SEMIFINAL_BOTTOM_LEFT
};

static const struct TourneyTreeLineSection sLineSectionTrainer8Final[] =
{
    LINESECTION_ROUND1_TRAINER8(0x6023)
    LINESECTION_ROUND2_MATCH3(0x6021)
    LINESECTION_SEMIFINAL_BOTTOM_LEFT
    LINESECTION_FINAL_LEFT
};

static const struct TourneyTreeLineSection sLineSectionTrainer16Round1[] =
{
    LINESECTION_ROUND1_TRAINER16(0x6043)
};

static const struct TourneyTreeLineSection sLineSectionTrainer16Round2[] =
{
    LINESECTION_ROUND1_TRAINER16(0x6023)
    LINESECTION_ROUND2_MATCH3(0x6021)
};

static const struct TourneyTreeLineSection sLineSectionTrainer16Semifinal[] =
{
    LINESECTION_ROUND1_TRAINER16(0x6023)
    LINESECTION_ROUND2_MATCH3(0x6021)
    LINESECTION_SEMIFINAL_BOTTOM_LEFT
};

static const struct TourneyTreeLineSection sLineSectionTrainer16Final[] =
{
    LINESECTION_ROUND1_TRAINER16(0x6023)
    LINESECTION_ROUND2_MATCH3(0x6021)
    LINESECTION_SEMIFINAL_BOTTOM_LEFT
    LINESECTION_FINAL_LEFT
};

static const struct TourneyTreeLineSection sLineSectionTrainer12Round1[] =
{
    LINESECTION_ROUND1_TRAINER12(0x6021)
};

static const struct TourneyTreeLineSection sLineSectionTrainer12Round2[] =
{
    LINESECTION_ROUND1_TRAINER12(0x6021)
    LINESECTION_ROUND2_MATCH4(0x6021)
};

static const struct TourneyTreeLineSection sLineSectionTrainer12Semifinal[] =
{
    LINESECTION_ROUND1_TRAINER12(0x6021)
    LINESECTION_ROUND2_MATCH4(0x6021)
    LINESECTION_SEMIFINAL_BOTTOM_LEFT
};

static const struct TourneyTreeLineSection sLineSectionTrainer12Final[] =
{
    LINESECTION_ROUND1_TRAINER12(0x6021)
    LINESECTION_ROUND2_MATCH4(0x6021)
    LINESECTION_SEMIFINAL_BOTTOM_LEFT
    LINESECTION_FINAL_LEFT
};

static const struct TourneyTreeLineSection sLineSectionTrainer4Round1[] =
{
    LINESECTION_ROUND1_TRAINER4(0x6021)
};

static const struct TourneyTreeLineSection sLineSectionTrainer4Round2[] =
{
    LINESECTION_ROUND1_TRAINER4(0x6021)
    LINESECTION_ROUND2_MATCH4(0x6021)
};

static const struct TourneyTreeLineSection sLineSectionTrainer4Semifinal[] =
{
    LINESECTION_ROUND1_TRAINER4(0x6021)
    LINESECTION_ROUND2_MATCH4(0x6021)
    LINESECTION_SEMIFINAL_BOTTOM_LEFT
};

static const struct TourneyTreeLineSection sLineSectionTrainer4Final[] =
{
    LINESECTION_ROUND1_TRAINER4(0x6021)
    LINESECTION_ROUND2_MATCH4(0x6021)
    LINESECTION_SEMIFINAL_BOTTOM_LEFT
    LINESECTION_FINAL_LEFT
};

static const struct TourneyTreeLineSection sLineSectionTrainer3Round1[] =
{
    LINESECTION_ROUND1_TRAINER3(0x6045)
};

static const struct TourneyTreeLineSection sLineSectionTrainer3Round2[] =
{
    LINESECTION_ROUND1_TRAINER3(0x6025)
    LINESECTION_ROUND2_MATCH5(0x6045)
};

static const struct TourneyTreeLineSection sLineSectionTrainer3Semifinal[] =
{
    LINESECTION_ROUND1_TRAINER3(0x6025)
    LINESECTION_ROUND2_MATCH5(0x6025)
    LINESECTION_SEMIFINAL_TOP_RIGHT
};

static const struct TourneyTreeLineSection sLineSectionTrainer3Final[] =
{
    LINESECTION_ROUND1_TRAINER3(0x6025)
    LINESECTION_ROUND2_MATCH5(0x6025)
    LINESECTION_SEMIFINAL_TOP_RIGHT
    LINESECTION_FINAL_RIGHT
};

static const struct TourneyTreeLineSection sLineSectionTrainer11Round1[] =
{
    LINESECTION_ROUND1_TRAINER11(0x6045)
};

static const struct TourneyTreeLineSection sLineSectionTrainer11Round2[] =
{
    LINESECTION_ROUND1_TRAINER11(0x6025)
    LINESECTION_ROUND2_MATCH5(0x6045)
};

static const struct TourneyTreeLineSection sLineSectionTrainer11Semifinal[] =
{
    LINESECTION_ROUND1_TRAINER11(0x6025)
    LINESECTION_ROUND2_MATCH5(0x6025)
    LINESECTION_SEMIFINAL_TOP_RIGHT
};

static const struct TourneyTreeLineSection sLineSectionTrainer11Final[] =
{
    LINESECTION_ROUND1_TRAINER11(0x6025)
    LINESECTION_ROUND2_MATCH5(0x6025)
    LINESECTION_SEMIFINAL_TOP_RIGHT
    LINESECTION_FINAL_RIGHT
};

static const struct TourneyTreeLineSection sLineSectionTrainer15Round1[] =
{
    LINESECTION_ROUND1_TRAINER15(0x6021)
};

static const struct TourneyTreeLineSection sLineSectionTrainer15Round2[] =
{
    LINESECTION_ROUND1_TRAINER15(0x6021)
    LINESECTION_ROUND2_MATCH6(0x6045)
};

static const struct TourneyTreeLineSection sLineSectionTrainer15Semifinal[] =
{
    LINESECTION_ROUND1_TRAINER15(0x6021)
    LINESECTION_ROUND2_MATCH6(0x6025)
    LINESECTION_SEMIFINAL_TOP_RIGHT
};

static const struct TourneyTreeLineSection sLineSectionTrainer15Final[] =
{
    LINESECTION_ROUND1_TRAINER15(0x6021)
    LINESECTION_ROUND2_MATCH6(0x6025)
    LINESECTION_SEMIFINAL_TOP_RIGHT
    LINESECTION_FINAL_RIGHT
};

static const struct TourneyTreeLineSection sLineSectionTrainer7Round1[] =
{
    LINESECTION_ROUND1_TRAINER7(0x6021)
};

static const struct TourneyTreeLineSection sLineSectionTrainer7Round2[] =
{
    LINESECTION_ROUND1_TRAINER7(0x6021)
    LINESECTION_ROUND2_MATCH6(0x6045)
};

static const struct TourneyTreeLineSection sLineSectionTrainer7Semifinal[] =
{
    LINESECTION_ROUND1_TRAINER7(0x6021)
    LINESECTION_ROUND2_MATCH6(0x6025)
    LINESECTION_SEMIFINAL_TOP_RIGHT
};

static const struct TourneyTreeLineSection sLineSectionTrainer7Final[] =
{
    LINESECTION_ROUND1_TRAINER7(0x6021)
    LINESECTION_ROUND2_MATCH6(0x6025)
    LINESECTION_SEMIFINAL_TOP_RIGHT
    LINESECTION_FINAL_RIGHT
};

static const struct TourneyTreeLineSection sLineSectionTrainer6Round1[] =
{
    LINESECTION_ROUND1_TRAINER6(0x6045)
};

static const struct TourneyTreeLineSection sLineSectionTrainer6Round2[] =
{
    LINESECTION_ROUND1_TRAINER6(0x6025)
    LINESECTION_ROUND2_MATCH7(0x6021)
};

static const struct TourneyTreeLineSection sLineSectionTrainer6Semifinal[] =
{
    LINESECTION_ROUND1_TRAINER6(0x6025)
    LINESECTION_ROUND2_MATCH7(0x6021)
    LINESECTION_SEMIFINAL_BOTTOM_RIGHT
};

static const struct TourneyTreeLineSection sLineSectionTrainer6Final[] =
{
    LINESECTION_ROUND1_TRAINER6(0x6025)
    LINESECTION_ROUND2_MATCH7(0x6021)
    LINESECTION_SEMIFINAL_BOTTOM_RIGHT
    LINESECTION_FINAL_RIGHT
};

static const struct TourneyTreeLineSection sLineSectionTrainer14Round1[] =
{
    LINESECTION_ROUND1_TRAINER14(0x6045)
};

static const struct TourneyTreeLineSection sLineSectionTrainer14Round2[] =
{
    LINESECTION_ROUND1_TRAINER14(0x6025)
    LINESECTION_ROUND2_MATCH7(0x6021)
};

static const struct TourneyTreeLineSection sLineSectionTrainer14Semifinal[] =
{
    LINESECTION_ROUND1_TRAINER14(0x6025)
    LINESECTION_ROUND2_MATCH7(0x6021)
    LINESECTION_SEMIFINAL_BOTTOM_RIGHT
};

static const struct TourneyTreeLineSection sLineSectionTrainer14Final[] =
{
    LINESECTION_ROUND1_TRAINER14(0x6025)
    LINESECTION_ROUND2_MATCH7(0x6021)
    LINESECTION_SEMIFINAL_BOTTOM_RIGHT
    LINESECTION_FINAL_RIGHT
};

static const struct TourneyTreeLineSection sLineSectionTrainer10Round1[] =
{
    LINESECTION_ROUND1_TRAINER10(0x6021)
};

static const struct TourneyTreeLineSection sLineSectionTrainer10Round2[] =
{
    LINESECTION_ROUND1_TRAINER10(0x6021)
    LINESECTION_ROUND2_MATCH8(0x6021)
};

static const struct TourneyTreeLineSection sLineSectionTrainer10Semifinal[] =
{
    LINESECTION_ROUND1_TRAINER10(0x6021)
    LINESECTION_ROUND2_MATCH8(0x6021)
    LINESECTION_SEMIFINAL_BOTTOM_RIGHT
};

static const struct TourneyTreeLineSection sLineSectionTrainer10Final[] =
{
    LINESECTION_ROUND1_TRAINER10(0x6021)
    LINESECTION_ROUND2_MATCH8(0x6021)
    LINESECTION_SEMIFINAL_BOTTOM_RIGHT
    LINESECTION_FINAL_RIGHT
};

static const struct TourneyTreeLineSection sLineSectionTrainer2Round1[] =
{
    LINESECTION_ROUND1_TRAINER2(0x6021)
};

static const struct TourneyTreeLineSection sLineSectionTrainer2Round2[] =
{
    LINESECTION_ROUND1_TRAINER2(0x6021)
    LINESECTION_ROUND2_MATCH8(0x6021)
};

static const struct TourneyTreeLineSection sLineSectionTrainer2Semifinal[] =
{
    LINESECTION_ROUND1_TRAINER2(0x6021)
    LINESECTION_ROUND2_MATCH8(0x6021)
    LINESECTION_SEMIFINAL_BOTTOM_RIGHT
};

static const struct TourneyTreeLineSection sLineSectionTrainer2Final[] =
{
    LINESECTION_ROUND1_TRAINER2(0x6021)
    LINESECTION_ROUND2_MATCH8(0x6021)
    LINESECTION_SEMIFINAL_BOTTOM_RIGHT
    LINESECTION_FINAL_RIGHT
};

static const struct TourneyTreeLineSection *const sTourneyTreeLineSections[DOME_TOURNAMENT_TRAINERS_COUNT][DOME_ROUNDS_COUNT] =
{
    {sLineSectionTrainer1Round1,  sLineSectionTrainer1Round2,  sLineSectionTrainer1Semifinal,  sLineSectionTrainer1Final},
    {sLineSectionTrainer2Round1,  sLineSectionTrainer2Round2,  sLineSectionTrainer2Semifinal,  sLineSectionTrainer2Final},
    {sLineSectionTrainer3Round1,  sLineSectionTrainer3Round2,  sLineSectionTrainer3Semifinal,  sLineSectionTrainer3Final},
    {sLineSectionTrainer4Round1,  sLineSectionTrainer4Round2,  sLineSectionTrainer4Semifinal,  sLineSectionTrainer4Final},
    {sLineSectionTrainer5Round1,  sLineSectionTrainer5Round2,  sLineSectionTrainer5Semifinal,  sLineSectionTrainer5Final},
    {sLineSectionTrainer6Round1,  sLineSectionTrainer6Round2,  sLineSectionTrainer6Semifinal,  sLineSectionTrainer6Final},
    {sLineSectionTrainer7Round1,  sLineSectionTrainer7Round2,  sLineSectionTrainer7Semifinal,  sLineSectionTrainer7Final},
    {sLineSectionTrainer8Round1,  sLineSectionTrainer8Round2,  sLineSectionTrainer8Semifinal,  sLineSectionTrainer8Final},
    {sLineSectionTrainer9Round1,  sLineSectionTrainer9Round2,  sLineSectionTrainer9Semifinal,  sLineSectionTrainer9Final},
    {sLineSectionTrainer10Round1, sLineSectionTrainer10Round2, sLineSectionTrainer10Semifinal, sLineSectionTrainer10Final},
    {sLineSectionTrainer11Round1, sLineSectionTrainer11Round2, sLineSectionTrainer11Semifinal, sLineSectionTrainer11Final},
    {sLineSectionTrainer12Round1, sLineSectionTrainer12Round2, sLineSectionTrainer12Semifinal, sLineSectionTrainer12Final},
    {sLineSectionTrainer13Round1, sLineSectionTrainer13Round2, sLineSectionTrainer13Semifinal, sLineSectionTrainer13Final},
    {sLineSectionTrainer14Round1, sLineSectionTrainer14Round2, sLineSectionTrainer14Semifinal, sLineSectionTrainer14Final},
    {sLineSectionTrainer15Round1, sLineSectionTrainer15Round2, sLineSectionTrainer15Semifinal, sLineSectionTrainer15Final},
    {sLineSectionTrainer16Round1, sLineSectionTrainer16Round2, sLineSectionTrainer16Semifinal, sLineSectionTrainer16Final},
};

static const u8 sTourneyTreeLineSectionArrayCounts[DOME_TOURNAMENT_TRAINERS_COUNT][DOME_ROUNDS_COUNT] =
{
    {ARRAY_COUNT(sLineSectionTrainer1Round1),  ARRAY_COUNT(sLineSectionTrainer1Round2),  ARRAY_COUNT(sLineSectionTrainer1Semifinal),  ARRAY_COUNT(sLineSectionTrainer1Final)},
    {ARRAY_COUNT(sLineSectionTrainer2Round1),  ARRAY_COUNT(sLineSectionTrainer2Round2),  ARRAY_COUNT(sLineSectionTrainer2Semifinal),  ARRAY_COUNT(sLineSectionTrainer2Final)},
    {ARRAY_COUNT(sLineSectionTrainer3Round1),  ARRAY_COUNT(sLineSectionTrainer3Round2),  ARRAY_COUNT(sLineSectionTrainer3Semifinal),  ARRAY_COUNT(sLineSectionTrainer3Final)},
    {ARRAY_COUNT(sLineSectionTrainer4Round1),  ARRAY_COUNT(sLineSectionTrainer4Round2),  ARRAY_COUNT(sLineSectionTrainer4Semifinal),  ARRAY_COUNT(sLineSectionTrainer4Final)},
    {ARRAY_COUNT(sLineSectionTrainer5Round1),  ARRAY_COUNT(sLineSectionTrainer5Round2),  ARRAY_COUNT(sLineSectionTrainer5Semifinal),  ARRAY_COUNT(sLineSectionTrainer5Final)},
    {ARRAY_COUNT(sLineSectionTrainer6Round1),  ARRAY_COUNT(sLineSectionTrainer6Round2),  ARRAY_COUNT(sLineSectionTrainer6Semifinal),  ARRAY_COUNT(sLineSectionTrainer6Final)},
    {ARRAY_COUNT(sLineSectionTrainer7Round1),  ARRAY_COUNT(sLineSectionTrainer7Round2),  ARRAY_COUNT(sLineSectionTrainer7Semifinal),  ARRAY_COUNT(sLineSectionTrainer7Final)},
    {ARRAY_COUNT(sLineSectionTrainer8Round1),  ARRAY_COUNT(sLineSectionTrainer8Round2),  ARRAY_COUNT(sLineSectionTrainer8Semifinal),  ARRAY_COUNT(sLineSectionTrainer8Final)},
    {ARRAY_COUNT(sLineSectionTrainer9Round1),  ARRAY_COUNT(sLineSectionTrainer9Round2),  ARRAY_COUNT(sLineSectionTrainer9Semifinal),  ARRAY_COUNT(sLineSectionTrainer9Final)},
    {ARRAY_COUNT(sLineSectionTrainer10Round1), ARRAY_COUNT(sLineSectionTrainer10Round2), ARRAY_COUNT(sLineSectionTrainer10Semifinal), ARRAY_COUNT(sLineSectionTrainer10Final)},
    {ARRAY_COUNT(sLineSectionTrainer11Round1), ARRAY_COUNT(sLineSectionTrainer11Round2), ARRAY_COUNT(sLineSectionTrainer11Semifinal), ARRAY_COUNT(sLineSectionTrainer11Final)},
    {ARRAY_COUNT(sLineSectionTrainer12Round1), ARRAY_COUNT(sLineSectionTrainer12Round2), ARRAY_COUNT(sLineSectionTrainer12Semifinal), ARRAY_COUNT(sLineSectionTrainer12Final)},
    {ARRAY_COUNT(sLineSectionTrainer13Round1), ARRAY_COUNT(sLineSectionTrainer13Round2), ARRAY_COUNT(sLineSectionTrainer13Semifinal), ARRAY_COUNT(sLineSectionTrainer13Final)},
    {ARRAY_COUNT(sLineSectionTrainer14Round1), ARRAY_COUNT(sLineSectionTrainer14Round2), ARRAY_COUNT(sLineSectionTrainer14Semifinal), ARRAY_COUNT(sLineSectionTrainer14Final)},
    {ARRAY_COUNT(sLineSectionTrainer15Round1), ARRAY_COUNT(sLineSectionTrainer15Round2), ARRAY_COUNT(sLineSectionTrainer15Semifinal), ARRAY_COUNT(sLineSectionTrainer15Final)},
    {ARRAY_COUNT(sLineSectionTrainer16Round1), ARRAY_COUNT(sLineSectionTrainer16Round2), ARRAY_COUNT(sLineSectionTrainer16Semifinal), ARRAY_COUNT(sLineSectionTrainer16Final)},
};

void CallBattleDomeFunction(void)
{
    sBattleDomeFunctions[gSpecialVar_0x8004]();
}

static void InitDomeChallenge(void)
{
}

static void GetDomeData(void)
{
}

static void SetDomeData(void)
{
}

static void InitDomeTrainers(void)
{
}

#define CALC_STAT(base, statIndex)                                                          \
{                                                                                           \
    u8 baseStat = gSpeciesInfo[species].base;                                                 \
    stats[statIndex] = (((2 * baseStat + ivs + evs[statIndex] / 4) * level) / 100) + 5;     \
    stats[statIndex] = (u8) ModifyStatByNature(nature, stats[statIndex], statIndex);        \
}

static void CalcDomeMonStats(u16 species, int level, int ivs, u8 evBits, u8 nature, int *stats)
{
    int i, count;
    u8 bits;
    u16 resultingEvs;
    int evs[NUM_STATS];

    count = 0, bits = evBits;
    for (i = 0; i < NUM_STATS; bits >>= 1, i++)
    {
        if (bits & 1)
            count++;
    }

    resultingEvs = MAX_TOTAL_EVS / count;
    for (i = 0; i < NUM_STATS; bits <<= 1, i++)
    {
        evs[i] = 0;
        if (evBits & bits)
            evs[i] = resultingEvs;
    }

    if (species == SPECIES_SHEDINJA)
    {
        stats[STAT_HP] = 1;
    }
    else
    {
        int n = 2 * gSpeciesInfo[species].baseHP;
        stats[STAT_HP] = (((n + ivs + evs[STAT_HP] / 4) * level) / 100) + level + 10;
    }

    CALC_STAT(baseAttack, STAT_ATK);
    CALC_STAT(baseDefense, STAT_DEF);
    CALC_STAT(baseSpeed, STAT_SPEED);
    CALC_STAT(baseSpAttack, STAT_SPATK);
    CALC_STAT(baseSpDefense, STAT_SPDEF);
}

static void SwapDomeTrainers(int id1, int id2, u16 *statsArray)
{
}

static void BufferDomeRoundText(void)
{
}

static void BufferDomeOpponentName(void)
{
}

static void InitDomeOpponentParty(void)
{
    gPlayerPartyLostHP = 0;
    sPlayerPartyMaxHP =  GetMonData(&gPlayerParty[0], MON_DATA_MAX_HP, NULL);
    sPlayerPartyMaxHP += GetMonData(&gPlayerParty[1], MON_DATA_MAX_HP, NULL);
    CalculatePlayerPartyCount();
    CreateDomeOpponentMons(TrainerIdToTournamentId(gTrainerBattleOpponent_A));
}

static void CreateDomeOpponentMon(u8 monPartyId, u16 tournamentTrainerId, u8 tournamentMonId, u32 otId)
{
}

static void CreateDomeOpponentMons(u16 tournamentTrainerId)
{
    u8 monsCount = 0;
    u32 otId = 0;
    int i, selectedMonBits;

    ZeroEnemyPartyMons();
    selectedMonBits = GetDomeTrainerSelectedMons(tournamentTrainerId);
    otId = Random32();

    if (Random() % 10 > 5)
    {
        // Create mon if it was selected, starting from front
        for (i = 0; i < FRONTIER_PARTY_SIZE; i++)
        {
            if (selectedMonBits & 1)
            {
                CreateDomeOpponentMon(monsCount, tournamentTrainerId, i, otId);
                monsCount++;
            }
            selectedMonBits >>= 1;
        }
    }
    else
    {
        // Create mon if it was selected, starting from back
        for (i = FRONTIER_PARTY_SIZE - 1; i >= 0; i--)
        {
            if (selectedMonBits & (1 << (FRONTIER_PARTY_SIZE - 1)))
            {
                CreateDomeOpponentMon(monsCount, tournamentTrainerId, i, otId);
                monsCount++;
            }
            selectedMonBits <<= 1;
        }
    }
}

// Returns a bitmask representing which 2 of the trainer's 3 pokemon to select.
// The choice is calculated solely depending on the type effectiveness of their
// movesets against the player's pokemon.
// There is a 50% chance of either a "good" or "bad" selection mode being used.
// In the good mode movesets are preferred which are more effective against the
// player, and in the bad mode the opposite is true. If all 3 pokemon tie, the
// other mode will be tried. If they tie again, the pokemon selection is random.
int GetDomeTrainerSelectedMons(u16 tournamentTrainerId)
{
    int selectedMonBits;
    if (Random() & 1)
    {
        selectedMonBits = SelectOpponentMons_Good(tournamentTrainerId, FALSE);
        if (selectedMonBits == 0)
            selectedMonBits = SelectOpponentMons_Bad(tournamentTrainerId, TRUE);
    }
    else
    {
        selectedMonBits = SelectOpponentMons_Bad(tournamentTrainerId, FALSE);
        if (selectedMonBits == 0)
            selectedMonBits = SelectOpponentMons_Good(tournamentTrainerId, TRUE);
    }

    return selectedMonBits;
}

static int SelectOpponentMons_Good(u16 tournamentTrainerId, bool8 allowRandom)
{
}

// Identical to function above, but uses EFFECTIVENESS_MODE_BAD
static int SelectOpponentMons_Bad(u16 tournamentTrainerId, bool8 allowRandom)
{
}

static int SelectOpponentMonsFromParty(int *partyMovePoints, bool8 allowRandom)
{
    int i, j;
    int selectedMonBits = 0;
    int partyPositions[FRONTIER_PARTY_SIZE];

    for (i = 0; i < FRONTIER_PARTY_SIZE; i++)
        partyPositions[i] = i;

    // All party mons have equal move score totals, choose randomly
    if (partyMovePoints[0] == partyMovePoints[1]
     && partyMovePoints[0] == partyMovePoints[2])
    {
        if (allowRandom)
        {
            i = 0;
            while (i != DOME_BATTLE_PARTY_SIZE)
            {
                u32 rand = Random() & FRONTIER_PARTY_SIZE;
                if (rand != FRONTIER_PARTY_SIZE && !(selectedMonBits & gBitTable[rand]))
                {
                    selectedMonBits |= gBitTable[rand];
                    i++;
                }
            }
        }
    }
    else
    {
        for (i = 0; i < DOME_BATTLE_PARTY_SIZE; i++)
        {
            for (j = i + 1; j < FRONTIER_PARTY_SIZE; j++)
            {
                int temp;

                if (partyMovePoints[i] < partyMovePoints[j])
                {
                    SWAP(partyMovePoints[i], partyMovePoints[j],temp)
                    SWAP(partyPositions[i], partyPositions[j], temp)
                }

                if (partyMovePoints[i] == partyMovePoints[j] && (Random() & 1))
                {
                    SWAP(partyMovePoints[i], partyMovePoints[j],temp)
                    SWAP(partyPositions[i], partyPositions[j], temp)
                }
            }
        }

        for (i = 0; i < DOME_BATTLE_PARTY_SIZE; i++)
        {
            selectedMonBits |= gBitTable[partyPositions[i]];
        }
    }

    return selectedMonBits;
}

#define TYPE_x0     0
#define TYPE_x0_25  5
#define TYPE_x0_50  10
#define TYPE_x1     20
#define TYPE_x2     40
#define TYPE_x4     80

static int GetTypeEffectivenessPoints(int move, int targetSpecies, int mode)
{
    int defType1, defType2, defAbility, moveType;
    int i = 0;
    int typePower = TYPE_x1;

    if (move == MOVE_NONE || move == MOVE_UNAVAILABLE || IS_MOVE_STATUS(move))
        return 0;

    defType1 = gSpeciesInfo[targetSpecies].types[0];
    defType2 = gSpeciesInfo[targetSpecies].types[1];
    defAbility = gSpeciesInfo[targetSpecies].abilities[0];
    moveType = gBattleMoves[move].type;

    if (defAbility == ABILITY_LEVITATE && moveType == TYPE_GROUND)
    {
        // They likely meant to return here, as 8 is the number of points normally used in this mode for moves with no effect.
        // Because there's no return the value instead gets interpreted by the switch, and the number of points becomes 0.
        if (mode == EFFECTIVENESS_MODE_BAD)
        {
            typePower = 8;
        #ifdef BUGFIX
            return typePower;
        #endif
        }
    }
    else
    {
        u32 typeEffectiveness1 = UQ_4_12_TO_INT(GetTypeModifier(moveType, defType1) * 2) * 5;
        u32 typeEffectiveness2 = UQ_4_12_TO_INT(GetTypeModifier(moveType, defType2) * 2) * 5;

        typePower = (typeEffectiveness1 * typePower) / 10;
        if (defType2 != defType1)
            typePower = (typeEffectiveness2 * typePower) / 10;

        if (defAbility == ABILITY_WONDER_GUARD && typeEffectiveness1 != TYPE_x1 && typeEffectiveness2 != TYPE_x1)
            typePower = 0;
    }

    switch (mode)
    {
    case EFFECTIVENESS_MODE_GOOD:
        // Weights moves that more effective.
        switch (typePower)
        {
        case TYPE_x0:
        case TYPE_x0_25:
        case TYPE_x0_50:
        default:
            typePower = 0;
            break;
        case TYPE_x1:
            typePower = 2;
            break;
        case TYPE_x2:
            typePower = 4;
            break;
        case TYPE_x4:
            typePower = 8;
            break;
        }
        break;
    case EFFECTIVENESS_MODE_BAD:
        // Weights moves that are less effective.
        // Odd that there's no limit on this being used, even the Frontier Brain could end up using this.
        switch (typePower)
        {
        case TYPE_x0:
            typePower = 8;
            break;
        case TYPE_x0_25:
            typePower = 4;
            break;
        case TYPE_x0_50:
            typePower = 2;
            break;
        default:
        case TYPE_x1:
            typePower = 0;
            break;
        case TYPE_x2:
            typePower = -2;
            break;
        case TYPE_x4:
            typePower = -4;
            break;
        }
        break;
    case EFFECTIVENESS_MODE_AI_VS_AI:
        // Used as part of calculating the winner in a battle between two AIs.
        // Weights moves that are more effective much more strongly in both directions.
        switch (typePower)
        {
        case TYPE_x0:
            typePower = -16;
            break;
        case TYPE_x0_25:
            typePower = -8;
            break;
        case TYPE_x0_50:
        default:
            typePower = 0;
            break;
        case TYPE_x1:
            typePower = 4;
            break;
        case TYPE_x2:
            typePower = 12;
            break;
        case TYPE_x4:
            typePower = 20;
            break;
        }
        break;
    }

    return typePower;
}

// Duplicate of GetFrontierTrainerFixedIvs
// NOTE: In CreateDomeOpponentMon a tournament trainer ID (0-15) is passed instead, resulting in all IVs of 3
//       To fix, see CreateDomeOpponentMon
static u8 GetDomeTrainerMonIvs(u16 trainerId)
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

static int TournamentIdOfOpponent(int roundId, int trainerId)
{
}

static void SetDomeOpponentId(void)
{
    gTrainerBattleOpponent_A = TrainerIdOfPlayerOpponent();
}

// While not an issue in-game, this will overflow if called after the player's opponent for the current round has been eliminated
static u16 TrainerIdOfPlayerOpponent(void)
{
}

static void SetDomeOpponentGraphicsId(void)
{
    SetBattleFacilityTrainerGfxId(gTrainerBattleOpponent_A, 0);
}

static void SaveDomeChallenge(void)
{
}

static void IncrementDomeStreaks(void)
{
}

// For showing the opponent info card of the upcoming trainer
static void ShowDomeOpponentInfo(void)
{
    u8 taskId = CreateTask(Task_ShowTourneyInfoCard, 0);
    gTasks[taskId].tState = 0;
    gTasks[taskId].tTournamentId = TrainerIdToTournamentId(TrainerIdOfPlayerOpponent());
    gTasks[taskId].tMode = INFOCARD_NEXT_OPPONENT;
    gTasks[taskId].tPrevTaskId = 0;

    SetMainCallback2(CB2_TourneyTree);
}

// For showing the opponent info card or the match info card
static void Task_ShowTourneyInfoCard(u8 taskId)
{
    int i;
    int tournamentId = gTasks[taskId].tTournamentId;
    int mode = gTasks[taskId].tMode;
    int id = gTasks[taskId].tPrevTaskId;

    switch (gTasks[taskId].tState)
    {
    case 0:
        SetHBlankCallback(NULL);
        SetVBlankCallback(NULL);
        EnableInterrupts(INTR_FLAG_VBLANK);
        CpuFill32(0, (void *)VRAM, VRAM_SIZE);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sInfoCardBgTemplates, ARRAY_COUNT(sInfoCardBgTemplates));
        InitWindows(sInfoCardWindowTemplates);
        DeactivateAllTextPrinters();
        gBattle_BG0_X = 0;
        gBattle_BG0_Y = 0;
        gBattle_BG1_X = 0;
        gBattle_BG1_Y = 0;
        gBattle_BG3_X = 0;
        gBattle_BG3_Y = 0;
        if (mode == INFOCARD_MATCH)
            gBattle_BG2_X = 0, gBattle_BG2_Y = 0;
        else
            gBattle_BG2_X = 0, gBattle_BG2_Y = DISPLAY_HEIGHT;

        gTasks[taskId].tState++;
        break;
    case 1:
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_MOSAIC, 0);
        SetGpuReg(REG_OFFSET_WIN0H, 0);
        SetGpuReg(REG_OFFSET_WIN0V, 0);
        SetGpuReg(REG_OFFSET_WIN1H, 0);
        SetGpuReg(REG_OFFSET_WIN1V, 0);
        SetGpuReg(REG_OFFSET_WININ, 0);
        SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG_ALL | WINOUT_WIN01_OBJ | WINOUT_WIN01_CLR);
        ResetPaletteFade();
        ResetSpriteData();
        FreeAllSpritePalettes();
        gReservedSpritePaletteCount = 4;
        gTasks[taskId].tState++;
        break;
    case 2:
        DecompressAndLoadBgGfxUsingHeap(2, gDomeTourneyInfoCard_Gfx, 0x2000, 0, 0);
        DecompressAndLoadBgGfxUsingHeap(2, gDomeTourneyInfoCard_Tilemap, 0x2000, 0, 1);
        DecompressAndLoadBgGfxUsingHeap(3, gDomeTourneyInfoCardBg_Tilemap, 0x800, 0, 1);
        LoadCompressedSpriteSheet(sTourneyTreeButtonsSpriteSheet);
        LoadCompressedPalette(gDomeTourneyTree_Pal, BG_PLTT_OFFSET, BG_PLTT_SIZE);
        LoadCompressedPalette(gDomeTourneyTreeButtons_Pal, OBJ_PLTT_OFFSET, OBJ_PLTT_SIZE);
        LoadCompressedPalette(gBattleWindowTextPalette, BG_PLTT_ID(15), PLTT_SIZE_4BPP);
        if (mode == INFOCARD_MATCH)
            LoadCompressedPalette(gDomeTourneyMatchCardBg_Pal, BG_PLTT_ID(5), PLTT_SIZE_4BPP); // Changes the moving info card bg to orange when in match card mode
        CpuFill32(0, gPlttBufferFaded, PLTT_SIZE);
        ShowBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        gTasks[taskId].tState++;
        break;
    case 3:
        SetVBlankCallback(VblankCb_TourneyInfoCard);
        sInfoCard = AllocZeroed(sizeof(*sInfoCard));
        for (i = 0; i < NUM_INFOCARD_SPRITES; i++)
            sInfoCard->spriteIds[i] = SPRITE_NONE;
        LoadMonIconPalettes();
        i = CreateTask(Task_HandleInfoCardInput, 0);
        gTasks[i].data[0] = 0;
        gTasks[i].data[2] = 0;
        gTasks[i].data[3] = mode;
        gTasks[i].data[4] = id;
        if (mode == INFOCARD_MATCH)
        {
            DisplayMatchInfoOnCard(0, tournamentId);
            sInfoCard->pos = 1;
        }
        else
        {
            DisplayTrainerInfoOnCard(0, tournamentId);
        }
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_BG_ALL_ON | DISPCNT_OBJ_1D_MAP);
        if (mode != INFOCARD_NEXT_OPPONENT)
        {
            // Scroll up arrow
            id = CreateSprite(&sVerticalScrollArrowSpriteTemplate, 120, 4, 0);
            StartSpriteAnim(&gSprites[id], 0);
            gSprites[id].data[0] = i;

            // Scroll down arrow
            id = CreateSprite(&sVerticalScrollArrowSpriteTemplate, 120, 156, 0);
            StartSpriteAnim(&gSprites[id], 1);
            gSprites[id].data[0] = i;

            // Scroll left arrow
            id = CreateSprite(&sHorizontalScrollArrowSpriteTemplate, 6, 80, 0);
            StartSpriteAnim(&gSprites[id], 0);
            gSprites[id].data[0] = i;
            gSprites[id].data[1] = 0;
            if (mode == INFOCARD_TRAINER)
                gSprites[id].invisible = TRUE;

            // Scroll right arrow
            id = CreateSprite(&sHorizontalScrollArrowSpriteTemplate, 234, 80, 0);
            StartSpriteAnim(&gSprites[id], 1);
            gSprites[id].data[0] = i;
            gSprites[id].data[1] = 1;
        }
        DestroyTask(taskId);
        break;
    }
}

// Note: Card scrolling up means the current card goes down and another one appears from top.
// The same is true for scrolling left.
// That means that the sprite needs to move with the moving card in the opposite scrolling direction.
static void SpriteCB_TrainerIconCardScrollUp(struct Sprite *sprite)
{
    sprite->y += 4;
    if (sprite->data[0] != 0)
    {
        if (sprite->y >= -32)
            sprite->invisible = FALSE;
        if (++sprite->data[1] == 40)
            sprite->callback = SpriteCallbackDummy;
    }
    else
    {
        if (sprite->y >= 192)
        {
            sInfoCard->spriteIds[sprite->data[2]] = SPRITE_NONE;
            FreeAndDestroyTrainerPicSprite(sprite->data[3]);
        }
    }
}

static void SpriteCB_TrainerIconCardScrollDown(struct Sprite *sprite)
{
    sprite->y -= 4;
    if (sprite->data[0] != 0)
    {
        if (sprite->y <= 192)
            sprite->invisible = FALSE;
        if (++sprite->data[1] == 40)
            sprite->callback = SpriteCallbackDummy;
    }
    else
    {
        if (sprite->y <= -32)
        {
            sInfoCard->spriteIds[sprite->data[2]] = SPRITE_NONE;
            FreeAndDestroyTrainerPicSprite(sprite->data[3]);
        }
    }
}

static void SpriteCB_TrainerIconCardScrollLeft(struct Sprite *sprite)
{
    sprite->x += 4;
    if (sprite->data[0] != 0)
    {
        if (sprite->x >= -32)
            sprite->invisible = FALSE;
        if (++sprite->data[1] == 64)
            sprite->callback = SpriteCallbackDummy;
    }
    else
    {
        if (sprite->x >= DISPLAY_WIDTH + 32)
        {
            sInfoCard->spriteIds[sprite->data[2]] = SPRITE_NONE;
            FreeAndDestroyTrainerPicSprite(sprite->data[3]);
        }
    }
}

static void SpriteCB_TrainerIconCardScrollRight(struct Sprite *sprite)
{
    sprite->x -= 4;
    if (sprite->data[0] != 0)
    {
        if (sprite->x <= DISPLAY_WIDTH + 32)
            sprite->invisible = FALSE;
        if (++sprite->data[1] == 64)
            sprite->callback = SpriteCallbackDummy;
    }
    else
    {
        if (sprite->x <= -32)
        {
            sInfoCard->spriteIds[sprite->data[2]] = SPRITE_NONE;
            FreeAndDestroyTrainerPicSprite(sprite->data[3]);
        }
    }
}

#define sMonIconStill data[3]

static void SpriteCB_MonIconDomeInfo(struct Sprite *sprite)
{
    if (!sprite->sMonIconStill)
        UpdateMonIconFrame(sprite);
}

static void SpriteCB_MonIconCardScrollUp(struct Sprite *sprite)
{
    if (!sprite->sMonIconStill)
        UpdateMonIconFrame(sprite);
    sprite->y += 4;
    if (sprite->data[0] != 0)
    {
        if (sprite->y >= -16)
            sprite->invisible = FALSE;
        if (++sprite->data[1] == 40)
            sprite->callback = SpriteCB_MonIconDomeInfo;
    }
    else
    {
        if (sprite->y >= 176)
        {
            sInfoCard->spriteIds[sprite->data[2]] = SPRITE_NONE;
            FreeAndDestroyMonIconSprite(sprite);
        }
    }
}

static void SpriteCB_MonIconCardScrollDown(struct Sprite *sprite)
{
    if (!sprite->sMonIconStill)
        UpdateMonIconFrame(sprite);
    sprite->y -= 4;
    if (sprite->data[0] != 0)
    {
        if (sprite->y <= 176)
            sprite->invisible = FALSE;
        if (++sprite->data[1] == 40)
            sprite->callback = SpriteCB_MonIconDomeInfo;
    }
    else
    {
        if (sprite->y <= -16)
        {
            sInfoCard->spriteIds[sprite->data[2]] = SPRITE_NONE;
            FreeAndDestroyMonIconSprite(sprite);
        }
    }
}

static void SpriteCB_MonIconCardScrollLeft(struct Sprite *sprite)
{
    if (!sprite->sMonIconStill)
        UpdateMonIconFrame(sprite);
    sprite->x += 4;
    if (sprite->data[0] != 0)
    {
        if (sprite->x >= -16)
            sprite->invisible = FALSE;
        if (++sprite->data[1] == 64)
            sprite->callback = SpriteCB_MonIconDomeInfo;
    }
    else
    {
        if (sprite->x >= DISPLAY_WIDTH + 16)
        {
            sInfoCard->spriteIds[sprite->data[2]] = SPRITE_NONE;
            FreeAndDestroyMonIconSprite(sprite);
        }
    }
}

static void SpriteCB_MonIconCardScrollRight(struct Sprite *sprite)
{
    if (!sprite->sMonIconStill)
        UpdateMonIconFrame(sprite);
    sprite->x -= 4;
    if (sprite->data[0] != 0)
    {
        if (sprite->x <= DISPLAY_WIDTH + 16)
            sprite->invisible = FALSE;
        if (++sprite->data[1] == 64)
            sprite->callback = SpriteCB_MonIconDomeInfo;
    }
    else
    {
        if (sprite->x <= -16)
        {
            sInfoCard->spriteIds[sprite->data[2]] = SPRITE_NONE;
            FreeAndDestroyMonIconSprite(sprite);
        }
    }
}

static void SpriteCB_HorizontalScrollArrow(struct Sprite *sprite)
{
}

static void SpriteCB_VerticalScrollArrow(struct Sprite *sprite)
{
    int taskId1 = sprite->data[0];

    if (gTasks[taskId1].data[3] == 1)
    {
        if (sInfoCard->pos != 0)
        {
            if (gTasks[taskId1].data[0] == 2)
                sprite->invisible = TRUE;
        }
        else
        {
            sprite->invisible = FALSE;
        }
    }
    else
    {
        if (sInfoCard->pos != 1)
        {
            if (gTasks[taskId1].data[0] == 2)
                sprite->invisible = TRUE;
        }
        else
        {
            sprite->invisible = FALSE;
        }
    }
}

// Task states for Task_HandleInfoCardInput
#define STATE_FADE_IN      0
#define STATE_WAIT_FADE    1
#define STATE_GET_INPUT    2
#define STATE_REACT_INPUT  3
#define STATE_MOVE_UP      4
#define STATE_MOVE_DOWN    5
#define STATE_MOVE_LEFT    6
#define STATE_MOVE_RIGHT   7
#define STATE_CLOSE_CARD   8

#define tUsingAlternateSlot data[2] // CARD_ALTERNATE_SLOT

static void Task_HandleInfoCardInput(u8 taskId)
{
    int i;
    int windowId = 0;
    int mode = gTasks[taskId].data[3];
    int taskId2 = gTasks[taskId].data[4];
    int trainerTourneyId = 0;
    int matchNo = 0;

    switch (gTasks[taskId].tState)
    {
    case STATE_FADE_IN:
        if (!gPaletteFade.active)
        {
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0x10, 0, RGB_BLACK);
            gTasks[taskId].tState = STATE_WAIT_FADE;
        }
        break;
    case STATE_WAIT_FADE:
        if (!gPaletteFade.active)
            gTasks[taskId].tState = STATE_GET_INPUT;
        break;
    case STATE_GET_INPUT:
        i = Task_GetInfoCardInput(taskId);
        switch (i)
        {
        case INFOCARD_INPUT_AB:
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
            gTasks[taskId].tState = STATE_CLOSE_CARD;
            break;
        case TRAINERCARD_INPUT_UP ... TRAINERCARD_INPUT_RIGHT:
        case MATCHCARD_INPUT_UP ... MATCHCARD_INPUT_RIGHT:
            gTasks[taskId].data[5] = i;
            if (gTasks[taskId].tUsingAlternateSlot)
                windowId = 9;
            else
                windowId = 0;

            for (i = windowId; i < windowId + 9; i++)
            {
                CopyWindowToVram(i, COPYWIN_GFX);
                FillWindowPixelBuffer(i, PIXEL_FILL(0));
            }
            gTasks[taskId].tState = STATE_REACT_INPUT;
            break;
        case INFOCARD_INPUT_NONE:
            break;
        }
        break;
    case STATE_REACT_INPUT:
        i = gTasks[taskId].data[5];
        switch (i)
        {
        case TRAINERCARD_INPUT_UP:
        case MATCHCARD_INPUT_UP:
            if (gTasks[taskId].tUsingAlternateSlot)
            {
                gBattle_BG0_X = 0;
                gBattle_BG0_Y = 0;
                gBattle_BG1_X = 0;
                gBattle_BG1_Y = DISPLAY_HEIGHT;
            }
            else
            {
                gBattle_BG0_X = 0;
                gBattle_BG0_Y = DISPLAY_HEIGHT;
                gBattle_BG1_X = 0;
                gBattle_BG1_Y = 0;
            }

            if (i == TRAINERCARD_INPUT_UP)
            {
                if (sInfoCard->pos == 0)
                {
                    gBattle_BG2_X = 0;
                    gBattle_BG2_Y = DISPLAY_HEIGHT * 2;
                    trainerTourneyId = sTourneyTreeTrainerIds[gTasks[taskId2].data[1]];
                    DisplayTrainerInfoOnCard(gTasks[taskId].tUsingAlternateSlot | MOVE_CARD_UP, trainerTourneyId);
                }
                else
                {
                    gBattle_BG2_X = DISPLAY_WIDTH + 16;
                    gBattle_BG2_Y = 0;
                    trainerTourneyId = sTourneyTreeTrainerIds[gTasks[taskId2].data[1]];
                    DisplayTrainerInfoOnCard(gTasks[taskId].tUsingAlternateSlot | MOVE_CARD_UP, trainerTourneyId);
                    sInfoCard->pos = 0;
                }
            }
            else // i == MATCHCARD_INPUT_UP
            {
                if (sInfoCard->pos == 0)
                {
                    matchNo = gTasks[taskId2].data[1] - 16;
                    BufferDomeWinString(matchNo, sInfoCard->tournamentIds);
                    gBattle_BG2_X = 0;
                    gBattle_BG2_Y = DISPLAY_HEIGHT * 2;
                    trainerTourneyId = sInfoCard->tournamentIds[0];
                    DisplayTrainerInfoOnCard(gTasks[taskId].tUsingAlternateSlot | MOVE_CARD_UP, trainerTourneyId);
                }
                else if (sInfoCard->pos == 2)
                {
                    matchNo = gTasks[taskId2].data[1] - 16;
                    BufferDomeWinString(matchNo, sInfoCard->tournamentIds);
                    gBattle_BG2_X = 0;
                    gBattle_BG2_Y = DISPLAY_HEIGHT * 2;
                    trainerTourneyId = sInfoCard->tournamentIds[1];
                    DisplayTrainerInfoOnCard(gTasks[taskId].tUsingAlternateSlot | MOVE_CARD_UP, trainerTourneyId);
                }
                else
                {
                    gBattle_BG2_X = DISPLAY_WIDTH + 16;
                    gBattle_BG2_Y = DISPLAY_HEIGHT;
                    matchNo = gTasks[taskId2].data[1] - 16;
                    DisplayMatchInfoOnCard(gTasks[taskId].tUsingAlternateSlot | MOVE_CARD_UP, matchNo);
                }
            }

            for (i = 0; i < NUM_INFOCARD_SPRITES / 2; i++)
            {
                if (i < 2)
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                    {
                        gSprites[sInfoCard->spriteIds[i]].callback = SpriteCB_TrainerIconCardScrollUp;
                        gSprites[sInfoCard->spriteIds[i]].data[0] = gTasks[taskId].tUsingAlternateSlot ^ 1;
                        gSprites[sInfoCard->spriteIds[i]].data[1] = 0;
                        gSprites[sInfoCard->spriteIds[i]].data[2] = i;
                        gSprites[sInfoCard->spriteIds[i]].data[3] = sInfoCard->spriteIds[i];
                    }
                }
                else
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                    {
                        gSprites[sInfoCard->spriteIds[i]].callback = SpriteCB_MonIconCardScrollUp;
                        gSprites[sInfoCard->spriteIds[i]].data[0] = gTasks[taskId].tUsingAlternateSlot ^ 1;
                        gSprites[sInfoCard->spriteIds[i]].data[1] = 0;
                        gSprites[sInfoCard->spriteIds[i]].data[2] = i;
                    }
                }
            }
            for (i = NUM_INFOCARD_SPRITES / 2; i < NUM_INFOCARD_SPRITES; i++)
            {
                if (i < 10)
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                    {
                        gSprites[sInfoCard->spriteIds[i]].callback = SpriteCB_TrainerIconCardScrollUp;
                        gSprites[sInfoCard->spriteIds[i]].data[0] = gTasks[taskId].tUsingAlternateSlot;
                        gSprites[sInfoCard->spriteIds[i]].data[1] = 0;
                        gSprites[sInfoCard->spriteIds[i]].data[2] = i;
                        gSprites[sInfoCard->spriteIds[i]].data[3] = sInfoCard->spriteIds[i];
                    }
                }
                else
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                    {
                        gSprites[sInfoCard->spriteIds[i]].callback = SpriteCB_MonIconCardScrollUp;
                        gSprites[sInfoCard->spriteIds[i]].data[0] = gTasks[taskId].tUsingAlternateSlot;
                        gSprites[sInfoCard->spriteIds[i]].data[1] = 0;
                        gSprites[sInfoCard->spriteIds[i]].data[2] = i;
                    }
                }
            }

            gTasks[taskId].tState = STATE_MOVE_UP;
            gTasks[taskId].data[5] = 0;
            break;
        case TRAINERCARD_INPUT_DOWN:
        case MATCHCARD_INPUT_DOWN:
            if (gTasks[taskId].tUsingAlternateSlot)
            {
                gBattle_BG0_X = 0;
                gBattle_BG0_Y = 0;
                gBattle_BG1_X = 0;
                gBattle_BG1_Y = -DISPLAY_HEIGHT;
            }
            else
            {
                gBattle_BG0_X = 0;
                gBattle_BG0_Y = -DISPLAY_HEIGHT;
                gBattle_BG1_X = 0;
                gBattle_BG1_Y = 0;
            }

            if (i == TRAINERCARD_INPUT_DOWN)
            {
                if (sInfoCard->pos == 0)
                {
                    gBattle_BG2_X = 0;
                    gBattle_BG2_Y = DISPLAY_HEIGHT;
                    trainerTourneyId = sTourneyTreeTrainerIds[gTasks[taskId2].data[1]];
                    DisplayTrainerInfoOnCard(gTasks[taskId].tUsingAlternateSlot | MOVE_CARD_DOWN, trainerTourneyId);
                }
                else
                {
                    gBattle_BG2_X = 0;
                    gBattle_BG2_Y = 0;
                    trainerTourneyId = sTourneyTreeTrainerIds[gTasks[taskId2].data[1]];
                    DisplayTrainerInfoOnCard(gTasks[taskId].tUsingAlternateSlot | MOVE_CARD_DOWN, trainerTourneyId);
                    sInfoCard->pos = 0;
                }
            }
            else // i == MATCHCARD_INPUT_DOWN
            {
                if (sInfoCard->pos == 0)
                {
                    matchNo = gTasks[taskId2].data[1] - 16;
                    BufferDomeWinString(matchNo, sInfoCard->tournamentIds);
                    gBattle_BG2_X = 0;
                    gBattle_BG2_Y = DISPLAY_HEIGHT;
                    trainerTourneyId = sInfoCard->tournamentIds[0];
                    DisplayTrainerInfoOnCard(gTasks[taskId].tUsingAlternateSlot | MOVE_CARD_DOWN, trainerTourneyId);
                }
                else if (sInfoCard->pos == 2)
                {
                    matchNo = gTasks[taskId2].data[1] - 16;
                    BufferDomeWinString(matchNo, sInfoCard->tournamentIds);
                    gBattle_BG2_X = 0;
                    gBattle_BG2_Y = DISPLAY_HEIGHT;
                    trainerTourneyId = sInfoCard->tournamentIds[1];
                    DisplayTrainerInfoOnCard(gTasks[taskId].tUsingAlternateSlot | MOVE_CARD_DOWN, trainerTourneyId);
                }
                else
                {
                    gBattle_BG2_X = DISPLAY_WIDTH + 16;
                    gBattle_BG2_Y = 0;
                    matchNo = gTasks[taskId2].data[1] - 16;
                    DisplayMatchInfoOnCard(gTasks[taskId].tUsingAlternateSlot | MOVE_CARD_DOWN, matchNo);
                }
            }

            for (i = 0; i < NUM_INFOCARD_SPRITES / 2; i++)
            {
                if (i < 2)
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                    {
                        gSprites[sInfoCard->spriteIds[i]].callback = SpriteCB_TrainerIconCardScrollDown;
                        gSprites[sInfoCard->spriteIds[i]].data[0] = gTasks[taskId].tUsingAlternateSlot ^ 1;
                        gSprites[sInfoCard->spriteIds[i]].data[1] = 0;
                        gSprites[sInfoCard->spriteIds[i]].data[2] = i;
                        gSprites[sInfoCard->spriteIds[i]].data[3] = sInfoCard->spriteIds[i];
                    }
                }
                else
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                    {
                        gSprites[sInfoCard->spriteIds[i]].callback = SpriteCB_MonIconCardScrollDown;
                        gSprites[sInfoCard->spriteIds[i]].data[0] = gTasks[taskId].tUsingAlternateSlot ^ 1;
                        gSprites[sInfoCard->spriteIds[i]].data[1] = 0;
                        gSprites[sInfoCard->spriteIds[i]].data[2] = i;
                    }
                }
            }
            for (i = NUM_INFOCARD_SPRITES / 2; i < NUM_INFOCARD_SPRITES; i++)
            {
                if (i < 10)
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                    {
                        gSprites[sInfoCard->spriteIds[i]].callback = SpriteCB_TrainerIconCardScrollDown;
                        gSprites[sInfoCard->spriteIds[i]].data[0] = gTasks[taskId].tUsingAlternateSlot;
                        gSprites[sInfoCard->spriteIds[i]].data[1] = 0;
                        gSprites[sInfoCard->spriteIds[i]].data[2] = i;
                        gSprites[sInfoCard->spriteIds[i]].data[3] = sInfoCard->spriteIds[i];
                    }
                }
                else
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                    {
                        gSprites[sInfoCard->spriteIds[i]].callback = SpriteCB_MonIconCardScrollDown;
                        gSprites[sInfoCard->spriteIds[i]].data[0] = gTasks[taskId].tUsingAlternateSlot;
                        gSprites[sInfoCard->spriteIds[i]].data[1] = 0;
                        gSprites[sInfoCard->spriteIds[i]].data[2] = i;
                    }
                }
            }

            gTasks[taskId].tState = STATE_MOVE_DOWN;
            gTasks[taskId].data[5] = 0;
            break;
        case TRAINERCARD_INPUT_LEFT:
            if (gTasks[taskId].tUsingAlternateSlot)
            {
                gBattle_BG0_X = 0;
                gBattle_BG0_Y = 0;
                gBattle_BG1_X = DISPLAY_WIDTH + 16;
                gBattle_BG1_Y = 0;
            }
            else
            {
                gBattle_BG0_X = DISPLAY_WIDTH + 16;
                gBattle_BG0_Y = 0;
                gBattle_BG1_X = 0;
                gBattle_BG1_Y = 0;
            }

            if (sInfoCard->pos == 0)
            {
                gBattle_BG2_X = DISPLAY_WIDTH + 16;
                gBattle_BG2_Y = DISPLAY_HEIGHT;
                trainerTourneyId = sTourneyTreeTrainerIds[gTasks[taskId2].data[1]];
                DisplayTrainerInfoOnCard(gTasks[taskId].tUsingAlternateSlot | MOVE_CARD_LEFT, trainerTourneyId);
            }
            else
            {
                gBattle_BG2_X = DISPLAY_WIDTH + 16;
                gBattle_BG2_Y = 0;
                matchNo = sIdToMatchNumber[gTasks[taskId2].data[1]][sInfoCard->pos - 1];
                DisplayMatchInfoOnCard(gTasks[taskId].tUsingAlternateSlot | MOVE_CARD_LEFT, matchNo);
            }

            for (i = 0; i < NUM_INFOCARD_SPRITES / 2; i++)
            {
                if (i < 2)
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                    {
                        gSprites[sInfoCard->spriteIds[i]].callback = SpriteCB_TrainerIconCardScrollLeft;
                        gSprites[sInfoCard->spriteIds[i]].data[0] = gTasks[taskId].tUsingAlternateSlot ^ 1;
                        gSprites[sInfoCard->spriteIds[i]].data[1] = 0;
                        gSprites[sInfoCard->spriteIds[i]].data[2] = i;
                        gSprites[sInfoCard->spriteIds[i]].data[3] = sInfoCard->spriteIds[i];
                    }
                }
                else
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                    {
                        gSprites[sInfoCard->spriteIds[i]].callback = SpriteCB_MonIconCardScrollLeft;
                        gSprites[sInfoCard->spriteIds[i]].data[0] = gTasks[taskId].tUsingAlternateSlot ^ 1;
                        gSprites[sInfoCard->spriteIds[i]].data[1] = 0;
                        gSprites[sInfoCard->spriteIds[i]].data[2] = i;
                    }
                }
            }
            for (i = NUM_INFOCARD_SPRITES / 2; i < NUM_INFOCARD_SPRITES; i++)
            {
                if (i < 10)
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                    {
                        gSprites[sInfoCard->spriteIds[i]].callback = SpriteCB_TrainerIconCardScrollLeft;
                        gSprites[sInfoCard->spriteIds[i]].data[0] = gTasks[taskId].tUsingAlternateSlot;
                        gSprites[sInfoCard->spriteIds[i]].data[1] = 0;
                        gSprites[sInfoCard->spriteIds[i]].data[2] = i;
                        gSprites[sInfoCard->spriteIds[i]].data[3] = sInfoCard->spriteIds[i];
                    }
                }
                else
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                    {
                        gSprites[sInfoCard->spriteIds[i]].callback = SpriteCB_MonIconCardScrollLeft;
                        gSprites[sInfoCard->spriteIds[i]].data[0] = gTasks[taskId].tUsingAlternateSlot;
                        gSprites[sInfoCard->spriteIds[i]].data[1] = 0;
                        gSprites[sInfoCard->spriteIds[i]].data[2] = i;
                    }
                }
            }

            gTasks[taskId].tState = STATE_MOVE_LEFT;
            gTasks[taskId].data[5] = 0;
            break;
        case MATCHCARD_INPUT_LEFT:
            if (gTasks[taskId].tUsingAlternateSlot)
            {
                gBattle_BG0_X = 0;
                gBattle_BG0_Y = 0;
                gBattle_BG1_X = DISPLAY_WIDTH + 16;
                gBattle_BG1_Y = 0;
            }
            else
            {
                gBattle_BG0_X = DISPLAY_WIDTH + 16;
                gBattle_BG0_Y = 0;
                gBattle_BG1_X = 0;
                gBattle_BG1_Y = 0;
            }

            if (sInfoCard->pos == 0)
            {
                gBattle_BG2_X = DISPLAY_WIDTH + 16;
                gBattle_BG2_Y = DISPLAY_HEIGHT;
                trainerTourneyId = sInfoCard->tournamentIds[0];
                DisplayTrainerInfoOnCard(gTasks[taskId].tUsingAlternateSlot | MOVE_CARD_LEFT, trainerTourneyId);
            }
            else
            {
                gBattle_BG2_X = 0;
                gBattle_BG2_Y = DISPLAY_HEIGHT;
                matchNo = gTasks[taskId2].data[1] - 16;
                DisplayMatchInfoOnCard(gTasks[taskId].tUsingAlternateSlot | MOVE_CARD_LEFT, matchNo);
            }

            for (i = 0; i < NUM_INFOCARD_SPRITES / 2; i++)
            {
                if (i < 2)
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                    {
                        gSprites[sInfoCard->spriteIds[i]].callback = SpriteCB_TrainerIconCardScrollLeft;
                        gSprites[sInfoCard->spriteIds[i]].data[0] = gTasks[taskId].tUsingAlternateSlot ^ 1;
                        gSprites[sInfoCard->spriteIds[i]].data[1] = 0;
                        gSprites[sInfoCard->spriteIds[i]].data[2] = i;
                        gSprites[sInfoCard->spriteIds[i]].data[3] = sInfoCard->spriteIds[i];
                    }
                }
                else
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                    {
                        gSprites[sInfoCard->spriteIds[i]].callback = SpriteCB_MonIconCardScrollLeft;
                        gSprites[sInfoCard->spriteIds[i]].data[0] = gTasks[taskId].tUsingAlternateSlot ^ 1;
                        gSprites[sInfoCard->spriteIds[i]].data[1] = 0;
                        gSprites[sInfoCard->spriteIds[i]].data[2] = i;
                    }
                }
            }
            for (i = NUM_INFOCARD_SPRITES / 2; i < NUM_INFOCARD_SPRITES; i++)
            {
                if (i < 10)
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                    {
                        gSprites[sInfoCard->spriteIds[i]].callback = SpriteCB_TrainerIconCardScrollLeft;
                        gSprites[sInfoCard->spriteIds[i]].data[0] = gTasks[taskId].tUsingAlternateSlot;
                        gSprites[sInfoCard->spriteIds[i]].data[1] = 0;
                        gSprites[sInfoCard->spriteIds[i]].data[2] = i;
                        gSprites[sInfoCard->spriteIds[i]].data[3] = sInfoCard->spriteIds[i];
                    }
                }
                else
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                    {
                        gSprites[sInfoCard->spriteIds[i]].callback = SpriteCB_MonIconCardScrollLeft;
                        gSprites[sInfoCard->spriteIds[i]].data[0] = gTasks[taskId].tUsingAlternateSlot;
                        gSprites[sInfoCard->spriteIds[i]].data[1] = 0;
                        gSprites[sInfoCard->spriteIds[i]].data[2] = i;
                    }
                }
            }

            gTasks[taskId].tState = STATE_MOVE_LEFT;
            gTasks[taskId].data[5] = 0;
            break;
        case TRAINERCARD_INPUT_RIGHT:
            if (gTasks[taskId].tUsingAlternateSlot)
            {
                gBattle_BG0_X = 0;
                gBattle_BG0_Y = 0;
                gBattle_BG1_X = -(DISPLAY_WIDTH + 16);
                gBattle_BG1_Y = 0;
            }
            else
            {
                gBattle_BG0_X = -(DISPLAY_WIDTH + 16);
                gBattle_BG0_Y = 0;
                gBattle_BG1_X = 0;
                gBattle_BG1_Y = 0;
            }

            if (sInfoCard->pos == 1)
            {
                gBattle_BG2_X = 0;
                gBattle_BG2_Y = DISPLAY_HEIGHT;
            }
            else
            {
                gBattle_BG2_X = 0;
                gBattle_BG2_Y = 0;
            }
            matchNo = sIdToMatchNumber[gTasks[taskId2].data[1]][sInfoCard->pos - 1];
            DisplayMatchInfoOnCard(gTasks[taskId].tUsingAlternateSlot | MOVE_CARD_RIGHT, matchNo);

            for (i = 0; i < NUM_INFOCARD_SPRITES / 2; i++)
            {
                if (i < 2)
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                    {
                        gSprites[sInfoCard->spriteIds[i]].callback = SpriteCB_TrainerIconCardScrollRight;
                        gSprites[sInfoCard->spriteIds[i]].data[0] = gTasks[taskId].tUsingAlternateSlot ^ 1;
                        gSprites[sInfoCard->spriteIds[i]].data[1] = 0;
                        gSprites[sInfoCard->spriteIds[i]].data[2] = i;
                        gSprites[sInfoCard->spriteIds[i]].data[3] = sInfoCard->spriteIds[i];
                    }
                }
                else
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                    {
                        gSprites[sInfoCard->spriteIds[i]].callback = SpriteCB_MonIconCardScrollRight;
                        gSprites[sInfoCard->spriteIds[i]].data[0] = gTasks[taskId].tUsingAlternateSlot ^ 1;
                        gSprites[sInfoCard->spriteIds[i]].data[1] = 0;
                        gSprites[sInfoCard->spriteIds[i]].data[2] = i;
                    }
                }
            }
            for (i = NUM_INFOCARD_SPRITES / 2; i < NUM_INFOCARD_SPRITES; i++)
            {
                if (i < 10)
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                    {
                        gSprites[sInfoCard->spriteIds[i]].callback = SpriteCB_TrainerIconCardScrollRight;
                        gSprites[sInfoCard->spriteIds[i]].data[0] = gTasks[taskId].tUsingAlternateSlot;
                        gSprites[sInfoCard->spriteIds[i]].data[1] = 0;
                        gSprites[sInfoCard->spriteIds[i]].data[2] = i;
                        gSprites[sInfoCard->spriteIds[i]].data[3] = sInfoCard->spriteIds[i];
                    }
                }
                else
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                    {
                        gSprites[sInfoCard->spriteIds[i]].callback = SpriteCB_MonIconCardScrollRight;
                        gSprites[sInfoCard->spriteIds[i]].data[0] = gTasks[taskId].tUsingAlternateSlot;
                        gSprites[sInfoCard->spriteIds[i]].data[1] = 0;
                        gSprites[sInfoCard->spriteIds[i]].data[2] = i;
                    }
                }
            }

            gTasks[taskId].tState = STATE_MOVE_RIGHT;
            gTasks[taskId].data[5] = 0;
            break;
        case MATCHCARD_INPUT_RIGHT:
            if (gTasks[taskId].tUsingAlternateSlot)
            {
                gBattle_BG0_X = 0;
                gBattle_BG0_Y = 0;
                gBattle_BG1_X = -(DISPLAY_WIDTH + 16);
                gBattle_BG1_Y = 0;
            }
            else
            {
                gBattle_BG0_X = -(DISPLAY_WIDTH + 16);
                gBattle_BG0_Y = 0;
                gBattle_BG1_X = 0;
                gBattle_BG1_Y = 0;
            }

            if (sInfoCard->pos == 2)
            {
                gBattle_BG2_X = DISPLAY_WIDTH + 16;
                gBattle_BG2_Y = DISPLAY_HEIGHT;
                trainerTourneyId = sInfoCard->tournamentIds[1];
                DisplayTrainerInfoOnCard(gTasks[taskId].tUsingAlternateSlot | MOVE_CARD_RIGHT, trainerTourneyId);
            }
            else
            {
                gBattle_BG2_X = 0;
                gBattle_BG2_Y = DISPLAY_HEIGHT;
                matchNo = gTasks[taskId2].data[1] - 16;
                DisplayMatchInfoOnCard(gTasks[taskId].tUsingAlternateSlot | MOVE_CARD_RIGHT, matchNo);
            }

            for (i = 0; i < NUM_INFOCARD_SPRITES / 2; i++)
            {
                if (i < 2)
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                    {
                        gSprites[sInfoCard->spriteIds[i]].callback = SpriteCB_TrainerIconCardScrollRight;
                        gSprites[sInfoCard->spriteIds[i]].data[0] = gTasks[taskId].tUsingAlternateSlot ^ 1;
                        gSprites[sInfoCard->spriteIds[i]].data[1] = 0;
                        gSprites[sInfoCard->spriteIds[i]].data[2] = i;
                        gSprites[sInfoCard->spriteIds[i]].data[3] = sInfoCard->spriteIds[i];
                    }
                }
                else
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                    {
                        gSprites[sInfoCard->spriteIds[i]].callback = SpriteCB_MonIconCardScrollRight;
                        gSprites[sInfoCard->spriteIds[i]].data[0] = gTasks[taskId].tUsingAlternateSlot ^ 1;
                        gSprites[sInfoCard->spriteIds[i]].data[1] = 0;
                        gSprites[sInfoCard->spriteIds[i]].data[2] = i;
                    }
                }
            }
            for (i = NUM_INFOCARD_SPRITES / 2; i < NUM_INFOCARD_SPRITES; i++)
            {
                if (i < 10)
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                    {
                        gSprites[sInfoCard->spriteIds[i]].callback = SpriteCB_TrainerIconCardScrollRight;
                        gSprites[sInfoCard->spriteIds[i]].data[0] = gTasks[taskId].tUsingAlternateSlot;
                        gSprites[sInfoCard->spriteIds[i]].data[1] = 0;
                        gSprites[sInfoCard->spriteIds[i]].data[2] = i;
                        gSprites[sInfoCard->spriteIds[i]].data[3] = sInfoCard->spriteIds[i];
                    }
                }
                else
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                    {
                        gSprites[sInfoCard->spriteIds[i]].callback = SpriteCB_MonIconCardScrollRight;
                        gSprites[sInfoCard->spriteIds[i]].data[0] = gTasks[taskId].tUsingAlternateSlot;
                        gSprites[sInfoCard->spriteIds[i]].data[1] = 0;
                        gSprites[sInfoCard->spriteIds[i]].data[2] = i;
                    }
                }
            }

            gTasks[taskId].tState = STATE_MOVE_RIGHT;
            gTasks[taskId].data[5] = 0;
            break;
        }
        break;
    case STATE_MOVE_UP:
        if (++gTasks[taskId].data[5] != 41)
        {
            gBattle_BG0_Y -= 4;
            gBattle_BG1_Y -= 4;
            gBattle_BG2_Y -= 4;
        }
        else
        {
            gTasks[taskId].tState = STATE_GET_INPUT;
        }
        break;
    case STATE_MOVE_DOWN:
        if (++gTasks[taskId].data[5] != 41)
        {
            gBattle_BG0_Y += 4;
            gBattle_BG1_Y += 4;
            gBattle_BG2_Y += 4;
        }
        else
        {
            gTasks[taskId].tState = STATE_GET_INPUT;
        }
        break;
    case STATE_MOVE_LEFT:
        if (++gTasks[taskId].data[5] != 65)
        {
            gBattle_BG0_X -= 4;
            gBattle_BG1_X -= 4;
            gBattle_BG2_X -= 4;
        }
        else
        {
            gTasks[taskId].tState = STATE_GET_INPUT;
        }
        break;
    case STATE_MOVE_RIGHT:
        if (++gTasks[taskId].data[5] != 65)
        {
            gBattle_BG0_X += 4;
            gBattle_BG1_X += 4;
            gBattle_BG2_X += 4;
        }
        else
        {
            gTasks[taskId].tState = STATE_GET_INPUT;
        }
        break;
    case STATE_CLOSE_CARD:
        if (!gPaletteFade.active)
        {
            for (i = 0; i < NUM_INFOCARD_SPRITES / 2; i++)
            {
                if (i < 2)
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                        FreeAndDestroyTrainerPicSprite(sInfoCard->spriteIds[i]);
                }
                else
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                        FreeAndDestroyMonIconSprite(&gSprites[sInfoCard->spriteIds[i]]);
                }
            }
            for (i = NUM_INFOCARD_SPRITES / 2; i < NUM_INFOCARD_SPRITES; i++)
            {
                if (i < 10)
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                        FreeAndDestroyTrainerPicSprite(sInfoCard->spriteIds[i]);
                }
                else
                {
                    if (sInfoCard->spriteIds[i] != SPRITE_NONE)
                        FreeAndDestroyMonIconSprite(&gSprites[sInfoCard->spriteIds[i]]);
                }
            }

            FreeMonIconPalettes();
            FREE_AND_SET_NULL(sInfoCard);
            FreeAllWindowBuffers();

            if (mode == INFOCARD_NEXT_OPPONENT)
            {
                SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
            }
            else
            {
                i = CreateTask(Task_ShowTourneyTree, 0);
                gTasks[i].data[0] = 0;
                gTasks[i].tNotInteractive = FALSE;
                gTasks[i].data[2] = 3;
                gTasks[i].data[3] = gTasks[taskId].data[4];
                gTasks[i].tIsPrevTourneyTree = gTasks[taskId2].data[6];
            }
            DestroyTask(taskId);
        }
        break;
    }
}

// undefine task states for Task_HandleInfoCardInput
#undef STATE_FADE_IN
#undef STATE_WAIT_FADE
#undef STATE_GET_INPUT
#undef STATE_REACT_INPUT
#undef STATE_MOVE_UP
#undef STATE_MOVE_DOWN
#undef STATE_MOVE_LEFT
#undef STATE_MOVE_RIGHT
#undef STATE_CLOSE_CARD

static u8 Task_GetInfoCardInput(u8 taskId)
{
}

#undef tUsingAlternateSlot

// allocatedArray below needs to be large enough to hold stat totals for each mon, or totals of each type of move points
#define ALLOC_ARRAY_SIZE max(NUM_STATS * FRONTIER_PARTY_SIZE, NUM_MOVE_POINT_TYPES)

static u8 SetDomeMonIconPalette(struct Sprite * sprite, u16 species, u8 index) {
  u16 isShiny = species >> 15;
  species &= 0x7FFF;
  if (index < 16) {
    LoadCompressedPalette(GetMonSpritePalFromSpeciesAndPersonality(species, 0, isShiny ? 0 : SHINY_ODDS), index*16 + 0x100, 32);
    sprite->oam.paletteNum = index;
    return index;
  }
  return 0xFF;
}

// Displays a single trainer's info on a card
static void DisplayTrainerInfoOnCard(u8 flags, u8 trainerTourneyId)
{
}

static int BufferDomeWinString(u8 matchNum, u8 *tournamentIds)
{
}

static void DisplayMatchInfoOnCard(u8 flags, u8 matchNo)
{
}

static void ShowDomeTourneyTree(void)
{
    u8 taskId = CreateTask(Task_ShowTourneyTree, 0);
    gTasks[taskId].tState = 0;
    gTasks[taskId].tNotInteractive = FALSE;
    gTasks[taskId].data[2] = 2;
    gTasks[taskId].tIsPrevTourneyTree = FALSE;
    SetMainCallback2(CB2_TourneyTree);
}

// To show the results of the last tourney on the computer in the lobby
static void ShowPreviousDomeTourneyTree(void)
{
}

// Task states for Task_HandleTourneyTreeInput
#define STATE_FADE_IN               0
#define STATE_WAIT_FADE             1
#define STATE_GET_INPUT             2
#define STATE_SHOW_INFOCARD_TRAINER 3
#define STATE_SHOW_INFOCARD_MATCH   5
#define STATE_CLOSE_TOURNEY_TREE    7

static void Task_HandleTourneyTreeInput(u8 taskId)
{
    u8 newTaskId = 0;
    int spriteId = gTasks[taskId].data[1];

    switch (gTasks[taskId].tState)
    {
    case STATE_FADE_IN:
        if (!gPaletteFade.active)
        {
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0x10, 0, RGB_BLACK);
            gTasks[taskId].tState = STATE_WAIT_FADE;
            StartSpriteAnim(&gSprites[spriteId], 1);
        }
        break;
    case STATE_WAIT_FADE:
        if (!gPaletteFade.active)
            gTasks[taskId].tState = STATE_GET_INPUT;
        break;
    case STATE_GET_INPUT:
        switch (UpdateTourneyTreeCursor(taskId))
        {
        case TOURNEY_TREE_SELECTED_CLOSE:
        default:
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
            gTasks[taskId].tState = STATE_CLOSE_TOURNEY_TREE;
            break;
        case TOURNEY_TREE_NO_SELECTION:
            break;
        case TOURNEY_TREE_SELECTED_TRAINER:
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
            gTasks[taskId].tState = STATE_SHOW_INFOCARD_TRAINER;
            break;
        case TOURNEY_TREE_SELECTED_MATCH:
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
            gTasks[taskId].tState = STATE_SHOW_INFOCARD_MATCH;
            break;
        }
        break;
    case STATE_SHOW_INFOCARD_TRAINER:
        if (!gPaletteFade.active)
        {
            FreeAllWindowBuffers();
            ScanlineEffect_Stop();
            FREE_AND_SET_NULL(sTilemapBuffer);
            newTaskId = CreateTask(Task_ShowTourneyInfoCard, 0);
            gTasks[newTaskId].tState = 0;
            gTasks[newTaskId].tTournamentId = sTourneyTreeTrainerIds[spriteId];
            gTasks[newTaskId].tMode = INFOCARD_TRAINER;
            gTasks[newTaskId].tPrevTaskId = taskId;

            gTasks[taskId].tState = STATE_SHOW_INFOCARD_TRAINER + 1;
            sInfoCard->pos = 0;
        }
        break;
    case STATE_SHOW_INFOCARD_TRAINER + 1:
        break;
    case STATE_SHOW_INFOCARD_MATCH:
        if (!gPaletteFade.active)
        {
            FreeAllWindowBuffers();
            ScanlineEffect_Stop();
            FREE_AND_SET_NULL(sTilemapBuffer);
            newTaskId = CreateTask(Task_ShowTourneyInfoCard, 0);
            gTasks[newTaskId].tState = 0;
            gTasks[newTaskId].tTournamentId = spriteId - DOME_TOURNAMENT_TRAINERS_COUNT;
            gTasks[newTaskId].tMode = INFOCARD_MATCH;
            gTasks[newTaskId].tPrevTaskId = taskId;

            gTasks[taskId].tState = STATE_SHOW_INFOCARD_MATCH + 1;
        }
        break;
    case STATE_SHOW_INFOCARD_MATCH + 1:
        break;
    case STATE_CLOSE_TOURNEY_TREE:
        if (!gPaletteFade.active)
        {
            FreeAllWindowBuffers();
            ScanlineEffect_Stop();
            FREE_AND_SET_NULL(sTilemapBuffer);
            SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
            DestroyTask(gTasks[taskId].data[7]);
            DestroyTask(taskId);
        }
        break;
    }
}

// undefine task states for Task_HandleTourneyTreeInput
#undef STATE_FADE_IN
#undef STATE_WAIT_FADE
#undef STATE_GET_INPUT
#undef STATE_SHOW_INFOCARD_TRAINER
#undef STATE_SHOW_INFOCARD_MATCH
#undef STATE_CLOSE_TOURNEY_TREE


#define MOVE_DIR_UP    0
#define MOVE_DIR_DOWN  1
#define MOVE_DIR_LEFT  2
#define MOVE_DIR_RIGHT 3
#define MOVE_DIR_NONE  4

// Move the tourney tree cursor
// The 'cursor' is actually just which button sprite is currently doing the 'selected' animation
static u8 UpdateTourneyTreeCursor(u8 taskId)
{
}

#undef MOVE_DIR_UP
#undef MOVE_DIR_DOWN
#undef MOVE_DIR_LEFT
#undef MOVE_DIR_RIGHT
#undef MOVE_DIR_NONE

// Shows the results of the just-completed round for the current tourney
static void ShowNonInteractiveDomeTourneyTree(void)
{
    u8 taskId = CreateTask(Task_ShowTourneyTree, 0);
    gTasks[taskId].tState = 0;
    gTasks[taskId].tNotInteractive = TRUE;
    gTasks[taskId].data[2] = 2;
    gTasks[taskId].tIsPrevTourneyTree = FALSE;
    SetMainCallback2(CB2_TourneyTree);
}

static void ResolveDomeRoundWinners(void)
{
}

// Decides the winning move of an NPC vs NPC match
static u16 GetWinningMove(int winnerTournamentId, int loserTournamentId, u8 roundId)
{
}

static void Task_ShowTourneyTree(u8 taskId)
{
}

static void DrawTourneyAdvancementLine(u8 tournamentId, u8 roundId)
{
    int i;
    const struct TourneyTreeLineSection *lineSection = sTourneyTreeLineSections[tournamentId][roundId];

    for (i = 0; i < sTourneyTreeLineSectionArrayCounts[tournamentId][roundId]; i++)
        CopyToBgTilemapBufferRect_ChangePalette(1, &lineSection[i].src, lineSection[i].x, lineSection[i].y, 1, 1, 17);

    CopyBgTilemapBufferToVram(1);
}

#define STATE_FADE_IN             0
#define STATE_SHOW_RESULTS        1
#define STATE_DELAY               2
#define STATE_WAIT_FOR_INPUT      3
#define STATE_CLOSE_TOURNEY_TREE  4

// The non-interactive tourney tree that's shown when a round is completed
static void Task_HandleStaticTourneyTreeInput(u8 taskId)
{
}

#undef STATE_FADE_IN
#undef STATE_SHOW_RESULTS
#undef STATE_DELAY
#undef STATE_WAIT_FOR_INPUT
#undef STATE_CLOSE_TOURNEY_TREE

static void CB2_TourneyTree(void)
{
    AnimateSprites();
    BuildOamBuffer();
    RunTextPrinters();
    UpdatePaletteFade();
    RunTasks();
}

static void VblankCb_TourneyInfoCard(void)
{
    ChangeBgX(3, 0x80, BG_COORD_ADD);
    ChangeBgY(3, 0x80, BG_COORD_SUB);
    SetGpuReg(REG_OFFSET_BG0HOFS, gBattle_BG0_X);
    SetGpuReg(REG_OFFSET_BG0VOFS, gBattle_BG0_Y);
    SetGpuReg(REG_OFFSET_BG1HOFS, gBattle_BG1_X);
    SetGpuReg(REG_OFFSET_BG1VOFS, gBattle_BG1_Y);
    SetGpuReg(REG_OFFSET_BG2HOFS, gBattle_BG2_X);
    SetGpuReg(REG_OFFSET_BG2VOFS, gBattle_BG2_Y);
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

#define SET_WIN0H_WIN1H(win0H, win1H)                       \
{                                                           \
    *(vu32*)(REG_ADDR_WIN0H) = ((win0H << 16) | (win1H));   \
}

static void HblankCb_TourneyTree(void)
{
    u16 vCount = REG_VCOUNT;

    if (vCount < 42)
    {
        REG_WININ = WININ_WIN0_BG_ALL | WININ_WIN0_CLR | WININ_WIN0_OBJ
                | WININ_WIN1_BG_ALL | WININ_WIN1_CLR | WININ_WIN1_OBJ;
        SET_WIN0H_WIN1H(0, 0);
    }
    else if (vCount < 50)
    {
        REG_WININ = WININ_WIN0_BG0 | WININ_WIN0_BG1 | WININ_WIN0_BG3 | WININ_WIN0_OBJ | WININ_WIN0_CLR
                    | WININ_WIN1_BG0 | WININ_WIN1_BG1 | WININ_WIN1_BG3 | WININ_WIN1_OBJ | WININ_WIN1_CLR;
        SET_WIN0H_WIN1H(WIN_RANGE(152, 155), WIN_RANGE(85, 88));
    }
    else if (vCount < 58)
    {
        REG_WININ = WININ_WIN0_BG_ALL | WININ_WIN0_CLR | WININ_WIN0_OBJ
                | WININ_WIN1_BG_ALL | WININ_WIN1_CLR | WININ_WIN1_OBJ;
        SET_WIN0H_WIN1H(0, 0);
    }
    else if (vCount < 75)
    {
        REG_WININ = WININ_WIN0_BG0 | WININ_WIN0_BG1 | WININ_WIN0_BG3 | WININ_WIN0_OBJ | WININ_WIN0_CLR
                    | WININ_WIN1_BG0 | WININ_WIN1_BG1 | WININ_WIN1_BG3 | WININ_WIN1_OBJ | WININ_WIN1_CLR;
        SET_WIN0H_WIN1H(WIN_RANGE(144, 152), WIN_RANGE(88, 96));
    }
    else if (vCount < 82)
    {
        REG_WININ = WININ_WIN0_BG0 | WININ_WIN0_BG1 | WININ_WIN0_BG3 | WININ_WIN0_OBJ | WININ_WIN0_CLR
                    | WININ_WIN1_BG0 | WININ_WIN1_BG1 | WININ_WIN1_BG3 | WININ_WIN1_OBJ | WININ_WIN1_CLR;
        SET_WIN0H_WIN1H(WIN_RANGE(152, 155), WIN_RANGE(85, 88));
    }
    else if (vCount < 95)
    {
        REG_WININ = WININ_WIN0_BG_ALL | WININ_WIN0_CLR | WININ_WIN0_OBJ
                | WININ_WIN1_BG_ALL | WININ_WIN1_CLR | WININ_WIN1_OBJ;
        SET_WIN0H_WIN1H(0, 0);
    }
    else if (vCount < 103)
    {
        REG_WININ = WININ_WIN0_BG0 | WININ_WIN0_BG1 | WININ_WIN0_BG2 | WININ_WIN0_OBJ | WININ_WIN0_CLR
                    | WININ_WIN1_BG0 | WININ_WIN1_BG1 | WININ_WIN1_BG2 | WININ_WIN1_OBJ | WININ_WIN1_CLR;
        SET_WIN0H_WIN1H(WIN_RANGE(152, 155), WIN_RANGE(85, 88));
    }
    else if (vCount < 119)
    {
        REG_WININ = WININ_WIN0_BG0 | WININ_WIN0_BG1 | WININ_WIN0_BG2 | WININ_WIN0_OBJ | WININ_WIN0_CLR
                    | WININ_WIN1_BG0 | WININ_WIN1_BG1 | WININ_WIN1_BG2 | WININ_WIN1_OBJ | WININ_WIN1_CLR;
        SET_WIN0H_WIN1H(WIN_RANGE(144, 152), WIN_RANGE(88, 96));
    }
    else if (vCount < 127)
    {
        REG_WININ = WININ_WIN0_BG_ALL | WININ_WIN0_CLR | WININ_WIN0_OBJ
                | WININ_WIN1_BG_ALL | WININ_WIN1_CLR | WININ_WIN1_OBJ;
        SET_WIN0H_WIN1H(0, 0);
    }
    else if (vCount < 135)
    {
        REG_WININ = WININ_WIN0_BG0 | WININ_WIN0_BG1 | WININ_WIN0_BG2 | WININ_WIN0_OBJ | WININ_WIN0_CLR
                    | WININ_WIN1_BG0 | WININ_WIN1_BG1 | WININ_WIN1_BG2 | WININ_WIN1_OBJ | WININ_WIN1_CLR;
        SET_WIN0H_WIN1H(WIN_RANGE(152, 155), WIN_RANGE(85, 88));
    }
    else
    {
        REG_WININ = WININ_WIN0_BG_ALL | WININ_WIN0_CLR | WININ_WIN0_OBJ
                | WININ_WIN1_BG_ALL | WININ_WIN1_CLR | WININ_WIN1_OBJ;
        SET_WIN0H_WIN1H(0, 0);
    }
}

static void VblankCb_TourneyTree(void)
{
    SetGpuReg(REG_OFFSET_BG0HOFS, gBattle_BG0_X);
    SetGpuReg(REG_OFFSET_BG0VOFS, gBattle_BG0_Y);
    SetGpuReg(REG_OFFSET_BG1HOFS, gBattle_BG1_X);
    SetGpuReg(REG_OFFSET_BG1VOFS, gBattle_BG1_Y);
    ChangeBgY(2, 0x80, BG_COORD_SUB);
    ChangeBgY(3, 0x80, BG_COORD_ADD);
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
    ScanlineEffect_InitHBlankDmaTransfer();
}

static void SetFacilityTrainerAndMonPtrs(void)
{
    gFacilityTrainerMons = gBattleFrontierMons;
    gFacilityTrainers = gBattleFrontierTrainers;
}

static void ResetSketchedMoves(void)
{
}

static void RestoreDomePlayerPartyHeldItems(void)
{
}

static void ReduceDomePlayerPartyToSelectedMons(void)
{
    ReducePlayerPartyToSelectedMons();
}

static void GetPlayerSeededBeforeOpponent(void)
{
    // A higher tournament ID is a worse seed
    if (TrainerIdToTournamentId(gTrainerBattleOpponent_A) > TrainerIdToTournamentId(TRAINER_PLAYER))
        gSpecialVar_Result = 1;
    else
        gSpecialVar_Result = 2;
}

static void BufferLastDomeWinnerName(void)
{
}

// For showing the previous tourney results before the player has entered a challenge
static void InitRandomTourneyTreeResults(void)
{
}

static int TrainerIdToTournamentId(u16 trainerId)
{
}

// The same as the above one, but has global scope.
int TrainerIdToDomeTournamentId(u16 trainerId)
{
}

static u8 GetOpposingNPCTournamentIdByRound(u8 tournamentId, u8 round)
{
    u8 tournamentIds[2];
    BufferDomeWinString(sTrainerAndRoundToLastMatchCardNum[sTournamentIdToPairedTrainerIds[tournamentId] / 2][round] - 16, tournamentIds);
    if (tournamentId == tournamentIds[0])
        return tournamentIds[1];
    else
        return tournamentIds[0];
}

// Determines which trainers won in the NPC vs NPC battles
static void DecideRoundWinners(u8 roundId)
{
}

static void CopyDomeTrainerName(u8 *str, u16 trainerId)
{
    int i = 0;
    SetFacilityPtrsGetLevel();

    if (trainerId == TRAINER_FRONTIER_BRAIN)
    {
        CopyDomeBrainTrainerName(str);
    }
    else
    {
        if (trainerId == TRAINER_PLAYER)
        {
            for (i = 0; i < PLAYER_NAME_LENGTH; i++)
                str[i] = gSaveBlock2Ptr->playerName[i];
        }
        else if (trainerId < FRONTIER_TRAINERS_COUNT)
        {
            for (i = 0; i < PLAYER_NAME_LENGTH; i++)
                str[i] = gFacilityTrainers[trainerId].trainerName[i];
        }
        str[i] = EOS;
    }
}

static u8 GetDomeBrainTrainerPicId(void)
{
    return gTrainers[TRAINER_NONE].trainerPic;
}

static u8 GetDomeBrainTrainerClass(void)
{
    return gTrainers[TRAINER_NONE].trainerClass;
}

static void CopyDomeBrainTrainerName(u8 *str)
{
    int i;

    for (i = 0; i < PLAYER_NAME_LENGTH; i++)
        str[i] = gTrainers[TRAINER_NONE].trainerName[i];
    str[i] = EOS;
}
