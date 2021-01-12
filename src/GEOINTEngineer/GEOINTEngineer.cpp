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
#include "LocalGeospatialServer.h"

#include "Basemap.h"
#include "FeatureCollectionLayer.h"
#include "FeatureCollectionTable.h"
#include "FeatureQueryResult.h"
#include "GeoprocessingFeatures.h"
#include "Map.h"
#include "MapQuickView.h"
#include "PolygonBuilder.h"
#include "SimpleFillSymbol.h"
#include "SimpleLineSymbol.h"
#include "SimpleRenderer.h"

#include <QUrl>

#include <memory>

using namespace Esri::ArcGISRuntime;

GEOINTEngineer::GEOINTEngineer(QObject* parent /* = nullptr */):
    QObject(parent),
    m_map(new Map(Basemap::openStreetMap(this), this)),
    m_inputFeatureLayer(new FeatureCollectionLayer(new FeatureCollection(this), this)),
    m_localGeospatialServer(new LocalGeospatialServer(this))
{
    connect(m_localGeospatialServer, &LocalGeospatialServer::taskCompleted, this, &GEOINTEngineer::onTaskCompleted);
}

GEOINTEngineer::~GEOINTEngineer()
{
}

MapQuickView* GEOINTEngineer::mapView() const
{
    return m_mapView;
}

// Set the view (created in QML)
void GEOINTEngineer::setMapView(MapQuickView* mapView)
{
    if (!mapView || mapView == m_mapView)
    {
        return;
    }

    m_mapView = mapView;
    m_mapView->setMap(m_map);

    // Add the operational layers
    initOperationalLayers();

    emit mapViewChanged();

    // Start the local server instance
    switch (m_localGeospatialServer->start())
    {
    case LocalGeospatialServer::Status::Failed:
        qDebug() << "Local geospatial server could not be started!";
        break;

    default:
        break;
    }
}

void GEOINTEngineer::initOperationalLayers()
{
    // Input feature layer
    QList<Field> fields;
    fields.append(Field::createText("Description", "Description", 0));
    m_inputFeatures = new FeatureCollectionTable(fields, GeometryType::Polygon, m_mapView->spatialReference(), this);
    connect(m_inputFeatures, &FeatureCollectionTable::addFeatureCompleted, this, &GEOINTEngineer::onInputFeatureAdded);
    connect(m_inputFeatures, &FeatureCollectionTable::deleteFeaturesCompleted, this, &GEOINTEngineer::onFeaturesDeleted);
    connect(m_inputFeatures, &FeatureCollectionTable::queryFeaturesCompleted, this, &GEOINTEngineer::onQueryFeaturesCompleted);

    SimpleLineSymbol* envelopeBoundarySymbol = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor("cyan"), 2.0, this);
    SimpleFillSymbol* envelopeSymbol = new SimpleFillSymbol(SimpleFillSymbolStyle::DiagonalCross, QColor("cyan"), envelopeBoundarySymbol, this);
    SimpleRenderer* envelopeRenderer = new SimpleRenderer(envelopeSymbol, this);
    m_inputFeatures->setRenderer(envelopeRenderer);
    m_inputFeatureLayer->featureCollection()->tables()->append(m_inputFeatures);
    m_map->operationalLayers()->append(m_inputFeatureLayer);

    // Output feature layer
    FeatureCollection* outputFeatureCollection = new FeatureCollection(this);
    m_outputFeatureLayer = new FeatureCollectionLayer(outputFeatureCollection, this);
    m_map->operationalLayers()->append(m_outputFeatureLayer);
}

void GEOINTEngineer::deleteAllInputFeatures()
{
    QueryParameters allFeaturesQuery;
    allFeaturesQuery.setWhereClause("1=1");
    m_inputFeatures->queryFeatures(allFeaturesQuery);
}

void GEOINTEngineer::deleteAllOutputFeatures()
{

}

void GEOINTEngineer::deleteAllFeatures()
{

}

QList<Feature*> GEOINTEngineer::extractFeatures(FeatureQueryResult *queryResult)
{
    QList<Feature*> features;
    auto uniqueQueryResult = std::unique_ptr<FeatureQueryResult>(queryResult);
    auto iterator = queryResult->iterator();
    if (!iterator.hasNext())
    {
        qDebug() << "No features returned!";
        return features;
    }

    // iterate over the result object
    while(queryResult->iterator().hasNext())
    {
        Feature* feature = queryResult->iterator().next(this);
        features.append(feature);
    }

    return features;
}

void GEOINTEngineer::executeAllTasks()
{
    // First of all delete all input features
    // when delete all was executed, the current map extent
    // is added as a new input feature
    deleteAllInputFeatures();
}

void GEOINTEngineer::executeTasksUsingCurrentExtent()
{
    Viewpoint boundingViewpoint = m_mapView->currentViewpoint(ViewpointType::BoundingGeometry);
    Envelope boundingBox = boundingViewpoint.targetGeometry();
    PolygonBuilder *polygonBuilder = new PolygonBuilder(boundingBox.spatialReference(), this);
    polygonBuilder->addPoint(boundingBox.xMin(), boundingBox.yMin());
    polygonBuilder->addPoint(boundingBox.xMin(), boundingBox.yMax());
    polygonBuilder->addPoint(boundingBox.xMax(), boundingBox.yMax());
    polygonBuilder->addPoint(boundingBox.xMax(), boundingBox.yMin());
    Polygon envelopeAsPolygon = polygonBuilder->toPolygon();
    QVariantMap emptyAttributes;
    Feature *envelopeAsFeature = m_inputFeatures->createFeature(this);
    envelopeAsFeature->setGeometry(envelopeAsPolygon);

    m_inputFeatures->addFeature(envelopeAsFeature);
}

void GEOINTEngineer::onQueryFeaturesCompleted(QUuid, FeatureQueryResult *queryResult)
{
    if (nullptr == queryResult)
    {
        qDebug() << "Features query result is not valid!";
        return;
    }

    QList<Feature*> featuresForDeletion = extractFeatures(queryResult);
    if (featuresForDeletion.isEmpty())
    {
        executeTasksUsingCurrentExtent();
        return;
    }

    Feature *firstFeature = featuresForDeletion[0];
    firstFeature->featureTable()->deleteFeatures(featuresForDeletion);
}

void GEOINTEngineer::onFeaturesDeleted(QUuid, bool deleted)
{
    if (deleted)
    {
        executeTasksUsingCurrentExtent();
    }
}

void GEOINTEngineer::onInputFeatureAdded(QUuid, bool added)
{
    if (added)
    {
        qDebug() << "Executing all tasks using the current map extent...";
        GeoprocessingFeatures* mapExtentAsFeatures = new GeoprocessingFeatures(m_inputFeatures, this);
        m_localGeospatialServer->executeTasks(mapExtentAsFeatures);
        return;
    }

    qDebug() << "Added the current map extent as feature failed!";
}

void GEOINTEngineer::onTaskCompleted(Esri::ArcGISRuntime::GeoprocessingFeatures* outputFeatures)
{
    FeatureSet* outputFeatureSet = outputFeatures->features();
    FeatureCollectionTable* newResultFeatures = new FeatureCollectionTable(outputFeatureSet, this);
    m_outputFeatureLayer->featureCollection()->tables()->append(newResultFeatures);
}
