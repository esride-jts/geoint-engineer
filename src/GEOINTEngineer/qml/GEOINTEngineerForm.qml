
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

import QtQuick 2.6
import QtQuick.Controls 2.2
import Esri.GEOINTEngineer 1.0

Item {
    id: geointForm

    function addMapExtentAsGraphic() {
        model.addMapExtentAsGraphic();
    }

    function deleteAllInputFeatures() {
        model.deleteAllInputFeatures();
    }

    function executeTask(taskModel, taskIndex) {
        model.executeTask(taskModel, taskIndex);
    }

    function executeAllTasks(taskModel) {
        model.executeAllTasks(taskModel);
    }

    signal taskLoaded(LocalGeospatialTask geospatialTask);

    // Create MapQuickView here, and create its Map etc. in C++ code
    MapView {
        id: view
        anchors.fill: parent
        // set focus to enable keyboard navigation
        focus: true
    }

    // Declare the C++ instance which creates the map etc. and supply the view
    GEOINTEngineer {
        id: model
        mapView: view

        onTaskLoaded: {
            geointForm.taskLoaded(geospatialTask);
        }
    }
}
