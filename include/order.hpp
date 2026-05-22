#pragma once

#include <cstdint>
#include <deque>
#include <format>
#include <memory>
#include <stdexcept>

#include "price.hpp"

using OrderId = std::uint64_t;
using Quantity = std::uint32_t;

enum class Side { Buy, Sell };
enum class OrderType { GoodTillCancel, FillOrKill };

struct OrderConfig {
  OrderId id;
  OrderType orderType;
  Side side;
  Price price;
  Quantity quantity;
};

class Order {
 public:
  explicit Order(OrderConfig orderConfig)
      : m_orderId{orderConfig.id},
        m_orderType{orderConfig.orderType},
        m_side{orderConfig.side},
        m_price{orderConfig.price},
        m_initialQuantity{orderConfig.quantity},
        m_remainingQuantity{orderConfig.quantity} {}

  [[nodiscard]] OrderId getOrderId() const { return m_orderId; }
  [[nodiscard]] Side getSide() const { return m_side; }
  [[nodiscard]] Price getPrice() const { return m_price; }
  [[nodiscard]] OrderType getOrderType() const { return m_orderType; }
  [[nodiscard]] Quantity getInitialQuantity() const {
    return m_initialQuantity;
  }
  [[nodiscard]] Quantity getRemainingQuantity() const {
    return m_remainingQuantity;
  }
  [[nodiscard]] Quantity getFilledQuantity() const {
    return getInitialQuantity() - getRemainingQuantity();
  }

  [[nodiscard]] bool isFilled() const {
    return getRemainingQuantity() == 0;
  }

  void fill(Quantity quantity) {
    if (quantity > m_remainingQuantity) {
      throw std::invalid_argument(std::format(
          "Quantity exceeds remaining quantity: {} > {} of order {}.", quantity,
          m_remainingQuantity, m_orderId));
    }
    m_remainingQuantity -= quantity;
  }

 private:
  OrderId m_orderId;
  OrderType m_orderType;
  Side m_side;
  Price m_price;
  Quantity m_initialQuantity;
  Quantity m_remainingQuantity;
};

using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::deque<OrderPointer>;

class OrderModify {
 private:
  OrderId m_orderId{};
  Side m_side{};
  Price m_price{};
  Quantity m_quantity{};

 public:
  OrderModify(OrderId orderId, Side side, Price price, Quantity quantity)
      : m_orderId{orderId},
        m_side{side},
        m_price{price},
        m_quantity{quantity} {}

  [[nodiscard]] OrderId getOrderId() const { return m_orderId; }
  [[nodiscard]] Price getPrice() const { return m_price; }
  [[nodiscard]] Side getSide() const { return m_side; }
  [[nodiscard]] Quantity getQuantity() const { return m_quantity; }

  OrderPointer toOrderPointer(OrderType type) const {
    return std::make_shared<Order>(OrderConfig{.id = getOrderId(),
                                               .orderType = type,
                                               .side = getSide(),
                                               .price = getPrice(),
                                               .quantity = getQuantity()});
  }
};
