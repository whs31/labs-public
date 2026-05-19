#include "appcontroller.h"

#include <QCoreApplication>
#include <QDir>
#include <QRegularExpression>
#include <QUrl>

AppController::AppController(
  TariffListModel* const tariff_model,
  TicketListModel* const ticket_model,
  QObject* const parent
)
  : QObject(parent)
  , m_tariff_model(tariff_model)
  , m_ticket_model(ticket_model)
  , m_database_mode(false)
{
  auto error = QString();
  auto const db_path = QDir(QCoreApplication::applicationDirPath()).filePath("lab_4.sqlite");
  this->m_database_mode = this->m_sql_storage.open(db_path, &error);
  if(this->m_database_mode) {
    auto data = DeskData();
    if(this->m_sql_storage.load(&data, &error))
      this->m_cash_desk.set_data(data.tariffs, data.tickets);
    this->set_status("Database mode: " + db_path);
  }
  else
    this->set_status("Local mode: " + error);

  this->seed_if_empty();
  this->refresh_models();
  this->sync_database();
}

AppController::~AppController() = default;

QString AppController::status_message() const { return this->m_status_message; }

QString AppController::mode_text() const
{
  return this->m_database_mode ? "Database mode" : "Local mode";
}

bool AppController::database_mode() const { return this->m_database_mode; }

QString AppController::total_revenue_text() const
{
  return this->m_cash_desk.total_revenue().formatted();
}

QString AppController::max_tariff_text() const
{
  auto const tariff = this->m_cash_desk.max_cost_tariff();
  if(! tariff)
    return "No tariffs";
  return tariff->destination() + " / " + tariff->effective_price().formatted();
}

bool AppController::addTariff(
  QString destination,
  QString const price,
  QString const discount,
  int const class_index
)
{
  auto price_money = Money();
  auto discount_money = Money();
  auto error = QString();
  destination = destination.trimmed();

  if(! this->parse_money(price, &price_money, "Price")
     || ! this->parse_money(discount, &discount_money, "Discount"))
    return false;
  if(! this->validate_tariff(destination, price_money, discount_money, &error)) {
    this->set_status(error);
    return false;
  }

  this->m_cash_desk += Tariff(
    0,
    destination,
    price_money,
    ticket_class_from_int(class_index),
    discount_money
  );
  this->refresh_models();
  this->sync_database();
  this->set_status("Tariff added");
  return true;
}

bool AppController::updateTariff(
  int const index,
  QString destination,
  QString const price,
  QString const discount,
  int const class_index
)
{
  auto price_money = Money();
  auto discount_money = Money();
  auto error = QString();
  destination = destination.trimmed();

  if(! this->parse_money(price, &price_money, "Price")
     || ! this->parse_money(discount, &discount_money, "Discount"))
    return false;
  if(! this->validate_tariff(destination, price_money, discount_money, &error)) {
    this->set_status(error);
    return false;
  }
  if(! this->m_cash_desk.update_tariff(
       index,
       Tariff(0, destination, price_money, ticket_class_from_int(class_index), discount_money)
     )) {
    this->set_status("Select tariff to update");
    return false;
  }

  this->refresh_models();
  this->sync_database();
  this->set_status("Tariff updated");
  return true;
}

bool AppController::removeTariff(int const index)
{
  if(! this->m_cash_desk.remove_tariff(index)) {
    this->set_status("Tariff cannot be removed: select row or remove related tickets first");
    return false;
  }

  this->refresh_models();
  this->sync_database();
  this->set_status("Tariff removed");
  return true;
}

bool AppController::buyTicket(int const tariff_index, QString passenger_name, QString passport)
{
  auto error = QString();
  passenger_name = passenger_name.trimmed();
  passport = passport.trimmed();
  if(not this->validate_passenger(passenger_name, passport, &error)) {
    this->set_status(error);
    return false;
  }

  auto const ticket = this->m_cash_desk
                        .buy_ticket(tariff_index, Passenger(passenger_name, passport));
  if(ticket.id() == 0) {
    this->set_status("Select tariff before selling ticket");
    return false;
  }

  this->refresh_models();
  this->sync_database();
  this->set_status("Ticket sold");
  return true;
}

bool AppController::removeTicket(int const index)
{
  if(! this->m_cash_desk.remove_ticket(index)) {
    this->set_status("Select ticket to remove");
    return false;
  }

  this->refresh_models();
  this->sync_database();
  this->set_status("Ticket removed");
  return true;
}

void AppController::sortTariffsByDestination()
{
  this->m_cash_desk.sort_tariffs_by_destination();
  this->refresh_models();
  this->sync_database();
  this->set_status("Tariffs sorted by destination");
}

void AppController::sortTariffsByPrice()
{
  this->m_cash_desk.sort_tariffs_by_price();
  this->refresh_models();
  this->sync_database();
  this->set_status("Tariffs sorted by effective price");
}

void AppController::sortTicketsByPassenger()
{
  this->m_cash_desk.sort_tickets_by_passenger();
  this->refresh_models();
  this->sync_database();
  this->set_status("Tickets sorted by passenger");
}

bool AppController::exportToFile(QString const path)
{
  auto error = QString();
  auto const normalized = this->normalize_path(path);
  if(normalized.isEmpty()) {
    this->set_status("File path is empty");
    return false;
  }

  if(! this->m_binary_storage.save(normalized, this->current_data(), &error)) {
    this->set_status("Export failed: " + error);
    return false;
  }

  this->set_status("Saved to " + normalized);
  return true;
}

