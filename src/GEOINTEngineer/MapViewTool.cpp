// GEOINTEngineer
// Copyright © 2021 Esri Deutschland GmbH
// Jan Tschada (j.tschada@esri.de)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Additional permission under GNU LGPL version 3 section 4 and 5
// If you modify this Program, or any covered work, by linking or combining
// it with ArcGIS Runtime for Qt (or a modified version of that library),
// containing parts covered by the terms of ArcGIS Runtime for Qt,
// the licensors of this Program grant you additional permission to convey the resulting work.
// See <https://developers.arcgis.com/qt/> for further information.


#include "MapViewTool.h"

#include "Graphic.h"
#include "GraphicListModel.h"
#include "GraphicsOverlay.h"
#include "GraphicsOverlayListModel.h"
#include "MapQuickView.h"
#include "Point.h"
#include "PolygonBuilder.h"
#include "SimpleFillSymbol.h"
#include "SimpleLineSymbol.h"
#include "SimpleMarkerSymbol.h"
#include "SimpleRenderer.h"
#include "SpatialReference.h"
#include "SymbolTypes.h"

#include <QDebug>

using namespace Esri::ArcGISRuntime;

MapViewTool::MapViewTool(QObject *parent) : QObject(parent)
{

}



PolygonSketchTool::PolygonSketchTool(QObject *parent) :
    MapViewTool(parent),
    m_polygonGraphic(new Graphic(this)),
    m_verticesRenderer(new SimpleRenderer(this)),
    m_polylineRenderer(new SimpleRenderer(this)),
    m_polygonRenderer(new SimpleRenderer(this))
{
    QColor bwBlack("#312d2a");
    const float pencilSize = 5;
    SimpleMarkerSymbol *vertexSymbol = new SimpleMarkerSymbol(SimpleMarkerSymbolStyle::Circle, bwBlack, pencilSize, this);
    m_verticesRenderer->setSymbol(vertexSymbol);

    SimpleLineSymbol *lineSymbol = new SimpleLineSymbol(SimpleLineSymbolStyle::Solid, bwBlack, pencilSize, this);
    m_polylineRenderer->setSymbol(lineSymbol);

    SimpleFillSymbol *polygonSymbol = new SimpleFillSymbol(SimpleFillSymbolStyle::DiagonalCross, bwBlack, lineSymbol, this);
    m_polygonRenderer->setSymbol(polygonSymbol);
}

void PolygonSketchTool::activate(Esri::ArcGISRuntime::MapQuickView *mapView)
{
    m_currentMapView = mapView;
    if (nullptr == m_sketchOverlay)
    {
        m_sketchOverlay = new GraphicsOverlay(this);
        m_sketchOverlay->setRenderer(m_verticesRenderer);
        m_resultOverlay = new GraphicsOverlay(this);
        m_resultOverlay->setRenderer(m_polygonRenderer);
        m_resultOverlay->graphics()->append(m_polygonGraphic);
        m_currentMapView->graphicsOverlays()->append(m_sketchOverlay);
        m_currentMapView->graphicsOverlays()->append(m_resultOverlay);
        m_polygonBuilder = new PolygonBuilder(m_currentMapView->spatialReference(), this);
    }
}

void PolygonSketchTool::deactivate()
{
    clearSketches();
}

void PolygonSketchTool::clearSketches()
{
    Polygon emptyPolygon;
    m_polygonBuilder->replaceGeometry(emptyPolygon);

    QList<Graphic*> sketchGraphics;
    for (Graphic *graphic : *m_sketchOverlay->graphics())
    {
        sketchGraphics.append(graphic);
    }
    m_sketchOverlay->graphics()->clear();
    foreach (Graphic *graphic, sketchGraphics)
    {
        delete graphic;
    }

    m_polygonGraphic->setGeometry(emptyPolygon);
}

void PolygonSketchTool::mousePressed(QMouseEvent &mouseEvent)
{
    mouseEvent.accept();

    qDebug() << "pressed";

    if (Qt::MouseButton::RightButton == mouseEvent.button())
    {
        if (2 < m_sketchOverlay->graphics()->size())
        {
            Polygon polygon = m_polygonBuilder->toPolygon();
            emit polygonConstructed(polygon);
        }

        clearSketches();
        return;
    }

    Point vertex = m_currentMapView->screenToLocation(mouseEvent.x(), mouseEvent.y());
    Graphic *vertexGraphic = new Graphic(vertex, m_verticesRenderer->symbol(), this);
    m_sketchOverlay->graphics()->append(vertexGraphic);
    m_polygonBuilder->addPoint(vertex);
}

void PolygonSketchTool::mouseMoved(QMouseEvent &mouseEvent)
{
    mouseEvent.accept();

    qDebug() << "moved";
}

void PolygonSketchTool::mouseReleased(QMouseEvent &mouseEvent)
{
    mouseEvent.accept();

    qDebug() << "released";

    if (2 < m_sketchOverlay->graphics()->size())
    {
        Polygon polygon = m_polygonBuilder->toPolygon();
        m_polygonGraphic->setGeometry(polygon);
    }
}
