#include "CameraControlWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QThread>
#include <chrono>

CameraControlWindow::CameraControlWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_connected(false)
    , m_currentPan(0.0)
    , m_currentTilt(0.0)
    , m_currentZoom(1.0)
{
    setWindowTitle("OBSBOT Meet 2 Control");
    setupUI();
    connectToCamera();

    // Update status every 2 seconds
    m_statusTimer = new QTimer(this);
    connect(m_statusTimer, &QTimer::timeout, this, &CameraControlWindow::updateStatus);
    m_statusTimer->start(2000);
}

CameraControlWindow::~CameraControlWindow()
{
}

void CameraControlWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Device info
    m_deviceInfoLabel = new QLabel("Connecting to camera...", this);
    m_deviceInfoLabel->setStyleSheet("font-weight: bold; padding: 10px; background-color: #f0f0f0;");
    mainLayout->addWidget(m_deviceInfoLabel);

    // ===== Tracking Group =====
    QGroupBox *trackingGroup = new QGroupBox("Face Tracking", this);
    QVBoxLayout *trackingLayout = new QVBoxLayout(trackingGroup);

    m_trackingCheckBox = new QCheckBox("Enable Auto-Framing", this);
    connect(m_trackingCheckBox, &QCheckBox::toggled, this, &CameraControlWindow::onTrackingToggled);
    trackingLayout->addWidget(m_trackingCheckBox);

    mainLayout->addWidget(trackingGroup);

    // ===== PTZ Controls Group =====
    QGroupBox *ptzGroup = new QGroupBox("Pan/Tilt/Zoom Controls", this);
    QVBoxLayout *ptzLayout = new QVBoxLayout(ptzGroup);

    // Pan/Tilt buttons
    QGridLayout *panTiltGrid = new QGridLayout();

    m_tiltUpBtn = new QPushButton("↑ Tilt Up", this);
    m_tiltDownBtn = new QPushButton("↓ Tilt Down", this);
    m_panLeftBtn = new QPushButton("← Pan Left", this);
    m_panRightBtn = new QPushButton("→ Pan Right", this);
    m_centerBtn = new QPushButton("⊙ Center", this);

    panTiltGrid->addWidget(m_tiltUpBtn, 0, 1);
    panTiltGrid->addWidget(m_panLeftBtn, 1, 0);
    panTiltGrid->addWidget(m_centerBtn, 1, 1);
    panTiltGrid->addWidget(m_panRightBtn, 1, 2);
    panTiltGrid->addWidget(m_tiltDownBtn, 2, 1);

    connect(m_tiltUpBtn, &QPushButton::clicked, this, &CameraControlWindow::onTiltUpClicked);
    connect(m_tiltDownBtn, &QPushButton::clicked, this, &CameraControlWindow::onTiltDownClicked);
    connect(m_panLeftBtn, &QPushButton::clicked, this, &CameraControlWindow::onPanLeftClicked);
    connect(m_panRightBtn, &QPushButton::clicked, this, &CameraControlWindow::onPanRightClicked);
    connect(m_centerBtn, &QPushButton::clicked, this, &CameraControlWindow::onCenterClicked);

    ptzLayout->addLayout(panTiltGrid);

    // Position label
    m_positionLabel = new QLabel("Position: Pan 0.0, Tilt 0.0", this);
    ptzLayout->addWidget(m_positionLabel);

    // Zoom slider
    QHBoxLayout *zoomLayout = new QHBoxLayout();
    zoomLayout->addWidget(new QLabel("Zoom:", this));
    m_zoomSlider = new QSlider(Qt::Horizontal, this);
    m_zoomSlider->setMinimum(10);  // 1.0x
    m_zoomSlider->setMaximum(20);  // 2.0x
    m_zoomSlider->setValue(10);
    connect(m_zoomSlider, &QSlider::valueChanged, this, &CameraControlWindow::onZoomChanged);
    zoomLayout->addWidget(m_zoomSlider);
    m_zoomLabel = new QLabel("1.0x", this);
    zoomLayout->addWidget(m_zoomLabel);

    ptzLayout->addLayout(zoomLayout);

    mainLayout->addWidget(ptzGroup);

    // ===== Camera Settings Group =====
    QGroupBox *settingsGroup = new QGroupBox("Camera Settings", this);
    QVBoxLayout *settingsLayout = new QVBoxLayout(settingsGroup);

    // HDR
    m_hdrCheckBox = new QCheckBox("HDR", this);
    connect(m_hdrCheckBox, &QCheckBox::toggled, this, &CameraControlWindow::onHDRToggled);
    settingsLayout->addWidget(m_hdrCheckBox);

    // FOV
    QHBoxLayout *fovLayout = new QHBoxLayout();
    fovLayout->addWidget(new QLabel("Field of View:", this));
    m_fovComboBox = new QComboBox(this);
    m_fovComboBox->addItem("Wide (86°)", 0);
    m_fovComboBox->addItem("Medium (78°)", 1);
    m_fovComboBox->addItem("Narrow (65°)", 2);
    connect(m_fovComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CameraControlWindow::onFOVChanged);
    fovLayout->addWidget(m_fovComboBox);
    settingsLayout->addLayout(fovLayout);

    // Face AE
    m_faceAECheckBox = new QCheckBox("Face-based Auto Exposure", this);
    connect(m_faceAECheckBox, &QCheckBox::toggled, this, &CameraControlWindow::onFaceAEToggled);
    settingsLayout->addWidget(m_faceAECheckBox);

    // Face Focus
    m_faceFocusCheckBox = new QCheckBox("Face-based Auto Focus", this);
    connect(m_faceFocusCheckBox, &QCheckBox::toggled, this, &CameraControlWindow::onFaceFocusToggled);
    settingsLayout->addWidget(m_faceFocusCheckBox);

    mainLayout->addWidget(settingsGroup);

    // ===== Status =====
    m_statusLabel = new QLabel("Status: Initializing...", this);
    m_statusLabel->setStyleSheet("padding: 10px; background-color: #e0e0e0;");
    mainLayout->addWidget(m_statusLabel);

    mainLayout->addStretch();

    resize(500, 600);
}

