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

#include "ArcGISRuntimeEnvironment.h"
#include "LocalServer.h"
#include "Portal.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcessEnvironment>

using namespace Esri::ArcGISRuntime;

LocalGeospatialServer::LocalGeospatialServer(QObject *parent) : QObject(parent)
{

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
