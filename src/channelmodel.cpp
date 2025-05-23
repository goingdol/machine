
#include "channelmodel.h"
#include <QDebug> // For logging

// Constructor
ChannelModel::ChannelModel(QObject *parent) : QAbstractListModel(parent) {}

// Adds a new channel to the model and notifies views
void ChannelModel::addChannel(const Channel &channel) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount()); // Notify views about insertion
    m_channels.append(channel);
    endInsertRows(); // Finalize insertion
}

// Returns the number of rows (channels) in the model
int ChannelModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0; // Not a tree model
    return m_channels.count();
}

// Provides data for a given index and role
QVariant ChannelModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_channels.count() || index.row() < 0)
        return QVariant(); // Invalid index

    const Channel &channel = m_channels[index.row()];

    // Switch on the role to return the appropriate data field
    switch (static_cast<ChannelRoles>(role)) {
        case IdRole: return channel.id;
        case NameRole: return channel.name;
        case IsOnRole: return channel.isOn;
    }
    return QVariant(); // Default for unhandled roles
}

// Sets data for a given index and role (e.g., when QML Switch is toggled)
bool ChannelModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || index.row() >= m_channels.count() || index.row() < 0)
        return false;

    Channel &channel = m_channels[index.row()]; // Get a reference to modify
    bool changed = false;

    switch (static_cast<ChannelRoles>(role)) {
        case IsOnRole:
            if (channel.isOn != value.toBool()) {
                // This is an optimistic UI update triggered by QML.
                // The actual state confirmation should come from hardware.
                // We emit a command to the hardware here.
                channel.isOn = value.toBool(); // Optimistically update internal state
                changed = true;
                qDebug() << "ChannelModel: User toggled channel" << channel.id << "to" << channel.isOn << ". Emitting commandToggleChannel.";
                emit commandToggleChannel(channel.id, channel.isOn); // Signal to MainController/HardwareManager
            }
            break;
        // Other roles could be made editable if needed
        default:
            return false; // Role not editable or not handled
    }

    if (changed) {
        emit dataChanged(index, index, {role}); // Notify views of the data change
        return true;
    }
    return false;
}

// Returns item flags for a given index (e.g., editable)
Qt::ItemFlags ChannelModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    // Makes the IsOnRole editable via setData
    return Qt::ItemIsEditable | QAbstractListModel::flags(index);
}

// Maps role enums to string names for QML access (e.g., model.channelId)
QHash<int, QByteArray> ChannelModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "channelId";
    roles[NameRole] = "channelName";
    roles[IsOnRole] = "isOn";
    return roles;
}

// Q_INVOKABLE method for QML to easily toggle a channel
void ChannelModel::toggleChannelViaModel(int channelId) {
    for (int i = 0; i < m_channels.count(); ++i) {
        if (m_channels[i].id == channelId) {
            QModelIndex idx = index(i, 0);
            // Call setData, which handles the logic and emits commandToggleChannel
            setData(idx, !m_channels[i].isOn, IsOnRole);
            return;
        }
    }
    qWarning() << "ChannelModel: toggleChannelViaModel called for unknown channelId" << channelId;
}

// Updates channel state based on feedback from hardware (via MainController)
void ChannelModel::updateChannelState(int id, bool isOn) {
    for (int i = 0; i < m_channels.count(); ++i) {
        if (m_channels[i].id == id) {
            if (m_channels[i].isOn != isOn) { // Update only if state actually changed
                m_channels[i].isOn = isOn;    // Update internal state
                // Notify views that data for this channel's IsOnRole has changed
                emit dataChanged(index(i, 0), index(i, 0), {IsOnRole});
                qDebug() << "ChannelModel: Hardware confirmed channel" << id << "state to" << isOn;
            }
            return; // Found and processed the channel
        }
    }
    // Optionally handle case where ID is not found, though model should be in sync
    // qDebug() << "ChannelModel: updateChannelState called for unknown id" << id;
}