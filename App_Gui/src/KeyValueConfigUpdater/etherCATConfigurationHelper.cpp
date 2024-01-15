#include "etherCATStartupUpdater.h"
#include "etherCATConfigUpdater.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

int main(int argc, char *argv[])
{
    QCoreApplication::setSetuidAllowed(true);
    QCoreApplication app{argc, argv};

    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption enableOption{QStringLiteral("enable"), QStringLiteral("Enable EtherCAT startup")};
    parser.addOption(enableOption);
    QCommandLineOption disableOption{QStringLiteral("disable"), QStringLiteral("Disable EtherCAT startup")};
    parser.addOption(disableOption);
    QCommandLineOption macOption{QStringLiteral("macAddress"), QStringLiteral("MacAddress for EtherCAT device"), QStringLiteral("mac")};
    parser.addOption(macOption);
    parser.process(app);

    if (parser.isSet(enableOption) || parser.isSet(disableOption))
    {
        precitec::EtherCATStartupUpdater updater{parser.isSet(enableOption) && !parser.isSet(disableOption)};
        updater();
    }
    if (parser.isSet(macOption))
    {
        precitec::EtherCATConfigUpdater updater{parser.value(macOption).toUtf8()};
        updater();
    }

    return 0;
}
