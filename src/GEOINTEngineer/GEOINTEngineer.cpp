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
#include "LocalGeospatialServer.h"
#include "LocalGeospatialTask.h"
#include "MapViewTool.h"

#include "ArcGISMapImageLayer.h"
#include "Basemap.h"
#include "Envelope.h"
#include "Feature.h"
#include "FeatureCollection.h"
#include "FeatureCollectionLayer.h"
#include "FeatureCollectionTable.h"
#include "FeatureCollectionTableListModel.h"
#include "FeatureIterator.h"
#include "FeatureQueryResult.h"
#include "Field.h"
#include "GeoprocessingFeatures.h"
#include "GeoprocessingResult.h"
#include "GeoprocessingTypes.h"
#include "LayerListModel.h"
#include "Map.h"
#include "MapQuickView.h"
#include "MapTypes.h"
#include "PolygonBuilder.h"
#include "QueryParameters.h"
#include "SimpleFillSymbol.h"
#include "SimpleLineSymbol.h"
#include "SimpleRenderer.h"
#include "SpatialReference.h"
#include "SymbolTypes.h"
#include "TaskWatcher.h"
#include "Viewpoint.h"

#include <QUrl>

#include <memory>

using namespace Esri::ArcGISRuntime;

GEOINTEngineer::GEOINTEngineer(QObject *parent /* = nullptr */):
    QObject(parent),
    m_map(new Map(BasemapStyle::OsmStandard, this)),
    m_inputFeatureLayer(new FeatureCollectionLayer(new FeatureCollection(this), this)),
    m_localGeospatialServer(new LocalGeospatialServer(this)),
    m_operationalLayerInitialized(false),
    m_polygonSketchTool(new PolygonSketchTool(this))
{
    connect(m_localGeospatialServer, &LocalGeospatialServer::mapLoaded, this, &GEOINTEngineer::onMapLoaded);
    connect(m_localGeospatialServer, &LocalGeospatialServer::mapServiceLoaded, this, &GEOINTEngineer::onMapServiceLoaded);
    connect(m_localGeospatialServer, &LocalGeospatialServer::taskLoaded, this, &GEOINTEngineer::onTaskLoaded);
    connect(m_localGeospatialServer, &LocalGeospatialServer::taskCompleted, this, &GEOINTEngineer::onTaskCompleted);

    connect(m_polygonSketchTool, &PolygonSketchTool::polygonConstructed, this, &GEOINTEngineer::onPolygonConstructed);
}

GEOINTEngineer::~GEOINTEngineer()
{
}

MapQuickView* GEOINTEngineer::mapView() const
{
    return m_mapView;
}

// Set the view (created in QML)
void GEOINTEngineer::setMapView(MapQuickView *mapView)
{
    if (!mapView || mapView == m_mapView)
    {
        return;
    }

    m_mapView = mapView;
    m_mapView->setMap(m_map);

    connect(m_mapView, &MapQuickView::mousePressed, this, &GEOINTEngineer::onMousePressed);
    connect(m_mapView, &MapQuickView::mouseMoved, this, &GEOINTEngineer::onMouseMoved);
    connect(m_mapView, &MapQuickView::mouseReleased, this, &GEOINTEngineer::onMouseReleased);

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
    connect(m_inputFeatures, &FeatureCollectionTable::queryFeaturesCompleted, this, &GEOINTEngineer::onDeleteAllInputFeatures);

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

    m_operationalLayerInitialized = true;
}

void GEOINTEngineer::deleteAllInputFeatures()
{
    if (nullptr == m_inputFeatures)
    {
        return;
    }

    m_deletePostAction = DeletePostAction::None;
    QueryParameters allFeaturesQuery;
    allFeaturesQuery.setWhereClause("1=1");
    m_inputFeatures->queryFeatures(allFeaturesQuery);
}

void GEOINTEngineer::deleteAllOutputFeatures()
{
    QList<ArcGISMapImageLayer*> outputLayers;
    for(Layer *operationalLayer : *m_map->operationalLayers())
    {
        ArcGISMapImageLayer *mapImageLayer = dynamic_cast<ArcGISMapImageLayer*>(operationalLayer);
        if (nullptr != mapImageLayer)
        {
            outputLayers.append(mapImageLayer);
        }
    }

    foreach (ArcGISMapImageLayer *outputLayer, outputLayers)
    {
        m_map->operationalLayers()->removeOne(outputLayer);
        delete outputLayer;
    }
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
    QObject *lifetimeManager = new QObject();
    while(queryResult->iterator().hasNext())
    {
        Feature* feature = queryResult->iterator().next(lifetimeManager);
        features.append(feature);
    }

    delete uniqueQueryResult.release();
    return features;
}

