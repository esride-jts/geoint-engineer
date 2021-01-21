// GEOINT Engineer
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

#include "LocalGeospatialServer.h"
#include "LocalGeospatialTask.h"

#include "ArcGISRuntimeEnvironment.h"
#include "GeoprocessingFeatures.h"
#include "GeoprocessingTask.h"
#include "LocalGeoprocessingService.h"
#include "LocalMapService.h"
#include "LocalServer.h"
#include "MobileMapPackage.h"
#include "Portal.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcessEnvironment>

using namespace Esri::ArcGISRuntime;

LocalGeospatialServer::LocalGeospatialServer(QObject *parent) :
    QObject(parent),
    m_networkAccessManager(new QNetworkAccessManager(this))
{
    connect(m_networkAccessManager, &QNetworkAccessManager::finished, this, &LocalGeospatialServer::networkRequestFinished);
}

QFileInfoList LocalGeospatialServer::listFiles(QString const &directoryPath, QString const &fileExtension) const
{
    QDir dataDirectory(directoryPath);
    if (dataDirectory.exists())
    {
        dataDirectory.setFilter(QDir::Files);
        dataDirectory.setNameFilters(QStringList() << fileExtension);
        return dataDirectory.entryInfoList();
    }

    return QFileInfoList();
}

QFileInfoList LocalGeospatialServer::geoprocessingPackages() const
{
    QString pathKeyName = "geoint.modelpath";
    QProcessEnvironment systemEnvironment = QProcessEnvironment::systemEnvironment();
    if (systemEnvironment.contains(pathKeyName))
    {
        QString directoryPath = systemEnvironment.value(pathKeyName);
        return listFiles(directoryPath, "*.gpk");
    }

    return QFileInfoList();
}

QFileInfoList LocalGeospatialServer::mapPackages() const
{
    QString pathKeyName = "geoint.datapath";
    QProcessEnvironment systemEnvironment = QProcessEnvironment::systemEnvironment();
    if (systemEnvironment.contains(pathKeyName))
    {
        QString directoryPath = systemEnvironment.value(pathKeyName);
        return listFiles(directoryPath, "*.mpkx");
    }

    return QFileInfoList();
}

QFileInfoList LocalGeospatialServer::mobileMapPackages() const
{
    QString pathKeyName = "geoint.datapath";
    QProcessEnvironment systemEnvironment = QProcessEnvironment::systemEnvironment();
    if (systemEnvironment.contains(pathKeyName))
    {
        QString directoryPath = systemEnvironment.value(pathKeyName);
        return listFiles(directoryPath, "*.mmpk");
    }

    return QFileInfoList();
}

QString LocalGeospatialServer::licenseFilePath() const
{
    return QDir::temp().filePath("geoint-engineer-license.lic");
}

LocalGeospatialServer::Status LocalGeospatialServer::start()
{
    if (!LocalServer::instance()->isInstallValid())
    {
        return Status::Failed;
    }

    connect(LocalServer::instance(), &LocalServer::statusChanged, this, &LocalGeospatialServer::statusChanged);

    if (updateLicenseFromFile())
    {
        // License file can be used
    }
    else
    {
        // Connect to the portal instance
        QString portalUrlKeyName = "arcgisruntime.portal.url";
        QProcessEnvironment systemEnvironment = QProcessEnvironment::systemEnvironment();
        if (systemEnvironment.contains(portalUrlKeyName))
        {
            QString portalUrl = systemEnvironment.value(portalUrlKeyName);

            // Connect to the portal instance using credentials
            QString portalUserKeyName = "arcgisruntime.portal.user";
            QString portalSecretKeyName = "arcgisruntime.portal.secret";
            if (systemEnvironment.contains(portalUserKeyName)
                && systemEnvironment.contains(portalSecretKeyName))
            {
                QString user = systemEnvironment.value(portalUserKeyName);
                QString secret = systemEnvironment.value(portalSecretKeyName);

                Credential* portalCredential = new Credential(user, secret, this);
                m_geospatialPortal = new Portal(portalUrl, portalCredential, this);
            }
            else
            {
                // Connect to the portal instance using SSO
                m_geospatialPortal = new Portal(portalUrl, this);
            }
        }

        if (nullptr != m_geospatialPortal)
        {
            // License file cannot be used
            connect(m_geospatialPortal, &Portal::loadStatusChanged, this, &LocalGeospatialServer::portalStatusChanged);
            connect(m_geospatialPortal, &Portal::fetchLicenseInfoCompleted, this, [this](QUuid, const LicenseInfo& licenseInfo)
            {
                // Update and save license
                updateLicense(&licenseInfo, true);
            });
            m_geospatialPortal->load();
        }
    }

    return Status::Starting;
}

