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

#include "GeospatialTaskParameterModel.h"
#include "LocalGeospatialTask.h"

#include "GeoprocessingParameterInfo.h"

using namespace Esri::ArcGISRuntime;

GeospatialTaskParameterModel::GeospatialTaskParameterModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void GeospatialTaskParameterModel::updateParameters(LocalGeospatialTask *localGeospatialTask)
{
    beginResetModel();
    m_localGeospatialTask = localGeospatialTask;
    endResetModel();
}

QHash<int, QByteArray> GeospatialTaskParameterModel::roleNames() const
{
    QHash<int, QByteArray> roleNames;
    roleNames.insert(RoleNames::ParameterNameRole, QString("parameterName").toUtf8());
    roleNames.insert(RoleNames::UiEditorRole, QString("uiEditorSource").toUtf8());
    return roleNames;
}

int GeospatialTaskParameterModel::rowCount(const QModelIndex &parent) const
{
    if (nullptr == m_localGeospatialTask)
    {
        return 0;
    }

    return m_localGeospatialTask->parameters().size();
}

QVariant GeospatialTaskParameterModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || m_localGeospatialTask->parameters().size() <= index.row())
    {
        qDebug() << "Wrong index!";
        return QVariant();
    }

    GeoprocessingParameterInfo parameterInfo = m_localGeospatialTask->parameters().at(index.row());
    switch (role)
    {
    case ParameterNameRole:
        return parameterInfo.displayName();

    case UiEditorRole:
        switch (parameterInfo.dataType())
        {
         default:
            return "GpStringInput.qml";
        }

    default:
        qDebug() << "Unknown role!";
        return QVariant();
    }
}