void GEOINTEngineer::addMapExtentAsGraphic()
{
    if (!m_operationalLayerInitialized)
    {
        // Add the operational layers
        initOperationalLayers();
    }

    Viewpoint boundingViewpoint = m_mapView->currentViewpoint(ViewpointType::BoundingGeometry);
    // TODO: Validate if 200.0 delivers a valid extent!
    //Envelope boundingBox = boundingViewpoint.targetGeometry()
    Envelope boundingBox = boundingViewpoint.targetGeometry().extent();
    PolygonBuilder *polygonBuilder = new PolygonBuilder(boundingBox.spatialReference(), this);
    polygonBuilder->addPoint(boundingBox.xMin(), boundingBox.yMin());
    polygonBuilder->addPoint(boundingBox.xMin(), boundingBox.yMax());
    polygonBuilder->addPoint(boundingBox.xMax(), boundingBox.yMax());
    polygonBuilder->addPoint(boundingBox.xMax(), boundingBox.yMin());
    m_inputPolygon = polygonBuilder->toPolygon();

    // First of all delete all input features
    // when delete all was executed, the current map extent
    // is added as a new input feature
    m_deletePostAction = DeletePostAction::AddInputFeature;
    QueryParameters allFeaturesQuery;
    allFeaturesQuery.setWhereClause("1=1");
    m_inputFeatures->queryFeatures(allFeaturesQuery);
}

void GEOINTEngineer::activatePolygonSketchTool()
{
    m_currentTool = m_polygonSketchTool;
    m_currentTool->activate(m_mapView);
}

void GEOINTEngineer::deactivateMapTool()
{
    if (nullptr != m_currentTool)
    {
        m_currentTool->deactivate();
    }

    m_currentTool = nullptr;
}

void GEOINTEngineer::executeTask(GeospatialTaskListModel *taskModel, int taskIndex)
{
    m_geospatialTaskListModel = taskModel;

    // Set the current task
    m_currentGeospatialTask = m_geospatialTaskListModel->task(taskIndex);

    if (!m_operationalLayerInitialized)
    {
        qDebug() << "Operational input layers were not initialized!";
        return;
    }

    qDebug() << "Executing " << m_currentGeospatialTask->displayName() << " using the input features...";
    GeoprocessingFeatures* mapExtentAsFeatures = new GeoprocessingFeatures(m_inputFeatures, this);
    m_localGeospatialServer->executeTask(m_currentGeospatialTask, mapExtentAsFeatures);
}

void GEOINTEngineer::executeAllTasks(GeospatialTaskListModel *taskModel)
{
    m_geospatialTaskListModel = taskModel;

    // Unset the current task
    m_currentGeospatialTask = nullptr;

    if (!m_operationalLayerInitialized)
    {
        qDebug() << "Operational input layers were not initialized!";
        return;
    }

    qDebug() << "Executing all tasks using the input features...";
    GeoprocessingFeatures* mapExtentAsFeatures = new GeoprocessingFeatures(m_inputFeatures, this);
    m_localGeospatialServer->executeTasks(mapExtentAsFeatures);
}

void GEOINTEngineer::addInputFeatures(Polygon &polygon)
{
    QVariantMap emptyAttributes;
    Feature *envelopeAsFeature = m_inputFeatures->createFeature(this);
    envelopeAsFeature->setGeometry(polygon);

    m_inputFeatures->addFeature(envelopeAsFeature);
}

void GEOINTEngineer::onDeleteAllInputFeatures(QUuid, FeatureQueryResult *queryResult)
{
    if (nullptr == queryResult)
    {
        qDebug() << "Features query result is not valid!";
        return;
    }

    QList<Feature*> featuresForDeletion = extractFeatures(queryResult);
    if (featuresForDeletion.isEmpty())
    {
        switch (m_deletePostAction)
        {
        case DeletePostAction::AddInputFeature:
            // No need for delete just add the feature
            addInputFeatures(m_inputPolygon);
            return;

        default:
            return;
        }
    }

    // "DB-Delete" all features from the table
    // Schedule release memory when the delete was completed
    // All features have the same parent
    Feature *firstFeature = featuresForDeletion[0];
    QUuid taskId = firstFeature->featureTable()->deleteFeatures(featuresForDeletion).taskId();
    m_featuresLifetimes.insert(taskId, firstFeature->parent());
}

