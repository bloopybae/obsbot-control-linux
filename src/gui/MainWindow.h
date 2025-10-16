#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include "CameraController.h"
#include "TrackingControlWidget.h"
#include "PTZControlWidget.h"
#include "CameraSettingsWidget.h"

/**
 * @brief Main application window
 *
 * Orchestrates the UI and switches between three modes:
 * - Simple: Just face tracking on/off
 * - Advanced: + PTZ controls
 * - Expert: + All camera settings
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum UIMode {
        SimpleMode,      // Just AI on/off
        AdvancedMode,    // + PTZ, zoom, focus
        ExpertMode       // Everything
    };

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onModeChanged(int mode);
    void onCameraConnected(const CameraController::CameraInfo &info);
    void onCameraDisconnected();
    void onStateChanged(const CameraController::CameraState &state);
    void onCommandFailed(const QString &description, int errorCode);
    void updateStatus();
    void onStateChangedSaveConfig();

private:
    void setupUI();
    void setUIMode(UIMode mode);
    void loadConfiguration();
    void handleConfigErrors(const std::vector<Config::ValidationError> &errors);
    CameraController::CameraState getUIState() const;  // Get current UI state

    // Controller
    CameraController *m_controller;

    // UI
    UIMode m_currentMode;
    QComboBox *m_modeSelector;
    QLabel *m_deviceInfoLabel;
    QLabel *m_statusLabel;
    QVBoxLayout *m_controlsLayout;

    // Control widgets
    TrackingControlWidget *m_trackingWidget;
    PTZControlWidget *m_ptzWidget;
    CameraSettingsWidget *m_settingsWidget;

    // Status timer
    QTimer *m_statusTimer;
};

#endif // MAINWINDOW_H
