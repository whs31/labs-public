#pragma once

#include <qcolor.h>
#include <qpointer.h>
#include <qquickview.h>
#include <QString>

class VisualMode
{
 public:
  virtual ~VisualMode();

  [[nodiscard]] virtual QColor accent_color() const = 0;
  [[nodiscard]] virtual QString mode_name() const = 0;
};

class InfoWindow final : public QQuickView,
                         public VisualMode
{
  Q_OBJECT

 public:
  explicit InfoWindow(QString title, QWindow* parent = nullptr);
  InfoWindow(
    QString title,
    QString message,
    QString details,
    QColor accent,
    QWindow* parent = nullptr
  );
  ~InfoWindow() override;

  [[nodiscard]] QColor accent_color() const override;
  [[nodiscard]] QString mode_name() const override;

 private:
  void load(QString const& title, QString const& message, QString const& details, bool compact);

  QColor m_accent;
  QString m_mode_name;
};
