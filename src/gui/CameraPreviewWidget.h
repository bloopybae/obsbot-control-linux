#ifndef CAMERAPREVIEWWIDGET_H
#define CAMERAPREVIEWWIDGET_H

#include <QWidget>
#include <QCamera>
#include <QVideoWidget>
#include <QPushButton>
#include <QGroupBox>
#include <QMediaCaptureSession>

/**
 * @brief Camera preview widget with enable/disable control
 *
 * Features:
 * - Starts disabled by default
 * - Auto-disables when window is minimized or hidden
 * - Opens camera in shared mode (doesn't block other apps)
 * - Allows user to review effects of camera settings
 */
class CameraPreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CameraPreviewWidget(QWidget *parent = nullptr);
    ~CameraPreviewWidget();

    bool isPreviewEnabled() const;
    void enablePreview(bool enabled);

signals:
    void previewStateChanged(bool enabled);

private slots:
    void onTogglePreview();
    void onCameraError(QCamera::Error error);

private:
    void setupUI();
    void startPreview();
    void stopPreview();

    QCamera *m_camera;
    QMediaCaptureSession *m_captureSession;
    QVideoWidget *m_videoWidget;
    QPushButton *m_toggleButton;
    QGroupBox *m_groupBox;

    bool m_previewEnabled;
};

#endif // CAMERAPREVIEWWIDGET_H
