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

#include "GeospatialTaskListModel.h"
#include "LocalGeospatialTask.h"

GeospatialTaskListModel::GeospatialTaskListModel(QObject *parent) : QAbstractListModel(parent)
{
}

void GeospatialTaskListModel::addTask(LocalGeospatialTask *geospatialTask)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_geospatialTasks.append(geospatialTask);
    endInsertRows();
}

LocalGeospatialTask* GeospatialTaskListModel::task(int taskIndex) const
{
    if (m_geospatialTasks.size() <= taskIndex)
    {
        qDebug() << "Wrong index!";
        return nullptr;
    }

    return m_geospatialTasks[taskIndex];
}

QHash<int, QByteArray> GeospatialTaskListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(RoleNames::TitleRole, QString("title").toUtf8());
    roles.insert(RoleNames::DescriptionRole, QString("description").toUtf8());
    return roles;
}

int GeospatialTaskListModel::rowCount(const QModelIndex&) const
{
    return m_geospatialTasks.size();
}

QVariant GeospatialTaskListModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || m_geospatialTasks.size() <= index.row())
    {
        qDebug() << "Wrong index!";
        return QVariant();
    }

    LocalGeospatialTask *geospatialTask = m_geospatialTasks.at(index.row());
    switch (role)
    {
    case TitleRole:
        qDebug() << "title:" << geospatialTask->displayName();
        return geospatialTask->displayName();

    case DescriptionRole:
        qDebug() << "description:" << geospatialTask->description();
        return geospatialTask->description();

    default:
        qDebug() << "Unknown role!";
        return QVariant();
    }
}
