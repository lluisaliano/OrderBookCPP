#pragma once

#include <vector>

#include "order.hpp"
#include "price.hpp"

struct TradeInfo {
  OrderId orderId{};
  Price price{};
};

class Trade {
 private:
  TradeInfo m_bidTrade{};
  TradeInfo m_askTrade{};
  Quantity m_quantity{};

 public:
  Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade, Quantity quantity)
      : m_bidTrade{bidTrade},
        m_askTrade{askTrade},
        m_quantity{quantity} {}

  const TradeInfo& getBidTrade() { return m_bidTrade; }
  const TradeInfo& getAskTrade() { return m_askTrade; }
  const Quantity& getQuantity() const { return m_quantity; }
};

using Trades = std::vector<Trade>;
