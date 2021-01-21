
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
    width: 900
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
                engineerForm.executeAllTasks(gpTaskListModel);
            }
        }
    }

    Pane {
        anchors.fill: parent
        RowLayout {
            anchors.fill: parent

            ColumnLayout {
                Layout.margins: 15

                GEOINTEngineerForm {
                    id: engineerForm
                    Layout.topMargin: 6
                    Layout.fillHeight: true
                    Layout.fillWidth: true

                    onTaskLoaded: {
                        gpTaskListModel.addTask(geospatialTask);
                    }
                }

                ListModel {
                    id: gpTaskDesignListModel
                    ListElement {
                        title: "Average Nearest Neighbor"
                        description: "Calculates a nearest neighbor index based on the average distance from each feature to its nearest neighboring feature."
                    }
                    ListElement {
                        title: "High/Low Clustering"
                        description: "Measures the degree of clustering for either high or low values using the Getis-Ord General G statistic."
                    }
                }


                GeospatialTaskListModel {
                    id: gpTaskListModel
                }
            }

            ColumnLayout {
                Layout.margins: 15
                Layout.fillWidth: false
                Layout.preferredWidth: 300

                RowLayout {
                    Layout.preferredWidth: parent.Layout.preferredWidth

                    Button {
                        Layout.alignment: Qt.AlignLeft
                        text: qsTr("<")
                        enabled: 0 < stackLayout.currentIndex
                        onClicked: stackLayout.currentIndex--
                    }

                    Button {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr(">")
                        enabled: stackLayout.currentIndex < stackLayout.count - 1
                        onClicked: stackLayout.currentIndex++
                    }
                }

                StackLayout {
                    id: stackLayout

                    Repeater {
                        id: gpTaskRepeater
                        model: gpTaskListModel

                        ColumnLayout {

                            Label {
                                id: titleLabel
                                text: model.title
                                Layout.fillWidth: true
                                horizontalAlignment: Qt.AlignHCenter
                                verticalAlignment: Qt.AlignTop
                                wrapMode: Text.Wrap
                            }

                            Label {
                                id: descriptionLabel
                                text: model.description
                                Layout.fillWidth: true
                                wrapMode: Text.WordWrap
                                font.italic: true
                            }
                        }
                    }
                }

                Button {
                    Layout.alignment: Qt.AlignRight

                    text: qsTr("Execute")
                    onClicked: {
                        engineerForm.executeTask(gpTaskListModel, gpTaskDesignListModel.index);
                    }
                }

                PageIndicator {
                    currentIndex: stackLayout.currentIndex
                    count: stackLayout.count
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
