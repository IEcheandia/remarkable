#include "LaserPoint.h"
#include "figureEditorSettings.h"

using precitec::scanmaster::components::wobbleFigureEditor::FigureEditorSettings;

namespace precitec
{
namespace scantracker
{
namespace components
{
namespace wobbleFigureEditor
{

LaserPoint::LaserPoint(QObject* parent)
    : qan::Node(parent)
{
    connect(this, &LaserPoint::laserPowerChanged, this, &LaserPoint::setEditableDependingOnLaserPower);
    connect(this, &LaserPoint::laserPowerChanged, this, &LaserPoint::startModifyIngAttachedEdge);
}

LaserPoint::~LaserPoint() = default;

int LaserPoint::ID() const
{
    return m_ID;
}

void LaserPoint::setID(const int id)
{
    if (m_ID != id)
    {
        m_ID = id;
        emit IDChanged();
    }
}

bool LaserPoint::editable() const
{
    return m_editable;
}

void LaserPoint::setEditable(bool edit)
{
    if (m_editable != edit)
    {
        m_editable = edit;
        emit editableChanged();
    }
}

double LaserPoint::laserPower() const
{
    return m_laserPower;
}

void LaserPoint::setLaserPower(double power)
{
    if (qFuzzyCompare(m_laserPower, power))
    {
        return;
    }

    m_laserPower = power;
    emit laserPowerChanged();
}

void LaserPoint::setRingPower(double newPower)
{
    if (qFuzzyCompare(m_ringPower, newPower))
    {
        return;
    }

    m_ringPower = newPower;
    emit ringPowerChanged();
}

void LaserPoint::setVelocity(double newVelocity)
{
    if (qFuzzyCompare(m_velocity, newVelocity))
    {
        return;
    }

    m_velocity = newVelocity;
    emit velocityChanged();
}

void LaserPoint::setType(FileType newType)
{
    if (m_type == newType)
    {
        return;
    }

    m_type = newType;
    emit typeChanged();
}

QColor LaserPoint::color() const
{
    return m_color;
}

void LaserPoint::setColor(const QColor& newColor)
{
    if (m_color == newColor)
    {
        return;
    }
    m_color = newColor;
    emit colorChanged();
}

QPointF LaserPoint::center() const
{
    return geometry().center();
}

void LaserPoint::setCenter(const QPointF& center)
{
    auto boundary = geometry();
    if (center == boundary.center())
    {
        return;
    }

    boundary.moveCenter(center);
    setGeometry(boundary);
    emit centerChanged();
}

void LaserPoint::setIsRampPoint(bool rampEndPoint)
{
    if (m_isRampPoint == rampEndPoint)
    {
        return;
    }

    m_isRampPoint = rampEndPoint;
    emit isRampPointChanged();
}

QRectF LaserPoint::geometry() const
{
    return {getItem()->x(), getItem()->y(), getItem()->width(), getItem()->height()};
}

void LaserPoint::setGeometry(const QRectF& geometry)
{
    getItem()->setRect(geometry);
}

void LaserPoint::initConnectionToNodeItem()
{
    if (auto node = getItem())
    {
        connect(node, &qan::NodeItem::xChanged, this, &LaserPoint::centerChanged);
        connect(node, &qan::NodeItem::yChanged, this, &LaserPoint::centerChanged);
    }
}

void LaserPoint::modifyAttachedEdge(bool startModifyingFollowedPoint, bool modifingFromRampColorValidator)
{
    if (get_out_nodes().size() == 0 || get_out_edges().size() > 1)
    {
        return;
    }

    const auto outGoingEdge = get_out_edges().at(0);

    bool dashedLine{false};
    QColor lineColor{Qt::gray};
    if (!modifingFromRampColorValidator)
    {
        if (qFuzzyIsNull(m_laserPower)) //jump
        {
            dashedLine = false;
            lineColor = Qt::black;
            setLineStyle(dashedLine, lineColor);
        }
        else if (qFuzzyCompare(m_laserPower, -1.0)) //take last valid power
        {
            dashedLine = true;
            lineColor = Qt::gray;
            setLineStyle(dashedLine, lineColor);
            setEdgeColorLikeEdgeBefore();
        }
        else
        {
            dashedLine = false;
            if (!m_isRampPoint)
                lineColor = FigureEditorSettings::instance()->colorFromValue(m_laserPower);
            setLineStyle(dashedLine, lineColor);
        }
    }

    outGoingEdge->getItem()->setStyle(&m_ownEdgeStyle);
    if (startModifyingFollowedPoint)
    {
        setEdgeStyleOfAllFollowingNodes();
    }
}

void LaserPoint::startModifyIngAttachedEdge()
{
    modifyAttachedEdge(true);
}

void LaserPoint::setLineStyle(bool dashedLine, QColor lineColor)
{
    m_ownEdgeStyle.setDashed(dashedLine);
    m_ownEdgeStyle.setLineColor(lineColor);
}

QQmlComponent* LaserPoint::delegate(QQmlEngine& engine)
{
    static std::unique_ptr<QQmlComponent> delegate;
    if (!delegate)
    {
        delegate = std::make_unique<QQmlComponent>(&engine, "qrc:/figureeditor/LaserPoint.qml");
    }
    return delegate.get();
}

void LaserPoint::setEditableDependingOnLaserPower()
{
    setEditable(!qFuzzyCompare(m_laserPower, -1));
}

void LaserPoint::setEdgeColorLikeEdgeBefore()
{
    if (get_in_edges().size() == 0 || get_in_edges().size() > 1)
    {
        return;
    }
    const auto style = get_in_edges().at(0)->getItem()->getStyle();
    setLineStyle(true, style->getLineColor());
}

void LaserPoint::setEdgeStyleOfAllFollowingNodes()
{
    if (get_out_nodes().size() == 0 || get_out_nodes().size() > 1)
    {
        return;
    }

    auto nextLaserPoint = qobject_cast<LaserPoint*>(get_out_nodes().at(0));

    while (nextLaserPoint && nextLaserPoint->get_out_nodes().size() == 1)
    {
        if (qFuzzyCompare(nextLaserPoint->laserPower(), -1.0))
        {
            nextLaserPoint->modifyAttachedEdge();
        }
        else
        {
            return;
        }
        nextLaserPoint = qobject_cast<LaserPoint*>(nextLaserPoint->get_out_nodes().at(0));
    }
}

}
}
}
}
