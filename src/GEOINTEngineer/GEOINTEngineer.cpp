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
#include "GeoprocessingFeatures.h"
#include "Map.h"
#include "MapQuickView.h"
#include "PolygonBuilder.h"
#include "SimpleFillSymbol.h"
#include "SimpleLineSymbol.h"
#include "SimpleRenderer.h"

#include <QUrl>

using namespace Esri::ArcGISRuntime;

GEOINTEngineer::GEOINTEngineer(QObject* parent /* = nullptr */):
    QObject(parent),
    m_map(new Map(Basemap::openStreetMap(this), this)),
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

void GEOINTEngineer::executeAllTasks()
{
    // TODO: Release from memory
    m_map->operationalLayers()->clear();

    FeatureCollection* inputFeatureCollection = new FeatureCollection(this);
    FeatureCollectionLayer* inputFeatureLayer = new FeatureCollectionLayer(inputFeatureCollection, this);
    m_map->operationalLayers()->append(inputFeatureLayer);

    QList<Field> fields;
    fields.append(Field::createText("Description", "Description", 0));
    FeatureCollectionTable* inputFeatures = new FeatureCollectionTable(fields, GeometryType::Polygon, m_mapView->spatialReference(), this);
    SimpleLineSymbol* envelopeBoundarySymbol = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, QColor("cyan"), 2.0, this);
    SimpleFillSymbol* envelopeSymbol = new SimpleFillSymbol(SimpleFillSymbolStyle::DiagonalCross, QColor("cyan"), envelopeBoundarySymbol, this);
    SimpleRenderer* envelopeRenderer = new SimpleRenderer(envelopeSymbol, this);
    inputFeatures->setRenderer(envelopeRenderer);

    connect(inputFeatureLayer, &FeatureCollectionLayer::loadStatusChanged, this, [this, inputFeatures](LoadStatus loadStatus)
    {
        switch (loadStatus)
        {
        case LoadStatus::Loaded:
            {
                qDebug() << "Input features loaded.";
                Viewpoint boundingViewpoint = m_mapView->currentViewpoint(ViewpointType::BoundingGeometry);
                Envelope boundingBox = boundingViewpoint.targetGeometry();
                PolygonBuilder* polygonBuilder = new PolygonBuilder(boundingBox.spatialReference(), this);
                polygonBuilder->addPoint(boundingBox.xMin(), boundingBox.yMin());
                polygonBuilder->addPoint(boundingBox.xMin(), boundingBox.yMax());
                polygonBuilder->addPoint(boundingBox.xMax(), boundingBox.yMax());
                polygonBuilder->addPoint(boundingBox.xMax(), boundingBox.yMin());
                Polygon envelopeAsPolygon = polygonBuilder->toPolygon();
                QVariantMap emptyAttributes;
                Feature* envelopeAsFeature = inputFeatures->createFeature(this);
                envelopeAsFeature->setGeometry(envelopeAsPolygon);
                qDebug() << envelopeAsPolygon.isEmpty();
                qDebug() << envelopeAsPolygon.spatialReference().toJson();

                connect(inputFeatures, &FeatureCollectionTable::addFeatureCompleted, this, [this, inputFeatures](QUuid, bool succeeded)
                {
                    if (succeeded)
                    {
                        qDebug() << "Executing all tasks using the current map extent...";
                        GeoprocessingFeatures* mapExtentAsFeatures = new GeoprocessingFeatures(inputFeatures, this);
                        m_localGeospatialServer->executeTasks(mapExtentAsFeatures);
                        return;
                    }

                    qDebug() << "Added the current map extent as feature failed!";
                });
                inputFeatures->addFeature(envelopeAsFeature);
            }
            break;

        default:
            qDebug() << "Load status...";
            return;
        }
    });
    //inputFeatures->load();
    qDebug() << "Loading input features...";
    inputFeatureCollection->tables()->append(inputFeatures);

}

void GEOINTEngineer::onTaskCompleted(Esri::ArcGISRuntime::GeoprocessingFeatures* outputFeatures)
{
    // TODO: Release from memory
    m_map->operationalLayers()->clear();

    FeatureSet* outputFeatureSet = outputFeatures->features();
    FeatureCollectionTable* newResultFeatures = new FeatureCollectionTable(outputFeatureSet, this);
    FeatureCollection* outputFeatureCollection = new FeatureCollection(this);
    outputFeatureCollection->tables()->append(newResultFeatures);
    FeatureCollectionLayer* outputFeatureLayer = new FeatureCollectionLayer(outputFeatureCollection, this);
    m_map->operationalLayers()->append(outputFeatureLayer);
}
