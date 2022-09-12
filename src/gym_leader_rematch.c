#include "global.h"
#include "random.h"
#include "event_data.h"
#include "battle_setup.h"
#include "gym_leader_rematch.h"

static void UpdateGymLeaderRematchFromArray(const u16 *data, size_t size, u32 maxRematch);
static s32 GetRematchIndex(u32 trainerIdx);

static const u16 GymLeaderRematches_AfterNewMauville[] = {
};

static const u16 GymLeaderRematches_BeforeNewMauville[] = {
};

void UpdateGymLeaderRematch(void)
{
    if (FlagGet(FLAG_SYS_GAME_CLEAR) && (Random() % 100) <= 30)
    {
        if (FlagGet(FLAG_WATTSON_REMATCH_AVAILABLE))
            UpdateGymLeaderRematchFromArray(GymLeaderRematches_AfterNewMauville, ARRAY_COUNT(GymLeaderRematches_AfterNewMauville), 5);
        else
            UpdateGymLeaderRematchFromArray(GymLeaderRematches_BeforeNewMauville, ARRAY_COUNT(GymLeaderRematches_BeforeNewMauville), 1);
    }
}

static void UpdateGymLeaderRematchFromArray(const u16 *data, size_t size, u32 maxRematch)
{
}

static s32 GetRematchIndex(u32 trainerIdx)
{
    s32 i;
    for (i = 0; i < 5; i++)
    {
        if (!HasTrainerBeenFought(gRematchTable[trainerIdx].trainerIds[i]))
        {
            return i;
        }
    }
    return 5;
}
