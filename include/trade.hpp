#pragma once

#include <vector>

#include "order.hpp"
#include "price.hpp"

struct TradeInfo {
  OrderId orderId{};
  Price price{};
  Quantity quantity{};
};

class Trade {
 private:
  TradeInfo m_bidTrade{};
  TradeInfo m_askTrade{};

 public:
  Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade)
      : m_bidTrade{bidTrade},
        m_askTrade{askTrade} {}

  const TradeInfo& getBidTrade() { return m_bidTrade; }
  const TradeInfo& getAskTrade() { return m_askTrade; }
};

using Trades = std::vector<Trade>;
