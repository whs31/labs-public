#pragma once

#include <memory>

#include <qobject.h>
#include <qpointer.h>
#include <qstring.h>
#include <qvector.h>

#include "domain.h"
#include "infowindow.h"
#include "models.h"
#include "storage.h"

class AppController final : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString statusMessage READ status_message NOTIFY statusMessageChanged)
  Q_PROPERTY(QString modeText READ mode_text NOTIFY modeChanged)
  Q_PROPERTY(bool databaseMode READ database_mode NOTIFY modeChanged)
  Q_PROPERTY(QString totalRevenueText READ total_revenue_text NOTIFY totalsChanged)
  Q_PROPERTY(QString maxTariffText READ max_tariff_text NOTIFY totalsChanged)

 public:
  AppController(
    TariffListModel* tariff_model,
    TicketListModel* ticket_model,
    QObject* parent = nullptr
  );
  ~AppController() override;

  [[nodiscard]] QString status_message() const;
  [[nodiscard]] QString mode_text() const;
  [[nodiscard]] bool database_mode() const;
  [[nodiscard]] QString total_revenue_text() const;
  [[nodiscard]] QString max_tariff_text() const;

  Q_INVOKABLE bool addTariff(QString destination, QString price, QString discount, int class_index);
  Q_INVOKABLE bool updateTariff(
    int index,
    QString destination,
    QString price,
    QString discount,
    int class_index
  );
  Q_INVOKABLE bool removeTariff(int index);
  Q_INVOKABLE bool buyTicket(int tariff_index, QString passenger_name, QString passport);
  Q_INVOKABLE bool removeTicket(int index);
  Q_INVOKABLE void sortTariffsByDestination();
  Q_INVOKABLE void sortTariffsByPrice();
  Q_INVOKABLE void sortTicketsByPassenger();
  Q_INVOKABLE bool exportToFile(QString path);
  Q_INVOKABLE bool importFromFile(QString path);
  Q_INVOKABLE bool saveDatabaseToFile(QString path);
  Q_INVOKABLE bool loadFileToDatabase(QString path);
  Q_INVOKABLE void openAboutWindow();
  Q_INVOKABLE void openReportWindow();

 signals:
  void statusMessageChanged();
  void modeChanged();
  void totalsChanged();

 private:
  [[nodiscard]] DeskData current_data() const;
  [[nodiscard]] QString normalize_path(QString const& value) const;
  [[nodiscard]] bool parse_money(QString const& text, Money* result, QString const& field_name);
  [[nodiscard]] bool validate_tariff(
    QString const& destination,
    Money price,
    Money discount,
    QString* error
  ) const;
  [[nodiscard]] bool validate_passenger(
    QString const& passenger_name,
    QString const& passport,
    QString* error
  ) const;

  void seed_if_empty();
  void refresh_models();
  void sync_database();
  void set_status(QString message);

  TariffListModel* m_tariff_model;
  TicketListModel* m_ticket_model;
  CashDesk m_cash_desk;
  BinaryStorage m_binary_storage;
  SqlStorage m_sql_storage;
  QString m_status_message;
  bool m_database_mode;
  QVector<QPointer<InfoWindow>> m_windows;
};
