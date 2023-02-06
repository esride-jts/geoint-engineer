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


#include "LocalGeospatialTask.h"

#include <QDebug>
#include <QUrl>
#include <QUuid>

#include "ArcGISMapImageLayer.h"
#include "GeoprocessingFeatures.h"
#include "GeoprocessingJob.h"
#include "GeoprocessingResult.h"
#include "GeoprocessingTask.h"
#include "GeoprocessingTaskInfo.h"
#include "GeoprocessingTypes.h"
#include "TaskTypes.h"
#include "TaskWatcher.h"

using namespace Esri::ArcGISRuntime;

LocalGeospatialTask::LocalGeospatialTask(GeoprocessingTask *geoprocessingTask, GeoprocessingServiceType serviceType, QObject *parent) :
    QObject(parent),
    m_geoprocessingTask(geoprocessingTask),
    m_serviceType(serviceType)
{
    connect(geoprocessingTask, &GeoprocessingTask::createDefaultParametersCompleted, this, &LocalGeospatialTask::taskParametersCreated);
}

QString LocalGeospatialTask::displayName() const
{
    return m_geoprocessingTask->geoprocessingTaskInfo().displayName();
}

QString LocalGeospatialTask::description() const
{
    return m_geoprocessingTask->geoprocessingTaskInfo().description();
}

QList<GeoprocessingParameterInfo> LocalGeospatialTask::parameters() const
{
    return m_geoprocessingTask->geoprocessingTaskInfo().parameterInfos();
}

bool LocalGeospatialTask::hasInputFeaturesParameter() const
{
    GeoprocessingTaskInfo taskInfo = m_geoprocessingTask->geoprocessingTaskInfo();
    QList<GeoprocessingParameterInfo> taskParameterInfos = taskInfo.parameterInfos();
    foreach (GeoprocessingParameterInfo const &parameterInfo, taskParameterInfos)
    {
        switch(parameterInfo.direction())
        {
        case GeoprocessingParameterDirection::Input:
            switch (parameterInfo.dataType())
            {
            case GeoprocessingParameterType::GeoprocessingFeatures:
                // Input parameter type is features
                return true;

            default:
                break;
            }
            break;

        default:
            break;
        }
    }

    return false;
}

void LocalGeospatialTask::executeTask(Esri::ArcGISRuntime::GeoprocessingFeatures *inputFeatures)
{
    m_inputFeatures = inputFeatures;
    m_geoprocessingTask->createDefaultParameters();
}

void LocalGeospatialTask::logInfos() const
{
    GeoprocessingTaskInfo taskInfo = m_geoprocessingTask->geoprocessingTaskInfo();
    QList<GeoprocessingParameterInfo> taskParameterInfos = taskInfo.parameterInfos();
    qDebug() << taskInfo.name();
    foreach (GeoprocessingParameterInfo const &parameterInfo, taskParameterInfos)
    {
        qDebug() << parameterInfo.name();
        switch (parameterInfo.dataType())
        {
        case GeoprocessingParameterType::GeoprocessingString:
            qDebug() << "GeoprocessingString";
            break;

        case GeoprocessingParameterType::GeoprocessingFeatures:
            qDebug() << "GeoprocessingFeatures";
            break;

        default:
            break;
        }
    }
}

int LocalGeospatialTask::findFirstInputFeaturesParameter() const
{
    GeoprocessingTaskInfo taskInfo = m_geoprocessingTask->geoprocessingTaskInfo();
    QList<GeoprocessingParameterInfo> taskParameterInfos = taskInfo.parameterInfos();

    for (int index = 0, parameterCount = taskParameterInfos.length(); index < parameterCount; index++)
    {
        GeoprocessingParameterInfo const &parameterInfo = taskParameterInfos[index];
        switch(parameterInfo.direction())
        {
        case GeoprocessingParameterDirection::Input:
            switch (parameterInfo.dataType())
            {
            case GeoprocessingParameterType::GeoprocessingFeatures:
                // Input parameter type is features
                return index;

            default:
                break;
            }
            break;

        default:
            break;
        }
    }

    return InvalidIndex;
}

