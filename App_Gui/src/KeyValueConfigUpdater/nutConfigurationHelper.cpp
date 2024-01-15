#include "nutConfigUpdater.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

int main(int argc, char *argv[])
{
    QCoreApplication::setSetuidAllowed(true);
    QCoreApplication app{argc, argv};

    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption noneOption{QStringLiteral("none"), QStringLiteral("No uninterruptible power supply")};
    parser.addOption(noneOption);
    QCommandLineOption blazerOption{QStringLiteral("blazerusb"), QStringLiteral("ABB Powervalue 11RT (blazer_usb)")};
    parser.addOption(blazerOption);
    QCommandLineOption usbhidOption{QStringLiteral("usbhidups"), QStringLiteral("APC Smart-ups (usbhid-ups)")};
    parser.addOption(usbhidOption);
    QCommandLineOption nutdrvQxOption{QStringLiteral("nutdrvqx"), QStringLiteral("ABB PowerValue 11RT G2 (nutdrv_qx)")};
    parser.addOption(nutdrvQxOption);
    QCommandLineOption blazerSerOmronOption{QStringLiteral("blazerserOmron"), QStringLiteral("Omron S8BA24D24D120LF (blazer_ser)")}; // for Omron UPS via serial port
    parser.addOption(blazerSerOmronOption); // for Omron UPS via serial port
    QCommandLineOption blazerUsbOmronOption{QStringLiteral("blazerusbOmron"), QStringLiteral("Omron S8BA24D24D120LF (blazer_usb)")}; // for Omron UPS via USB
    parser.addOption(blazerUsbOmronOption); // for Omron UPS via USB
    parser.process(app);

    precitec::NutConfigUpdater::Ups driver;
    if (parser.isSet(blazerOption))
    {
        driver = precitec::NutConfigUpdater::Ups::ABBPowerValue11RT;
    } else if (parser.isSet(usbhidOption))
    {
        driver = precitec::NutConfigUpdater::Ups::APCSmartUps;
    } else if (parser.isSet(nutdrvQxOption))
    {
        driver = precitec::NutConfigUpdater::Ups::ABBPowerValue11RTG2;
    } else if (parser.isSet(blazerSerOmronOption)) // for Omron UPS via serial port
    {
        driver = precitec::NutConfigUpdater::Ups::OmronS8BA24D24D120LF_ser;
    } else if (parser.isSet(blazerUsbOmronOption)) // for Omron UPS via USB
    {
        driver = precitec::NutConfigUpdater::Ups::OmronS8BA24D24D120LF_usb;
    } else if (parser.isSet(noneOption))
    {
        driver = precitec::NutConfigUpdater::Ups::None;
    } else
    {
        return 1;
    }

    precitec::NutConfigUpdater updater{driver};
    updater();

    return 0;
}