void LocalGeospatialServer::executeTask(LocalGeospatialTask *geospatialTask, Esri::ArcGISRuntime::GeoprocessingFeatures *inputFeatures)
{
    if (geospatialTask->hasInputFeaturesParameter())
    {
        geospatialTask->executeTask(inputFeatures);
    }
}

void LocalGeospatialServer::executeTasks(GeoprocessingFeatures *inputFeatures)
{
    foreach (LocalGeospatialTask *geospatialTask, m_geospatialTasks)
    {
        if (geospatialTask->hasInputFeaturesParameter())
        {
            geospatialTask->executeTask(inputFeatures);
        }
    }
}

void LocalGeospatialServer::startGeoprocessing()
{
    QFileInfoList packages = geoprocessingPackages();
    foreach (QFileInfo const &fileInfo, packages)
    {
        // Create a new local geoprocessing service with map server results
        QString packageFilePath = fileInfo.absoluteFilePath();
        qDebug() << packageFilePath;
        LocalGeoprocessingService *localGpService = new LocalGeoprocessingService(packageFilePath, this);
        // TODO: When is map server result supported?
        //localGpService->setServiceType(GeoprocessingServiceType::AsynchronousSubmitWithMapServerResult);
        localGpService->setServiceType(GeoprocessingServiceType::SynchronousExecute);
        connect(localGpService, &LocalGeoprocessingService::statusChanged, this, [this, localGpService]()
        {
            switch (localGpService->status())
            {
            case LocalServerStatus::Starting:
                qDebug() << "Local geospatial service " << localGpService->name() << " starting...";
                break;

            case LocalServerStatus::Started:
                qDebug() << "Local geospatial service " << localGpService->name() << " started.";
                qDebug() << localGpService->url();
                addGeoprocessingTasks(localGpService);
                break;

            case LocalServerStatus::Stopping:
                qDebug() << "Local geospatial service " << localGpService->name() << " stopping...";
                break;

            case LocalServerStatus::Stopped:
                qDebug() << "Local geospatial service " << localGpService->name() << " stopped.";
                break;

            case LocalServerStatus::Failed:
                qDebug() << "Local geospatial service " << localGpService->name() << " failed!";
                break;
            }

        });
        localGpService->start();
    }
}

