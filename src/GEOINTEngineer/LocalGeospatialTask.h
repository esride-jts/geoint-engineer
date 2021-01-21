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


#ifndef LOCALGEOSPATIALTASK_H
#define LOCALGEOSPATIALTASK_H

namespace Esri
{
namespace ArcGISRuntime
{
class GeoprocessingFeatures;
class GeoprocessingTask;
class GeoprocessingResult;
}
}

#include "GeoprocessingParameters.h"

#include <QObject>

class LocalGeospatialTask : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString title READ displayName)
    Q_PROPERTY(QString description READ description)

public:
    explicit LocalGeospatialTask(Esri::ArcGISRuntime::GeoprocessingTask *geoprocessingTask, QObject *parent = nullptr);

    QString displayName() const;
    QString description() const;

    bool hasInputFeaturesParameter() const;
    void executeTask(Esri::ArcGISRuntime::GeoprocessingFeatures *inputFeatures);
    void logInfos() const;

signals:
    void taskCompleted(Esri::ArcGISRuntime::GeoprocessingResult *result);

private slots:
    void taskParametersCreated(QUuid, const Esri::ArcGISRuntime::GeoprocessingParameters &defaultInputParameters);

private:
    int findFirstInputFeaturesParameter() const;
    const static int InvalidIndex = -1;

    Esri::ArcGISRuntime::GeoprocessingTask* m_geoprocessingTask;
    Esri::ArcGISRuntime::GeoprocessingFeatures* m_inputFeatures;
};

#endif // LOCALGEOSPATIALTASK_H
