#include "infowindow.h"

#include <qqmlcontext.h>

VisualMode::~VisualMode() = default;

InfoWindow::InfoWindow(QString title, QWindow* const parent)
  : QQuickView(parent)
  , m_accent("#3b82f6")
  , m_mode_name("compact")
{
  this->load(
    title,
    "Airport dummy manager",
    "Made by Dmitry Ryazantsev",
    true
  );
}

InfoWindow::InfoWindow(
  QString title,
  QString message,
  QString details,
  QColor accent,
  QWindow* const parent
)
  : QQuickView(parent)
  , m_accent(std::move(accent))
  , m_mode_name("report")
{
  this->load(title, message, details, false);
}

InfoWindow::~InfoWindow() = default;

QColor InfoWindow::accent_color() const { return this->m_accent; }

QString InfoWindow::mode_name() const { return this->m_mode_name; }

void InfoWindow::load(
  QString const& title,
  QString const& message,
  QString const& details,
  bool const compact
)
{
  this->rootContext()->setContextProperty("windowTitle", title);
  this->rootContext()->setContextProperty("message", message);
  this->rootContext()->setContextProperty("details", details);
  this->rootContext()->setContextProperty("accentColor", this->accent_color());
  this->rootContext()->setContextProperty("modeName", this->mode_name());
  this->rootContext()->setContextProperty("compactMode", compact);

  this->setTitle(title);
  this->setColor(QColor("#f8fafc"));
  this->setResizeMode(QQuickView::SizeRootObjectToView);
  this->setSource(QUrl("qrc:/InfoWindow.qml"));
  this->resize(compact ? QSize(420, 260) : QSize(540, 360));
}
