#pragma once

#include <qsqldatabase.h>
#include <qdatastream.h>
#include <qstring.h>
#include <qvector.h>

#include "domain.h"

struct DeskData
{
  QVector<Tariff> tariffs;
  QVector<Ticket> tickets;
};

QDataStream& operator<<(QDataStream& out, Tariff const& tariff);
QDataStream& operator>>(QDataStream& in, Tariff& tariff);
QDataStream& operator<<(QDataStream& out, Ticket const& ticket);
QDataStream& operator>>(QDataStream& in, Ticket& ticket);

class BinaryStorage
{
 public:
  [[nodiscard]] bool save(QString const& path, DeskData const& data, QString* error) const;
  [[nodiscard]] bool load(QString const& path, DeskData* data, QString* error) const;
};

class SqlStorage
{
 public:
  SqlStorage();
  ~SqlStorage();

  [[nodiscard]] bool open(QString const& path, QString* error);
  [[nodiscard]] bool is_available() const;
  [[nodiscard]] QString database_path() const;
  [[nodiscard]] bool load(DeskData* data, QString* error) const;
  [[nodiscard]] bool save(DeskData const& data, QString* error) const;

 private:
  [[nodiscard]] bool init_schema(QString* error) const;
  [[nodiscard]] bool exec(QString const& sql, QString* error) const;

  QString m_connection_name;
  QString m_database_path;
  bool m_available;
};
