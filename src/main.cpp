#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "maincontroller.h"
#include "callmanager.h"
#include "messagemanager.h"
#include "cameramanager.h"
#include "videoplayermanager.h"
#include "channelmodel.h"
#include <QTimer>
// Other managers are accessed via MainController

int main(int argc, char *argv[]) {
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // Register Enums for QML access.
    // This makes enums like MainController.AppMode usable in QML.
    // For Qt 6.2+ with QML_NAMED_ELEMENT and QML_ENUM_CLASS, this might be simpler.
    // Using uncreatable metatype for enums is a common cross-version approach.

    qmlRegisterUncreatableMetaObject(
        MainController::staticMetaObject,
        "com.company.enums", 1, 0, "MainControllerEnums", "Error: Only enums from MainController");
    qmlRegisterUncreatableMetaObject(
        CallManager::staticMetaObject,
        "com.company.enums", 1, 0, "CallManagerEnums", "Error: Only enums from CallManager");
    qmlRegisterUncreatableMetaObject(
        MessageManager::staticMetaObject,
        "com.company.enums", 1, 0, "MessageManagerEnums", "Error: Only enums from MessageManager");
     qmlRegisterUncreatableMetaObject(
        CameraManager::staticMetaObject,
        "com.company.enums", 1, 0, "CameraManagerEnums", "Error: Only enums from CameraManager");
    qmlRegisterUncreatableMetaObject(
        VideoPlayerManager::staticMetaObject,
        "com.company.enums", 1, 0, "VideoPlayerManagerEnums", "Error: Only enums from VideoPlayerManager");


    MainController mainController; // Create the main C++ controller

    // Expose C++ objects to QML
    engine.rootContext()->setContextProperty("mainController", &mainController);
    engine.rootContext()->setContextProperty("channelModel", mainController.channelModel());
    engine.rootContext()->setContextProperty("callManager", mainController.callManager());
    engine.rootContext()->setContextProperty("messageManager", mainController.messageManager());
    engine.rootContext()->setContextProperty("cameraManager", mainController.cameraManager());
    engine.rootContext()->setContextProperty("videoPlayerManager", mainController.videoPlayerManager());


    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1); // Exit if main QML file fails to load
        }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}