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
    void aspectRatioChanged(double ratio);  // width / height
    void previewStarted();  // Emitted when preview successfully starts
    void previewFailed(const QString &error);  // Emitted when preview fails to start

private slots:
    void onCameraError(QCamera::Error error);

private:
    void setupUI();
    void startPreview();
    void stopPreview();

    QCamera *m_camera;
    QMediaCaptureSession *m_captureSession;
    QVideoWidget *m_videoWidget;

    bool m_previewEnabled;
};

#endif // CAMERAPREVIEWWIDGET_H
