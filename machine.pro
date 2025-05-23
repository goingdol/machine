#  QT += quick
#
#  # You can make your code fail to compile if it uses deprecated APIs.
#  # In order to do so, uncomment the following line.
#  #DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
#
#  SOURCES += \
#          main.cpp
#
#  RESOURCES += qml.qrc
#
#  # Additional import path used to resolve QML modules in Qt Creator's code model
#  QML_IMPORT_PATH =
#
#  # Additional import path used to resolve QML modules just for Qt Quick Designer
#  QML_DESIGNER_IMPORT_PATH =
#
#  # Default rules for deployment.
#  qnx: target.path = /tmp/$${TARGET}/bin
#  else: unix:!android: target.path = /opt/$${TARGET}/bin
#  !isEmpty(target.path): INSTALLS += target

# QT += quick qml core serialport multimedia widgets # widgets for QTimer, multimedia for QMediaPlayer
QT += quick qml core multimedia widgets # widgets for QTimer, multimedia for QMediaPlayer

CONFIG += c++17
CONFIG += console
CONFIG += qml_debug

TARGET = machine
TEMPLATE = app

# Define source and header directories
SOURCES_DIR = src
HEADERS_DIR = src

# C++ Sources
SOURCES += \
    $$SOURCES_DIR/main.cpp \
    $$SOURCES_DIR/maincontroller.cpp \
    $$SOURCES_DIR/channelmodel.cpp \
    $$SOURCES_DIR/hardwaremanager.cpp \
    $$SOURCES_DIR/callmanager.cpp \
    $$SOURCES_DIR/messagemanager.cpp \
    $$SOURCES_DIR/cameramanager.cpp \
    $$SOURCES_DIR/videoplayermanager.cpp

# C++ Headers
HEADERS += \
    $$HEADERS_DIR/maincontroller.h \
    $$HEADERS_DIR/channelmodel.h \
    $$HEADERS_DIR/hardwaremanager.h \
    $$HEADERS_DIR/callmanager.h \
    $$HEADERS_DIR/messagemanager.h \
    $$HEADERS_DIR/cameramanager.h \
    $$HEADERS_DIR/videoplayermanager.h

# QML files and resources
RESOURCES += qml.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Ensure moc runs on headers if not using QML_ELEMENT for everything
# For Qt 6 with QML_ELEMENT, this is often handled automatically.
# QMAKE_AUTOMOC_RECURSIVE = ON # Usually not needed for modern Qt if headers are correctly included.

DISTFILES += \
    qml/main.qml \
    qml/HomeView.qml \
    qml/ChannelSwitchesView.qml \
    qml/CallView.qml \
    qml/CallLineDisplay.qml \
    qml/MessageView.qml \
    qml/CameraView.qml \
    qml/VideoPlayerView.qml
