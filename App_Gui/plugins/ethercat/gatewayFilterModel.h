#pragma once
#include "viConfigService.h"

#include <QSortFilterProxyModel>

namespace precitec
{
namespace gui
{
namespace components
{
namespace ethercat
{

/**
 * Filter model for EthercatGatewayModel to filter on input and output signals.
 **/
class GatewayFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    /**
     * Whether this model should filter on input or output signals.
     **/
    Q_PROPERTY(precitec::gui::components::ethercat::ViConfigService::SignalType signalType READ signalType WRITE setSignalType NOTIFY signalTypeChanged)
public:
    GatewayFilterModel(QObject *parent = nullptr);
    ~GatewayFilterModel() override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    ViConfigService::SignalType signalType() const
    {
        return m_signalType;
    }
    void setSignalType(ViConfigService::SignalType type);

Q_SIGNALS:
    void signalTypeChanged();

private:
    ViConfigService::SignalType m_signalType = ViConfigService::SignalType::Input;
};

}
}
}
}

Q_DECLARE_METATYPE(precitec::gui::components::ethercat::GatewayFilterModel*)
