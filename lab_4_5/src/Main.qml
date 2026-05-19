import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtQuick.Layouts

ApplicationWindow {
    id: root

    width: 1120
    height: 720
    minimumWidth: 900
    minimumHeight: 620
    visible: true
    title: "Airport Cash Desk"

    Material.theme: Material.Dark
    Material.accent: Material.color(Material.BlueGrey, Material.Shade500)
    Material.primary: Material.color(Material.BlueGrey, Material.Shade500)
    Material.roundedScale: Material.ExtraSmallScale

    component TariffRow : RowLayout {
        spacing: 20
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16

        property alias destText: _dest.text
        property alias classText: _class.text
        property alias priceText: _price.text
        property alias finalText: _final.text
        property bool isHeader: false

        Label { id: _dest; Layout.fillWidth: true; elide: Text.ElideRight; font.bold: isHeader }
        Label { id: _class; Layout.preferredWidth: 100; font.bold: isHeader }
        Label { id: _price; Layout.preferredWidth: 100; font.bold: isHeader }
        Label { id: _final; Layout.preferredWidth: 100; font.bold: isHeader; color: isHeader ? Material.foreground : Material.accent }
    }

    component TicketRow : RowLayout {
        spacing: 20
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16

        property alias passText: _pass.text
        property alias docText: _doc.text
        property alias destText: _dest.text
        property alias priceText: _price.text
        property bool isHeader: false

        Label { id: _pass; Layout.fillWidth: true; elide: Text.ElideRight; font.bold: isHeader }
        Label { id: _doc; Layout.preferredWidth: 120; font.bold: isHeader }
        Label { id: _dest; Layout.preferredWidth: 150; font.bold: isHeader; elide: Text.ElideRight }
        Label { id: _price; Layout.preferredWidth: 90; font.bold: isHeader }
    }

    FileDialog { id: saveDataDialog; fileMode: FileDialog.SaveFile; onAccepted: appController.exportToFile(selectedFile) }
    FileDialog { id: loadDataDialog; onAccepted: appController.importFromFile(selectedFile) }
    FileDialog { id: saveDatabaseDialog; fileMode: FileDialog.SaveFile; onAccepted: appController.saveDatabaseToFile(selectedFile) }
    FileDialog { id: loadDatabaseDialog; onAccepted: appController.loadFileToDatabase(selectedFile) }

    function fillTariffForm(row) {
        const item = tariffModel.get(row)
        if (!item) return
        destinationField.text = item.destination || ""
        priceField.text = Number(item.priceRubles || 0).toFixed(2)
        discountField.text = Number(item.discountRubles || 0).toFixed(2)
        classBox.currentIndex = item.classIndex || 0
    }

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            spacing: 12

            Label {
                text: "Airport Manager"
                font {
                    pixelSize: 18
                    letterSpacing: 1.2
                    bold: true
                    capitalization: Font.AllUppercase
                }
            }

            Label {
                text: appController.modeText
                Material.foreground: appController.databaseMode ? Material.Lime : Material.Amber
                font {
                    pixelSize: 12
                    bold: true
                    capitalization: Font.AllUppercase
                }
                Layout.leftMargin: 10
            }

            Item { Layout.fillWidth: true }

            ToolButton {
                text: "Report"
                flat: true
                onClicked: appController.openReportWindow()
            }

            ToolButton {
                text: "About"
                flat: true
                onClicked: appController.openAboutWindow()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        RowLayout {
            spacing: 16
            Pane {
                Layout.fillWidth: true
                RowLayout {
                    anchors.fill: parent; anchors.margins: 8; spacing: 25
                    Label {
                        text: "Total Revenue";
                        font {
                            pixelSize: 11
                            bold: true
                            capitalization: Font.AllUppercase
                        }
                        opacity: 0.6;
                    }

                    Label {
                        text: appController.totalRevenueText
                        font {
                            pixelSize: 22
                            bold: true
                            capitalization: Font.AllUppercase
                        }
                        Material.foreground: Material.Green
                    }

                    Rectangle { width: 1; Layout.fillHeight: true; color: Material.dividerColor; opacity: 0.3 }

                    Label {
                        text: "Max Tariff"
                        font {
                            pixelSize: 11
                            bold: true
                            capitalization: Font.AllUppercase
                        }
                        opacity: 0.6
                    }

                    Label {
                        text: appController.maxTariffText
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        font.pixelSize: 16
                    }
                }
            }
        }

        TabBar {
            id: tabs
            Layout.fillWidth: true
            TabButton { text: "Tariffs" }
            TabButton { text: "Tickets" }
            TabButton { text: "System" }
        }

        StackLayout {
            currentIndex: tabs.currentIndex
            Layout.fillWidth: true
            Layout.fillHeight: true

            RowLayout {
                spacing: 16
                Pane {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    padding: 0
                    ColumnLayout {
                        anchors.fill: parent; spacing: 0

                        RowLayout {
                            Layout.margins: 12
                            Label { text: "Price Catalog"; font.bold: true; Layout.fillWidth: true }
                            Button { text: "Sort Destination"; flat: true; onClicked: appController.sortTariffsByDestination() }
                            Button { text: "Sort Price"; flat: true; onClicked: appController.sortTariffsByPrice() }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 35
                            color: Material.primary
                            opacity: .4

                            TariffRow { isHeader: true; destText: "Destination"; classText: "Class"; priceText: "Base Price"; finalText: "Net Price" }
                        }

                        ListView {
                            id: tariffList
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            model: tariffModel
                            clip: true
                            spacing: 1
                            currentIndex: -1
                            onCurrentIndexChanged: if(currentIndex >= 0) root.fillTariffForm(currentIndex)

                            delegate: ItemDelegate {
                                width: ListView.view.width; height: 45
                                highlighted: ListView.isCurrentItem
                                onClicked: tariffList.currentIndex = index
                                contentItem: TariffRow {
                                    destText: model.destination
                                    classText: model.className
                                    priceText: model.priceText
                                    finalText: model.effectiveText
                                }
                            }
                        }
                    }
                }

                Pane {
                    Layout.preferredWidth: 320; Layout.fillHeight: true
                    ColumnLayout {
                        anchors.fill: parent; spacing: 12
                        Label { text: "Tariff Editor"; font.bold: true; font.pixelSize: 16 }
                        TextField { id: destinationField; placeholderText: "Destination"; Layout.fillWidth: true }
                        ComboBox { id: classBox; model: ["Economy", "Business", "First"]; Layout.fillWidth: true }
                        TextField { id: priceField; placeholderText: "Price"; Layout.fillWidth: true }
                        TextField { id: discountField; placeholderText: "Discount"; Layout.fillWidth: true }

                        Item { height: 10 }
                        Button { text: "Add New Tariff"; Layout.fillWidth: true; highlighted: true; onClicked: appController.addTariff(destinationField.text, priceField.text, discountField.text, classBox.currentIndex) }
                        Button { text: "Save Changes"; Layout.fillWidth: true; onClicked: appController.updateTariff(tariffList.currentIndex, destinationField.text, priceField.text, discountField.text, classBox.currentIndex) }
                        Button { text: "Delete"; Layout.fillWidth: true; Material.foreground: Material.Red; onClicked: appController.removeTariff(tariffList.currentIndex) }
                        Item { Layout.fillHeight: true }
                    }
                }
            }

            RowLayout {
                spacing: 16
                Pane {
                    Layout.fillWidth: true; Layout.fillHeight: true; padding: 0
                    ColumnLayout {
                        anchors.fill: parent; spacing: 0
                        RowLayout {
                            Layout.margins: 12
                            Label { text: "Sales Ledger"; font.bold: true; Layout.fillWidth: true }
                            Button { text: "Sort Passenger"; flat: true; onClicked: appController.sortTicketsByPassenger() }
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            height: 35
                            color: Material.primary
                            opacity: .4

                            TicketRow { isHeader: true; passText: "Passenger"; docText: "Passport"; destText: "To"; priceText: "Paid" }
                        }
                        ListView {
                            id: ticketList
                            Layout.fillWidth: true; Layout.fillHeight: true
                            model: ticketModel; clip: true; spacing: 1
                            delegate: ItemDelegate {
                                width: ListView.view.width; height: 45
                                highlighted: ListView.isCurrentItem
                                onClicked: ticketList.currentIndex = index
                                contentItem: TicketRow {
                                    passText: model.passengerName
                                    docText: model.passport
                                    destText: model.destination
                                    priceText: model.priceText
                                }
                            }
                        }
                    }
                }
                Pane {
                    Layout.preferredWidth: 320; Layout.fillHeight: true
                    ColumnLayout {
                        anchors.fill: parent; spacing: 12
                        Label { text: "New Sale"; font.bold: true; font.pixelSize: 16 }
                        ComboBox { id: ticketTariffBox; model: tariffModel; textRole: "destination"; Layout.fillWidth: true }
                        TextField { id: passengerField; placeholderText: "Passenger Name"; Layout.fillWidth: true }
                        TextField { id: passportField; placeholderText: "Passport ID"; Layout.fillWidth: true }
                        Item { height: 10 }
                        Button { text: "Confirm"; Layout.fillWidth: true; highlighted: true; onClicked: appController.buyTicket(ticketTariffBox.currentIndex, passengerField.text, passportField.text) }
                        Button { text: "Void"; Layout.fillWidth: true; Material.foreground: Material.Red; onClicked: appController.removeTicket(ticketList.currentIndex) }
                        Item { Layout.fillHeight: true }
                    }
                }
            }

            Pane {
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 30; spacing: 20
                    Label { text: "Database & File sync"; font.pixelSize: 22; font.bold: true }

                    GridLayout {
                        columns: 2; columnSpacing: 20; rowSpacing: 20
                        Button { text: "Save Memory to File"; Layout.fillWidth: true; onClicked: saveDataDialog.open() }
                        Button { text: "Load File to Memory"; Layout.fillWidth: true; onClicked: loadDataDialog.open() }
                        Button { text: "Export SQLite to File"; Layout.fillWidth: true; onClicked: saveDatabaseDialog.open() }
                        Button { text: "Import File to SQLite"; Layout.fillWidth: true; onClicked: loadDatabaseDialog.open() }
                    }

                    Item { Layout.fillHeight: true }
                }
            }
        }
    }
}