bool AppController::importFromFile(QString const path)
{
  auto error = QString();
  auto data = DeskData();
  auto const normalized = this->normalize_path(path);
  if(normalized.isEmpty()) {
    this->set_status("File path is empty");
    return false;
  }

  if(! this->m_binary_storage.load(normalized, &data, &error)) {
    this->set_status("Import failed: " + error);
    return false;
  }

  this->m_cash_desk.set_data(data.tariffs, data.tickets);
  this->refresh_models();
  this->sync_database();
  this->set_status("Loaded from " + normalized);
  return true;
}

bool AppController::saveDatabaseToFile(QString const path)
{
  if(! this->m_database_mode)
    return this->exportToFile(path);

  auto data = DeskData();
  auto error = QString();
  if(! this->m_sql_storage.load(&data, &error)) {
    this->set_status("Database read failed: " + error);
    return false;
  }

  auto const normalized = this->normalize_path(path);
  if(! this->m_binary_storage.save(normalized, data, &error)) {
    this->set_status("Database export failed: " + error);
    return false;
  }

  this->set_status("Database saved to " + normalized);
  return true;
}

bool AppController::loadFileToDatabase(QString const path)
{
  auto error = QString();
  auto data = DeskData();
  auto const normalized = this->normalize_path(path);
  if(! this->m_binary_storage.load(normalized, &data, &error)) {
    this->set_status("Database import failed: " + error);
    return false;
  }

  this->m_cash_desk.set_data(data.tariffs, data.tickets);
  if(this->m_database_mode && ! this->m_sql_storage.save(data, &error)) {
    this->set_status("Database write failed: " + error);
    return false;
  }

  this->refresh_models();
  this->set_status(this->m_database_mode ? "File loaded into database" : "File loaded locally");
  return true;
}

void AppController::openAboutWindow()
{
  auto* window = new InfoWindow("About lab 4 & 5");
  window->setModality(Qt::NonModal);
  window->show();
  this->m_windows.push_back(QPointer<InfoWindow>(window));
}

void AppController::openReportWindow()
{
  auto const message = QString("Tickets sold: %1\nTotal revenue: %2")
                         .arg(this->m_cash_desk.tickets().size())
                         .arg(this->total_revenue_text());
  auto const details = QString("Tariffs: %1\nMax tariff: %2\nMode: %3")
                         .arg(this->m_cash_desk.tariffs().size())
                         .arg(this->max_tariff_text())
                         .arg(this->mode_text());
  auto* window = new InfoWindow("Sales report", message, details, QColor("#0f766e"));
  window->setModality(Qt::NonModal);
  window->show();
  this->m_windows.push_back(QPointer<InfoWindow>(window));
}

DeskData AppController::current_data() const
{
  return DeskData {this->m_cash_desk.tariffs(), this->m_cash_desk.tickets()};
}

QString AppController::normalize_path(QString const& value) const
{
  auto path = value.trimmed();
  auto const url = QUrl(path);
  if(url.isValid() && url.isLocalFile())
    path = url.toLocalFile();
  if(QDir::isRelativePath(path))
    path = QDir(QCoreApplication::applicationDirPath()).filePath(path);
  return path;
}

bool AppController::parse_money(QString const& text, Money* const result, QString const& field_name)
{
  auto ok = false;
  auto normalized = text.trimmed();
  normalized.replace(',', '.');
  auto const value = normalized.toDouble(&ok);
  if(! ok || value < 0) {
    this->set_status(field_name + " must be a non-negative number");
    return false;
  }
  if(result != nullptr)
    *result = Money::from_decimal(value);
  return true;
}

bool AppController::validate_tariff(
  QString const& destination,
  Money const price,
  Money const discount,
  QString* const error
) const
{
  if(destination.isEmpty()) {
    *error = "Destination cannot be empty";
    return false;
  }
  if(price.cents() <= 0) {
    *error = "Price must be greater than zero";
    return false;
  }
  if(discount >= price) {
    *error = "Discount must be less than price";
    return false;
  }
  return true;
}

bool AppController::validate_passenger(
  QString const& passenger_name,
  QString const& passport,
  QString* const error
) const
{
  if(passenger_name.isEmpty()) {
    *error = "Passenger name cannot be empty";
    return false;
  }

  auto const passport_pattern = QRegularExpression("^[0-9A-Za-z -]{4,32}$");
  if(! passport_pattern.match(passport).hasMatch()) {
    *error = "Passport must contain 4-32 letters, digits, spaces or hyphens";
    return false;
  }
  return true;
}

void AppController::seed_if_empty()
{
  if(! this->m_cash_desk.tariffs().empty())
    return;

  this->m_cash_desk += Tariff(
    0,
    "Moscow",
    Money::from_decimal(45000.0),
    TicketClass::Economy,
    Money::from_decimal(0.0)
  );
  this->m_cash_desk += Tariff(
    0,
    "Saint Petersburg",
    Money::from_decimal(30000.0),
    TicketClass::Business,
    Money::from_decimal(5000.0)
  );
  this->m_cash_desk += Tariff(
    0,
    "Belgrade",
    Money::from_decimal(85000.0),
    TicketClass::First,
    Money::from_decimal(0.0)
  );
}

void AppController::refresh_models()
{
  this->m_tariff_model->set_tariffs(this->m_cash_desk.tariffs());
  this->m_ticket_model->set_tickets(this->m_cash_desk.tickets());
  emit this->totalsChanged();
}

void AppController::sync_database()
{
  if(! this->m_database_mode)
    return;

  auto error = QString();
  if(! this->m_sql_storage.save(this->current_data(), &error))
    this->set_status("Database sync failed: " + error);
}

void AppController::set_status(QString message)
{
  this->m_status_message = std::move(message);
  emit this->statusMessageChanged();
}