void CameraControlWindow::connectToCamera()
{
    // Setup device detection callback
    auto onDevChanged = [this](std::string dev_sn, bool connected, void *param) {
        if (connected) {
            QMetaObject::invokeMethod(this, "onDeviceDetected", Qt::QueuedConnection);
        }
    };

    Devices::get().setDevChangedCallback(onDevChanged, nullptr);
    Devices::get().setEnableMdnsScan(false);

    // Wait for device
    QTimer::singleShot(5000, this, &CameraControlWindow::onDeviceDetected);
}

void CameraControlWindow::onDeviceDetected()
{
    auto dev_list = Devices::get().getDevList();
    if (dev_list.empty()) {
        m_deviceInfoLabel->setText("❌ No OBSBOT devices found!");
        m_deviceInfoLabel->setStyleSheet("font-weight: bold; padding: 10px; background-color: #ffcccc;");
        return;
    }

    m_device = dev_list.front();
    m_connected = true;

    QString info = QString("✓ Connected: %1 (v%2)")
        .arg(QString::fromStdString(m_device->devName()))
        .arg(QString::fromStdString(m_device->devVersion()));

    m_deviceInfoLabel->setText(info);
    m_deviceInfoLabel->setStyleSheet("font-weight: bold; padding: 10px; background-color: #ccffcc;");

    // Get initial status
    updateStatus();
}

void CameraControlWindow::onTrackingToggled(bool enabled)
{
    if (!m_connected) return;

    if (enabled) {
        applyCameraCommand("Enable auto-framing", [this]() {
            // Step 1: Set media mode
            int32_t ret = m_device->cameraSetMediaModeU(Device::MediaModeAutoFrame);
            if (ret != 0) return ret;

            // Wait a bit
            QThread::msleep(500);

            // Step 2: Set auto-framing mode
            return m_device->cameraSetAutoFramingModeU(Device::AutoFrmSingle, Device::AutoFrmUpperBody);
        });
    } else {
        applyCameraCommand("Disable auto-framing", [this]() {
            return m_device->cameraSetMediaModeU(Device::MediaModeNormal);
        });
    }
}

void CameraControlWindow::onPanLeftClicked()
{
    if (!m_connected) return;

    m_currentPan -= PTZ_STEP;
    if (m_currentPan < -1.0) m_currentPan = -1.0;

    applyCameraCommand("Pan left", [this]() {
        return m_device->cameraSetPanTiltAbsolute(m_currentPan, m_currentTilt);
    });

    m_positionLabel->setText(QString("Position: Pan %1, Tilt %2")
        .arg(m_currentPan, 0, 'f', 2)
        .arg(m_currentTilt, 0, 'f', 2));
}

void CameraControlWindow::onPanRightClicked()
{
    if (!m_connected) return;

    m_currentPan += PTZ_STEP;
    if (m_currentPan > 1.0) m_currentPan = 1.0;

    applyCameraCommand("Pan right", [this]() {
        return m_device->cameraSetPanTiltAbsolute(m_currentPan, m_currentTilt);
    });

    m_positionLabel->setText(QString("Position: Pan %1, Tilt %2")
        .arg(m_currentPan, 0, 'f', 2)
        .arg(m_currentTilt, 0, 'f', 2));
}

