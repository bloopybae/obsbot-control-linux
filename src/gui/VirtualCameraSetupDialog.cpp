#include "VirtualCameraSetupDialog.h"

#include <QDialogButtonBox>
#include <QFile>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTextStream>
#include <QVBoxLayout>

namespace {
    constexpr auto kServiceResourcePath = ":/systemd/obsbot-virtual-camera.service";
    constexpr auto kModprobeResourcePath = ":/modprobe/obsbot-virtual-camera.conf";
    constexpr auto kScriptResourcePath = ":/scripts/obsbot-virtual-camera-setup.sh";
    constexpr auto kServiceName = "obsbot-virtual-camera.service";
}

VirtualCameraSetupDialog::VirtualCameraSetupDialog(const QString &devicePath, QWidget *parent)
    : QDialog(parent)
    , m_devicePath(devicePath.trimmed().isEmpty() ? QStringLiteral("/dev/video42") : devicePath)
    , m_statusSummaryLabel(nullptr)
    , m_detailsLabel(nullptr)
    , m_loadOnceButton(nullptr)
    , m_installButton(nullptr)
    , m_enableButton(nullptr)
    , m_disableButton(nullptr)
    , m_removeButton(nullptr)
    , m_unloadButton(nullptr)
{
    setWindowTitle(tr("Virtual Camera Setup"));
    setModal(true);
    resize(520, 360);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(16);

    auto *intro = new QLabel(tr("This assistant helps you load the v4l2loopback module and optionally "
                                 "install a system service so the OBSBOT virtual camera is available on boot.\n\n"
                                 "All privileged actions use PolicyKit (pkexec). You will be prompted for your "
                                 "administrator password when required. Nothing happens automatically without your approval."),
                              this);
    intro->setWordWrap(true);
    layout->addWidget(intro);

    m_statusSummaryLabel = new QLabel(this);
    m_statusSummaryLabel->setWordWrap(true);
    m_statusSummaryLabel->setObjectName(QStringLiteral("statusSummary"));
    layout->addWidget(m_statusSummaryLabel);

    m_detailsLabel = new QLabel(this);
    m_detailsLabel->setWordWrap(true);
    m_detailsLabel->setStyleSheet(QStringLiteral("color: palette(mid); font-size: 12px;"));
    layout->addWidget(m_detailsLabel);

    auto *actionGrid = new QGridLayout();
    actionGrid->setContentsMargins(0, 0, 0, 0);
    actionGrid->setHorizontalSpacing(12);
    actionGrid->setVerticalSpacing(10);

    m_loadOnceButton = new QPushButton(tr("Load Module (once)"), this);
    connect(m_loadOnceButton, &QPushButton::clicked, this, &VirtualCameraSetupDialog::onLoadOnce);
    actionGrid->addWidget(m_loadOnceButton, 0, 0);

    m_unloadButton = new QPushButton(tr("Unload Module"), this);
    connect(m_unloadButton, &QPushButton::clicked, this, &VirtualCameraSetupDialog::onUnloadModule);
    actionGrid->addWidget(m_unloadButton, 0, 1);

    m_installButton = new QPushButton(tr("Install Service"), this);
    connect(m_installButton, &QPushButton::clicked, this, &VirtualCameraSetupDialog::onInstallService);
    actionGrid->addWidget(m_installButton, 1, 0);

    m_enableButton = new QPushButton(tr("Enable on Boot"), this);
    connect(m_enableButton, &QPushButton::clicked, this, &VirtualCameraSetupDialog::onEnableService);
    actionGrid->addWidget(m_enableButton, 1, 1);

    m_disableButton = new QPushButton(tr("Disable Service"), this);
    connect(m_disableButton, &QPushButton::clicked, this, &VirtualCameraSetupDialog::onDisableService);
    actionGrid->addWidget(m_disableButton, 2, 0);

    m_removeButton = new QPushButton(tr("Remove Service Files"), this);
    connect(m_removeButton, &QPushButton::clicked, this, &VirtualCameraSetupDialog::onRemoveService);
    actionGrid->addWidget(m_removeButton, 2, 1);

    layout->addLayout(actionGrid);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &VirtualCameraSetupDialog::reject);
    layout->addWidget(buttonBox);

    refreshStatus();
}

void VirtualCameraSetupDialog::refreshStatus()
{
    const ServiceState state = currentServiceState();
    const bool moduleLoaded = isModuleLoaded();
    const bool devicePresent = isDeviceAvailable();

    QStringList details;
    details << tr("<b>Kernel module:</b> %1").arg(moduleLoaded ? tr("Loaded") : tr("Not loaded"));
    details << tr("<b>Device node (%1):</b> %2").arg(defaultDevicePath(),
                                                     devicePresent ? tr("Present") : tr("Missing"));
    details << tr("<b>System service:</b> %1").arg(describeServiceState(state));
    details << tr("<b>Requires:</b> pkexec, systemctl, modprobe");

    m_statusSummaryLabel->setText(describeServiceState(state));
    m_detailsLabel->setText(details.join("<br/>"));

    setButtonsEnabled(state, moduleLoaded);
}

