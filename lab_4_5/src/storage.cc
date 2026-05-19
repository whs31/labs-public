#include "storage.h"

#include <qfile.h>
#include <qsqlerror.h>
#include <qsqlquery.h>
#include <qvariant.h>

namespace
{
  constexpr quint32 file_magic = 0x41495234;
  constexpr quint32 file_version = 1;

  [[nodiscard]] QSqlDatabase database(QString const& connection_name)
  {
    return QSqlDatabase::database(connection_name);
  }
}  // namespace

QDataStream& operator<<(QDataStream& out, Tariff const& tariff)
{
  out << tariff.id();
  out << tariff.destination();
  out << static_cast<qint64>(tariff.base_price().cents());
  out << static_cast<qint32>(tariff.ticket_class());
  out << static_cast<qint64>(tariff.discount().cents());
  return out;
}

QDataStream& operator>>(QDataStream& in, Tariff& tariff)
{
  auto id = qint32();
  auto destination = QString();
  auto base_cents = qint64();
  auto class_value = qint32();
  auto discount_cents = qint64();

  in >> id >> destination >> base_cents >> class_value >> discount_cents;
  tariff = Tariff(
    id,
    destination,
    Money::from_cents(base_cents),
    ticket_class_from_int(class_value),
    Money::from_cents(discount_cents)
  );
  return in;
}

QDataStream& operator<<(QDataStream& out, Ticket const& ticket)
{
  out << ticket.id();
  out << ticket.passenger().name();
  out << ticket.passenger().passport();
  out << ticket.tariff_id();
  out << ticket.destination();
  out << static_cast<qint32>(ticket.ticket_class());
  out << static_cast<qint64>(ticket.price().cents());
  out << ticket.sold_at();
  return out;
}

QDataStream& operator>>(QDataStream& in, Ticket& ticket)
{
  auto id = qint32();
  auto passenger_name = QString();
  auto passport = QString();
  auto tariff_id = qint32();
  auto destination = QString();
  auto class_value = qint32();
  auto price_cents = qint64();
  auto sold_at = QDateTime();

  in >> id >> passenger_name >> passport >> tariff_id >> destination >> class_value >> price_cents
    >> sold_at;
  ticket = Ticket(
    id,
    Passenger(passenger_name, passport),
    tariff_id,
    destination,
    ticket_class_from_int(class_value),
    Money::from_cents(price_cents),
    sold_at
  );
  return in;
}

bool BinaryStorage::save(QString const& path, DeskData const& data, QString* error) const
{
  auto file = QFile(path);
  if(! file.open(QIODevice::WriteOnly)) {
    if(error != nullptr)
      *error = file.errorString();
    return false;
  }

  auto out = QDataStream(&file);
  out.setVersion(QDataStream::Qt_6_0);
  out << file_magic << file_version;
  out << static_cast<quint32>(data.tariffs.size());
  for(auto const& tariff : data.tariffs)
    out << tariff;
  out << static_cast<quint32>(data.tickets.size());
  for(auto const& ticket : data.tickets)
    out << ticket;

  if(out.status() != QDataStream::Ok) {
    if(error != nullptr)
      *error = "Binary write failed";
    return false;
  }
  return true;
}

bool BinaryStorage::load(QString const& path, DeskData* data, QString* error) const
{
  if(data == nullptr) {
    if(error != nullptr)
      *error = "Internal error: empty destination";
    return false;
  }

  auto file = QFile(path);
  if(! file.open(QIODevice::ReadOnly)) {
    if(error != nullptr)
      *error = file.errorString();
    return false;
  }

  auto in = QDataStream(&file);
  in.setVersion(QDataStream::Qt_6_0);

  auto magic = quint32();
  auto version = quint32();
  in >> magic >> version;
  if(magic != file_magic || version != file_version) {
    if(error != nullptr)
      *error = "Unsupported binary file format";
    return false;
  }

  auto result = DeskData();
  auto tariff_count = quint32();
  in >> tariff_count;
  for(auto i = quint32(); i < tariff_count; ++i) {
    auto tariff = Tariff();
    in >> tariff;
    result.tariffs.push_back(tariff);
  }

  auto ticket_count = quint32();
  in >> ticket_count;
  for(auto i = quint32(); i < ticket_count; ++i) {
    auto ticket = Ticket();
    in >> ticket;
    result.tickets.push_back(ticket);
  }

  if(in.status() != QDataStream::Ok) {
    if(error != nullptr)
      *error = "Binary read failed";
    return false;
  }

  *data = result;
  return true;
}

SqlStorage::SqlStorage()
  : m_connection_name("lab_4_connection")
  , m_available(false)
{}

SqlStorage::~SqlStorage()
{
  if(QSqlDatabase::contains(this->m_connection_name)) {
    auto db = database(this->m_connection_name);
    db.close();
  }
  QSqlDatabase::removeDatabase(this->m_connection_name);
}

bool SqlStorage::open(QString const& path, QString* error)
{
  if(! QSqlDatabase::isDriverAvailable("QSQLITE")) {
    if(error != nullptr)
      *error = "QSQLITE driver is not available";
    this->m_available = false;
    return false;
  }

  auto db = QSqlDatabase::addDatabase("QSQLITE", this->m_connection_name);
  db.setDatabaseName(path);
  if(! db.open()) {
    if(error != nullptr)
      *error = db.lastError().text();
    this->m_available = false;
    return false;
  }

  this->m_database_path = path;
  this->m_available = this->init_schema(error);
  return this->m_available;
}

