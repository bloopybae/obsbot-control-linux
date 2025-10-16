#include "CameraPreviewWidget.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QMediaDevices>

CameraPreviewWidget::CameraPreviewWidget(QWidget *parent)
    : QWidget(parent)
    , m_camera(nullptr)
    , m_captureSession(nullptr)
    , m_videoWidget(nullptr)
    , m_previewEnabled(false)
{
    setupUI();
}

CameraPreviewWidget::~CameraPreviewWidget()
{
    stopPreview();
}

void CameraPreviewWidget::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Video widget fills entire preview area
    m_videoWidget = new QVideoWidget(this);
    m_videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_videoWidget->setStyleSheet("background-color: black;");
    layout->addWidget(m_videoWidget);
}

bool CameraPreviewWidget::isPreviewEnabled() const
{
    return m_previewEnabled;
}

void CameraPreviewWidget::enablePreview(bool enabled)
{
    if (enabled == m_previewEnabled) {
        return;
    }

    if (enabled) {
        startPreview();
    } else {
        stopPreview();
    }
}

void CameraPreviewWidget::startPreview()
{
    if (m_previewEnabled) {
        return;
    }

    // Find OBSBOT camera
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    QCameraDevice selectedCamera;

    for (const QCameraDevice &cameraDevice : cameras) {
        if (cameraDevice.description().contains("OBSBOT", Qt::CaseInsensitive) ||
            cameraDevice.description().contains("Meet", Qt::CaseInsensitive)) {
            selectedCamera = cameraDevice;
            break;
        }
    }

    if (selectedCamera.isNull() && !cameras.isEmpty()) {
        // Fallback to first camera
        selectedCamera = cameras.first();
    }

    if (selectedCamera.isNull()) {
        emit previewFailed("No camera found");
        return;
    }

    // Create camera
    m_camera = new QCamera(selectedCamera, this);
    connect(m_camera, &QCamera::errorOccurred, this, &CameraPreviewWidget::onCameraError);

    // Create capture session
    m_captureSession = new QMediaCaptureSession(this);
    m_captureSession->setCamera(m_camera);
    m_captureSession->setVideoOutput(m_videoWidget);

    // Start camera
    m_camera->start();

    // Check if camera actually started
    if (m_camera->error() != QCamera::NoError) {
        emit previewFailed(m_camera->errorString());
        stopPreview();
        return;
    }

    m_previewEnabled = true;

    // Get camera resolution to calculate aspect ratio
    QCameraFormat format = m_camera->cameraFormat();
    QSize resolution = format.resolution();
    if (resolution.width() > 0 && resolution.height() > 0) {
        double aspectRatio = static_cast<double>(resolution.width()) / static_cast<double>(resolution.height());
        emit aspectRatioChanged(aspectRatio);
    } else {
        // Fallback to 16:9 if we can't get resolution
        emit aspectRatioChanged(16.0 / 9.0);
    }

    emit previewStateChanged(true);
    emit previewStarted();
}

void CameraPreviewWidget::stopPreview()
{
    if (!m_previewEnabled) {
        return;
    }

    if (m_camera) {
        m_camera->stop();
        delete m_camera;
        m_camera = nullptr;
    }

    if (m_captureSession) {
        delete m_captureSession;
        m_captureSession = nullptr;
    }

    m_previewEnabled = false;
    emit previewStateChanged(false);
}

void CameraPreviewWidget::onCameraError(QCamera::Error error)
{
    if (error != QCamera::NoError && m_camera) {
        emit previewFailed(m_camera->errorString());
        stopPreview();
    }
}
