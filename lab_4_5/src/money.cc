#include "money.h"

#include <cmath>

Money::Money(std::int64_t const cents)
  : m_cents(cents)
{}

Money Money::from_decimal(double const rubles)
{
  return Money(static_cast<std::int64_t>(std::round(rubles * 100.0)));
}

Money Money::from_cents(std::int64_t const cents) { return Money(cents); }

std::int64_t Money::cents() const { return this->m_cents; }

double Money::rubles() const { return this->m_cents / 100.0; }

QString Money::formatted() const { return QString("%1 RUB").arg(this->rubles(), 0, 'f', 2); }

Money Money::operator+(Money const other) const { return Money(this->m_cents + other.m_cents); }

Money Money::operator-(Money const other) const
{
  return Money(this->m_cents > other.m_cents ? this->m_cents - other.m_cents : 0);
}

Money Money::operator*(std::int64_t const count) const { return Money(this->m_cents * count); }
