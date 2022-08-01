#include "global.h"
#include "random.h"
#include "event_data.h"
#include "wonder_news.h"

/*
    Wonder News related functions.
    Because this feature is largely unused, the names in here are
    mostly nebulous and without a real indication of purpose.
*/

enum {
    NEWS_VAL_INVALID,
    NEWS_VAL_RECV_FRIEND,
    NEWS_VAL_RECV_WIRELESS,
    NEWS_VAL_NONE,
    NEWS_VAL_SENT,
    NEWS_VAL_SENT_MAX,
    NEWS_VAL_GET_MAX,
};

static u32 GetNewsId(struct WonderNewsMetadata *);
static void IncrementGetNewsCounter(struct WonderNewsMetadata *);
static u32 GetNewsValByNewsType(struct WonderNewsMetadata *);
static void IncrementSentNewsCounter(struct WonderNewsMetadata *);
static void ResetSentNewsCounter(struct WonderNewsMetadata *);

static u32 GetNewsId(struct WonderNewsMetadata *data)
{
    u32 id;
    data->newsType = WONDER_NEWS_NONE;
    id = data->rand + 132;
    data->rand = 0;
    IncrementGetNewsCounter(data);
    return id;
}

static void ResetSentNewsCounter(struct WonderNewsMetadata *data)
{
    data->sentCounter = 0;
}

static void IncrementSentNewsCounter(struct WonderNewsMetadata *data)
{
    data->sentCounter++;
    if (data->sentCounter > 4)
        data->sentCounter = 4;
}

static void IncrementGetNewsCounter(struct WonderNewsMetadata *data)
{
    data->getCounter++;
    if (data->getCounter > 5)
        data->getCounter = 5;
}

static u32 GetNewsValByNewsType(struct WonderNewsMetadata *data)
{
    if (data->getCounter == 5)
        return NEWS_VAL_GET_MAX;

    switch (data->newsType)
    {
    case WONDER_NEWS_NONE:
        return NEWS_VAL_NONE;
    case WONDER_NEWS_RECV_FRIEND:
        return NEWS_VAL_RECV_FRIEND;
    case WONDER_NEWS_RECV_WIRELESS:
        return NEWS_VAL_RECV_WIRELESS;
    case WONDER_NEWS_SENT:
        if (data->sentCounter < 3)
            return NEWS_VAL_SENT;
        return NEWS_VAL_SENT_MAX;
    default:
        AGB_ASSERT(0);
        return NEWS_VAL_INVALID;
    }
}
