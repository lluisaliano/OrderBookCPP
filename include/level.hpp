#include <vector>

#include "order.hpp"
#include "price.hpp"

struct LevelInfo {
  Price price;
  Quantity quantity;
};

using LevelInfos = std::vector<LevelInfo>;

class OrderBookLevelInfos {
 private:
  LevelInfos m_bids;
  LevelInfos m_asks;

 public:
  OrderBookLevelInfos(const LevelInfos& bids, const LevelInfos& asks)
      : m_bids{bids},
        m_asks{asks} {}

  const LevelInfos& getBids() { return m_bids; }
  const LevelInfos& getAsks() { return m_asks; }
};
