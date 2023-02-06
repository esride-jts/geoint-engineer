#-------------------------------------------------
#  Copyright 2019 ESRI
#
#  All rights reserved under the copyright laws of the United States
#  and applicable international laws, treaties, and conventions.
#
#  You may freely redistribute and use this sample code, with or
#  without modification, provided you include the original copyright
#  notice and use restrictions.
#
#  See the Sample code usage restrictions document for further information.
#-------------------------------------------------

TEMPLATE = app

CONFIG += c++17

# additional modules are pulled in via arcgisruntime.pri
QT += opengl qml quick quickcontrols2

TARGET = GEOINTEngineer

lessThan(QT_MAJOR_VERSION, 6) {
    error("$$TARGET requires Qt 6.2.4")
}

equals(QT_MAJOR_VERSION, 6) {
    lessThan(QT_MINOR_VERSION, 2) {
        error("$$TARGET requires Qt 6.2.4")
    }
        equals(QT_MINOR_VERSION, 2) : lessThan(QT_PATCH_VERSION, 4) {
                error("$$TARGET requires Qt 6.2.4")
        }
}

ARCGIS_RUNTIME_VERSION = 200.0.0
include($$PWD/arcgisruntime.pri)

HEADERS += \
    GEOINTEngineer.h \
    GeospatialTaskListModel.h \
    GeospatialTaskParameter.h \
    GeospatialTaskParameterModel.h \
    LocalGeospatialServer.h \
    LocalGeospatialTask.h \
    MapViewTool.h

SOURCES += \
    GeospatialTaskListModel.cpp \
    GeospatialTaskParameter.cpp \
    GeospatialTaskParameterModel.cpp \
    LocalGeospatialServer.cpp \
    LocalGeospatialTask.cpp \
    MapViewTool.cpp \
    main.cpp \
    GEOINTEngineer.cpp

RESOURCES += \
    qml/qml.qrc \
    Resources/Resources.qrc

#-------------------------------------------------------------------------------

win32 {
    include (Win/Win.pri)
}

macx {
    include (Mac/Mac.pri)
}

ios {
    include (iOS/iOS.pri)
}

android {
    include (Android/Android.pri)
}
