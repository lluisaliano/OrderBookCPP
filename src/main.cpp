#include <iostream>
#include <memory>

#include "order.hpp"
#include "order_book.hpp"

int main() {
  OrderBook orderBook;
  const OrderId orderId{1};
  orderBook.AddOrder(std::make_shared<Order>(
      OrderConfig{.id = orderId,
                  .orderType = OrderType::GoodTillCancel,
                  .side = Side::Buy,
                  .price = Price{100},
                  .quantity = 10}));

  std::cout << orderBook.Size() << std::endl;
  orderBook.CancelOrder(orderId);

  std::cout << orderBook.Size() << std::endl;

  return 0;
}
