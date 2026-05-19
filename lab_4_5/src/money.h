#pragma once

#include <cstdint>
#include <compare>

#include <qstring.h>

class Money
{
 public:
  explicit Money(std::int64_t cents = 0);

  [[nodiscard]] static Money from_decimal(double rubles);
  [[nodiscard]] static Money from_cents(std::int64_t cents);

  [[nodiscard]] std::int64_t cents() const;
  [[nodiscard]] double rubles() const;
  [[nodiscard]] QString formatted() const;

  Money operator+(Money other) const;
  Money operator-(Money other) const;
  Money operator*(std::int64_t count) const;
  auto operator<=>(Money const& other) const = default;

 private:
  std::int64_t m_cents;
};
