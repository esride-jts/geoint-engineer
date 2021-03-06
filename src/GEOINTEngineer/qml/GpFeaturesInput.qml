import QtQuick 2.3
import QtQuick.Controls 2.3
import QtQuick.Controls.Material 2.3
import QtQuick.Layouts 1.3

/*
  enum class GeoprocessingParameterType
  {
    GeoprocessingBoolean = 0,
    GeoprocessingDataFile = 1,
    GeoprocessingDate = 2,
    GeoprocessingDouble = 3,
    GeoprocessingFeatures = 4,
    GeoprocessingLinearUnit = 5,
    GeoprocessingLong = 6,
    GeoprocessingMultiValue = 7,
    GeoprocessingRaster = 8,
    GeoprocessingString = 9,
    GeoprocessingUnknownParameter = 10
  };
 */

Item {
    implicitHeight: childrenRect.height

    signal addMapExtentGraphic();
    signal activatePolygonSketchTool();
    signal deactivatePolygonSketchTool();

    ButtonGroup {
        id: mapToolGroup
    }

    Row {
        spacing: 10

        Button {
            id: mapExtentButton
            icon.name: "map"
            icon.source: "qrc:/Resources/map.svg"

            onClicked: {
                addMapExtentGraphic();
            }
        }

        Button {
            icon.name: "map"
            icon.source: "qrc:/Resources/pentagon-outline.svg"
            checkable: true
            //ButtonGroup.group: mapToolGroup
            checked: gpTaskParameterModel.polygonSketchToolActivated

            onCheckedChanged: {
                if (checked) {
                    activatePolygonSketchTool();
                } else {
                    deactivatePolygonSketchTool();
                }
            }
        }
    }
}
