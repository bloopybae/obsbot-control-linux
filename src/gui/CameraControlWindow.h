#ifndef CAMERACONTROLWINDOW_H
#define CAMERACONTROLWINDOW_H

#include <QMainWindow>
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QCheckBox>
#include <QComboBox>
#include <memory>
#include <dev/devs.hpp>

class CameraControlWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum ControlMode {
        SimpleMode,      // Just AI on/off
        AdvancedMode,    // + PTZ, zoom, focus
        ExpertMode       // Everything
    };

    explicit CameraControlWindow(QWidget *parent = nullptr);
    ~CameraControlWindow();

private slots:
    // Mode switching
    void onModeChanged(int mode);
    // Camera detection
    void onDeviceDetected();

    // Tracking controls
    void onTrackingToggled(bool enabled);

    // PTZ controls
    void onPanLeftClicked();
    void onPanRightClicked();
    void onTiltUpClicked();
    void onTiltDownClicked();
    void onCenterClicked();
    void onZoomChanged(int value);

    // Camera settings
    void onHDRToggled(bool enabled);
    void onFOVChanged(int index);
    void onFaceAEToggled(bool enabled);
    void onFaceFocusToggled(bool enabled);

    // Status update
    void updateStatus();

private:
    void setupUI();
    void connectToCamera();
    void applyCameraCommand(const QString &description, std::function<int32_t()> command);

    // Camera control
    std::shared_ptr<Device> m_device;
    bool m_connected;

    // PTZ state
    double m_currentPan;
    double m_currentTilt;
    double m_currentZoom;
    const double PTZ_STEP = 0.1;

    // UI Components
    QLabel *m_statusLabel;
    QLabel *m_deviceInfoLabel;

    // Tracking
    QCheckBox *m_trackingCheckBox;

    // PTZ controls
    QPushButton *m_panLeftBtn;
    QPushButton *m_panRightBtn;
    QPushButton *m_tiltUpBtn;
    QPushButton *m_tiltDownBtn;
    QPushButton *m_centerBtn;
    QSlider *m_zoomSlider;
    QLabel *m_zoomLabel;
    QLabel *m_positionLabel;

    // Camera settings
    QCheckBox *m_hdrCheckBox;
    QComboBox *m_fovComboBox;
    QCheckBox *m_faceAECheckBox;
    QCheckBox *m_faceFocusCheckBox;

    // Status timer
    QTimer *m_statusTimer;
};

#endif // CAMERACONTROLWINDOW_H
