
#include "hardwaremanager.h"
#include <QDebug>
#include <QRandomGenerator> // For generating random data in simulation

// Constructor: Initializes simulation timer or serial port
HardwareManager::HardwareManager(QObject *parent) : QObject(parent) {
    m_simulationTimer = new QTimer(this);
    connect(m_simulationTimer, &QTimer::timeout, this, &HardwareManager::simulateChannelChange);

    // --- Example QSerialPort setup (if you were using it) ---
    // m_serialPort = new QSerialPort(this);
    // connect(m_serialPort, &QSerialPort::readyRead, this, &HardwareManager::onSerialPortReadyRead);
    // connect(m_serialPort, &QSerialPort::errorOccurred, this, &HardwareManager::onSerialPortError);
    // m_serialPort->setPortName("COM3"); // Or "/dev/ttyUSB0", etc.
    // m_serialPort->setBaudRate(QSerialPort::Baud115200);
    // if (m_serialPort->open(QIODevice::ReadWrite)) {
    //     qDebug() << "HardwareManager: Serial port opened successfully.";
    // } else {
    //     qWarning() << "HardwareManager: Failed to open serial port:" << m_serialPort->errorString();
    // }
    qDebug() << "HardwareManager initialized.";
}

// Destructor: Cleans up resources (e.g., closes serial port)
HardwareManager::~HardwareManager() {
    // if (m_serialPort && m_serialPort->isOpen()) {
    //     m_serialPort->close();
    // }
    qDebug() << "HardwareManager destroyed.";
}

// Sends a command to the hardware to set a channel's state
void HardwareManager::setHardwareChannelState(int channelId, bool isOn) {
    qDebug() << "HardwareManager: Sending command to STM32 -> Set Channel" << channelId << "to" << (isOn ? "ON" : "OFF");

    // --- Actual hardware command sending (example) ---
    // QByteArray command = QString("SET CH%1 %2\n").arg(channelId).arg(isOn ? 1 : 0).toUtf8();
    // if (m_serialPort && m_serialPort->isOpen() && m_serialPort->isWritable()) {
    //     m_serialPort->write(command);
    // } else {
    //     qWarning() << "HardwareManager: Cannot send command, serial port not open/writable.";
    // }

    // For simulation: Assume the command is successful and the hardware will confirm.
    // The confirmation will be simulated by `simulateChannelChange` or a direct emit here.
    // To make it more realistic, the hardware would send a response that gets parsed.
    // Here, we'll just simulate the hardware confirming this specific change after a short delay.
    QTimer::singleShot(100 + QRandomGenerator::global()->bounded(200), [this, channelId, isOn]() { // Simulate slight variable delay
        qDebug() << "HardwareManager: STM32 >> Confirmed channel" << channelId << "set to" << isOn;
        emit channelStateFromHardware(channelId, isOn);
    });
}

// Starts the simulation timer
void HardwareManager::startSimulation() {
    if (!m_simulationTimer->isActive()) {
        m_simulationTimer->start(7000); // Simulate a random event every 7 seconds
        qDebug() << "HardwareManager: Simulation started. Events every 7s.";
    }
}

// Simulates a change originating from the hardware
void HardwareManager::simulateChannelChange() {
    // Simulate an external event (e.g., a physical button press on the device)
    int randomChannel = QRandomGenerator::global()->bounded(0, 16); // Assuming 0-15 channels
    bool newState = QRandomGenerator::global()->bounded(0, 2) == 1; // Random true/false

    qDebug() << "HardwareManager: STM32 >> Simulated external event on channel" << randomChannel << "changed to" << newState;
    emit channelStateFromHardware(randomChannel, newState); // Emit signal as if hardware sent this update
}

/*
// --- Example QSerialPort slot implementations ---
void HardwareManager::onSerialPortReadyRead() {
    m_readBuffer.append(m_serialPort->readAll());
    // Process buffer for complete messages, then call parseReceivedData
    // For example, if messages are newline-terminated:
    while (m_readBuffer.contains('\n')) {
        int EOL_idx = m_readBuffer.indexOf('\n');
        QByteArray message = m_readBuffer.left(EOL_idx + 1);
        m_readBuffer.remove(0, EOL_idx + 1);
        parseReceivedData(message.trimmed());
    }
}

void HardwareManager::parseReceivedData(const QByteArray& data) {
    qDebug() << "HardwareManager: Received from STM32 <<" << data;
    // Example: "STATUS CH5 1" (Channel 5 is ON)
    // Example: "INCOMING_CALL 123456789"
    QString dataStr = QString::fromUtf8(data);
    if (dataStr.startsWith("STATUS CH")) {
        QStringList parts = dataStr.split(' ');
        if (parts.size() == 3) {
            bool okId, okState;
            int id = parts[1].mid(2).toInt(&okId); // "CH5" -> "5"
            int state = parts[2].toInt(&okState);
            if (okId && okState) {
                emit channelStateFromHardware(id, static_cast<bool>(state));
            }
        }
    } else if (dataStr.startsWith("INCOMING_CALL")) {
        // emit incomingCallDetected(dataStr.mid(14).trimmed());
    }
    // Add more parsing rules for other hardware messages
}

void HardwareManager::onSerialPortError(QSerialPort::SerialPortError error) {
    if (error != QSerialPort::NoError && error != QSerialPort::TimeoutError) { // TimeoutError can be normal
        qWarning() << "HardwareManager: Serial port error:" << m_serialPort->errorString();
    }
}
*/