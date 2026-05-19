#include "domain.h"

#include <algorithm>
#include <numeric>
#include <utility>

QString to_string(TicketClass const ticket_class)
{
  switch(ticket_class) {
    case TicketClass::Economy: return "Economy";
    case TicketClass::Business: return "Business";
    case TicketClass::First: return "First";
  }
  return "Unknown";
}

TicketClass ticket_class_from_int(int const value)
{
  switch(value) {
    case 1: return TicketClass::Business;
    case 2: return TicketClass::First;
    default: return TicketClass::Economy;
  }
}

Passenger::Passenger() = default;

Passenger::Passenger(QString name, QString passport)
  : m_name(std::move(name))
  , m_passport(std::move(passport))
{}

Passenger::~Passenger() = default;

QString Passenger::name() const { return this->m_name; }

QString Passenger::passport() const { return this->m_passport; }

IPricingStrategy::~IPricingStrategy() = default;

Money StandardPricing::calculate(Money const base_price) const { return base_price; }

FixedDiscountPricing::FixedDiscountPricing(Money const discount)
  : m_discount(discount)
{}

Money FixedDiscountPricing::calculate(Money const base_price) const
{
  return base_price - this->m_discount;
}

Tariff::Tariff()
  : m_id(0)
  , m_ticket_class(TicketClass::Economy)
{}

Tariff::Tariff(
  int const id,
  QString destination,
  Money const base_price,
  TicketClass const ticket_class,
  Money const discount
)
  : m_id(id)
  , m_destination(std::move(destination))
  , m_base_price(base_price)
  , m_ticket_class(ticket_class)
  , m_discount(discount)
{}

Tariff::~Tariff() = default;

int Tariff::id() const { return this->m_id; }

QString Tariff::destination() const { return this->m_destination; }

Money Tariff::base_price() const { return this->m_base_price; }

Money Tariff::discount() const { return this->m_discount; }

TicketClass Tariff::ticket_class() const { return this->m_ticket_class; }

Money Tariff::effective_price() const
{
  if(this->m_discount.cents() > 0)
    return FixedDiscountPricing(this->m_discount).calculate(this->m_base_price);
  return StandardPricing().calculate(this->m_base_price);
}

void Tariff::set_id(int const id) { this->m_id = id; }

bool Tariff::operator<(Tariff const& other) const
{
  return this->effective_price() < other.effective_price();
}

bool Tariff::operator==(Tariff const& other) const { return this->m_id == other.m_id; }

Ticket::Ticket()
  : m_id(0)
  , m_tariff_id(0)
  , m_ticket_class(TicketClass::Economy)
{}

Ticket::Ticket(
  int const id,
  Passenger passenger,
  int const tariff_id,
  QString destination,
  TicketClass const ticket_class,
  Money const price,
  QDateTime sold_at
)
  : m_id(id)
  , m_passenger(std::move(passenger))
  , m_tariff_id(tariff_id)
  , m_destination(std::move(destination))
  , m_ticket_class(ticket_class)
  , m_price(price)
  , m_sold_at(std::move(sold_at))
{}

Ticket::~Ticket() = default;

int Ticket::id() const { return this->m_id; }

Passenger Ticket::passenger() const { return this->m_passenger; }

int Ticket::tariff_id() const { return this->m_tariff_id; }

QString Ticket::destination() const { return this->m_destination; }

TicketClass Ticket::ticket_class() const { return this->m_ticket_class; }

Money Ticket::price() const { return this->m_price; }

QDateTime Ticket::sold_at() const { return this->m_sold_at; }

void Ticket::set_id(int const id) { this->m_id = id; }

bool Ticket::operator==(Ticket const& other) const { return this->m_id == other.m_id; }

CashDesk::CashDesk() = default;

CashDesk::~CashDesk() = default;

QVector<Tariff> const& CashDesk::tariffs() const { return this->m_tariffs; }

