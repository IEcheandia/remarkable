#include "module.h"

#include <QtConcurrentRun>

// interfaces
#include "common/connectionConfiguration.h"

namespace precitec
{
namespace gui
{
namespace mockAxis
{


Module::Module(QQuickItem *parent)
    : QQuickItem(parent)
    , ModuleManagerConnector(system::module::MockAxisModul)
    , m_ethercatInputsProxy(std::make_shared<TEthercatInputs<EventProxy>>())
{
    auto &axisInput = m_dataToProcesses.axis.emplace_back(eProductIndex_ACCELNET, eInstance1);
    axisInput.axis.actualPosition = 0;
    axisInput.axis.actualTorque = 0;
    axisInput.axis.actualVelocity = 0;
    axisInput.axis.errorReg = 0;
    axisInput.axis.manufacStatus = 0;
    axisInput.axis.modesOfOpDisp = 0;
    axisInput.axis.statusWord = 0;
    axisInput.axis.m_oDigitalInputs  = 0;
    axisInput.axis.m_oDigitalOutputs = 0;
    axisInput.axis.m_oFollowingError = 0;
}

Module::~Module() = default;


void Module::componentComplete()
{
    QQuickItem::componentComplete();
    QtConcurrent::run(this, &Module::init);
}

void Module::init()
{
    initModuleManager(ConnectionConfiguration::instance());

    registerPublication(m_ethercatInputsProxy.get());

    subscribeAllInterfaces();
    publishAllInterfaces();
}

ModuleSpec Module::getMyModuleSpec()
{
    return ModuleSpec(getMyAppId(), QCoreApplication::applicationFilePath().toStdString());
}

void Module::sendOperationMode(int index)
{
    auto &axisInput = m_dataToProcesses.axis.front().axis;
    switch (index)
    {
    case 0:
        axisInput.modesOfOpDisp = -1;
        break;
    case 1:
        axisInput.modesOfOpDisp = 0;
        break;
    case 2:
        axisInput.modesOfOpDisp = 1;
        break;
    case 3:
        axisInput.modesOfOpDisp = 11;
        break;
    case 4:
        axisInput.modesOfOpDisp = 12;
        break;
    case 5:
        axisInput.modesOfOpDisp = 3;
        break;
    case 6:
        axisInput.modesOfOpDisp = 6;
        break;
    }
    sendAxis();
}

void Module::sendPosition(int um)
{
    m_dataToProcesses.axis.front().axis.actualPosition = um * 4.114;
    sendAxis();
}

void Module::sendAxis()
{
    m_ethercatInputsProxy->ecatData(m_dataToProcesses);
}

}
}
}
