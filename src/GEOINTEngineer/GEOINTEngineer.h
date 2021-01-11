
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

#ifndef GEOINTENGINEER_H
#define GEOINTENGINEER_H

class LocalGeospatialServer;

namespace Esri
{
namespace ArcGISRuntime
{
class GeoprocessingFeatures;
class Map;
class MapQuickView;
}
}

#include <QObject>

class GEOINTEngineer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Esri::ArcGISRuntime::MapQuickView* mapView READ mapView WRITE setMapView NOTIFY mapViewChanged)

public:
    explicit GEOINTEngineer(QObject* parent = nullptr);
    ~GEOINTEngineer() override;

    Q_INVOKABLE void executeAllTasks();

signals:
    void mapViewChanged();

private slots:
    void onTaskCompleted(Esri::ArcGISRuntime::GeoprocessingFeatures* outputFeatures);

private:
    Esri::ArcGISRuntime::MapQuickView* mapView() const;
    void setMapView(Esri::ArcGISRuntime::MapQuickView* mapView);

    Esri::ArcGISRuntime::Map* m_map = nullptr;
    Esri::ArcGISRuntime::MapQuickView* m_mapView = nullptr;
    LocalGeospatialServer* m_localGeospatialServer = nullptr;
};

#endif // GEOINTENGINEER_H
