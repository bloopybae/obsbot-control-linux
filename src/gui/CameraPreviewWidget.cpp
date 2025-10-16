#include "CameraPreviewWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMediaDevices>

CameraPreviewWidget::CameraPreviewWidget(QWidget *parent)
    : QWidget(parent)
    , m_camera(nullptr)
    , m_captureSession(nullptr)
    , m_videoWidget(nullptr)
    , m_toggleButton(nullptr)
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
    layout->setContentsMargins(10, 10, 0, 10);  // No margin on right (sidebar border)
    layout->setSpacing(10);

    // Toggle button at top
    QHBoxLayout *controlLayout = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("Camera Preview", this);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 12pt;");
    controlLayout->addWidget(titleLabel);
    controlLayout->addStretch();

    m_toggleButton = new QPushButton("Enable", this);
    m_toggleButton->setCheckable(true);
    m_toggleButton->setMaximumWidth(100);
    connect(m_toggleButton, &QPushButton::clicked, this, &CameraPreviewWidget::onTogglePreview);
    controlLayout->addWidget(m_toggleButton);
    layout->addLayout(controlLayout);

    // Video widget fills remaining space
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

    m_toggleButton->setChecked(enabled);
    onTogglePreview();
}

void CameraPreviewWidget::onTogglePreview()
{
    bool shouldEnable = m_toggleButton->isChecked();

    if (shouldEnable) {
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

    // Find OBSBOT Meet 2 camera
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
        QMessageBox::warning(this, "Camera Preview",
            "No camera found. Please ensure the OBSBOT Meet 2 is connected.");
        m_toggleButton->setChecked(false);
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

    m_previewEnabled = true;
    m_toggleButton->setText("Disable");

    emit previewStateChanged(true);
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
    m_toggleButton->setText("Enable");
    m_toggleButton->setChecked(false);

    emit previewStateChanged(false);
}

void CameraPreviewWidget::onCameraError(QCamera::Error error)
{
    if (error != QCamera::NoError && m_camera) {
        QMessageBox::warning(this, "Camera Error",
            QString("Camera error: %1").arg(m_camera->errorString()));
        stopPreview();
    }
}
