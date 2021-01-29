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

#include "GeospatialTaskParameter.h"

#include "GeoprocessingParameter.h"

using namespace Esri::ArcGISRuntime;

GeospatialTaskParameter::GeospatialTaskParameter(const QString &parameterName,
                                                 GeoprocessingParameter *geoprocessingParameter,
                                                 QObject *parent) : QObject(parent), m_parameterName(parameterName), m_geoprocessingParameter(geoprocessingParameter)
{

}

QString GeospatialTaskParameter::parameterName() const
{
    return m_parameterName;
}
