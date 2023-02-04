#include "global.h"
#include "random.h"
#include "event_data.h"
#include "wonder_news.h"
#include "constants/items.h"

// Every 4th reward for sending Wonder News to a link partner is a "big" reward.
#define MAX_SENT_REWARD 4

// Only up to 5 rewards can be received in a short period. After this the player
// must take 500 steps before any more rewards can be received.
#define MAX_REWARD 5

static u32 GetRewardItem(struct WonderNewsMetadata *);
static u32 GetRewardType(struct WonderNewsMetadata *);
static void IncrementRewardCounter(struct WonderNewsMetadata *);
static void IncrementSentRewardCounter(struct WonderNewsMetadata *);
static void ResetSentRewardCounter(struct WonderNewsMetadata *);

static u32 GetNewsId(struct WonderNewsMetadata *data)
{
    u32 itemId;
    data->newsType = WONDER_NEWS_NONE;
    itemId = data->berry + FIRST_BERRY_INDEX - 1;
    data->berry = 0;
    IncrementRewardCounter(data);
    return itemId;
}

static void ResetSentRewardCounter(struct WonderNewsMetadata *data)
{
    data->sentRewardCounter = 0;
}

// Track number of times a reward was received (or attmepted to receive) for sending Wonder News to a link partner.
static void IncrementSentRewardCounter(struct WonderNewsMetadata *data)
{
    data->sentRewardCounter++;
    if (data->sentRewardCounter > MAX_SENT_REWARD)
        data->sentRewardCounter = MAX_SENT_REWARD;
}

static void IncrementRewardCounter(struct WonderNewsMetadata *data)
{
    data->rewardCounter++;
    if (data->rewardCounter > MAX_REWARD)
        data->rewardCounter = MAX_REWARD;
}

static u32 GetRewardType(struct WonderNewsMetadata *data)
{
    if (data->rewardCounter == MAX_REWARD)
        return NEWS_REWARD_AT_MAX;

    switch (data->newsType)
    {
    case WONDER_NEWS_NONE:
        return NEWS_REWARD_WAITING;
    case WONDER_NEWS_RECV_FRIEND:
        return NEWS_REWARD_RECV_SMALL;
    case WONDER_NEWS_RECV_WIRELESS:
        return NEWS_REWARD_RECV_BIG;
    case WONDER_NEWS_SENT:
        if (data->sentRewardCounter < MAX_SENT_REWARD - 1)
            return NEWS_REWARD_SENT_SMALL;
        return NEWS_REWARD_SENT_BIG;
    default:
        AGB_ASSERT(0);
        return NEWS_REWARD_NONE;
    }
}
