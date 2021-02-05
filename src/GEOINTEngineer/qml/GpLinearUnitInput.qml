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

RowLayout {
    spacing: 10

    TextField {
        placeholderText: model.parameterName
        validator: DoubleValidator {
        }
    }

    ComboBox {
        model: ListModel {
            ListElement {
                text: "KM"
            }
            ListElement {
                text: "M"
            }
            ListElement {
                text: "NM"
            }
        }
    }
}
