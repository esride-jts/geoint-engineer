// GEOINTEngineer
// Copyright © 2020 Esri Deutschland GmbH
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
//

#ifndef LOCALGEOSPATIALSERVER_H
#define LOCALGEOSPATIALSERVER_H

namespace Esri
{
namespace ArcGISRuntime
{
class GeoprocessingFeatures;
class GeoprocessingTask;
class LicenseInfo;
enum class LoadStatus;
class LocalGeoprocessingService;
class Portal;
}
}

#include <QFileInfoList>
#include <QNetworkAccessManager>
#include <QObject>

class LocalGeospatialServer : public QObject
{
    Q_OBJECT
public:
    explicit LocalGeospatialServer(QObject *parent = nullptr);

    enum class Status {
        Stopped = 0,
        Starting = 1,
        Started = 2,
        Failed = 3
    };
    Status start();

    void executeTasks(Esri::ArcGISRuntime::GeoprocessingFeatures *inputFeatures);

signals:
    void taskCompleted(Esri::ArcGISRuntime::GeoprocessingFeatures *outputFeatures);

private slots:
    void networkRequestFinished(QNetworkReply *networkReply);
    void portalStatusChanged();
    void statusChanged();

private:
    QString licenseFilePath() const;

    QFileInfoList geoprocessingPackages() const;
    void startGeoprocessing();

    void saveLicense(Esri::ArcGISRuntime::LicenseInfo const *licenseInfo);
    void updateLicense(Esri::ArcGISRuntime::LicenseInfo const *licenseInfo, bool save);
    bool updateLicenseFromFile();

    void addGeoprocessingTasks(Esri::ArcGISRuntime::LocalGeoprocessingService *geoprocessingService);

    void logGeoprocessingTaskInfos();
    void logLoadStatus(QString const &prefix, Esri::ArcGISRuntime::LoadStatus loadStatus);

    Esri::ArcGISRuntime::Portal* m_geospatialPortal;
    QList<Esri::ArcGISRuntime::GeoprocessingTask*> m_geoprocessingTasks;
    //QList<Esri::ArcGISRuntime::
    QNetworkAccessManager* m_networkAccessManager;
};

#endif // LOCALGEOSPATIALSERVER_H