void LocalGeospatialServer::startMapping()
{
    QFileInfoList mobileMapPackages = this->mobileMapPackages();
    foreach (QFileInfo const &fileInfo, mobileMapPackages)
    {
        // Create a new local geoprocessing service with map server results
        QString packageFilePath = fileInfo.absoluteFilePath();
        MobileMapPackage *mobileMapPackage = new MobileMapPackage(packageFilePath, this);
        connect(mobileMapPackage, &MobileMapPackage::loadStatusChanged, this, [packageFilePath, mobileMapPackage, this](LoadStatus loadStatus)
        {
            switch (loadStatus)
            {
            case LoadStatus::Loaded:
                {
                    qDebug() << "Local mobile map package " << packageFilePath << " loaded.";
                    QList<Map*> offlineMaps = mobileMapPackage->maps();
                    foreach (Map *offlineMap, offlineMaps)
                    {
                        emit mapLoaded(offlineMap);
                    }
                }
                break;

            default:
                break;
            }
        });
        mobileMapPackage->load();
    }

    QFileInfoList mapPackages = this->mapPackages();
    foreach (QFileInfo const &fileInfo, mapPackages)
    {
        // Create a new local geoprocessing service with map server results
        QString packageFilePath = fileInfo.absoluteFilePath();
        LocalMapService *localMapService = new LocalMapService(packageFilePath, this);
        qDebug() << packageFilePath;
        connect(localMapService, &LocalMapService::statusChanged, this, [packageFilePath, localMapService, this]()
        {
            switch (localMapService->status())
            {
            case LocalServerStatus::Started:
                {
                    qDebug() << "Local map server using " << packageFilePath << " started.";
                    ArcGISMapImageLayer *mapImageLayer = new ArcGISMapImageLayer(localMapService->url(), this);
                    connect(mapImageLayer, &ArcGISMapImageLayer::loadStatusChanged, this, [this, mapImageLayer](LoadStatus loadStatus)
                    {
                        switch (loadStatus)
                        {
                        case LoadStatus::Loaded:
                            {
                                qDebug() << "Local map service " << mapImageLayer->url() << " loaded.";
                                emit mapServiceLoaded(mapImageLayer);
                            }
                            break;

                        default:
                            break;
                        }
                    });
                    mapImageLayer->load();
                }
                break;

            case LocalServerStatus::Failed:
                qDebug() << "Local map server using " << packageFilePath << " failed!";
                break;

            default:
                break;
            }
        });
        localMapService->start();
    }
}

void LocalGeospatialServer::saveLicense(LicenseInfo const *licenseInfo)
{
    QString licenseAsJson = licenseInfo->toJson();
    QFile licenseFile(licenseFilePath());
    if (!licenseFile.open(QIODevice::WriteOnly))
    {
        qDebug() << "Cannot save license file!";
        return;
    }

    licenseFile.write(licenseAsJson.toUtf8());
    licenseFile.close();
}

void LocalGeospatialServer::updateLicense(LicenseInfo const *licenseInfo, bool save)
{
    LicenseResult licenseResult = ArcGISRuntimeEnvironment::setLicense(*licenseInfo);
    switch (licenseResult.licenseStatus())
    {
    case LicenseStatus::Valid:
        qDebug() << "Fetched license is valid.";
        if (save)
        {
            saveLicense(licenseInfo);
        }

        // Start the local server
        LocalServer::start();
        break;

    case LicenseStatus::LoginRequired:
        qDebug() << "Login is required!";
        break;

    case LicenseStatus::Expired:
        qDebug() << "License is expired!";
        break;

    case LicenseStatus::Invalid:
        qDebug() << "License is invalid!";
        break;
    }
}

bool LocalGeospatialServer::updateLicenseFromFile()
{
    QFile licenseFile(licenseFilePath());
    if (licenseFile.exists())
    {
        qDebug() << licenseFile.fileName();
        licenseFile.open(QIODevice::ReadOnly);
        LicenseInfo licenseInfo = LicenseInfo::fromJson(licenseFile.readAll());
        LicenseResult licenseResult = ArcGISRuntimeEnvironment::setLicense(licenseInfo);
        if (LicenseStatus::Valid == licenseResult.licenseStatus())
        {
            // Start the local server
            LocalServer::start();
            return true;
        }
    }

    return false;
}

void LocalGeospatialServer::addGeoprocessingTasks(LocalGeoprocessingService *geoprocessingService)
{
    QString infoEndpoint = geoprocessingService->url().toString() + "?f=json";
    QNetworkRequest geoprocessingInfoRequest(infoEndpoint);
    m_networkAccessManager->get(geoprocessingInfoRequest);
}

