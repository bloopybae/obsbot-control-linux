#include "TrackingControlWidget.h"
#include <QLabel>

TrackingControlWidget::TrackingControlWidget(CameraController *controller, QWidget *parent)
    : QWidget(parent)
    , m_controller(controller)
    , m_userInitiated(false)
{
    // Create debounce timer for command completion
    m_commandTimer = new QTimer(this);
    m_commandTimer->setSingleShot(true);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QGroupBox *groupBox = new QGroupBox("Face Tracking", this);
    QVBoxLayout *groupLayout = new QVBoxLayout(groupBox);

    QLabel *descLabel = new QLabel(
        "Enable automatic face tracking to keep you centered in the frame.", this);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("font-size: 11px;");
    groupLayout->addWidget(descLabel);

    m_trackingCheckBox = new QCheckBox("Enable Auto-Framing", this);
    m_trackingCheckBox->setStyleSheet("font-weight: bold; font-size: 14px;");
    connect(m_trackingCheckBox, &QCheckBox::toggled, this, &TrackingControlWidget::onTrackingToggled);
    groupLayout->addWidget(m_trackingCheckBox);

    layout->addWidget(groupBox);
}

void TrackingControlWidget::onTrackingToggled(bool checked)
{
    // Checkbox already shows the new state (optimistic UI)
    // Just send the command to the camera in the background
    m_userInitiated = true;
    m_controller->enableAutoFraming(checked);

    // Keep m_userInitiated true for 1 second to prevent timer updates
    // from overwriting the checkbox during command execution
    m_commandTimer->start(1000);
}

void TrackingControlWidget::updateFromState(const CameraController::CameraState &state)
{
    // Only update if:
    // 1. State differs from what's shown AND
    // 2. Not during user action AND
    // 3. Command completion timer has expired AND
    // 4. Camera is not settling
    bool shouldBeChecked = (state.aiMode != 0);
    bool commandInFlight = m_commandTimer->isActive();
    bool isSettling = m_controller->isSettling();

    if (m_trackingCheckBox->isChecked() != shouldBeChecked && !m_userInitiated && !commandInFlight && !isSettling) {
        m_trackingCheckBox->blockSignals(true);
        m_trackingCheckBox->setChecked(shouldBeChecked);
        m_trackingCheckBox->blockSignals(false);
    }

    // If command completed and timer expired, we can now accept state updates
    if (!commandInFlight && m_userInitiated) {
        m_userInitiated = false;
    }
}
