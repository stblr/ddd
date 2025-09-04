#include "ETH.h"

#include <portable/Log.h>

BOOL ETHGetLinkStateAsync(BOOL *status) {
    DEBUG("%d", *status);

    BOOL result = REPLACED(ETHGetLinkStateAsync)(status);

    DEBUG("%d %d", *status, result);

    return result;
}
