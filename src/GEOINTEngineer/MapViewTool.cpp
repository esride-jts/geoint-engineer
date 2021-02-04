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

#include "GraphicsOverlay.h"
#include "MapQuickView.h"
#include "PolygonBuilder.h"

#include <QDebug>

using namespace Esri::ArcGISRuntime;

MapViewTool::MapViewTool(QObject *parent) : QObject(parent)
{

}

void MapViewTool::setMapView(Esri::ArcGISRuntime::MapQuickView *mapView)
{
    m_currentMapView = mapView;
    if (nullptr == m_sketchOverlay)
    {
        m_sketchOverlay = new GraphicsOverlay(this);
        m_currentMapView->graphicsOverlays()->append(m_sketchOverlay);
    }
}



PolygonSketchTool::PolygonSketchTool(QObject *parent) : MapViewTool(parent)
{

}

void PolygonSketchTool::mousePressed(QMouseEvent &mouseEvent)
{
    mouseEvent.accept();

    qDebug() << "pressed";
    if (nullptr == m_polygonBuilder)
    {
        m_polygonBuilder = new PolygonBuilder(m_currentMapView->spatialReference(), this);
    }

    m_polygonBuilder->addPoint(m_currentMapView->screenToLocation(mouseEvent.x(), mouseEvent.y()));
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
}