void LocalGeospatialTask::taskParametersCreated(QUuid, const Esri::ArcGISRuntime::GeoprocessingParameters &defaultInputParameters)
{
    int parameterIndex = findFirstInputFeaturesParameter();
    if (InvalidIndex == parameterIndex)
    {
        return;
    }

    GeoprocessingTaskInfo taskInfo = m_geoprocessingTask->geoprocessingTaskInfo();
    GeoprocessingParameterInfo parameterInfo = taskInfo.parameterInfos()[parameterIndex];
    qDebug() << "Geoprocessing input parameters" << taskInfo.name() << "created.";
    QMap<QString, GeoprocessingParameter*> inputs = defaultInputParameters.inputs();
    inputs.insert(parameterInfo.name(), m_inputFeatures);

    // Define the execution type
    GeoprocessingParameters inputParameters(defaultInputParameters.executionType());
    switch (inputParameters.executionType())
    {
    case GeoprocessingExecutionType::AsynchronousSubmit:
        qDebug() << "Default execution type is asynchronous submit.";
        break;
    case GeoprocessingExecutionType::SynchronousExecute:
        qDebug() << "Default execution type is synchronous execute.";
        break;
    case GeoprocessingExecutionType::Unknown:
        qDebug() << "Default execution type is unknown.";
        break;
    }
    inputParameters.setInputs(inputs);

    // Log the service type
    switch (m_serviceType)
    {
    case GeoprocessingServiceType::AsynchronousSubmitWithMapServerResult:
        qDebug() << "Service type is asynchronous submit with map server result.";
        break;
    case GeoprocessingServiceType::AsynchronousSubmit:
        qDebug() << "Service type is asynchronous submit.";
        break;
    case GeoprocessingServiceType::SynchronousExecute:
        qDebug() << "Service type is synchronous execute.";
        break;
    }

    GeoprocessingJob *newGeoprocessingJob = m_geoprocessingTask->createJob(inputParameters);
    connect(newGeoprocessingJob, &GeoprocessingJob::jobDone, this, [this, newGeoprocessingJob]()
    {
        switch (newGeoprocessingJob->jobStatus())
        {
        case JobStatus::Started:
            qDebug() << "Geoprocessing job " << newGeoprocessingJob->serverJobId() << " started.";
            break;

        case JobStatus::NotStarted:
            qDebug() << "Geoprocessing job " << newGeoprocessingJob->serverJobId() << " not started.";
            break;

        case JobStatus::Paused:
            qDebug() << "Geoprocessing job " << newGeoprocessingJob->serverJobId() << " paused.";
            break;

        case JobStatus::Succeeded:
            {
                qDebug() << "Geoprocessing job " << newGeoprocessingJob->serverJobId() << " succeeded.";
                GeoprocessingResult *newGeoprocessingResult = newGeoprocessingJob->result();
                ArcGISMapImageLayer *newMapImageLayer = nullptr;
                if (GeoprocessingServiceType::AsynchronousSubmitWithMapServerResult == m_serviceType
                        && nullptr == newGeoprocessingResult->mapImageLayer())
                {
                    // TODO: Investigate why there is no map image layer!
                    QString taskEndpoint = m_geoprocessingTask->url().toString();
                    const int Invalid_Index = -1;
                    int gpServerCharPos = taskEndpoint.lastIndexOf("/GPServer/");
                    if (Invalid_Index != gpServerCharPos)
                    {
                        QString mapImageServerEndpoint = taskEndpoint.left(gpServerCharPos) + "/MapServer/jobs/" + newGeoprocessingJob->serverJobId();
                        newMapImageLayer = new ArcGISMapImageLayer(QUrl(mapImageServerEndpoint), this);
                    }
                }

                // Emit that a task succeeded
                emit taskCompleted(newGeoprocessingResult, newMapImageLayer);
            }
            break;

        case JobStatus::Failed:
            qDebug() << "Geoprocessing job " << newGeoprocessingJob->serverJobId() << " failed!";
            break;
        }
    });

    qDebug() << "Geoprocessing job " << newGeoprocessingJob->serverJobId() << " starting...";
    newGeoprocessingJob->start();
}
