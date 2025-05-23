
#ifndef VIDEOPLAYERMANAGER_H
#define VIDEOPLAYERMANAGER_H

#include <QObject>
#include <QStringListModel> // For video file list display
#include <QMediaPlayer>     // For actual video playback (Qt Multimedia module)
#include <QUrl>             // For specifying media URLs
#include <QList>            // For storing video file info
#include <QPair>            // For (Title, URL) pairs
#include <QQmlEngine>       // For QML_ELEMENT

// Manages video file listing and playback
class VideoPlayerManager : public QObject {
    Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_ELEMENT
#endif

    // Properties exposed to QML for UI binding and control
    Q_PROPERTY(QStringListModel* videoListModel READ videoListModel CONSTANT)
    Q_PROPERTY(State currentState READ currentState NOTIFY stateChanged) // UI state
    Q_PROPERTY(QString currentVideoTitle READ currentVideoTitle NOTIFY currentVideoChanged)
    Q_PROPERTY(QUrl currentVideoUrl READ currentVideoUrl NOTIFY currentVideoChanged) // For info, not direct playback control by QML
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged) // Current playback position
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged) // Total duration of current video
    Q_PROPERTY(QMediaPlayer::PlaybackState playbackState READ playbackState NOTIFY playbackStateChanged) // From QMediaPlayer
    Q_PROPERTY(QObject* mediaPlayerObject READ mediaPlayerObject CONSTANT) // Exposes QMediaPlayer to QML VideoOutput

public:
    // UI states for the video player feature
    enum class State { LISTING, PLAYING, PAUSED, STOPPED, ERROR }; // Added ERROR state
    Q_ENUM(State)

    // Events (mostly user actions from QML)
    // Note: QMediaPlayer has its own state machine; these are more for UI flow control.
    enum class Event {
        SELECT_VIDEO_FROM_LIST, // User selects a video from the list to play
        USER_PLAY,              // User presses play button
        USER_PAUSE,             // User presses pause button
        USER_STOP,              // User presses stop button
        USER_SEEK               // User interacts with seek slider
    };
    Q_ENUM(Event)

    explicit VideoPlayerManager(QObject *parent = nullptr);
    ~VideoPlayerManager();

    // Accessors for Q_PROPERTY
    QStringListModel* videoListModel() const { return m_videoListModel; }
    State currentState() const { return m_currentState; }
    QString currentVideoTitle() const { return m_currentVideoTitle; }
    QUrl currentVideoUrl() const { return m_currentVideoUrl; }
    qint64 position() const;
    qint64 duration() const;
    QMediaPlayer::PlaybackState playbackState() const;
    QObject* mediaPlayerObject() const { return m_mediaPlayer; } // Expose QMediaPlayer

    // Q_INVOKABLE methods for QML to control playback
    Q_INVOKABLE void processUiEvent(Event event, const QVariantMap& data = {}); // Renamed to avoid confusion
    Q_INVOKABLE void playFileByIndex(int index); // Convenience for QML ListView
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void seek(qint64 position);

signals:
    void stateChanged();         // Emitted when UI state (LISTING, PLAYING, etc.) changes
    void currentVideoChanged();  // Emitted when the current video title/URL changes
    // Signals directly re-emitted or derived from QMediaPlayer
    void positionChanged();
    void durationChanged();
    void playbackStateChanged(); // Reflects QMediaPlayer's state

private slots:
    // Slots to connect to QMediaPlayer signals
    void onMediaPlayerPositionChanged(qint64 pos);
    void onMediaPlayerDurationChanged(qint64 dur);
    void onMediaPlayerPlaybackStateChanged(QMediaPlayer::PlaybackState qmpState);
    void onMediaPlayerError(); // Simplified error signal
    void onMediaPlayerSourceChanged();


private:
    void initializeVideoFiles(); // Scans for or loads video file list
    void setUiState(State newState); // Sets the internal UI state

    State m_currentState;                       // Current UI state
    QStringListModel* m_videoListModel;       // Model for QML ListView
    QList<QPair<QString, QUrl>> m_videoFilesData; // Internal storage for (Title, URL)
    int m_currentVideoIndex;                    // Index of the currently loaded video

    QString m_currentVideoTitle;                // Title of the current video
    QUrl m_currentVideoUrl;                     // URL of the current video

    QMediaPlayer* m_mediaPlayer;                // The core Qt Multimedia player object
};

#endif // VIDEOPLAYERMANAGER_H