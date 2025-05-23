
#ifndef HARDWAREMANAGER_H
#define HARDWAREMANAGER_H

#include <QObject>
#include <QTimer>       // For simulation purposes
// #include <QSerialPort>  // For actual USB serial communication (if used)
#include <QQmlEngine>   // For QML_ELEMENT

// Manages communication with the physical hardware (e.g., STM32)
class HardwareManager : public QObject {
    Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_ELEMENT
#endif

public:
    explicit HardwareManager(QObject *parent = nullptr);
    ~HardwareManager();

    // Method callable from C++ (e.g., MainController via ChannelModel signal)
    // to send a command to change a hardware channel's state.
    Q_INVOKABLE void setHardwareChannelState(int channelId, bool isOn);

    // Starts the hardware simulation (for demonstration without actual hardware)
    void startSimulation();

signals:
    // Signal emitted when the hardware reports a change in a channel's state
    void channelStateFromHardware(int channelId, bool isOn);

    // Other signals for different hardware events could be added here:
    // void incomingCallDetected(const QString& callerId);
    // void messageReceivedFromHardware(const QByteArray& messageData);

private slots:
    // Slot for the simulation timer to trigger a simulated channel change
    void simulateChannelChange();

    // Slots for actual QSerialPort communication (if implemented)
    // void onSerialPortReadyRead();
    // void onSerialPortError(QSerialPort::SerialPortError error);

private:
    QTimer* m_simulationTimer; // Timer for driving simulations
    // QSerialPort* m_serialPort; // Serial port object for communication
    // QByteArray m_readBuffer;   // Buffer for incoming serial data
    // void parseReceivedData(const QByteArray& data); // Parses data from hardware
};

#endif // HARDWAREMANAGER_H