#ifndef GUARD_DATA_H
#define GUARD_DATA_H

#include "constants/moves.h"

#define SPECIES_SHINY_TAG 500
#define N_FOLLOWER_HAPPY_MESSAGES 31
#define N_FOLLOWER_NEUTRAL_MESSAGES 14
#define N_FOLLOWER_SAD_MESSAGES 3
#define N_FOLLOWER_UPSET_MESSAGES 3
#define N_FOLLOWER_ANGRY_MESSAGES 5
#define N_FOLLOWER_PENSIVE_MESSAGES 20
#define N_FOLLOWER_LOVE_MESSAGES 10
#define N_FOLLOWER_SURPRISE_MESSAGES 20
#define N_FOLLOWER_CURIOUS_MESSAGES 7
#define N_FOLLOWER_MUSIC_MESSAGES 14
#define N_FOLLOWER_POISONED_MESSAGES 1

#define MAX_TRAINER_ITEMS 4

enum {
    BATTLER_AFFINE_NORMAL,
    BATTLER_AFFINE_EMERGE,
    BATTLER_AFFINE_RETURN,
};

struct MonCoords
{
    // This would use a bitfield, but some function
    // uses it as a u8 and casting won't match.
    u8 size; // u8 width:4, height:4;
    u8 y_offset;
};

#define MON_COORDS_SIZE(width, height)(DIV_ROUND_UP(width, 8) << 4 | DIV_ROUND_UP(height, 8))
#define GET_MON_COORDS_WIDTH(size)((size >> 4) * 8)
#define GET_MON_COORDS_HEIGHT(size)((size & 0xF) * 8)

struct TrainerMon
{
    u8 nickname[POKEMON_NAME_LENGTH + 1];
    u8 ivs[NUM_STATS];
    u8 evs[NUM_STATS];
    u8 lvl;
    u16 species;
    u16 heldItem;
    u16 moves[MAX_MON_MOVES];
    u8 ball;
    u16 ability:2;
    u16 friendship:2;
    u16 gender:2;
    u16 build:3;
    u16 shiny:1;
    u16 nature:5;
    u16 cantEvolve:1;
};

union TrainerMonPtr
{
    const struct TrainerMon *TrainerMon;
};


struct Trainer
{
    u8 partyFlags; // Unread
    u8 trainerClass;
    u8 encounterMusic_gender; // last bit is gender
    u8 trainerPic;
    u8 trainerName[12];
    u16 items[4];
    bool8 doubleBattle;
    u32 aiFlags;
    u8 partySize;
    u8 hasCustomTransition:1;
    /*0x28*/ union TrainerMonPtr party;
};

#define TRAINER_ENCOUNTER_MUSIC(trainer)((gTrainers[trainer].encounterMusic_gender & 0x7F))

struct FollowerMsgInfo {
    const u8 *text;
    const u8 *script;
};

struct FollowerMessagePool
{
    const struct FollowerMsgInfo * messages;
    const u8 * script;
    u16 length;
};


extern const u16 gMinigameDigits_Pal[];
extern const u32 gMinigameDigits_Gfx[];

extern const struct SpriteFrameImage gBattlerPicTable_PlayerLeft[];
extern const struct SpriteFrameImage gBattlerPicTable_OpponentLeft[];
extern const struct SpriteFrameImage gBattlerPicTable_PlayerRight[];
extern const struct SpriteFrameImage gBattlerPicTable_OpponentRight[];
extern const struct SpriteFrameImage gTrainerBackPicTable_Brendan[];
extern const struct SpriteFrameImage gTrainerBackPicTable_May[];
extern const struct SpriteFrameImage gTrainerBackPicTable_Red[];
extern const struct SpriteFrameImage gTrainerBackPicTable_Leaf[];
extern const struct SpriteFrameImage gTrainerBackPicTable_RubySapphireBrendan[];
extern const struct SpriteFrameImage gTrainerBackPicTable_RubySapphireMay[];
extern const struct SpriteFrameImage gTrainerBackPicTable_Wally[];
extern const struct SpriteFrameImage gTrainerBackPicTable_Steven[];
extern const struct SpriteFrameImage gTrainerBackPicTable_AdventuresBrendan[];
extern const struct SpriteFrameImage gTrainerBackPicTable_AdventuresMay[];

extern const union AffineAnimCmd *const gAffineAnims_BattleSpritePlayerSide[];
extern const union AffineAnimCmd *const gAffineAnims_BattleSpriteOpponentSide[];
extern const union AffineAnimCmd *const gAffineAnims_BattleSpriteContest[];

extern const union AnimCmd *const gAnims_MonPic[];
extern const struct MonCoords gMonFrontPicCoords[];
extern const struct MonCoords gMonBackPicCoords[];
extern const struct CompressedSpriteSheet gMonBackPicTable[];
extern const struct CompressedSpriteSheet gMonBackPicTableFemale[];
extern const struct CompressedSpritePalette gMonPaletteTable[];
extern const struct CompressedSpritePalette gMonPaletteTableFemale[];
extern const struct CompressedSpritePalette gMonShinyPaletteTable[];
extern const struct CompressedSpritePalette gMonShinyPaletteTableFemale[];
extern const union AnimCmd *const *const gTrainerFrontAnimsPtrTable[];
extern const struct MonCoords gTrainerFrontPicCoords[];
extern const struct CompressedSpriteSheet gTrainerFrontPicTable[];
extern const struct SpritePalette gTrainerFrontPicPaletteTable[];
extern const union AnimCmd *const *const gTrainerBackAnimsPtrTable[];
extern const struct MonCoords gTrainerBackPicCoords[];
extern const struct CompressedSpriteSheet gTrainerBackPicTable[]; // functionally unused
extern const struct SpritePalette gTrainerBackPicPaletteTable[];
extern const u8 gCostumeBackPics[COSTUME_COUNT][GENDER_COUNT];
extern const u8 gCostumeFrontPics[COSTUME_COUNT][GENDER_COUNT];

extern const u8 gEnemyMonElevation[NUM_SPECIES];

extern const union AnimCmd *const *const gMonFrontAnimsPtrTable[];
extern const struct CompressedSpriteSheet gMonFrontPicTable[];
extern const struct CompressedSpriteSheet gMonFrontPicTableFemale[];

extern const struct Trainer gTrainers[];
extern const u8 gTrainerClassNames[][13];
extern const u8 gSpeciesNames[][POKEMON_NAME_LENGTH + 1];
extern const u8 gMoveNames[MOVES_COUNT][MOVE_NAME_LENGTH + 1];
extern const u8 *const gZMoveNames[];

// Follower text messages
extern const struct FollowerMsgInfo gFollowerHappyMessages[];
extern const struct FollowerMsgInfo gFollowerNeutralMessages[];
extern const struct FollowerMsgInfo gFollowerSadMessages[];
extern const struct FollowerMsgInfo gFollowerUpsetMessages[];
extern const struct FollowerMsgInfo gFollowerAngryMessages[];
extern const struct FollowerMsgInfo gFollowerPensiveMessages[];
extern const struct FollowerMsgInfo gFollowerLoveMessages[];
extern const struct FollowerMsgInfo gFollowerSurpriseMessages[];
extern const struct FollowerMsgInfo gFollowerCuriousMessages[];
extern const struct FollowerMsgInfo gFollowerMusicMessages[];
extern const struct FollowerMsgInfo gFollowerPoisonedMessages[];

extern const struct CompressedSpritePalette gEgg1PaletteTable[];
extern const struct CompressedSpritePalette gEgg2PaletteTable[];

#endif // GUARD_DATA_H
