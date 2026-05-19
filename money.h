#include <cstdint>

class Money
{
  std::int64_t m_cents;

 public:
  constexpr explicit Money(std::int64_t const cents = 0)
    : m_cents(cents)
  {}

  [[nodiscard]] static constexpr Money from_decimal(double const r)
  {
    return Money(static_cast<std::int64_t>(r * 100 + .5));
  }

  [[nodiscard]] constexpr double rubles() const { return this->m_cents / 100.0; }

  constexpr Money operator+(Money const other) const
  {
    return Money(this->m_cents + other.m_cents);
  }

  auto operator<=>(Money const&) const = default;

  Money operator-(Money const other) const
  {
    return Money(this->m_cents > other.m_cents ? this->m_cents - other.m_cents : 0);
  }
};
