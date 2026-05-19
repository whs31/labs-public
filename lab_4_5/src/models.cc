#include "models.h"

TariffListModel::TariffListModel(QObject* const parent)
  : QAbstractListModel(parent)
{}

int TariffListModel::rowCount(QModelIndex const& parent) const
{
  if(parent.isValid())
    return 0;
  return this->m_tariffs.size();
}

QVariant TariffListModel::data(QModelIndex const& index, int const role) const
{
  if(! index.isValid() || index.row() < 0 || index.row() >= this->m_tariffs.size())
    return {};

  auto const& tariff = this->m_tariffs[index.row()];
  switch(role) {
    case IdRole: return tariff.id();
    case DestinationRole: return tariff.destination();
    case ClassNameRole: return to_string(tariff.ticket_class());
    case ClassIndexRole: return static_cast<int>(tariff.ticket_class());
    case PriceRublesRole: return tariff.base_price().rubles();
    case DiscountRublesRole: return tariff.discount().rubles();
    case EffectiveRublesRole: return tariff.effective_price().rubles();
    case PriceTextRole: return tariff.base_price().formatted();
    case DiscountTextRole: return tariff.discount().formatted();
    case EffectiveTextRole: return tariff.effective_price().formatted();
    default: return {};
  }
}

QHash<int, QByteArray> TariffListModel::roleNames() const
{
  return {
    {IdRole, "tariffId"},
    {DestinationRole, "destination"},
    {ClassNameRole, "className"},
    {ClassIndexRole, "classIndex"},
    {PriceRublesRole, "priceRubles"},
    {DiscountRublesRole, "discountRubles"},
    {EffectiveRublesRole, "effectiveRubles"},
    {PriceTextRole, "priceText"},
    {DiscountTextRole, "discountText"},
    {EffectiveTextRole, "effectiveText"},
  };
}

QVariantMap TariffListModel::get(int const row) const
{
  if(row < 0 || row >= this->m_tariffs.size())
    return {};

  auto const index = this->index(row, 0);
  return {
    {"tariffId", this->data(index, IdRole)},
    {"destination", this->data(index, DestinationRole)},
    {"className", this->data(index, ClassNameRole)},
    {"classIndex", this->data(index, ClassIndexRole)},
    {"priceRubles", this->data(index, PriceRublesRole)},
    {"discountRubles", this->data(index, DiscountRublesRole)},
    {"effectiveRubles", this->data(index, EffectiveRublesRole)},
    {"priceText", this->data(index, PriceTextRole)},
    {"discountText", this->data(index, DiscountTextRole)},
    {"effectiveText", this->data(index, EffectiveTextRole)},
  };
}

void TariffListModel::set_tariffs(QVector<Tariff> tariffs)
{
  this->beginResetModel();
  this->m_tariffs = std::move(tariffs);
  this->endResetModel();
}

TicketListModel::TicketListModel(QObject* const parent)
  : QAbstractListModel(parent)
{}

int TicketListModel::rowCount(QModelIndex const& parent) const
{
  if(parent.isValid())
    return 0;
  return this->m_tickets.size();
}

QVariant TicketListModel::data(QModelIndex const& index, int const role) const
{
  if(! index.isValid() || index.row() < 0 || index.row() >= this->m_tickets.size())
    return {};

  auto const& ticket = this->m_tickets[index.row()];
  switch(role) {
    case IdRole: return ticket.id();
    case PassengerNameRole: return ticket.passenger().name();
    case PassportRole: return ticket.passenger().passport();
    case DestinationRole: return ticket.destination();
    case ClassNameRole: return to_string(ticket.ticket_class());
    case PriceRublesRole: return ticket.price().rubles();
    case PriceTextRole: return ticket.price().formatted();
    case DateTextRole: return ticket.sold_at().toString("dd.MM.yyyy hh:mm");
    default: return {};
  }
}

QHash<int, QByteArray> TicketListModel::roleNames() const
{
  return {
    {IdRole, "ticketId"},
    {PassengerNameRole, "passengerName"},
    {PassportRole, "passport"},
    {DestinationRole, "destination"},
    {ClassNameRole, "className"},
    {PriceRublesRole, "priceRubles"},
    {PriceTextRole, "priceText"},
    {DateTextRole, "dateText"},
  };
}

QVariantMap TicketListModel::get(int const row) const
{
  if(row < 0 || row >= this->m_tickets.size())
    return {};

  auto const index = this->index(row, 0);
  return {
    {"ticketId", this->data(index, IdRole)},
    {"passengerName", this->data(index, PassengerNameRole)},
    {"passport", this->data(index, PassportRole)},
    {"destination", this->data(index, DestinationRole)},
    {"className", this->data(index, ClassNameRole)},
    {"priceRubles", this->data(index, PriceRublesRole)},
    {"priceText", this->data(index, PriceTextRole)},
    {"dateText", this->data(index, DateTextRole)},
  };
}

void TicketListModel::set_tickets(QVector<Ticket> tickets)
{
  this->beginResetModel();
  this->m_tickets = std::move(tickets);
  this->endResetModel();
}
