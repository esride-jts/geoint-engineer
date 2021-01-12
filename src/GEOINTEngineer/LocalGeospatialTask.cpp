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

#include "GeoprocessingFeatures.h"
#include "GeoprocessingTask.h"

using namespace Esri::ArcGISRuntime;

LocalGeospatialTask::LocalGeospatialTask(GeoprocessingTask *geoprocessingTask, QObject *parent) :
    QObject(parent),
    m_geoprocessingTask(geoprocessingTask)
{
    connect(geoprocessingTask, &GeoprocessingTask::createDefaultParametersCompleted, this, &LocalGeospatialTask::taskParametersCreated);
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
    GeoprocessingParameters inputParameters(defaultInputParameters.executionType());
    inputParameters.setInputs(inputs);

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
                GeoprocessingResult* newGeoprocessingResult = newGeoprocessingJob->result();
                QMap<QString, GeoprocessingParameter*> outputs = newGeoprocessingResult->outputs();
                foreach (GeoprocessingParameter const *outputParameter, outputs.values())
                {
                    switch (outputParameter->parameterType())
                    {
                    case GeoprocessingParameterType::GeoprocessingFeatures:
                        {
                            GeoprocessingFeatures *outputFeatures = (GeoprocessingFeatures*)outputParameter;
                            if (nullptr != outputFeatures)
                            {
                                // Emit that a task with features as output succeeded
                                emit taskCompleted(outputFeatures);
                            }
                        }
                        break;

                    default:
                        break;
                    }
                }
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
