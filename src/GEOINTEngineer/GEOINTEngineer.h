
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

class GeospatialTaskListModel;
class LocalGeospatialServer;
class LocalGeospatialTask;

namespace Esri
{
namespace ArcGISRuntime
{
class ArcGISMapImageLayer;
class Feature;
class FeatureCollectionLayer;
class FeatureCollectionTable;
class FeatureQueryResult;
class GeoprocessingFeatures;
class GeoprocessingResult;
class Map;
class MapQuickView;
}
}

#include <QMap>
#include <QObject>
#include <QUuid>

class GEOINTEngineer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Esri::ArcGISRuntime::MapQuickView* mapView READ mapView WRITE setMapView NOTIFY mapViewChanged)

public:
    explicit GEOINTEngineer(QObject *parent = nullptr);
    ~GEOINTEngineer() override;

    Q_INVOKABLE void addMapExtentAsGraphic();
    Q_INVOKABLE void deleteAllInputFeatures();
    Q_INVOKABLE void deleteAllOutputFeatures();
    Q_INVOKABLE void deleteAllFeatures();
    Q_INVOKABLE void executeTask(GeospatialTaskListModel *taskModel, int taskIndex);
    Q_INVOKABLE void executeAllTasks(GeospatialTaskListModel *taskModel);

signals:
    void mapViewChanged();
    void taskLoaded(LocalGeospatialTask *geospatialTask);

private slots:
    void onQueryFeaturesCompleted(QUuid, Esri::ArcGISRuntime::FeatureQueryResult *queryResult);
    void onFeaturesDeleted(QUuid, bool);
    void onInputFeatureAdded(QUuid, bool);
    void onMapLoaded(Esri::ArcGISRuntime::Map *map);
    void onMapServiceLoaded(Esri::ArcGISRuntime::ArcGISMapImageLayer *mapImageLayer);
    void onTaskLoaded(LocalGeospatialTask *geospatialTask);
    void onTaskCompleted(Esri::ArcGISRuntime::GeoprocessingResult *result, Esri::ArcGISRuntime::ArcGISMapImageLayer *mapImageLayerResult);

private:
    void addInputFeaturesUsingCurrentExtent();
    void initOperationalLayers();
    QList<Esri::ArcGISRuntime::Feature*> extractFeatures(Esri::ArcGISRuntime::FeatureQueryResult *queryResult);

    Esri::ArcGISRuntime::MapQuickView* mapView() const;
    void setMapView(Esri::ArcGISRuntime::MapQuickView *mapView);

    Esri::ArcGISRuntime::Map* m_map = nullptr;
    Esri::ArcGISRuntime::MapQuickView* m_mapView = nullptr;
    Esri::ArcGISRuntime::FeatureCollectionLayer* m_inputFeatureLayer = nullptr;
    Esri::ArcGISRuntime::FeatureCollectionTable* m_inputFeatures = nullptr;
    Esri::ArcGISRuntime::FeatureCollectionLayer* m_outputFeatureLayer = nullptr;
    Esri::ArcGISRuntime::FeatureCollectionTable* m_ouputFeatures = nullptr;
    QMap<QUuid, QObject*> m_featuresLifetimes;

    LocalGeospatialServer* m_localGeospatialServer = nullptr;
    GeospatialTaskListModel* m_geospatialTaskListModel = nullptr;
    LocalGeospatialTask* m_currentGeospatialTask = nullptr;

    bool m_operationalLayerInitialized;
};

#endif // GEOINTENGINEER_H
