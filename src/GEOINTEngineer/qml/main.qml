
// Copyright 2019 ESRI
//
// All rights reserved under the copyright laws of the United States
// and applicable international laws, treaties, and conventions.
//
// You may freely redistribute and use this sample code, with or
// without modification, provided you include the original copyright
// notice and use restrictions.
//
// See the Sample code usage restrictions document for further information.
//

import QtQuick 2.3
import QtQuick.Controls 2.3
import QtQuick.Controls.Material 2.3
import QtQuick.Layouts 1.3
import Esri.GEOINTEngineer 1.0

ApplicationWindow {
    visible: true
    width: 800
    height: 600

    Material.theme: Material.Dark
    Material.accent: "#a7ad6d"      // BW Hellgrün
    //Material.accent: "#616847"      // BW Helloliv
    Material.background: "#312d2a"  // BW Schwarz
    Material.foreground: "#d3c2a6"  // BW Beige
    Material.primary: "#434a39"     // BW Dunkelgrün

    menuBar: MenuBar {
        MenuItem {
            text: qsTr("Close")
        }
    }

    header: ToolBar {
        Button {
            text: qsTr("Execute")

            onClicked: {
                engineerForm.executeAllTasks();
            }
        }
    }

    Pane {
        anchors.fill: parent
        RowLayout {
            anchors.fill: parent

            ColumnLayout {

                GEOINTEngineerForm {
                    id: engineerForm
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }
            }

            ColumnLayout {
                Layout.fillWidth: false
                Layout.preferredWidth: 300

                StackView {
                    id: stack
                    initialItem: gpTool1
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }

                Component {
                    id: gpTool1

                    ColumnLayout {
                        Button {
                            Layout.alignment: Qt.AlignBottom | Qt.AlignRight
                            text: qsTr("Execute")
                        }
                    }
                }
                Component {
                    id: gpTool2
                    Button {
                        text: qsTr("Execute")
                    }
                }
                Component {
                    id: gpTool3s
                    Button {
                        text: qsTr("Execute")
                    }
                }

            }
        }
    }

    footer: TabBar {
        Button {
            text: qsTr("Status")
        }
    }
}