QString VirtualCameraSetupDialog::defaultDevicePath() const
{
    return m_devicePath.isEmpty() ? QStringLiteral("/dev/video42") : m_devicePath;
}

void VirtualCameraSetupDialog::setButtonsEnabled(ServiceState state, bool moduleLoaded)
{
    const bool serviceInstalled = state != ServiceState::NotInstalled && state != ServiceState::Failed;
    const bool serviceEnabled = state == ServiceState::EnabledRunning || state == ServiceState::EnabledStopped;

    if (m_loadOnceButton) {
        m_loadOnceButton->setEnabled(true);
        m_loadOnceButton->setToolTip(tr("Load v4l2loopback for the current session."));
    }
    if (m_unloadButton) {
        m_unloadButton->setEnabled(moduleLoaded);
        m_unloadButton->setToolTip(moduleLoaded ? QString() : tr("Module is not currently loaded."));
    }
    if (m_installButton) {
        m_installButton->setEnabled(!serviceInstalled);
        m_installButton->setToolTip(serviceInstalled ? tr("Service files already installed.") :
                                           tr("Install the service definition under /etc/systemd/system."));
    }
    if (m_enableButton) {
        m_enableButton->setEnabled(serviceInstalled && !serviceEnabled);
        m_enableButton->setToolTip(serviceInstalled ? tr("Enable the service and start it immediately.") :
                                                      tr("Install the service before enabling it."));
    }
    if (m_disableButton) {
        m_disableButton->setEnabled(serviceEnabled);
        m_disableButton->setToolTip(serviceEnabled ? QString() :
                                                  tr("Service is not enabled."));
    }
    if (m_removeButton) {
        m_removeButton->setEnabled(serviceInstalled);
        m_removeButton->setToolTip(serviceInstalled ? tr("Remove the service and modprobe configuration.") :
                                                      tr("Service files are not installed."));
    }
}

bool VirtualCameraSetupDialog::ensureTempAssets()
{
    if (!m_tempDir) {
        m_tempDir.reset(new QTemporaryDir());
        if (!m_tempDir->isValid()) {
            QMessageBox::critical(this, tr("Temporary Directory Error"),
                                  tr("Unable to create a temporary directory for setup assets."));
            return false;
        }

        const QString basePath = m_tempDir->path();
        m_serviceTempPath = basePath + "/obsbot-virtual-camera.service";
        m_modprobeTempPath = basePath + "/obsbot-virtual-camera.conf";
        m_scriptTempPath = basePath + "/obsbot-virtual-camera-setup.sh";

        auto copyResource = [this](const QString &resourcePath, const QString &destination, QFileDevice::Permissions permissions) -> bool {
            QFile source(resourcePath);
            if (!source.open(QIODevice::ReadOnly)) {
                QMessageBox::critical(this, tr("Resource Error"),
                                      tr("Failed to read resource: %1").arg(resourcePath));
                return false;
            }

            QFile dest(destination);
            if (!dest.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                QMessageBox::critical(this, tr("File Error"),
                                      tr("Failed to write file: %1").arg(destination));
                return false;
            }

            dest.write(source.readAll());
            dest.setPermissions(permissions);
            return true;
        };

        if (!copyResource(kServiceResourcePath, m_serviceTempPath,
                          QFileDevice::ReadOwner | QFileDevice::ReadGroup | QFileDevice::ReadOther)) {
            return false;
        }
        if (!copyResource(kModprobeResourcePath, m_modprobeTempPath,
                          QFileDevice::ReadOwner | QFileDevice::ReadGroup | QFileDevice::ReadOther)) {
            return false;
        }
        if (!copyResource(kScriptResourcePath, m_scriptTempPath,
                          QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner |
                              QFileDevice::ReadGroup | QFileDevice::ExeGroup |
                              QFileDevice::ReadOther | QFileDevice::ExeOther)) {
            return false;
        }
    }
    return true;
}

bool VirtualCameraSetupDialog::runPrivilegedAction(const QString &action)
{
    if (!ensureTempAssets()) {
        return false;
    }

    const QString pkexecPath = QStandardPaths::findExecutable(QStringLiteral("pkexec"));
    if (pkexecPath.isEmpty()) {
        showMissingPkexecMessage();
        return false;
    }

    QStringList arguments;
    arguments << m_scriptTempPath << action << m_serviceTempPath << m_modprobeTempPath;

    QProcess process;
    process.start(pkexecPath, arguments);
    const bool finished = process.waitForFinished(-1);

    if (!finished || process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        const QString errorStdErr = QString::fromUtf8(process.readAllStandardError());
        QMessageBox::warning(this, tr("Virtual Camera Setup"),
                             tr("The requested action did not complete successfully.\n\nDetails:\n%1")
                                 .arg(errorStdErr.trimmed()));
        return false;
    }

    emit serviceStateChanged();
    refreshStatus();
    return true;
}

