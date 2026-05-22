#pragma once

#include <functional>
#include <map>
#include <unordered_map>

#include "order.hpp"
#include "price.hpp"
#include "trade.hpp"

class OrderBook {
 private:
  struct OrderEntry {
    OrderPointer m_order_{nullptr};
    OrderPointers::iterator m_location{};
  };

  std::map<Price, OrderPointers, std::greater<Price>> m_bids;
  std::map<Price, OrderPointers, std::less<Price>> m_asks;
  std::unordered_map<OrderId, OrderEntry> m_orders;

  bool CanMatch(Side side, Price price) const {
    if (side == Side::Buy) {
      if (m_asks.empty()) return false;

      const auto& bestAsk = m_asks.begin()->first;  // Get best ask price
      return price >= bestAsk;
    } else {
      if (m_bids.empty()) return false;

      const auto& bestBid  = m_bids.begin()->first;  // Get best bid price
      return price <= bestBid;
    }
  }

  Trades MatchOrders() {
    Trades trades;
    trades.reserve(m_orders.size() / 2);

    while (true) {
      if(m_bids.empty() || m_asks.empty()) break;

      auto& [bidPrice, bidsLevel] = *m_bids.begin();
      auto& [askPrice, asksLevel] = *m_asks.begin();

      if (bidPrice < askPrice) break; // There is nothing to match in this case

      while (bidsLevel.size() && asksLevel.size()) {
        auto& bid = bidsLevel.front(); // These take the earliest bid and ask that entered at that price
        auto& ask = asksLevel.front();

        // The filled quantity of an order is the minimum between the remaining quantity of each order
        Quantity quantity = std::min(bid->getRemainingQuantity(), ask->getRemainingQuantity());

        bid->fill(quantity);
        ask->fill(quantity);

        if (bid->isFilled()) {
          m_orders.erase(bid->getOrderId());
          bidsLevel.pop_front(); // Call the bid destructor here
        }

        if (ask->isFilled()) {
          m_orders.erase(ask->getOrderId());
          asksLevel.pop_front(); // Call the ask destructor here
        }

        // If on level of orders is empty, we delete it from the map
        if (bidsLevel.empty()) {
          m_bids.erase(bidPrice);
        }
        if (asksLevel.empty()) {
          m_asks.erase(askPrice);
        }
      }
    }

  }
};
