/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/bitget/shared.hpp"

namespace roq {
namespace bitget {

Shared::Shared(server::Dispatcher &dispatcher, Settings const &settings)
    : dispatcher_(dispatcher), settings{settings}, api{API::create(settings)}, rate_limiter(settings.misc.request_limit, settings.misc.request_limit_interval),
      symbols(settings.ws.max_subscriptions_per_stream) {
}

}  // namespace bitget
}  // namespace roq
