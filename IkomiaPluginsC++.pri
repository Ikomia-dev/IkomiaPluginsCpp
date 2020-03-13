# Fichier de configuration Qt du projet GLPlugins
# Initialisation de toutes las variables communes aux projets de plugins
CONFIG += c++14

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


defineReplace(changePath) {
    var1 = $$1
    var2 = $$2
    var3 = $$3
    var4 = $$quote(install_name_tool -change $$var1$$var2 @executable_path/../Frameworks/$$var2 $$var3 $$escape_expand(\n\t))

    return($$var4)
}

defineReplace(changeOpencvPath) {
    var1 = $$1
    var2 = $$2
    var3 = $$3
    var4 = $$quote(install_name_tool -change @rpath/$$var1 @executable_path/../Frameworks/$$var2 $$var3 $$escape_expand(\n\t))

    return($$var4)
}

macx{
    QMAKE_CC = /usr/local/opt/llvm/bin/clang
    QMAKE_CXX = /usr/local/opt/llvm/bin/clang++
}

# Enable OpenMP
msvc {
  QMAKE_CXXFLAGS += -openmp -arch:AVX2 -D "_CRT_SECURE_NO_WARNINGS"
  QMAKE_CXXFLAGS_RELEASE *= -O2
}

##################################################################
# Load local machine settings (dependencies version, options...) #
##################################################################
include(../Ikomia/LocalSettings.pri)

VERSION = 1.0.0

#####################################################################################
#                       INCLUDE                                                     #
#####################################################################################
#Ikomia path
IKOMIA_INCLUDE = $$PWD/../Ikomia/Build/include

# Global include directory for Mac OS X
macx: INCLUDEPATH += /usr/local/include

# Boost
win32: INCLUDEPATH += $$(PROGRAMFILES)/Boost/include/boost-$${BOOST_VERSION}

#OpenCV
win32: INCLUDEPATH += $$(PROGRAMFILES)/OpenCV/include
unix:!macx: INCLUDEPATH += /usr/local/include/opencv4
macx: INCLUDEPATH += /usr/local/include/opencv4

#OpenCL
unix: INCLUDEPATH += $$PWD/../../..
win32: INCLUDEPATH += '$$(PROGRAMFILES)/NVIDIA GPU Computing Toolkit/CUDA/v$${CUDA_VERSION}/include'

#FFTW
win32: INCLUDEPATH += $$(PROGRAMFILES)/fftw-3.3.5-dll64

#####################################################################################
#                       LIB                                                         #
#####################################################################################
#Ikomia path
IKOMIA_LIBS = $$PWD/../Ikomia/Build/lib
LIBS = -L$$IKOMIA_LIBS
macx: LIBS += -L$$PWD/../Ikomia/Build/bin/Ikomia.app/Contents/Frameworks

# Global lib directory for:
# Linux
unix:!macx: LIBS += -L/usr/local/lib64/

# Mac OS X
macx: LIBS += -L/usr/local/lib/

# Boost
win32: LIBS += -L$$(PROGRAMFILES)/Boost/lib

# OpenCV
win32: LIBS += -L$$(ProgramW6432)/OpenCV/x64/vc$${MSVC_VERSION}/lib

# Cuda + OpenCL
win32: LIBS += -L$$(PROGRAMFILES)/OpenCL/lib
win32: LIBS += -L'$$(ProgramW6432)/NVIDIA GPU Computing Toolkit/CUDA/v$${CUDA_VERSION}/lib/x64'
unix:!macx: LIBS += -L/usr/lib64/nvidia/

# OpenGL
win32: LIBS += -L'C:\Program Files(x86)\Windows Kits\10\Lib\10.0.17134.0\um\x64'

# FFTW
win32: LIBS += -L$$(PROGRAMFILES)/fftw-3.3.5-dll64

# OMP
unix: QMAKE_CXXFLAGS += -fopenmp
unix:!macx: QMAKE_LFLAGS += -fopenmp
macx: LIBS += -lomp

# Automatic change for each lib path (for building bundle)
#macx: QMAKE_LFLAGS_SONAME  = -Wl,-install_name,@executable_path/../Frameworks/
macx: QMAKE_LFLAGS_SONAME  += -Wl,-rpath,@executable_path/../Frameworks/

# Modify library ELF header
# Set RPATH to target the current folder (where the plugin lives)
# $ORIGIN is not an environment variable but a linker variable
unix:!macx: QMAKE_LFLAGS_RPATH  =
unix:!macx: QMAKE_LFLAGS  = -Wl,-rpath,\'\$$ORIGIN\'

# DEPLOYMENT
makeDeploy.path = $$DESTDIR

macx {
TARGET_LIB = $$DESTDIR/$${QMAKE_PREFIX_SHLIB}$${TARGET}.$${QMAKE_EXTENSION_SHLIB}
QTDIR = /usr/local/opt/qt/lib/
# QtWidgets
QTLIB = QtWidgets.framework/Versions/5/QtWidgets
makeDeploy.commands += $$changePath($$QTDIR,$$QTLIB,$$TARGET_LIB)
# QtGui
QTLIB = QtGui.framework/Versions/5/QtGui
makeDeploy.commands += $$changePath($$QTDIR,$$QTLIB,$$TARGET_LIB)
# QtCore
QTLIB = QtCore.framework/Versions/5/QtCore
makeDeploy.commands += $$changePath($$QTDIR,$$QTLIB,$$TARGET_LIB)
# QtSql
QTLIB = QtSql.framework/Versions/5/QtSql
makeDeploy.commands += $$changePath($$QTDIR,$$QTLIB,$$TARGET_LIB)
}

#Plugin icons
iconInstall.path = $$DESTDIR/Icon
iconInstall.files = Images/icon.png
INSTALLS += iconInstall

# We prefix the report file name with the project file base name to prevent name collisions.
#VERA_TARGET = $$top_srcdir/Build/code_analysis/$$basename(_PRO_FILE_).vera++.xml
#vera++.commands = vera++ --checkstyle-report $$VERA_TARGET --show-rule $$absolute_paths($$HEADERS) $$absolute_paths($$SOURCES)
QMAKE_CLEAN += $$VERA_TARGET
QMAKE_EXTRA_TARGETS += vera++
