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


#ifndef GEOSPATIALTASKPARAMETER_H
#define GEOSPATIALTASKPARAMETER_H

namespace Esri
{
namespace ArcGISRuntime
{
class GeoprocessingParameter;
}
}

#include <QObject>

class GeospatialTaskParameter : public QObject
{
    Q_PROPERTY(QString name READ parameterName)

    Q_OBJECT
public:
    explicit GeospatialTaskParameter(QString const &parameterName, Esri::ArcGISRuntime::GeoprocessingParameter *geoprocessingParameter, QObject *parent = nullptr);

    QString parameterName() const;

signals:

private:
    QString m_parameterName;
    Esri::ArcGISRuntime::GeoprocessingParameter *m_geoprocessingParameter;
};

#endif // GEOSPATIALTASKPARAMETER_H
