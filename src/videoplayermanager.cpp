
#include "videoplayermanager.h"
#include <QDebug>
#include <QDir>               // For directory scanning
#include <QStandardPaths>     // For finding standard Movies location
#include <QFileInfo>          // For file information
#include <QTimer>          // For file information

// Constructor: Initializes models, media player, and video file list
VideoPlayerManager::VideoPlayerManager(QObject *parent)
    : QObject(parent),
      m_currentState(State::LISTING), // Default UI state
      m_currentVideoIndex(-1) {
    m_videoListModel = new QStringListModel(this);
    m_mediaPlayer = new QMediaPlayer(this); // Instantiate the media player

    // Connect QMediaPlayer signals to internal slots or directly to manager's signals
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &VideoPlayerManager::onMediaPlayerPositionChanged);
    connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, &VideoPlayerManager::onMediaPlayerDurationChanged);
    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged, this, &VideoPlayerManager::onMediaPlayerPlaybackStateChanged);
    connect(m_mediaPlayer, &QMediaPlayer::errorOccurred, this, &VideoPlayerManager::onMediaPlayerError); // Qt 6
    // connect(m_mediaPlayer, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), this, &VideoPlayerManager::onMediaPlayerError); // Qt 5
    connect(m_mediaPlayer, &QMediaPlayer::sourceChanged, this, &VideoPlayerManager::onMediaPlayerSourceChanged);


    initializeVideoFiles(); // Load the list of available video files
    qDebug() << "VideoPlayerManager initialized.";
}

VideoPlayerManager::~VideoPlayerManager() {
    qDebug() << "VideoPlayerManager destroyed.";
    // m_mediaPlayer is child of 'this', Qt handles deletion.
}

// Scans for video files or loads a predefined list
void VideoPlayerManager::initializeVideoFiles() {
    // Attempt to find videos in the standard "Movies" directory
    QStringList videoLocations = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation);
    if (!videoLocations.isEmpty()) {
        QDir moviesDir(videoLocations.first());
        QStringList nameFilters;
        nameFilters << "*.mp4" << "*.mkv" << "*.avi" << "*.mov" << "*.webm"; // Common video formats
        QFileInfoList foundFiles = moviesDir.entryInfoList(nameFilters, QDir::Files | QDir::Readable);
        for (const QFileInfo &fileInfo : foundFiles) {
            m_videoFilesData.append({fileInfo.baseName(), QUrl::fromLocalFile(fileInfo.absoluteFilePath())});
        }
    }

    // Add some fallback/sample URLs if no local files were found
    if (m_videoFilesData.isEmpty()) {
        qDebug() << "VideoPlayerManager: No local video files found in Movies directory. Using sample URLs.";
        // These are common test video URLs. Ensure network access if using these.
        m_videoFilesData.append({"Big Buck Bunny (Online)", QUrl("http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4")});
        m_videoFilesData.append({"Elephants Dream (Online)", QUrl("http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/ElephantsDream.mp4")});
    }

    // Populate the QStringListModel for QML
    QStringList titles;
    for(const auto& videoPair : m_videoFilesData) {
        titles.append(videoPair.first);
    }
    m_videoListModel->setStringList(titles);

    if (m_videoFilesData.isEmpty()) {
        qWarning() << "VideoPlayerManager: No video files available (local or sample). Player will be limited.";
    }
}

// Sets the internal UI state and emits a signal
void VideoPlayerManager::setUiState(State newState) {
    if (m_currentState != newState) {
        m_currentState = newState;
        emit stateChanged();
        qDebug() << "VideoPlayerManager: UI State changed to" << static_cast<int>(m_currentState);
    }
}

// Processes UI events from QML
void VideoPlayerManager::processUiEvent(Event event, const QVariantMap& data) {
    qDebug() << "VideoPlayerManager: Processing UI event" << static_cast<int>(event) << "with data" << data;
    switch(event) {
        case Event::SELECT_VIDEO_FROM_LIST:
            playFileByIndex(data.value("index", -1).toInt());
            break;
        case Event::USER_PLAY:
            play();
            break;
        case Event::USER_PAUSE:
            pause();
            break;
        case Event::USER_STOP:
            stop();
            break;
        case Event::USER_SEEK:
            seek(data.value("position", 0LL).toLongLong()); // Use 0LL for qint64 literal
            break;
    }
}

