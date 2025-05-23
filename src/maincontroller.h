#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QQmlEngine> // For QML_ELEMENT (Qt 6) or manual registration

#include "channelmodel.h"
#include "callmanager.h"
#include "messagemanager.h"
#include "cameramanager.h"
#include "videoplayermanager.h"

// Forward declarations to avoid circular dependencies in headers
class HardwareManager;

class MainController : public QObject {
    Q_OBJECT
    // For Qt 6, QML_ELEMENT makes the type usable in QML if you were to instantiate it there.
    // Since we create it in C++ and set it as a context property, this is less critical
    // but good for general QML visibility of the type itself.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_ELEMENT
#endif

    // Properties exposed to QML
    Q_PROPERTY(AppMode currentMode READ currentMode WRITE setCurrentMode NOTIFY currentModeChanged)
    Q_PROPERTY(ChannelModel* channelModel READ channelModel CONSTANT) // Read-only pointer to the model
    Q_PROPERTY(CallManager* callManager READ callManager CONSTANT)
    Q_PROPERTY(MessageManager* messageManager READ messageManager CONSTANT)
    Q_PROPERTY(CameraManager* cameraManager READ cameraManager CONSTANT)
    Q_PROPERTY(VideoPlayerManager* videoPlayerManager READ videoPlayerManager CONSTANT)

public:
    // Enum for application modes, exposed to QML via registration in main.cpp
    enum class AppMode {
        Home,
        Call,
        Message,
        Camera,
        VideoPlayer
    };
    Q_ENUM(AppMode) // Makes this enum introspectable by Qt's meta-object system

    explicit MainController(QObject *parent = nullptr);
    ~MainController();

    // Read-only accessors for Q_PROPERTY
    AppMode currentMode() const;
    ChannelModel* channelModel() const;
    CallManager* callManager() const;
    MessageManager* messageManager() const;
    CameraManager* cameraManager() const;
    VideoPlayerManager* videoPlayerManager() const;

    // Writable accessor for Q_PROPERTY
    void setCurrentMode(AppMode mode);

    // Method callable from QML to change the application mode
    Q_INVOKABLE void selectMode(AppMode mode);

signals:
    // Signal emitted when the application mode changes
    void currentModeChanged(AppMode mode);

public slots:
    // Slot to handle hardware channel state changes
    void onHardwareChannelStateChanged(int channelId, bool isOn);

private:
    AppMode m_currentMode; // Current application mode

    // Owned instances of manager classes
    ChannelModel* m_channelModel;
    HardwareManager* m_hardwareManager; // Manages hardware interaction
    CallManager* m_callManager;
    MessageManager* m_messageManager;
    CameraManager* m_cameraManager;
    VideoPlayerManager* m_videoPlayerManager;
};

#endif // MAINCONTROLLER_H