void CameraControlWindow::onTiltUpClicked()
{
    if (!m_connected) return;

    m_currentTilt += PTZ_STEP;
    if (m_currentTilt > 1.0) m_currentTilt = 1.0;

    applyCameraCommand("Tilt up", [this]() {
        return m_device->cameraSetPanTiltAbsolute(m_currentPan, m_currentTilt);
    });

    m_positionLabel->setText(QString("Position: Pan %1, Tilt %2")
        .arg(m_currentPan, 0, 'f', 2)
        .arg(m_currentTilt, 0, 'f', 2));
}

void CameraControlWindow::onTiltDownClicked()
{
    if (!m_connected) return;

    m_currentTilt -= PTZ_STEP;
    if (m_currentTilt < -1.0) m_currentTilt = -1.0;

    applyCameraCommand("Tilt down", [this]() {
        return m_device->cameraSetPanTiltAbsolute(m_currentPan, m_currentTilt);
    });

    m_positionLabel->setText(QString("Position: Pan %1, Tilt %2")
        .arg(m_currentPan, 0, 'f', 2)
        .arg(m_currentTilt, 0, 'f', 2));
}

void CameraControlWindow::onCenterClicked()
{
    if (!m_connected) return;

    m_currentPan = 0.0;
    m_currentTilt = 0.0;

    applyCameraCommand("Center", [this]() {
        return m_device->cameraSetPanTiltAbsolute(0.0, 0.0);
    });

    m_positionLabel->setText("Position: Pan 0.0, Tilt 0.0");
}

void CameraControlWindow::onZoomChanged(int value)
{
    if (!m_connected) return;

    m_currentZoom = value / 10.0;  // 10-20 -> 1.0-2.0

    applyCameraCommand("Zoom", [this]() {
        return m_device->cameraSetZoomAbsoluteR(m_currentZoom);
    });

    m_zoomLabel->setText(QString("%1x").arg(m_currentZoom, 0, 'f', 1));
}

void CameraControlWindow::onHDRToggled(bool enabled)
{
    if (!m_connected) return;

    applyCameraCommand(enabled ? "Enable HDR" : "Disable HDR", [this, enabled]() {
        return m_device->cameraSetWdrR(enabled ? Device::DevWdrModeDol2TO1 : Device::DevWdrModeClose);
    });
}

void CameraControlWindow::onFOVChanged(int index)
{
    if (!m_connected) return;

    Device::FovType fov;
    switch (index) {
        case 0: fov = Device::FovType86; break;
        case 1: fov = Device::FovType78; break;
        case 2: fov = Device::FovType65; break;
        default: fov = Device::FovType86; break;
    }

    applyCameraCommand("Change FOV", [this, fov]() {
        return m_device->cameraSetFovU(fov);
    });
}

void CameraControlWindow::onFaceAEToggled(bool enabled)
{
    if (!m_connected) return;

    applyCameraCommand(enabled ? "Enable Face AE" : "Disable Face AE", [this, enabled]() {
        return m_device->cameraSetFaceAeR(enabled);
    });
}

void CameraControlWindow::onFaceFocusToggled(bool enabled)
{
    if (!m_connected) return;

    applyCameraCommand(enabled ? "Enable Face Focus" : "Disable Face Focus", [this, enabled]() {
        return m_device->cameraSetFaceFocusR(enabled);
    });
}

void CameraControlWindow::updateStatus()
{
    if (!m_connected) return;

    auto status = m_device->cameraStatus();

    QString statusText = QString(
        "AI Mode: %1 | Zoom: %2% | HDR: %3 | Face AE: %4 | Focus: %5"
    ).arg(status.tiny.ai_mode == 0 ? "Off" : "On")
     .arg(status.tiny.zoom_ratio)
     .arg(status.tiny.hdr ? "On" : "Off")
     .arg(status.tiny.face_ae ? "On" : "Off")
     .arg(status.tiny.auto_focus ? "Auto" : "Manual");

    m_statusLabel->setText(statusText);

    // Update UI checkboxes to match camera state
    m_hdrCheckBox->blockSignals(true);
    m_hdrCheckBox->setChecked(status.tiny.hdr);
    m_hdrCheckBox->blockSignals(false);

    m_faceAECheckBox->blockSignals(true);
    m_faceAECheckBox->setChecked(status.tiny.face_ae);
    m_faceAECheckBox->blockSignals(false);

    m_faceFocusCheckBox->blockSignals(true);
    m_faceFocusCheckBox->setChecked(status.tiny.face_auto_focus);
    m_faceFocusCheckBox->blockSignals(false);
}

void CameraControlWindow::applyCameraCommand(const QString &description, std::function<int32_t()> command)
{
    int32_t ret = command();
    if (ret != 0) {
        QMessageBox::warning(this, "Command Failed",
            QString("%1 failed with error code: %2").arg(description).arg(ret));
    }
}
