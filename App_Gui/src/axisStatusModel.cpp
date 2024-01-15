#include "axisStatusModel.h"
#include "weldHeadServer.h"

namespace precitec
{
namespace gui
{

static const char *s_statusStrings[] = {
    QT_TRANSLATE_NOOP("Precitec.Service.WeldHead.ReadyToSwitchOn", "Ready to switch on"),
    QT_TRANSLATE_NOOP("Precitec.Service.WeldHead.SwitchedOn", "Switched on"),
    QT_TRANSLATE_NOOP("Precitec.Service.WeldHead.OperationEnabled", "Operation enabled"),
    QT_TRANSLATE_NOOP("Precitec.Service.WeldHead.Fault", "Fault"),
    QT_TRANSLATE_NOOP("Precitec.Service.WeldHead.VoltageEnabled", "Voltage enabled"),
    QT_TRANSLATE_NOOP("Precitec.Service.WeldHead.QuickStop", "Quick stop"),
    QT_TRANSLATE_NOOP("Precitec.Service.WeldHead.SwitchOnDisabled", "Switch-On disabled"),
    QT_TRANSLATE_NOOP("Precitec.Service.WeldHead.Warning", "Warning"),
    QT_TRANSLATE_NOOP("Precitec.Service.WeldHead.Speed", "Speed"),
    QT_TRANSLATE_NOOP("Precitec.Service.WeldHead.Remote", "Remote controlled"),
    QT_TRANSLATE_NOOP("Precitec.Service.WeldHead.TargetReached", "Target reached"),
    QT_TRANSLATE_NOOP("Precitec.Service.WeldHead.InternalLimitActive", "Internal limit active"),
    QT_TRANSLATE_NOOP("Precitec.Service.WeldHead.SetpointAcknowledged", "Setpoint acknowledged"),
    QT_TRANSLATE_NOOP("Precitec.Service.WeldHead.FollowingError", "Following error"),
    QT_TRANSLATE_NOOP("Precitec.Service.WeldHead.NotUsed", "Not used"),
    QT_TRANSLATE_NOOP("Precitec.Service.WeldHead.LastTrajectoryAborted", "Last trajectory aborted")
};

AxisStatusModel::AxisStatusModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

AxisStatusModel::~AxisStatusModel() = default;

QVariant AxisStatusModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    switch (role)
    {
    case Qt::DisplayRole:
        return tr(s_statusStrings[index.row()]);
    case Qt::UserRole:
        return m_status.test(index.row());
    }

    return {};
}

int AxisStatusModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return 16;
}

QHash<int, QByteArray> AxisStatusModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {Qt::UserRole, QByteArrayLiteral("flag")}
    };
}

void AxisStatusModel::setWeldHeadServer(WeldHeadServer *server)
{
    if (m_weldHeadServer == server)
    {
        return;
    }
    disconnect(m_weldHeadServerDestroyed);
    disconnect(m_yAxisChanged);
    m_weldHeadServer = server;
    if (m_weldHeadServer)
    {
        m_weldHeadServerDestroyed = connect(m_weldHeadServer, &QObject::destroyed, this, std::bind(&AxisStatusModel::setWeldHeadServer, this, nullptr));
        m_yAxisChanged = connect(m_weldHeadServer, &WeldHeadServer::yAxisChanged, this,
            [this]
            {
                if (!m_weldHeadServer)
                {
                    return;
                }
                auto headInfo = m_weldHeadServer->yAxisInfo();
                std::bitset<16> status{headInfo.statusWord};
                // some bits have reversed meaning for the ui, so let's flip them
                status.flip(3);
                status.flip(6);
                status.flip(7);
                status.flip(11);
                status.flip(13);
                status.flip(14);
                status.flip(15);
                if (m_status != status)
                {
                    m_status = status;
                    emit dataChanged(index(0, 0), index(15, 0), {Qt::UserRole});
                }
            }, Qt::QueuedConnection);
    } else
    {
        m_weldHeadServerDestroyed = QMetaObject::Connection{};
        m_yAxisChanged = QMetaObject::Connection{};
    }
    emit weldHeadServerChanged();
}

}
}