// Plays a video file based on its index in the list
void VideoPlayerManager::playFileByIndex(int index) {
    if (index >= 0 && index < m_videoFilesData.size()) {
        m_currentVideoIndex = index;
        const auto& videoPair = m_videoFilesData[index];
        if (m_currentVideoTitle != videoPair.first || m_currentVideoUrl != videoPair.second) {
             m_currentVideoTitle = videoPair.first;
             m_currentVideoUrl = videoPair.second;
             emit currentVideoChanged(); // Notify QML about the new video
        }
        qDebug() << "VideoPlayerManager: Attempting to play" << m_currentVideoTitle << "from" << m_currentVideoUrl;
        m_mediaPlayer->setSource(m_currentVideoUrl); // Set the media source
        m_mediaPlayer->play();                       // Start playback
        // UI state will be updated via onMediaPlayerPlaybackStateChanged
    } else {
        qWarning() << "VideoPlayerManager: Invalid index" << index << "for playFileByIndex.";
        m_mediaPlayer->stop(); // Stop if anything was playing
        if (m_currentVideoTitle != "" || !m_currentVideoUrl.isEmpty()) {
            m_currentVideoTitle = "";
            m_currentVideoUrl = QUrl();
            emit currentVideoChanged();
        }
        setUiState(State::ERROR);
    }
}

// Starts or resumes playback
void VideoPlayerManager::play() {
    if (m_mediaPlayer->source().isEmpty() && m_currentVideoIndex != -1) {
    // if (m_mediaPlayer->media().isNull() && m_currentVideoIndex != -1) { // Qt 5
        // If no media is set but an index was previously selected, reload and play.
        playFileByIndex(m_currentVideoIndex);
    } else if (!m_mediaPlayer->source().isEmpty()) {
    // } else if (!m_mediaPlayer->media().isNull()) { // Qt 5
        m_mediaPlayer->play();
    } else {
        qDebug() << "VideoPlayerManager: Play called but no source/index set.";
    }
}

// Pauses playback
void VideoPlayerManager::pause() {
    m_mediaPlayer->pause();
}

// Stops playback
void VideoPlayerManager::stop() {
    m_mediaPlayer->stop();
    // QMediaPlayer goes to StoppedState, which will update UI state via slot.
    // Optionally, clear current video info here if desired behavior is to "unload"
    // m_currentVideoTitle = "";
    // m_currentVideoUrl = QUrl();
    // m_currentVideoIndex = -1;
    // emit currentVideoChanged();
}

// Seeks to a specific position in the video
void VideoPlayerManager::seek(qint64 position) {
    if (m_mediaPlayer->isSeekable()) {
        m_mediaPlayer->setPosition(position);
    } else {
        qDebug() << "VideoPlayerManager: Media is not seekable.";
    }
}

// --- Accessors for QMediaPlayer properties ---
qint64 VideoPlayerManager::position() const { return m_mediaPlayer->position(); }
qint64 VideoPlayerManager::duration() const { return m_mediaPlayer->duration(); }
QMediaPlayer::PlaybackState VideoPlayerManager::playbackState() const { return m_mediaPlayer->playbackState(); }

// --- Slots for QMediaPlayer signals ---
void VideoPlayerManager::onMediaPlayerPositionChanged(qint64 pos) {
    emit positionChanged(); // Re-emit for QML
}

void VideoPlayerManager::onMediaPlayerDurationChanged(qint64 dur) {
    emit durationChanged(); // Re-emit for QML
}

void VideoPlayerManager::onMediaPlayerPlaybackStateChanged(QMediaPlayer::PlaybackState qmpState) {
    emit playbackStateChanged(); // Re-emit for QML
    // Update internal UI state based on QMediaPlayer's state
    switch (qmpState) {
        case QMediaPlayer::PlayingState:
            setUiState(State::PLAYING);
            break;
        case QMediaPlayer::PausedState:
            setUiState(State::PAUSED);
            break;
        case QMediaPlayer::StoppedState:
            setUiState(State::STOPPED);
            // When playback stops (e.g. end of media), reset position for QML slider
            if (m_mediaPlayer->position() >= m_mediaPlayer->duration() && m_mediaPlayer->duration() > 0) {
                 QTimer::singleShot(0, this, [this](){ emit positionChanged(); }); // force update to 0 or duration
            }
            break;
    }
    qDebug() << "VideoPlayerManager: QMediaPlayer state changed to" << qmpState;
}

void VideoPlayerManager::onMediaPlayerError() {
    qWarning() << "VideoPlayerManager: QMediaPlayer Error -" << m_mediaPlayer->errorString();
    setUiState(State::ERROR);
    // Update QML display about the error
    if (m_currentVideoTitle != "Error" || !m_currentVideoUrl.isEmpty()) {
        m_currentVideoTitle = "Error";
        m_currentVideoUrl = QUrl(); // Clear URL on error
        emit currentVideoChanged();
    }
}
void VideoPlayerManager::onMediaPlayerSourceChanged() {
    qDebug() << "VideoPlayerManager: QMediaPlayer source changed to:" << m_mediaPlayer->source();
    // Duration might not be available immediately after source change.
    // It will be updated via onMediaPlayerDurationChanged when available.
    // Reset position and duration for QML.
    emit durationChanged();
    emit positionChanged();
}