QVector<Ticket> const& CashDesk::tickets() const { return this->m_tickets; }

Money CashDesk::total_revenue() const
{
  return std::accumulate(
    this->m_tickets.begin(),
    this->m_tickets.end(),
    Money(),
    [](Money const result, Ticket const& ticket) { return result + ticket.price(); }
  );
}

std::optional<Tariff> CashDesk::max_cost_tariff() const
{
  if(this->m_tariffs.empty())
    return std::nullopt;

  auto const it = std::max_element(this->m_tariffs.begin(), this->m_tariffs.end());
  return *it;
}

Tariff CashDesk::add_tariff(Tariff const& tariff)
{
  auto copy = tariff;
  copy.set_id(copy.id() > 0 ? copy.id() : this->next_tariff_id());
  this->m_tariffs.push_back(copy);
  return copy;
}

bool CashDesk::update_tariff(int const index, Tariff const& tariff)
{
  if(index < 0 || index >= this->m_tariffs.size())
    return false;

  auto copy = tariff;
  copy.set_id(this->m_tariffs[index].id());
  this->m_tariffs[index] = copy;
  return true;
}

bool CashDesk::remove_tariff(int const index)
{
  if(index < 0 || index >= this->m_tariffs.size())
    return false;

  auto const tariff_id = this->m_tariffs[index].id();
  auto const has_sold_tickets = std::
    any_of(this->m_tickets.begin(), this->m_tickets.end(), [tariff_id](Ticket const& ticket) {
      return ticket.tariff_id() == tariff_id;
    });
  if(has_sold_tickets)
    return false;

  this->m_tariffs.removeAt(index);
  return true;
}

Ticket CashDesk::buy_ticket(int const tariff_index, Passenger const& passenger)
{
  if(tariff_index < 0 || tariff_index >= this->m_tariffs.size())
    return Ticket();

  auto const& tariff = this->m_tariffs[tariff_index];
  auto ticket = Ticket(
    this->next_ticket_id(),
    passenger,
    tariff.id(),
    tariff.destination(),
    tariff.ticket_class(),
    tariff.effective_price(),
    QDateTime::currentDateTime()
  );
  this->m_tickets.push_back(ticket);
  return ticket;
}

bool CashDesk::remove_ticket(int const index)
{
  if(index < 0 || index >= this->m_tickets.size())
    return false;

  this->m_tickets.removeAt(index);
  return true;
}

CashDesk& CashDesk::operator+=(Tariff const& tariff)
{
  this->add_tariff(tariff);
  return *this;
}

void CashDesk::set_data(QVector<Tariff> tariffs, QVector<Ticket> tickets)
{
  this->m_tariffs = std::move(tariffs);
  this->m_tickets = std::move(tickets);
}

void CashDesk::clear()
{
  this->m_tariffs.clear();
  this->m_tickets.clear();
}

void CashDesk::sort_tariffs_by_destination()
{
  std::sort(
    this->m_tariffs.begin(),
    this->m_tariffs.end(),
    [](Tariff const& left, Tariff const& right) { return left.destination() < right.destination(); }
  );
}

void CashDesk::sort_tariffs_by_price()
{
  std::sort(this->m_tariffs.begin(), this->m_tariffs.end());
}

void CashDesk::sort_tickets_by_passenger()
{
  std::sort(
    this->m_tickets.begin(),
    this->m_tickets.end(),
    [](Ticket const& left, Ticket const& right) {
      return left.passenger().name() < right.passenger().name();
    }
  );
}

int CashDesk::next_tariff_id() const
{
  auto max_id = 0;
  for(auto const& tariff : this->m_tariffs)
    max_id = std::max(max_id, tariff.id());
  return max_id + 1;
}

int CashDesk::next_ticket_id() const
{
  auto max_id = 0;
  for(auto const& ticket : this->m_tickets)
    max_id = std::max(max_id, ticket.id());
  return max_id + 1;
}
