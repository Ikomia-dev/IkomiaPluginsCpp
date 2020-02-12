#-------------------------------------------------
#
# Project created by QtCreator 2019-03-20T14:10:11
#
#-------------------------------------------------
QT += core gui widgets sql
TARGET = MaskRCNN

win32: DESTDIR = $$(USERPROFILE)/Ikomia/Plugins/C++/$$TARGET
unix: DESTDIR = $$(HOME)/Ikomia/Plugins/C++/$$TARGET

TEMPLATE = lib
CONFIG += plugin

DEFINES += MASKRCNN_LIBRARY BOOST_ALL_NO_LIB

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        MaskRCNN.cpp

HEADERS += \
        MaskRCNN.h \
        MaskRCNNGlobal.h

include(../IkomiaPluginsC++.pri)

#Dynamic link to Boost
win32: LIBS += -lboost_filesystem-vc$${BOOST_VC_VERSION}-mt-x64-$${BOOST_VERSION} -lboost_system-vc$${BOOST_VC_VERSION}-mt-x64-$${BOOST_VERSION} -lboost_python37-vc$${BOOST_VC_VERSION}-mt-x64-$${BOOST_VERSION}
unix: LIBS += -lboost_filesystem -lboost_system

#Dynamic link with OpenCV
win32:CONFIG(release, debug|release):LIBS += -lopencv_core$${OPENCV_VERSION} -lopencv_imgproc$${OPENCV_VERSION} -lopencv_dnn$${OPENCV_VERSION}
win32:CONFIG(debug, debug|release):LIBS += -lopencv_core$${OPENCV_VERSION}d -lopencv_imgproc$${OPENCV_VERSION}d -lopencv_dnn$${OPENCV_VERSION}d
unix:!macx: LIBS += -lopencv_core -lopencv_imgproc
macx: LIBS += -lopencv_core.$${OPENCV_VERSION} -lopencv_imgproc.$${OPENCV_VERSION} -lopencv_dnn.$${OPENCV_VERSION}

#Dynamic link with Utils
win32:CONFIG(release, debug|release): LIBS += -lUtils
else:win32:CONFIG(debug, debug|release): LIBS += -lUtils
else:unix: LIBS += -lUtils

INCLUDEPATH += $$GLPROJECT_INCLUDE/Utils

#Dynamic link with Core
win32:CONFIG(release, debug|release): LIBS += -lCore
else:win32:CONFIG(debug, debug|release): LIBS += -lCore
else:unix: LIBS += -lCore

INCLUDEPATH += $$GLPROJECT_INCLUDE/Core

#Dynamic link with DataProcess
win32:CONFIG(release, debug|release): LIBS += -lDataProcess
else:win32:CONFIG(debug, debug|release): LIBS += -lDataProcess
else:unix: LIBS += -lDataProcess

INCLUDEPATH += $$GLPROJECT_INCLUDE/DataProcess

# DEPLOYMENT
macx {
INSTALLS += makeDeploy
}

DISTFILES += \
    Model/coco_names.txt \
    Model/download_model.txt \
    Model/mask_rcnn_inception_v2_coco_2018_01_28.pbtxt
