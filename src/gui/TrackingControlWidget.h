#ifndef TRACKINGCONTROLWIDGET_H
#define TRACKINGCONTROLWIDGET_H

#include <QWidget>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QTimer>
#include "CameraController.h"

/**
 * @brief Simple widget for enabling/disabling face tracking
 * Shown in: Simple, Advanced, Expert modes
 */
class TrackingControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TrackingControlWidget(CameraController *controller, QWidget *parent = nullptr);

    void updateFromState(const CameraController::CameraState &state);
    bool isTrackingEnabled() const { return m_trackingCheckBox->isChecked(); }
    void setTrackingEnabled(bool enabled) {
        m_trackingCheckBox->blockSignals(true);
        m_trackingCheckBox->setChecked(enabled);
        m_trackingCheckBox->blockSignals(false);
    }

private slots:
    void onTrackingToggled(bool checked);

private:
    CameraController *m_controller;
    QCheckBox *m_trackingCheckBox;
    bool m_userInitiated;  // Track if change was user-initiated
    QTimer *m_commandTimer;  // Debounce timer for command completion
};

#endif // TRACKINGCONTROLWIDGET_H
