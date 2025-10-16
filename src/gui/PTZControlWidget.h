#ifndef PTZCONTROLWIDGET_H
#define PTZCONTROLWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QGroupBox>
#include "CameraController.h"

/**
 * @brief Widget for manual Pan/Tilt/Zoom control
 * Shown in: Advanced, Expert modes
 */
class PTZControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PTZControlWidget(CameraController *controller, QWidget *parent = nullptr);

    void updateFromState(const CameraController::CameraState &state);

private slots:
    void onPanLeftClicked();
    void onPanRightClicked();
    void onTiltUpClicked();
    void onTiltDownClicked();
    void onCenterClicked();
    void onZoomChanged(int value);

private:
    CameraController *m_controller;

    // PTZ buttons
    QPushButton *m_panLeftBtn;
    QPushButton *m_panRightBtn;
    QPushButton *m_tiltUpBtn;
    QPushButton *m_tiltDownBtn;
    QPushButton *m_centerBtn;

    // Zoom
    QSlider *m_zoomSlider;
    QLabel *m_zoomLabel;
    QLabel *m_positionLabel;
};

#endif // PTZCONTROLWIDGET_H
