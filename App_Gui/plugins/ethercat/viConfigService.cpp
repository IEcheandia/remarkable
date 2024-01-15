#include "viConfigService.h"

#include <QDir>
#include <QFutureWatcher>
#include <QXmlStreamReader>
#include <QtConcurrentRun>

namespace precitec
{
namespace gui
{
namespace components
{
namespace ethercat
{

ViConfigService::ViConfigService(QObject *parent)
    : QObject(parent)
{
    connect(this, &ViConfigService::configurationDirChanged, this, &ViConfigService::init);
}

ViConfigService::~ViConfigService() = default;

void ViConfigService::setConfigurationDir(const QString &config)
{
    if (m_configurationDir == config)
    {
        return;
    }
    m_configurationDir = config;
    emit configurationDirChanged();
}

std::vector<ViConfigService::Signal> ViConfigService::getSignals(quint32 vendorId, quint32 productCode, quint32 instance) const
{
    std::vector<Signal> ret;
    std::copy_if(m_signals.begin(), m_signals.end(), std::back_inserter(ret), [=] (const auto &signal) { return signal.vendorId == vendorId && signal.productCode == productCode && signal.instance == instance;});
    return ret;
}

void ViConfigService::init()
{
    if (m_configurationDir.isEmpty())
    {
        return;
    }
    auto watcher = new QFutureWatcher<std::vector<Signal>>{this};
    connect(watcher, &QFutureWatcher<std::vector<Signal>>::finished, this,
        [this, watcher]
        {
            watcher->deleteLater();
            m_signals = watcher->result();
            emit signalsChanged();
        }
    );
    watcher->setFuture(QtConcurrent::run(
        [this] () -> std::vector<Signal>
        {
            std::vector<Signal> ret;
            QFile viConfig{QDir{m_configurationDir}.filePath(QStringLiteral("VI_Config.xml"))};
            if (!viConfig.open(QIODevice::ReadOnly))
            {
                return ret;
            }
            QXmlStreamReader xml{&viConfig};
            xml.readNextStartElement();
            if (xml.qualifiedName() != QStringLiteral("VI_Config"))
            {
                return ret;
            }
            while (!xml.atEnd())
            {
                xml.readNextStartElement();
                if (xml.qualifiedName().compare(QLatin1String("VI_WeldHeadControl")) == 0)
                {
                    parseWeldHeadControl(xml, ret);
                }
                if (xml.qualifiedName().compare(QLatin1String("VI_InspectionControl")) == 0)
                {
                    parseInspectionControl(xml, ret);
                }
            }
            return ret;
        }
    ));
}

void ViConfigService::parseInspectionControl(QXmlStreamReader &xml, std::vector<Signal> &newSignals)
{
    while (!xml.atEnd())
    {
        if (!xml.readNextStartElement())
        {
            if (xml.qualifiedName().compare(QLatin1String("VI_InspectionControl")) == 0)
            {
                return;
            }
            continue;
        }
        parseInspectionSignal(xml, xml.qualifiedName(), newSignals);
    }
}

void ViConfigService::parseInspectionSignal(QXmlStreamReader &xml, const QStringRef &signalName, std::vector<Signal> &newSignals)
{
    while (!xml.atEnd())
    {
        if (!xml.readNextStartElement())
        {
            if (xml.qualifiedName().compare(signalName) == 0)
            {
                return;
            }
            continue;
        }
        Signal signal{};
        signal.name = signalName.toString();
        if (xml.qualifiedName().compare(QLatin1String("Input")) == 0)
        {
            signal.type = SignalType::Input;
        } else if (xml.qualifiedName().compare(QLatin1String("Output")) == 0)
        {
            signal.type = SignalType::Output;
        }
        parseSignal(xml, signal);
        newSignals.emplace_back(std::move(signal));
    }
}

void ViConfigService::parseSignal(QXmlStreamReader &xml, Signal &signal)
{
    QString currentElement;
    while (!xml.atEnd())
    {
        const auto element = xml.readNext();
        if (element == QXmlStreamReader::EndElement)
        {
            if (xml.qualifiedName().compare(QLatin1String("Input")) == 0 || xml.qualifiedName().compare(QLatin1String("Output")) == 0)
            {
                return;
            }
            continue;
        }
        if (element == QXmlStreamReader::StartElement)
        {
            currentElement = xml.qualifiedName().toString();
        }
        if (element == QXmlStreamReader::Characters && !xml.isWhitespace())
        {
            bool ok = false;
            const auto value = xml.text().toInt(&ok);
            if (!ok)
            {
                continue;
            }
            if (currentElement.compare(QLatin1String("ProductCode")) == 0)
            {
                signal.productCode = value;
            }
            if (currentElement.compare(QLatin1String("SlaveType")) == 0)
            {
                signal.slaveType = value;
            }
            if (currentElement.compare(QLatin1String("VendorID")) == 0)
            {
                signal.vendorId = value;
            }
            if (currentElement.compare(QLatin1String("Instance")) == 0)
            {
                signal.instance = value;
            }
            if (currentElement.compare(QLatin1String("StartBit")) == 0)
            {
                signal.startBit = value;
            }
            if (currentElement.compare(QLatin1String("Length")) == 0)
            {
                signal.length = value;
            }
        }
    }
}

void ViConfigService::parseWeldHeadControl(QXmlStreamReader &xml, std::vector<Signal> &newSignals)
{
    while (!xml.atEnd())
    {
        if (!xml.readNextStartElement())
        {
            if (xml.qualifiedName().compare(QLatin1String("VI_WeldHeadControl")) == 0)
            {
                return;
            }
            continue;
        }
        if (xml.qualifiedName().compare(QLatin1String("HeadmonitorGateway")) == 0)
        {
            parseHeadMonitor(xml, newSignals);
        }
        // TODO: implement
    }
}

void ViConfigService::parseHeadMonitor(QXmlStreamReader &xml, std::vector<Signal> &newSignals)
{
    while (!xml.atEnd())
    {
        if (!xml.readNextStartElement())
        {
            if (xml.qualifiedName().compare(QLatin1String("HeadmonitorGateway")) == 0)
            {
                return;
            }
            continue;
        }
        if (xml.qualifiedName().compare(QLatin1String("HM_Input")) == 0)
        {
            parseHeadMonitorSignals(xml, SignalType::Input, newSignals);
        }
        if (xml.qualifiedName().compare(QLatin1String("HM_Output")) == 0)
        {
            parseHeadMonitorSignals(xml, SignalType::Output, newSignals);
        }
    }
}

void ViConfigService::parseHeadMonitorSignals(QXmlStreamReader &xml, SignalType type, std::vector<Signal> &newSignals)
{
    quint32 productCode = -1;
    quint32 vendorId = -1;
    quint32 slaveType = -1;
    quint32 instance = -1;

    QString currentElement;
    while (!xml.atEnd())
    {
        const auto element = xml.readNext();
        if (element == QXmlStreamReader::EndElement)
        {
            if (xml.qualifiedName().compare(QLatin1String("HM_Input")) == 0 || xml.qualifiedName().compare(QLatin1String("HM_Output")) == 0)
            {
                return;
            }
            continue;
        }
        if (element == QXmlStreamReader::StartElement)
        {
            currentElement = xml.qualifiedName().toString();
            if (currentElement.compare(QLatin1String("GlasNotPresent")) == 0 ||
                currentElement.compare(QLatin1String("GlasDirty")) == 0 ||
                currentElement.compare(QLatin1String("TempGlasFail")) == 0 ||
                currentElement.compare(QLatin1String("TempHeadFail")) == 0)
            {
                Signal sig;
                sig.productCode = productCode;
                sig.vendorId = vendorId;
                sig.slaveType = slaveType;
                sig.instance = instance;
                sig.type = type;
                sig.length = 1;
                sig.name = currentElement;
                bool ok = false;
                sig.startBit = xml.attributes().value(QLatin1String("Bit")).toInt(&ok);
                if (ok)
                {
                    newSignals.emplace_back(std::move(sig));
                }
            }
        }
        if (element == QXmlStreamReader::Characters && !xml.isWhitespace())
        {
            bool ok = false;
            const auto value = xml.text().toInt(&ok);
            if (!ok)
            {
                continue;
            }
            if (currentElement.compare(QLatin1String("ProductCode")) == 0)
            {
                productCode = value;
            }
            if (currentElement.compare(QLatin1String("SlaveType")) == 0)
            {
                slaveType = value;
            }
            if (currentElement.compare(QLatin1String("VendorID")) == 0)
            {
                vendorId = value;
            }
            if (currentElement.compare(QLatin1String("Instance")) == 0)
            {
                instance = value;
            }
        }
    }
}

}
}
}
}
