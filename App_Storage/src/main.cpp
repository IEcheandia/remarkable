#include <QCoreApplication>
#include "module.h"

#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QSocketNotifier>

#include <sys/signalfd.h>
#include <signal.h>
#include <unistd.h>

using precitec::storage::Module;

int main(int argc, char *argv[])
{
    // setup signal handler
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGQUIT);
    sigprocmask(SIG_BLOCK, &mask, nullptr);
    int signalFd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    QCommandLineOption resultsOption(QStringLiteral("results"));
    parser.addOption(resultsOption);
    QCommandLineOption calibrationOption(QStringLiteral("calibration"));
    parser.addOption(calibrationOption);
    // use parse instead of process as -pipePath arguments would fail in process
    parser.parse(app.arguments());

    precitec::storage::Module module;
    module.setResultStorageOption(parser.isSet(resultsOption) ? Module::ResultStorage::Enabled : Module::ResultStorage::Disabled);
    module.setCalibrationStorageOption(parser.isSet(calibrationOption) ? Module::CalibrationStorage::Enabled : Module::CalibrationStorage::Disabled);

    module.processCommandLineArguments(argc, argv);
    module.init();

    if (signalFd != -1)
    {
        QSocketNotifier *notifier = new QSocketNotifier(signalFd, QSocketNotifier::Read, &app);
        QObject::connect(notifier, &QSocketNotifier::activated, &app, &QCoreApplication::quit);
    }

    return app.exec();
}
