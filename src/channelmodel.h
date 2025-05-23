
#ifndef CHANNELMODEL_H
#define CHANNELMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QString>
#include <QQmlEngine> // For QML_ELEMENT

// Structure to hold data for a single channel
struct Channel {
    int id;         // Unique identifier for the channel
    QString name;   // Display name of the channel
    bool isOn;      // Current state (on/off)
};

// Model for managing a list of channels, derived from QAbstractListModel
class ChannelModel : public QAbstractListModel {
    Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_ELEMENT // Makes this type usable in QML (e.g., for type hints or direct instantiation)
#endif

public:
    // Roles for accessing data items in the model from QML
    enum ChannelRoles {
        IdRole = Qt::UserRole + 1, // Start custom roles after Qt::UserRole
        NameRole,
        IsOnRole
    };

    explicit ChannelModel(QObject *parent = nullptr);

    // Method to add a new channel to the model
    void addChannel(const Channel &channel);

    // --- QAbstractListModel overrides ---
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QHash<int, QByteArray> roleNames() const override; // Maps roles to names for QML access

    // Method callable from QML to toggle a channel's state
    Q_INVOKABLE void toggleChannelViaModel(int channelId);

    // Method called by C++ (e.g., MainController) to update a channel's state from hardware
    void updateChannelState(int id, bool isOn);

signals:
    // Signal emitted when QML (via setData or toggleChannelViaModel) requests a hardware state change
    void commandToggleChannel(int channelId, bool newState);

private:
    QList<Channel> m_channels; // Internal list storing channel data
};

#endif // CHANNELMODEL_H