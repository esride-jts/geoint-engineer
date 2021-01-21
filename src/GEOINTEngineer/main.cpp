
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

#include "GEOINTEngineer.h"
#include "GeospatialTaskListModel.h"
#include "LocalGeospatialTask.h"

#include "ArcGISRuntimeEnvironment.h"
#include "MapQuickView.h"

#include <QDir>
#include <QGuiApplication>
#include <QProcessEnvironment>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

//------------------------------------------------------------------------------

using namespace Esri::ArcGISRuntime;

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    // Before initializing ArcGIS Runtime, first set the
    // ArcGIS Runtime license setting required for your application.
    //ArcGISRuntimeEnvironment::setLicense("Place license string in here");
    QString licenseKeyName = "arcgisruntime.license.key";
    QProcessEnvironment systemEnvironment = QProcessEnvironment::systemEnvironment();
    if (systemEnvironment.contains(licenseKeyName))
    {
        QString licenseKeyValue = systemEnvironment.value(licenseKeyName);
        //ArcGISRuntimeEnvironment::setLicense(licenseKeyValue);
    }

    // Register the map view for QML
    qmlRegisterType<MapQuickView>("Esri.GEOINTEngineer", 1, 0, "MapView");

    // Register the GEOINTEngineer (QQuickItem) for QML
    qmlRegisterType<GEOINTEngineer>("Esri.GEOINTEngineer", 1, 0, "GEOINTEngineer");
    qmlRegisterType<GeospatialTaskListModel>("Esri.GEOINTEngineer", 1, 0, "GeospatialTaskListModel");
    qmlRegisterUncreatableType<LocalGeospatialTask>("Esri.GEOINTEngineer", 1, 0, "LocalGeospatialTask", "Represents a local geospatial task.");

    // Activate the styling
    QQuickStyle::setStyle("Material");

    // Initialize application view
    QQmlApplicationEngine engine;

    // Add the import Path
    engine.addImportPath(QDir(QCoreApplication::applicationDirPath()).filePath("qml"));

#ifdef ARCGIS_TOOLKIT_IMPORT_PATH_2
    engine.addImportPath(ARCGIS_TOOLKIT_IMPORT_PATH_2);
#endif

    // Set the source
    engine.load(QUrl("qrc:/qml/main.qml"));

    return app.exec();
}

//------------------------------------------------------------------------------
