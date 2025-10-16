#include "MainWindow.h"
#include <QMessageBox>
#include <QWidget>
#include <QFrame>
#include <QIcon>
#include <QEvent>
#include <QScrollArea>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("OBSBOT Meet 2 Control");
    setWindowIcon(QIcon(":/icons/camera.svg"));

    // Create controller
    m_controller = new CameraController(this);

    // Connect signals
    connect(m_controller, &CameraController::cameraConnected,
            this, &MainWindow::onCameraConnected);
    connect(m_controller, &CameraController::cameraDisconnected,
            this, &MainWindow::onCameraDisconnected);
    connect(m_controller, &CameraController::stateChanged,
            this, &MainWindow::onStateChanged);
    connect(m_controller, &CameraController::commandFailed,
            this, &MainWindow::onCommandFailed);

    setupUI();

    // Load configuration
    loadConfiguration();

    // Start connecting to camera
    m_controller->connectToCamera();

    // Update status periodically
    m_statusTimer = new QTimer(this);
    connect(m_statusTimer, &QTimer::timeout, this, &MainWindow::updateStatus);
    m_statusTimer->start(2000);
}

MainWindow::~MainWindow()
{
    // Save config on exit
    if (m_controller->isConnected()) {
        m_controller->saveConfig();
    }
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Left: Preview drawer (collapsible, starts hidden)
    m_previewWidget = new CameraPreviewWidget(this);
    m_previewWidget->setVisible(false);
    mainLayout->addWidget(m_previewWidget, 1);  // Stretch to fill

    // Right: Controls sidebar (auto-sizes to content)
    QWidget *sidebar = new QWidget(this);
    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    sidebarLayout->setSpacing(0);

    // Top bar: Preview toggle and device info
    QWidget *topBar = new QWidget(this);
    QVBoxLayout *topLayout = new QVBoxLayout(topBar);
    topLayout->setContentsMargins(10, 10, 10, 10);

    // Preview toggle button
    QHBoxLayout *previewLayout = new QHBoxLayout();
    m_previewToggleButton = new QPushButton("Show Camera Preview", this);
    m_previewToggleButton->setCheckable(true);
    connect(m_previewToggleButton, &QPushButton::toggled, this, &MainWindow::onTogglePreview);
    previewLayout->addWidget(m_previewToggleButton);
    previewLayout->addStretch();
    topLayout->addLayout(previewLayout);

    // Device info
    m_deviceInfoLabel = new QLabel("Connecting to camera...", this);
    m_deviceInfoLabel->setStyleSheet("font-weight: bold; padding: 5px 0px;");
    topLayout->addWidget(m_deviceInfoLabel);

    sidebarLayout->addWidget(topBar);

    // Scrollable controls area
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *scrollContent = new QWidget();
    m_controlsLayout = new QVBoxLayout(scrollContent);
    m_controlsLayout->setContentsMargins(10, 10, 10, 10);
    m_controlsLayout->setSpacing(10);

    // Create all control widgets (always visible)
    m_trackingWidget = new TrackingControlWidget(m_controller, this);
    m_ptzWidget = new PTZControlWidget(m_controller, this);
    m_settingsWidget = new CameraSettingsWidget(m_controller, this);

    m_controlsLayout->addWidget(m_trackingWidget);
    m_controlsLayout->addWidget(m_ptzWidget);
    m_controlsLayout->addWidget(m_settingsWidget);
    m_controlsLayout->addStretch();

    scrollArea->setWidget(scrollContent);
    sidebarLayout->addWidget(scrollArea, 1);

    // Bottom: Status bar
    m_statusLabel = new QLabel("Status: Initializing...", this);
    m_statusLabel->setStyleSheet("padding: 10px; font-size: 11px; border-top: 1px solid palette(mid);");
    m_statusLabel->setWordWrap(true);
    sidebarLayout->addWidget(m_statusLabel);

    mainLayout->addWidget(sidebar, 0);  // No stretch, sizes to content
}

void MainWindow::onTogglePreview(bool enabled)
{
    if (enabled) {
        m_previewWidget->setVisible(true);
        m_previewWidget->enablePreview(true);
        m_previewToggleButton->setText("Hide Camera Preview");
    } else {
        m_previewWidget->enablePreview(false);
        m_previewWidget->setVisible(false);
        m_previewToggleButton->setText("Show Camera Preview");
    }
}

void MainWindow::onCameraConnected(const CameraController::CameraInfo &info)
{
    QString deviceText = QString("✓ Connected: %1 (v%2)")
        .arg(info.name)
        .arg(info.version);

    m_deviceInfoLabel->setText(deviceText);
    m_deviceInfoLabel->setStyleSheet("font-weight: bold; padding: 10px; color: green;");

    // Apply current UI state to camera asynchronously (respects user changes before connection)
    // Use a short delay to let the connection stabilize
    QTimer::singleShot(100, this, [this]() {
        auto uiState = getUIState();
        m_controller->applyCurrentStateToCamera(uiState);
    });

    updateStatus();
}

void MainWindow::onCameraDisconnected()
{
    m_deviceInfoLabel->setText("❌ Camera Disconnected");
    m_deviceInfoLabel->setStyleSheet("font-weight: bold; padding: 10px; color: red;");
    m_statusLabel->setText("Status: Not connected");
}

void MainWindow::onStateChanged(const CameraController::CameraState &state)
{
    // Update all widgets with new state
    m_trackingWidget->updateFromState(state);
    m_ptzWidget->updateFromState(state);
    m_settingsWidget->updateFromState(state);
}