void LocalGeospatialServer::networkRequestFinished(QNetworkReply *networkReply)
{
    if (networkReply->error())
    {
        qDebug() << networkReply->errorString();
        return;
    }

    QByteArray jsonResponse = networkReply->readAll();
    QJsonDocument geoprocessingServiceDocument = QJsonDocument::fromJson(jsonResponse);
    if (geoprocessingServiceDocument.isNull())
    {
        qDebug() << "JSON is invalid!";
        return;
    }
    if (!geoprocessingServiceDocument.isObject())
    {
        qDebug() << "JSON document is not an object!";
        return;
    }

    QJsonObject geoprocessingServiceObject = geoprocessingServiceDocument.object();
    QJsonArray geoprocessingTasksArray = geoprocessingServiceObject["tasks"].toArray();
    foreach (const QJsonValue &taskValue, geoprocessingTasksArray)
    {
        QUrl geoprocessingServiceUrl = networkReply->url();
        QString geoprocessingTaskEndpoint = geoprocessingServiceUrl.scheme()
                + "://" + networkReply->url().authority()
                + networkReply->url().path()
                + "/" + taskValue.toString();
        qDebug() << geoprocessingTaskEndpoint;

        GeoprocessingTask *geoprocessingTask = new GeoprocessingTask(QUrl(geoprocessingTaskEndpoint), this);
        connect(geoprocessingTask, &GeoprocessingTask::loadStatusChanged, this, [this, geoprocessingTask]()
        {
            LoadStatus taskLoadStatus = geoprocessingTask->loadStatus();
            logLoadStatus("GP task ", taskLoadStatus);

            switch (taskLoadStatus)
            {
            case LoadStatus::Loaded:
                {
                    // Add a new geospatial task
                    LocalGeospatialTask *geospatialTask = new LocalGeospatialTask(geoprocessingTask, this);
                    connect(geospatialTask, &LocalGeospatialTask::taskCompleted, this, &LocalGeospatialServer::localTaskCompleted);
                    m_geospatialTasks.append(geospatialTask);
                    logGeoprocessingTaskInfos();

                    // Emit the new geospatial task
                    emit taskLoaded(geospatialTask);
                }
                break;
            default:
                return;
            }
        });
        geoprocessingTask->load();
    }
}

void LocalGeospatialServer::logGeoprocessingTaskInfos()
{
    foreach (LocalGeospatialTask const *geospatialTask, m_geospatialTasks)
    {
        geospatialTask->logInfos();
    }
}

void LocalGeospatialServer::logLoadStatus(QString const &prefix, Esri::ArcGISRuntime::LoadStatus loadStatus)
{
    switch (loadStatus)
    {
    case LoadStatus::Loading:
        qDebug() << (prefix + "loading...");
        break;

    case LoadStatus::Loaded:
        qDebug() << (prefix + "loaded.");
        break;

    case LoadStatus::NotLoaded:
        qDebug() << (prefix + "not loaded.");
        break;

    case LoadStatus::FailedToLoad:
        qDebug() << (prefix + "failed to load!");
        break;

    case LoadStatus::Unknown:
        qDebug() << (prefix + "status unknown?");
        break;
    }
}

void LocalGeospatialServer::portalStatusChanged()
{
    switch (m_geospatialPortal->loadStatus())
    {
    case LoadStatus::Loading:
        qDebug() << "Portal loading...";
        break;

    case LoadStatus::Loaded:
        qDebug() << "Portal loaded.";
        m_geospatialPortal->fetchLicenseInfo();
        break;

    case LoadStatus::NotLoaded:
        qDebug() << "Portal not loaded.";
        break;

    case LoadStatus::FailedToLoad:
        qDebug() << "Portal failed to load!";
        break;

    case LoadStatus::Unknown:
        qDebug() << "Portal status unknown?";
        break;
    }
}

void LocalGeospatialServer::statusChanged()
{
    switch (LocalServer::instance()->status())
    {
    case LocalServerStatus::Starting:
        qDebug() << "Local geospatial server starting...";
        break;

    case LocalServerStatus::Started:
        qDebug() << "Local geospatial server started.";
        startGeoprocessing();
        startMapping();
        break;

    case LocalServerStatus::Stopping:
        qDebug() << "Local geospatial server stopping...";
        break;

    case LocalServerStatus::Stopped:
        qDebug() << "Local geospatial server stopped.";
        break;

    case LocalServerStatus::Failed:
        qDebug() << "Local geospatial server failed!";
        qDebug() << LocalServer::installPath();
        break;
    }
}

void LocalGeospatialServer::localTaskCompleted(GeoprocessingResult *result)
{
    emit taskCompleted(result);
}
