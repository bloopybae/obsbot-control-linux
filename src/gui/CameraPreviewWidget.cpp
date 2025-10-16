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
    , m_groupBox(nullptr)
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

    m_groupBox = new QGroupBox("Camera Preview", this);
    QVBoxLayout *groupLayout = new QVBoxLayout(m_groupBox);

    // Toggle button
    QHBoxLayout *controlLayout = new QHBoxLayout();
    m_toggleButton = new QPushButton("Enable Preview", this);
    m_toggleButton->setCheckable(true);
    connect(m_toggleButton, &QPushButton::clicked, this, &CameraPreviewWidget::onTogglePreview);
    controlLayout->addWidget(m_toggleButton);
    controlLayout->addStretch();
    groupLayout->addLayout(controlLayout);

    // Video widget
    m_videoWidget = new QVideoWidget(this);
    m_videoWidget->setMinimumSize(320, 240);
    m_videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_videoWidget->setVisible(false);
    groupLayout->addWidget(m_videoWidget);

    layout->addWidget(m_groupBox);
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

    m_videoWidget->setVisible(true);
    m_previewEnabled = true;
    m_toggleButton->setText("Disable Preview");

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

    m_videoWidget->setVisible(false);
    m_previewEnabled = false;
    m_toggleButton->setText("Enable Preview");
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
