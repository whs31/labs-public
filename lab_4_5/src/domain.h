#pragma once

#include <optional>

#include <qdatetime.h>
#include <qstring.h>
#include <qvector.h>

#include "money.h"

enum class TicketClass
{
  Economy,
  Business,
  First
};

[[nodiscard]] QString to_string(TicketClass ticket_class);
[[nodiscard]] TicketClass ticket_class_from_int(int value);

class Passenger
{
 public:
  Passenger();
  Passenger(QString name, QString passport);
  ~Passenger();

  [[nodiscard]] QString name() const;
  [[nodiscard]] QString passport() const;

 private:
  QString m_name;
  QString m_passport;
};

class IPricingStrategy
{
 public:
  virtual ~IPricingStrategy();

  [[nodiscard]] virtual Money calculate(Money base_price) const = 0;
};

class StandardPricing final : public IPricingStrategy
{
 public:
  [[nodiscard]] Money calculate(Money base_price) const override;
};

class FixedDiscountPricing final : public IPricingStrategy
{
 public:
  explicit FixedDiscountPricing(Money discount);

  [[nodiscard]] Money calculate(Money base_price) const override;

 private:
  Money m_discount;
};

class Tariff
{
 public:
  Tariff();
  Tariff(int id, QString destination, Money base_price, TicketClass ticket_class, Money discount);
  ~Tariff();

  [[nodiscard]] int id() const;
  [[nodiscard]] QString destination() const;
  [[nodiscard]] Money base_price() const;
  [[nodiscard]] Money discount() const;
  [[nodiscard]] TicketClass ticket_class() const;
  [[nodiscard]] Money effective_price() const;

  void set_id(int id);

  [[nodiscard]] bool operator<(Tariff const& other) const;
  [[nodiscard]] bool operator==(Tariff const& other) const;

 private:
  int m_id;
  QString m_destination;
  Money m_base_price;
  TicketClass m_ticket_class;
  Money m_discount;
};

class Ticket
{
 public:
  Ticket();
  Ticket(
    int id,
    Passenger passenger,
    int tariff_id,
    QString destination,
    TicketClass ticket_class,
    Money price,
    QDateTime sold_at
  );
  ~Ticket();

  [[nodiscard]] int id() const;
  [[nodiscard]] Passenger passenger() const;
  [[nodiscard]] int tariff_id() const;
  [[nodiscard]] QString destination() const;
  [[nodiscard]] TicketClass ticket_class() const;
  [[nodiscard]] Money price() const;
  [[nodiscard]] QDateTime sold_at() const;

  void set_id(int id);

  [[nodiscard]] bool operator==(Ticket const& other) const;

 private:
  int m_id;
  Passenger m_passenger;
  int m_tariff_id;
  QString m_destination;
  TicketClass m_ticket_class;
  Money m_price;
  QDateTime m_sold_at;
};

class CashDesk
{
 public:
  CashDesk();
  ~CashDesk();

  [[nodiscard]] QVector<Tariff> const& tariffs() const;
  [[nodiscard]] QVector<Ticket> const& tickets() const;

  [[nodiscard]] Money total_revenue() const;
  [[nodiscard]] std::optional<Tariff> max_cost_tariff() const;

  Tariff add_tariff(Tariff const& tariff);
  bool update_tariff(int index, Tariff const& tariff);
  bool remove_tariff(int index);
  Ticket buy_ticket(int tariff_index, Passenger const& passenger);
  bool remove_ticket(int index);

  CashDesk& operator+=(Tariff const& tariff);

  void set_data(QVector<Tariff> tariffs, QVector<Ticket> tickets);
  void clear();
  void sort_tariffs_by_destination();
  void sort_tariffs_by_price();
  void sort_tickets_by_passenger();

 private:
  [[nodiscard]] int next_tariff_id() const;
  [[nodiscard]] int next_ticket_id() const;

  QVector<Tariff> m_tariffs;
  QVector<Ticket> m_tickets;
};
