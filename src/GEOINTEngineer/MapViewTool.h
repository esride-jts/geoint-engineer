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


#ifndef MAPVIEWTOOL_H
#define MAPVIEWTOOL_H

namespace Esri
{
namespace ArcGISRuntime
{
class GeometryBuilder;
class GraphicsOverlay;
class MapQuickView;
class PolygonBuilder;
}
}

#include <QMouseEvent>
#include <QObject>

class MapViewTool : public QObject
{
    Q_OBJECT
public:
    explicit MapViewTool(QObject *parent = nullptr);

    void setMapView(Esri::ArcGISRuntime::MapQuickView *mapView);

    virtual void mousePressed(QMouseEvent &mouseEvent) = 0;
    virtual void mouseMoved(QMouseEvent &mouseEvent) = 0;
    virtual void mouseReleased(QMouseEvent &mouseEvent) = 0;

signals:

protected:
    Esri::ArcGISRuntime::MapQuickView *m_currentMapView = nullptr;
    Esri::ArcGISRuntime::GraphicsOverlay *m_sketchOverlay = nullptr;
};



class PolygonSketchTool : public MapViewTool
{
    Q_OBJECT
public:
    explicit PolygonSketchTool(QObject *parent = nullptr);

    virtual void mousePressed(QMouseEvent &mouseEvent) override;
    virtual void mouseMoved(QMouseEvent &mouseEvent) override;
    virtual void mouseReleased(QMouseEvent &mouseEvent) override;

private:
    Esri::ArcGISRuntime::PolygonBuilder *m_polygonBuilder = nullptr;
};

#endif // MAPVIEWTOOL_H
