#pragma once
// WM framework
#include "module/baseModule.h"
#include "event/ethercatInputs.proxy.h"
// Qt
#include <QQuickItem>

namespace precitec
{

typedef std::shared_ptr<precitec::interface::TEthercatInputs<precitec::interface::EventProxy>> EthercatInputsProxy;

namespace gui
{
namespace mockAxis
{

class Module : public QQuickItem, public precitec::framework::module::ModuleManagerConnector
{
    Q_OBJECT
    Q_PROPERTY(precitec::EthercatInputsProxy ethercatInputsProxy MEMBER m_ethercatInputsProxy CONSTANT)
public:
    Module(QQuickItem* parent = nullptr);
    ~Module() override;

    Q_INVOKABLE void sendOperationMode(int index);
    Q_INVOKABLE void sendPosition(int um);

protected:
    void componentComplete() override;

    precitec::interface::ModuleSpec getMyModuleSpec() override;

private:
    void init();
    void sendAxis();
    EthercatInputsProxy m_ethercatInputsProxy;
    EtherCAT::EcatInData m_dataToProcesses;
};

}
}
}
