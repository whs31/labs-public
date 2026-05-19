#pragma once

#include <qabstractitemmodel.h>
#include <qhash.h>
#include <qvariantmap.h>

#include "domain.h"

class TariffListModel final : public QAbstractListModel
{
  Q_OBJECT

 public:
  enum Role
  {
    IdRole = Qt::UserRole + 1,
    DestinationRole,
    ClassNameRole,
    ClassIndexRole,
    PriceRublesRole,
    DiscountRublesRole,
    EffectiveRublesRole,
    PriceTextRole,
    DiscountTextRole,
    EffectiveTextRole
  };

  explicit TariffListModel(QObject* parent = nullptr);

  [[nodiscard]] int rowCount(QModelIndex const& parent = QModelIndex()) const override;
  [[nodiscard]] QVariant data(QModelIndex const& index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  [[nodiscard]] Q_INVOKABLE QVariantMap get(int row) const;
  void set_tariffs(QVector<Tariff> tariffs);

 private:
  QVector<Tariff> m_tariffs;
};

class TicketListModel final : public QAbstractListModel
{
  Q_OBJECT

 public:
  enum Role
  {
    IdRole = Qt::UserRole + 1,
    PassengerNameRole,
    PassportRole,
    DestinationRole,
    ClassNameRole,
    PriceRublesRole,
    PriceTextRole,
    DateTextRole
  };

  explicit TicketListModel(QObject* parent = nullptr);

  [[nodiscard]] int rowCount(QModelIndex const& parent = QModelIndex()) const override;
  [[nodiscard]] QVariant data(QModelIndex const& index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  [[nodiscard]] Q_INVOKABLE QVariantMap get(int row) const;
  void set_tickets(QVector<Ticket> tickets);

 private:
  QVector<Ticket> m_tickets;
};