bool SqlStorage::is_available() const { return this->m_available; }

QString SqlStorage::database_path() const { return this->m_database_path; }

bool SqlStorage::load(DeskData* data, QString* error) const
{
  if(data == nullptr) {
    if(error != nullptr)
      *error = "Internal error: empty destination";
    return false;
  }
  if(! this->m_available)
    return false;

  auto result = DeskData();
  auto db = database(this->m_connection_name);

  auto tariffs_query = QSqlQuery(db);
  if(! tariffs_query.exec(
       "SELECT id, destination, base_cents, class, discount_cents FROM tariffs"
     )) {
    if(error != nullptr)
      *error = tariffs_query.lastError().text();
    return false;
  }

  while(tariffs_query.next()) {
    result.tariffs.push_back(Tariff(
      tariffs_query.value(0).toInt(),
      tariffs_query.value(1).toString(),
      Money::from_cents(tariffs_query.value(2).toLongLong()),
      ticket_class_from_int(tariffs_query.value(3).toInt()),
      Money::from_cents(tariffs_query.value(4).toLongLong())
    ));
  }

  auto tickets_query = QSqlQuery(db);
  if(! tickets_query.exec(
       "SELECT id, passenger_name, passport, tariff_id, destination, class, price_cents, sold_at "
       "FROM tickets"
     )) {
    if(error != nullptr)
      *error = tickets_query.lastError().text();
    return false;
  }

  while(tickets_query.next()) {
    result.tickets.push_back(Ticket(
      tickets_query.value(0).toInt(),
      Passenger(tickets_query.value(1).toString(), tickets_query.value(2).toString()),
      tickets_query.value(3).toInt(),
      tickets_query.value(4).toString(),
      ticket_class_from_int(tickets_query.value(5).toInt()),
      Money::from_cents(tickets_query.value(6).toLongLong()),
      QDateTime::fromString(tickets_query.value(7).toString(), Qt::ISODate)
    ));
  }

  *data = result;
  return true;
}

bool SqlStorage::save(DeskData const& data, QString* error) const
{
  if(! this->m_available)
    return false;

  auto db = database(this->m_connection_name);
  if(! db.transaction()) {
    if(error != nullptr)
      *error = db.lastError().text();
    return false;
  }

  auto rollback = [&]() {
    db.rollback();
    return false;
  };

  if(! this->exec("DELETE FROM tickets", error))
    return rollback();
  if(! this->exec("DELETE FROM tariffs", error))
    return rollback();

  auto tariff_query = QSqlQuery(db);
  tariff_query.prepare(
    "INSERT INTO tariffs(id, destination, base_cents, class, discount_cents) VALUES(?, ?, ?, ?, ?)"
  );
  for(auto const& tariff : data.tariffs) {
    tariff_query.addBindValue(tariff.id());
    tariff_query.addBindValue(tariff.destination());
    tariff_query.addBindValue(static_cast<qlonglong>(tariff.base_price().cents()));
    tariff_query.addBindValue(static_cast<int>(tariff.ticket_class()));
    tariff_query.addBindValue(static_cast<qlonglong>(tariff.discount().cents()));
    if(! tariff_query.exec()) {
      if(error != nullptr)
        *error = tariff_query.lastError().text();
      return rollback();
    }
  }

  auto ticket_query = QSqlQuery(db);
  ticket_query.prepare(
    "INSERT INTO tickets(id, passenger_name, passport, tariff_id, destination, class, price_cents, "
    "sold_at) VALUES(?, ?, ?, ?, ?, ?, ?, ?)"
  );
  for(auto const& ticket : data.tickets) {
    ticket_query.addBindValue(ticket.id());
    ticket_query.addBindValue(ticket.passenger().name());
    ticket_query.addBindValue(ticket.passenger().passport());
    ticket_query.addBindValue(ticket.tariff_id());
    ticket_query.addBindValue(ticket.destination());
    ticket_query.addBindValue(static_cast<int>(ticket.ticket_class()));
    ticket_query.addBindValue(static_cast<qlonglong>(ticket.price().cents()));
    ticket_query.addBindValue(ticket.sold_at().toString(Qt::ISODate));
    if(! ticket_query.exec()) {
      if(error != nullptr)
        *error = ticket_query.lastError().text();
      return rollback();
    }
  }

  if(! db.commit()) {
    if(error != nullptr)
      *error = db.lastError().text();
    return false;
  }
  return true;
}

bool SqlStorage::init_schema(QString* error) const
{
  return this->exec(
           "CREATE TABLE IF NOT EXISTS tariffs("
           "id INTEGER PRIMARY KEY,"
           "destination TEXT NOT NULL,"
           "base_cents INTEGER NOT NULL,"
           "class INTEGER NOT NULL,"
           "discount_cents INTEGER NOT NULL)",
           error
         )
      && this->exec(
        "CREATE TABLE IF NOT EXISTS tickets("
        "id INTEGER PRIMARY KEY,"
        "passenger_name TEXT NOT NULL,"
        "passport TEXT NOT NULL,"
        "tariff_id INTEGER NOT NULL,"
        "destination TEXT NOT NULL,"
        "class INTEGER NOT NULL,"
        "price_cents INTEGER NOT NULL,"
        "sold_at TEXT NOT NULL)",
        error
      );
}

bool SqlStorage::exec(QString const& sql, QString* error) const
{
  auto query = QSqlQuery(database(this->m_connection_name));
  if(query.exec(sql))
    return true;

  if(error != nullptr)
    *error = query.lastError().text();
  return false;
}