void VirtualCameraSetupDialog::showMissingPkexecMessage()
{
    QMessageBox::warning(this, tr("pkexec Not Found"),
                         tr("PolicyKit (pkexec) is not available on this system, so OBSBOT Control cannot "
                            "run privileged actions automatically.\n\n"
                            "To set up the virtual camera manually:\n"
                            "  1. Copy obsbot-virtual-camera.service to /etc/systemd/system/\n"
                            "  2. Copy obsbot-virtual-camera.conf to /etc/modprobe.d/\n"
                            "  3. Run: sudo systemctl enable --now obsbot-virtual-camera.service\n\n"
                            "You will find the service and module templates inside the application resources."));
}

bool VirtualCameraSetupDialog::hasCommand(const QString &command) const
{
    return !QStandardPaths::findExecutable(command).isEmpty();
}

QString VirtualCameraSetupDialog::readCommandOutput(const QStringList &arguments, int *exitCode) const
{
    if (!hasCommand(QStringLiteral("systemctl"))) {
        if (exitCode) {
            *exitCode = -1;
        }
        return {};
    }

    QProcess process;
    process.start(QStringLiteral("systemctl"), arguments);
    process.waitForFinished(2000);
    if (exitCode) {
        *exitCode = process.exitCode();
    }
    return QString::fromUtf8(process.readAllStandardOutput()).trimmed();
}

VirtualCameraSetupDialog::ServiceState VirtualCameraSetupDialog::currentServiceState() const
{
    const QFileInfo serviceFile(QStringLiteral("/etc/systemd/system/%1").arg(kServiceName));
    if (!serviceFile.exists()) {
        return ServiceState::NotInstalled;
    }

    int exitCode = 0;
    const QString enabledState = readCommandOutput(
        {QStringLiteral("--no-pager"), QStringLiteral("is-enabled"), QStringLiteral("obsbot-virtual-camera.service")},
        &exitCode);

    if (exitCode != 0) {
        return ServiceState::Failed;
    }

    const QString activeState = readCommandOutput(
        {QStringLiteral("--no-pager"), QStringLiteral("is-active"), QStringLiteral("obsbot-virtual-camera.service")},
        &exitCode);

    const bool enabled = enabledState == QStringLiteral("enabled");
    const bool active = activeState == QStringLiteral("active");

    if (enabled && active) {
        return ServiceState::EnabledRunning;
    }

    if (enabled && !active) {
        return ServiceState::EnabledStopped;
    }

    return ServiceState::InstalledDisabled;
}

QString VirtualCameraSetupDialog::describeServiceState(ServiceState state) const
{
    switch (state) {
    case ServiceState::NotInstalled:
        return tr("Service files are not installed.");
    case ServiceState::InstalledDisabled:
        return tr("Service installed but disabled.");
    case ServiceState::EnabledRunning:
        return tr("Service enabled and running.");
    case ServiceState::EnabledStopped:
        return tr("Service enabled but not running.");
    case ServiceState::Failed:
    default:
        return tr("Unable to query systemd service state.");
    }
}

bool VirtualCameraSetupDialog::isModuleLoaded() const
{
    return QFileInfo(QStringLiteral("/sys/module/v4l2loopback")).exists();
}

bool VirtualCameraSetupDialog::isDeviceAvailable() const
{
    return QFileInfo(defaultDevicePath()).exists();
}

void VirtualCameraSetupDialog::onLoadOnce()
{
    if (runPrivilegedAction(QStringLiteral("load-once"))) {
        QMessageBox::information(this, tr("Virtual Camera"),
                                 tr("v4l2loopback loaded for this session."));
    }
}

void VirtualCameraSetupDialog::onInstallService()
{
    runPrivilegedAction(QStringLiteral("install"));
}

void VirtualCameraSetupDialog::onEnableService()
{
    if (runPrivilegedAction(QStringLiteral("enable"))) {
        QMessageBox::information(this, tr("Virtual Camera"),
                                 tr("Service enabled and started."));
    }
}

void VirtualCameraSetupDialog::onDisableService()
{
    if (runPrivilegedAction(QStringLiteral("disable"))) {
        QMessageBox::information(this, tr("Virtual Camera"),
                                 tr("Service disabled and stopped."));
    }
}

void VirtualCameraSetupDialog::onRemoveService()
{
    if (QMessageBox::question(this, tr("Remove Service"),
                              tr("Remove the systemd service and modprobe configuration? This will stop the "
                                 "virtual camera from loading automatically."))
        == QMessageBox::Yes) {
        runPrivilegedAction(QStringLiteral("remove"));
    }
}

void VirtualCameraSetupDialog::onUnloadModule()
{
    if (runPrivilegedAction(QStringLiteral("unload"))) {
        QMessageBox::information(this, tr("Virtual Camera"),
                                 tr("v4l2loopback module unloaded."));
    }
}
