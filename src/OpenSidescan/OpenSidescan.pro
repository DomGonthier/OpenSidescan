#-------------------------------------------------
#
# Project created by QtCreator 2019-08-15T12:34:57
#
#-------------------------------------------------

QT       += core gui xml network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OpenSidescan
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

win32 {
    RC_ICONS = resources\\cidco_icon.ico

    INCLUDEPATH += "C:\\LIBS\\eigen" \
                   "C:\\LIBS\\opencv4.5\\opencv\\build\\include"

    LIBS += "C:\\LIBS\\opencv4.5\\opencv\\build\\x64\\vc15\\libopencv_core454.lib"
    LIBS += "C:\\LIBS\\opencv4.5\\opencv\\build\\x64\\vc15\\opencv_features2d454.lib"
    LIBS += "C:\\LIBS\\opencv4.5\\opencv\\build\\x64\\vc15\\opencv_imgcodecs454.lib"
    LIBS += "C:\\LIBS\\opencv4.5\\opencv\\build\\x64\\vc15\\opencv_imgproc454.lib"
    LIBS += "C:\\LIBS\\opencv4.5\\opencv\\build\\x64\\vc15\\opencv_ml454.lib"
    LIBS += "C:\\LIBS\\opencv4.5\\opencv\\build\\x64\\vc15\\opencv_photo454.lib"
    LIBS += "C:\\LIBS\\opencv4.5\\opencv\\build\\x64\\vc15\\opencv_flann454.lib"
    LIBS += "C:\\LIBS\\opencv4.5\\opencv\\build\\x64\\vc15\\opencv_highgui454.lib"
    LIBS += "C:\\LIBS\\opencv4.5\\opencv\\build\\x64\\vc15\\opencv_calib3d454.lib"
  }
SOURCES += main.cpp\
    ../thirdParty/MBES-lib/src/datagrams/DatagramParser.cpp \
    ../thirdParty/MBES-lib/src/datagrams/DatagramParserFactory.cpp \
    ../thirdParty/MBES-lib/src/datagrams/s7k/S7kParser.cpp \
    ../thirdParty/MBES-lib/src/datagrams/kongsberg/KongsbergParser.cpp \
    ../thirdParty/MBES-lib/src/datagrams/xtf/XtfParser.cpp \
    ../thirdParty/MBES-lib/src/utils/NmeaUtils.cpp \
    ../thirdParty/MBES-lib/src/utils/StringUtils.cpp \
    ../thirdParty/MBES-lib/src/sidescan/SidescanPing.cpp \
    ../thirdParty/opencv/apps/createsamples/utility.cpp \
    machinelearning/dbscan.cpp \
    project/project.cpp \
    inventoryobject/inventoryobject.cpp \
    sidescan/sidescanfile.cpp \
    sidescan/sidescanimage.cpp \
    sidescan/sidescanimager.cpp \
    ui/detection/monitorwindow.cpp \
    ui/docks/projectwindow/projecttreeitem.cpp \
    ui/docks/projectwindow/projecttreemodel.cpp \
    ui/docks/projectwindow/projectwindow.cpp \
    ui/docks/channelpropertieswindow.cpp \
    ui/docks/filepropertieswindow.cpp \
    ui/docks/inventorywindow.cpp \
    ui/about/aboutdialog.cpp \
    detector/detector.cpp \
    refactorme/progressdialognotclosingrightawayoncancel.cpp \
    ui/detection/detectionwindow.cpp \
    ui/detection/workerdetection.cpp \
    ui/inventory/inventoryobjectmenu.cpp \
    ui/inventory/inventoryobjectwindow.cpp \
    ui/mainimage/imagetab.cpp \
    ui/mainimage/imagetablabel.cpp \
    ui/mainwindow/mainwindow.cpp \
    ui/mainwindow/workerimportsidescanfiles.cpp \
    ui/mainwindow/workeropenproject.cpp \
    ui/training/trainingsampleswindow.cpp \
    ui/training/workertrainingsamples.cpp \
    utilities/opencvhelper.cpp \
    utilities/qthelper.cpp \
    ui/training/parameterscvCreateTrainingSamples.cpp \
    ui/docks/vesselgeometrywindow.cpp \
    detector/roidetector.cpp \
    ui/mainwindow/monitorthread.cpp \
    detector/houghdetector.cpp \
    telemetrymanager/telemetrymanager.cpp


HEADERS  +=     machinelearning/dbscan.h \
    project/project.h \
    inventoryobject/inventoryobject.h \
    ui/detection/monitorwindow.h \
    ui/training/parameterscvCreateTrainingSamples.h \
    detector/detector.h \
    ui/about/aboutdialog.h \
    refactorme/boolwithmutex.h \
    refactorme/progressdialognotclosingrightawayoncancel.h \
    sidescan/sidescanfile.h \
    sidescan/sidescanimage.h \
    sidescan/sidescanimager.h \
    ui/detection/detectionwindow.h \
    ui/detection/workerdetection.h \
    ui/docks/projectwindow/projecttreeitem.h \
    ui/docks/projectwindow/projecttreemodel.h \
    ui/docks/projectwindow/projectwindow.h \
    ui/docks/channelpropertieswindow.h \
    ui/docks/filepropertieswindow.h \
    ui/docks/inventorywindow.h \
    ui/inventory/inventoryobjectmenu.h \
    ui/inventory/inventoryobjectwindow.h \
    ui/mainimage/imagetab.h \
    ui/mainimage/imagetablabel.h \
    ui/mainwindow/mainwindow.h \
    ui/mainwindow/workerimportsidescanfiles.h \
    ui/mainwindow/workeropenproject.h \
    ui/training/trainingsampleswindow.h \
    ui/training/workertrainingsamples.h \
    utilities/opencvhelper.h \
    utilities/qthelper.h \
    ui/docks/vesselgeometrywindow.h \
    detector/roidetector.h \
    ui/mainwindow/monitorthread.h \
    detector/houghdetector.h \
    telemetrymanager/telemetrymanager.h

FORMS    += ui/mainwindow/mainwindow.ui \
    ui/about/aboutdialog.ui

RESOURCES +=  resources.qrc

RESOURCES +=  ../thirdParty/QDarkStyleSheet/qdarkstyle/style.qrc

CONFIG+=link_pkgconfig

#CONFIG -= precompile_header

unix{
        PKGCONFIG+=eigen3
        LIBS+= -L/usr/local/lib -lopencv_stitching -lopencv_highgui -lopencv_dnn -lopencv_ml -lopencv_videoio -lopencv_video -lopencv_objdetect -lopencv_calib3d -lopencv_imgcodecs -lopencv_features2d -lopencv_flann -lopencv_photo -lopencv_imgproc -lopencv_core
        INCLUDEPATH += /usr/local/include/opencv4
}
