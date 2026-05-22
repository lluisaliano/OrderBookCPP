#pragma once
#include <compare>
#include <cstdint>
#include <stdexcept>

class Price {
 private:
  static constexpr std::int16_t scale =
      1000;  // Scale will be used to convert Price to double (Prices will have
             // 3 decimals)
  std::int64_t m_value{};

 public:
  explicit Price(std::int64_t value = 0) : m_value(value) {
    if (value < 0) {
      throw std::invalid_argument("Price value cannot be negative");
    }
  }

  Price getPrice() const { return Price(m_value); }

  Price operator-(const Price& other) const {
    return Price(m_value - other.m_value);
  }

  operator double() const { return static_cast<double>(m_value) / scale; }

  std::strong_ordering operator<=>(const Price& other) const = default;
};
