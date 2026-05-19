#include <qapplication.h>
#include <qqmlapplicationengine.h>
#include <qqmlcontext.h>
#include <qquickstyle.h>

#include "appcontroller.h"
#include "models.h"

int main(int argc, char* argv[])
{
  auto app = QApplication(argc, argv);
  QApplication::setOrganizationDomain("ru.spbstu.edu");
  QApplication::setOrganizationName("Dmitry Ryazantsev");

  QQuickStyle::setStyle("Material");

  auto tariff_model = TariffListModel();
  auto ticket_model = TicketListModel();
  auto controller = AppController(&tariff_model, &ticket_model);

  auto engine = QQmlApplicationEngine();
  engine.rootContext()->setContextProperty("appController", &controller);
  engine.rootContext()->setContextProperty("tariffModel", &tariff_model);
  engine.rootContext()->setContextProperty("ticketModel", &ticket_model);
  engine.load(QUrl("qrc:/Main.qml"));

  if(engine.rootObjects().isEmpty())
    return -1;

  return app.exec();
}
