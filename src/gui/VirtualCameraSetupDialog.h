#pragma once

#include <QDialog>
#include <QScopedPointer>
#include <QTemporaryDir>

class QLabel;
class QPushButton;

class VirtualCameraSetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VirtualCameraSetupDialog(const QString &devicePath, QWidget *parent = nullptr);

signals:
    void serviceStateChanged();

private slots:
    void onLoadOnce();
    void onInstallService();
    void onEnableService();
    void onDisableService();
    void onRemoveService();
    void onUnloadModule();

private:
    enum class ServiceState {
        NotInstalled,
        InstalledDisabled,
        EnabledRunning,
        EnabledStopped,
        Failed
    };

    void refreshStatus();
    bool ensureTempAssets();
    bool runPrivilegedAction(const QString &action);
    bool hasCommand(const QString &command) const;
    bool isModuleLoaded() const;
    bool isDeviceAvailable() const;
    QString readCommandOutput(const QStringList &arguments, int *exitCode = nullptr) const;
    ServiceState currentServiceState() const;
    QString describeServiceState(ServiceState state) const;
    void setButtonsEnabled(ServiceState state, bool moduleLoaded);
    QString defaultDevicePath() const;
    void showMissingPkexecMessage();

    QString m_devicePath;
    QLabel *m_statusSummaryLabel;
    QLabel *m_detailsLabel;
    QPushButton *m_loadOnceButton;
    QPushButton *m_installButton;
    QPushButton *m_enableButton;
    QPushButton *m_disableButton;
    QPushButton *m_removeButton;
    QPushButton *m_unloadButton;

    QScopedPointer<QTemporaryDir> m_tempDir;
    QString m_serviceTempPath;
    QString m_modprobeTempPath;
    QString m_scriptTempPath;
};
