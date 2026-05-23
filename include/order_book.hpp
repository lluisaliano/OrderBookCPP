#pragma once

#include <functional>
#include <iterator>
#include <map>
#include <numeric>
#include <unordered_map>

#include "level.hpp"
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

  // This should account for complete sizes
  bool CanMatch(Side side, Price price) const {
    if (side == Side::Buy) {
      if (m_asks.empty()) return false;

      const auto& bestAsk = m_asks.begin()->first;  // Get best ask price
      return price >= bestAsk;
    } else {
      if (m_bids.empty()) return false;

      const auto& bestBid = m_bids.begin()->first;  // Get best bid price
      return price <= bestBid;
    }
  }

  Trades MatchOrders() {
    Trades trades;
    trades.reserve(m_orders.size() / 2);

    while (true) {
      if (m_bids.empty() || m_asks.empty()) break;

      auto& [bidPrice, bidsLevel] = *m_bids.begin();
      auto& [askPrice, asksLevel] = *m_asks.begin();

      if (bidPrice < askPrice) break;  // There is nothing to match in this case

      while (bidsLevel.size() && asksLevel.size()) {
        auto& bid = bidsLevel.front();  // These take the earliest bid and ask
                                        // that entered at that price
        auto& ask = asksLevel.front();

        // The filled quantity of an order is the minimum between the remaining
        // quantity of each order
        Quantity quantity =
            std::min(bid->getRemainingQuantity(), ask->getRemainingQuantity());

        bid->fill(quantity);
        ask->fill(quantity);

        if (bid->isFilled()) {
          m_orders.erase(bid->getOrderId());
          bidsLevel.pop_front();  // Call the bid destructor here
        }

        if (ask->isFilled()) {
          m_orders.erase(ask->getOrderId());
          asksLevel.pop_front();  // Call the ask destructor here
        }

        // If one level of orders is empty, we delete it from the map
        if (bidsLevel.empty()) {
          m_bids.erase(bidPrice);
        }
        if (asksLevel.empty()) {
          m_asks.erase(askPrice);
        }

        trades.push_back(Trade{
            TradeInfo{.orderId = bid->getOrderId(), .price = bid->getPrice()},
            TradeInfo{.orderId = ask->getOrderId(), .price = ask->getPrice()},
            quantity});
      }
    }

    // ?? In case there were FOK Orders that could have been completely matched
    // but were not, we have to cancel them
    if (!m_bids.empty()) {
      auto& bidsLevel = m_bids.begin()->second;
      auto& order = bidsLevel.front();
      if (order->getOrderType() == OrderType::FillOrKill) {
        CancelOrder(order->getOrderId());
      }
    }

    if (!m_asks.empty()) {
      auto& askLevel = m_asks.begin()->second;
      auto& order = askLevel.front();
      if (order->getOrderType() == OrderType::FillOrKill) {
        CancelOrder(order->getOrderId());
      }
    }

    return trades;
  }

 public:
  Trades AddOrder(OrderPointer order) {
    if (m_orders.contains(order->getOrderId())) {
      return {};
    }

    if (order->getOrderType() == OrderType::FillOrKill &&
        CanMatch(order->getSide(), order->getPrice())) {
      return {};
    }

    OrderPointers::iterator iterator;

    if (order->getSide() == Side::Buy) {
      auto& orders = m_bids[order->getPrice()];
      orders.push_back(order);
      iterator =
          std::prev(orders.end());  // Get iterator to the element we pushed
    } else {
      auto& orders = m_asks[order->getPrice()];
      orders.push_back(order);
      iterator = std::prev(orders.end());
    }

    m_orders.insert({order->getOrderId(), OrderEntry{order, iterator}});

    return MatchOrders();
  }

  void CancelOrder(OrderId orderId) {
    if (!m_orders.contains(orderId)) {
      return;
    }

    const auto& [order, iterator] = m_orders[orderId];
    m_orders.erase(orderId);

    auto price = order->getPrice();
    if (order->getSide() == Side::Sell) {
      auto& orders = m_asks.at(price);
      orders.erase(iterator);
      if (orders.empty()) {
        m_asks.erase(price);
      }
    } else {
      auto& orders = m_bids.at(price);
      orders.erase(iterator);
      if (orders.empty()) {
        m_bids.erase(price);
      }
    }
  }

  Trades MatchOrder(OrderModify order) {
    if (!m_orders.contains(order.getOrderId())) {
      return {};
    }

    const auto& [existingOrder, _] = m_orders.at(order.getOrderId());
    CancelOrder(order.getOrderId());
    return AddOrder(order.toOrderPointer(existingOrder->getOrderType()));
  }

  std::size_t Size() const { return m_orders.size(); }

  OrderBookLevelInfos GetOrderInfos() const {
    LevelInfos bidInfos, askInfos;
    bidInfos.reserve(m_orders.size() / 2);
    askInfos.reserve(m_orders.size() / 2);

    auto CreateLevelInfos = [](Price price, const OrderPointers& orders) {
      return LevelInfo{
          price, std::accumulate(
                     orders.begin(), orders.end(), Quantity{1},
                     [](Quantity runningSum, const OrderPointer& order) {
                       return runningSum + order->getRemainingQuantity();
                     })};
    };

    for (const auto& [price, orders] : m_bids) {
      bidInfos.push_back(CreateLevelInfos(price, orders));
    }

    for (const auto& [price, orders] : m_asks) {
      askInfos.push_back(CreateLevelInfos(price, orders));
    }

    return {bidInfos, askInfos};
  }
};
