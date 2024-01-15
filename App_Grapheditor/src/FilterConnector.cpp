#include "FilterConnector.h"

using namespace precitec::gui::components::grapheditor;

FilterConnector::FilterConnector(QQuickItem* parent) : qan::PortItem(parent)
{
    setAcceptHoverEvents(true);
}

FilterConnector::~FilterConnector() = default;

QString FilterConnector::colorValue() const
{
    return m_colorValue;
}

void FilterConnector::setColorValue(const QString& color)
{
    if (m_colorValue != color)
    {
        m_colorValue = color;
        emit colorValueChanged();
    }
}

void FilterConnector::setColorValue(int connectorType)
{
    //Colorinfo from: weldmasterDefault/win/wmMain/Precitec.Settings.GraphEditor/GraphEditor/EditorItems/PortConnector.cs Line: 395
    //Type information is in: /home/weldmasterDefault/win/Util/FilterEditor/FilterEditor/Models/Connector.cs
    static const std::map<int, QString> s_map{{
        std::make_pair( 0, QStringLiteral("black")),
        std::make_pair( 1, QStringLiteral("yellow")),
        std::make_pair( 2, QStringLiteral("blueviolet")),                 //= lila
        std::make_pair( 3, QStringLiteral("orange")),
        std::make_pair( 4, QStringLiteral("turquoise")),                  //= light turquoise
        std::make_pair( 5, QStringLiteral("deeppink")),
        std::make_pair( 6, QStringLiteral("darkblue")),
        std::make_pair( 7, QStringLiteral("lightgreen")),
        std::make_pair( 8, QStringLiteral("white")),
        std::make_pair( 9, QStringLiteral("lightgrey")),
        std::make_pair(10, QStringLiteral("darkgrey")),
        std::make_pair(11, QStringLiteral("plum")),
    }};

    if (auto it = s_map.find(connectorType); it != s_map.end())
    {
        setColorValue(it->second);
    }
    else
    {
        setColorValue(QStringLiteral("darkgreen"));
    }
}

int FilterConnector::connectorType() const
{
    return m_connectorType;
}

void FilterConnector::setConnectorType(int type)
{
    if (m_connectorType != type)
    {
        m_connectorType = type;
        emit connectorTypeChanged();
    }
}

QUuid FilterConnector::ID() const
{
    return m_ID;
}

void FilterConnector::setID ( const QUuid& ID )
{
    if (m_ID != ID)
    {
        m_ID = ID;
        emit IDChanged();
    }
}

QString FilterConnector::tag() const
{
    return m_tag;
}

void FilterConnector::setTag(const QString& tag)
{
    if (m_tag != tag)
    {
        m_tag = tag;
        emit tagChanged();
    }
}

unsigned int FilterConnector::group() const
{
    return m_group;
}

void FilterConnector::setGroup(unsigned int group)
{
    if (m_group != group)
    {
        m_group = group;
        emit groupChanged();
    }
}

QRectF FilterConnector::getItemGeometry() const
{
    return {m_geometry.x(), m_geometry.y(), m_geometry.width(), m_geometry.height()};
}

void FilterConnector::setItemGeometry(const QRectF& geometry)
{
    if (m_geometry != geometry)
    {
        m_geometry = geometry;
    }
}

QQmlComponent * FilterConnector::delegate(QQmlEngine &engine)
{
    static std::unique_ptr<QQmlComponent> delegate;
    if (!delegate)
    {
        delegate = std::make_unique<QQmlComponent>(&engine, "qrc:/grapheditor/FilterConnector.qml");
    }
    return delegate.get();
}

void FilterConnector::hoverEnterEvent(QHoverEvent* event)
{
    emit hoverEvent(this);
}

void FilterConnector::hoverLeaveEvent(QHoverEvent* event)
{
    emit hoverEvent(nullptr);
}

void FilterConnector::setMatchingInPipeConnection(bool matching)
{
    if (m_matching == matching)
    {
        return;
    }
    m_matching = matching;
    emit matchingInPipeConnectionChanged();
}

void FilterConnector::setGeometryInputConnector(const QRectF& nodeGeometry, int actualConnector, int maxNumberConnectors)
{
    QPointF coordinates {0.0,0.0};

    coordinates.setX(0.0 - 10);
    coordinates.setY(0.0 + nodeGeometry.height());

    coordinates.setY(coordinates.y() - 10);

    //Calculate position from the number of the actual connector because they start in the bottom and ends in the top.
    coordinates.setY(coordinates.y() - (13*(maxNumberConnectors-(actualConnector+1)))); //Works for FilterVerticalDock.qml with bottom: hostNodeItem.bottom

    setItemGeometry({coordinates.x(), coordinates.y(), 10.0, 10.0});
}

void FilterConnector::setGeometryOutputConnector(const QRectF& nodeGeometry, int actualConnector, int maxNumberConnectors)
{
    QPointF coordinates {0.0,0.0};

    coordinates.setX(0.0 + nodeGeometry.width());
    coordinates.setY(0.0 + nodeGeometry.height());

    coordinates.setY(coordinates.y() - 10);

    //Calculate position from the number of the actual connector because they start in the bottom and ends in the top.
    coordinates.setY(coordinates.y() - (13*(maxNumberConnectors-(actualConnector+1))));

    setItemGeometry({coordinates.x(), coordinates.y(), 10.0, 10.0});
}