void GEOINTEngineer::onFeaturesDeleted(QUuid taskId, bool deleted)
{
    // Release the memory for the features
    if (m_featuresLifetimes.contains(taskId))
    {
        QObject *lifetimeManager = m_featuresLifetimes.value(taskId);
        m_featuresLifetimes.remove(taskId);
        delete lifetimeManager;
        qDebug() << "Features for task " << taskId << " were released from memory.";
    }

    if (deleted)
    {
        switch (m_deletePostAction)
        {
        case DeletePostAction::AddInputFeature:
            addInputFeatures(m_inputPolygon);
            return;

        default:
            return;
        }
    }

    qDebug() << "Delete input features failed!";
}

void GEOINTEngineer::onInputFeatureAdded(QUuid, bool added)
{
    if (added)
    {
        qDebug() << "Input feature was added.";
        return;
    }

    qDebug() << "Adding input feature failed!";
}

void GEOINTEngineer::onMapLoaded(Map* map)
{
    // Replace the current focus map
    if (nullptr != m_mapView)
    {
        m_mapView->setMap(map);
        if (nullptr != m_map)
        {
            delete m_map;
        }
        m_map = map;
    }
}

void GEOINTEngineer::onMapServiceLoaded(ArcGISMapImageLayer *mapImageLayer)
{
    if (nullptr != m_map)
    {
        m_map->operationalLayers()->append(mapImageLayer);
    }
}

void GEOINTEngineer::onTaskLoaded(LocalGeospatialTask *geospatialTask)
{
    //m_geospatialTaskListModel->addTask(geospatialTask);
    emit taskLoaded(geospatialTask);
}

void GEOINTEngineer::onTaskCompleted(GeoprocessingResult *result, ArcGISMapImageLayer *mapImageLayerResult)
{
    ArcGISMapImageLayer* resultMapImageLayer = result->mapImageLayer();
    if (nullptr != resultMapImageLayer)
    {
        m_map->operationalLayers()->append(resultMapImageLayer);
        return;
    }
    if (nullptr != mapImageLayerResult)
    {
        m_map->operationalLayers()->append(mapImageLayerResult);
        return;
    }

    // Result is not drawn as a map image layer
    // We have to directly access the features
    // TODO: Specific renderer must be implemented!
    QMap<QString, GeoprocessingParameter*> outputs = result->outputs();
    foreach (GeoprocessingParameter const *outputParameter, outputs.values())
    {
        switch (outputParameter->parameterType())
        {
        case GeoprocessingParameterType::GeoprocessingFeatures:
            {
                GeoprocessingFeatures *outputFeatures = (GeoprocessingFeatures*)outputParameter;
                if (nullptr != outputFeatures)
                {
                    // Task with features as output succeeded
                    FeatureSet* outputFeatureSet = outputFeatures->features();
                    FeatureCollectionTable* newResultFeatures = new FeatureCollectionTable(outputFeatureSet, this);
                    m_outputFeatureLayer->featureCollection()->tables()->append(newResultFeatures);
                }
            }
            break;

        default:
            break;
        }
    }
}

void GEOINTEngineer::onMousePressed(QMouseEvent &mouseEvent)
{
    if (nullptr == m_currentTool)
    {
        return;
    }

    m_currentTool->mousePressed(mouseEvent);
}

void GEOINTEngineer::onMouseMoved(QMouseEvent &mouseEvent)
{
    if (nullptr == m_currentTool)
    {
        return;
    }

    m_currentTool->mouseMoved(mouseEvent);
}

void GEOINTEngineer::onMouseReleased(QMouseEvent &mouseEvent)
{
    if (nullptr == m_currentTool)
    {
        return;
    }

    m_currentTool->mouseReleased(mouseEvent);
}

void GEOINTEngineer::mousePositionChanged(qreal x, qreal y)
{
    QMouseEvent mouseEvent(QMouseEvent::Type::MouseMove, QPointF(x, y), Qt::MouseButton::NoButton, Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
    onMouseMoved(mouseEvent);
}

void GEOINTEngineer::onPolygonConstructed(Polygon &polygon)
{
    if (!m_operationalLayerInitialized)
    {
        // Add the operational layers
        initOperationalLayers();
    }

    m_inputPolygon = polygon;

    // First of all delete all input features
    // when delete all was executed, the current map extent
    // is added as a new input feature
    m_deletePostAction = DeletePostAction::AddInputFeature;
    QueryParameters allFeaturesQuery;
    allFeaturesQuery.setWhereClause("1=1");
    m_inputFeatures->queryFeatures(allFeaturesQuery);
}