void MainWindow::onCommandFailed(const QString &description, int errorCode)
{
    QMessageBox::warning(this, "Command Failed",
        QString("%1 failed with error code: %2").arg(description).arg(errorCode));
}

void MainWindow::updateStatus()
{
    if (!m_controller->isConnected()) {
        return;
    }

    auto state = m_controller->getCurrentState();

    QStringList statusParts;
    statusParts << QString("AI: %1").arg(state.aiMode == 0 ? "Off" : "On");
    statusParts << QString("Zoom: %1%").arg(state.zoomRatio);
    statusParts << QString("HDR: %1").arg(state.hdrEnabled ? "On" : "Off");
    statusParts << QString("Face AE: %1").arg(state.faceAEEnabled ? "On" : "Off");
    statusParts << QString("Focus: %1").arg(state.autoFocusEnabled ? "Auto" : "Manual");

    m_statusLabel->setText("Status: " + statusParts.join(" | "));
}

void MainWindow::loadConfiguration()
{
    std::vector<Config::ValidationError> errors;
    if (!m_controller->loadConfig(errors)) {
        // Config has validation errors
        handleConfigErrors(errors);
    }

    // Initialize UI widgets from config
    auto settings = m_controller->getConfig().getSettings();
    m_trackingWidget->setTrackingEnabled(settings.faceTracking);
    m_settingsWidget->setHDREnabled(settings.hdr);
    m_settingsWidget->setFOVMode(settings.fov);
    m_settingsWidget->setFaceAEEnabled(settings.faceAE);
    m_settingsWidget->setFaceFocusEnabled(settings.faceFocus);

    // Image controls
    m_settingsWidget->setBrightnessAuto(settings.brightnessAuto);
    m_settingsWidget->setBrightness(settings.brightness);
    m_settingsWidget->setContrastAuto(settings.contrastAuto);
    m_settingsWidget->setContrast(settings.contrast);
    m_settingsWidget->setSaturationAuto(settings.saturationAuto);
    m_settingsWidget->setSaturation(settings.saturation);
    m_settingsWidget->setWhiteBalance(settings.whiteBalance);
}

void MainWindow::handleConfigErrors(const std::vector<Config::ValidationError> &errors)
{
    QString errorMsg = "Configuration file has errors:\n\n";
    for (const auto &err : errors) {
        if (err.lineNumber > 0) {
            errorMsg += QString("Line %1: %2\n")
                .arg(err.lineNumber)
                .arg(QString::fromStdString(err.message));
        } else {
            errorMsg += QString::fromStdString(err.message) + "\n";
        }
    }

    errorMsg += "\nWhat would you like to do?";

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Config Error");
    msgBox.setText(errorMsg);
    msgBox.setIcon(QMessageBox::Warning);

    QPushButton *ignoreBtn = msgBox.addButton("Ignore (Don't Save)", QMessageBox::RejectRole);
    QPushButton *defaultsBtn = msgBox.addButton("Reset to Defaults", QMessageBox::AcceptRole);
    QPushButton *retryBtn = msgBox.addButton("Try Again", QMessageBox::ActionRole);

    msgBox.exec();

    if (msgBox.clickedButton() == ignoreBtn) {
        // User chose to ignore - disable saving
        m_controller->getConfig().disableSaving();
    } else if (msgBox.clickedButton() == defaultsBtn) {
        // User chose to reset to defaults
        m_controller->getConfig().resetToDefaults(true);
    } else if (msgBox.clickedButton() == retryBtn) {
        // User chose to try again - reload config
        loadConfiguration();
    }
}

void MainWindow::onStateChangedSaveConfig()
{
    // Debounced auto-save - only save if config saving is enabled
    // This prevents saving during transitional states
    if (m_controller->getConfig().isSavingEnabled()) {
        m_controller->saveConfig();
    }
}

CameraController::CameraState MainWindow::getUIState() const
{
    // Construct a state object from current UI widget values
    CameraController::CameraState state = {};

    // Get state from widgets
    state.autoFramingEnabled = m_trackingWidget->isTrackingEnabled();
    state.hdrEnabled = m_settingsWidget->isHDREnabled();
    state.fovMode = m_settingsWidget->getFOVMode();
    state.faceAEEnabled = m_settingsWidget->isFaceAEEnabled();
    state.faceFocusEnabled = m_settingsWidget->isFaceFocusEnabled();

    // Image controls
    state.brightnessAuto = m_settingsWidget->isBrightnessAuto();
    state.brightness = m_settingsWidget->getBrightness();
    state.contrastAuto = m_settingsWidget->isContrastAuto();
    state.contrast = m_settingsWidget->getContrast();
    state.saturationAuto = m_settingsWidget->isSaturationAuto();
    state.saturation = m_settingsWidget->getSaturation();
    state.whiteBalance = m_settingsWidget->getWhiteBalance();

    // Get PTZ state from controller (defaults from config)
    auto currentState = m_controller->getCurrentState();
    state.pan = currentState.pan;
    state.tilt = currentState.tilt;
    state.zoom = currentState.zoom;

    return state;
}

void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);

    if (event->type() == QEvent::WindowStateChange) {
        // Disable preview when window is minimized
        if (windowState() & Qt::WindowMinimized) {
            if (m_previewToggleButton->isChecked()) {
                m_previewToggleButton->setChecked(false);
            }
        }
    }
}
