#include "FilterGroup.h"
#include "FilterNode.h"
#include "FilterPort.h"
#include "FilterComment.h"

using namespace precitec::gui::components::grapheditor;

FilterGroup::FilterGroup(QQuickItem* parent) : qan::Group(parent)
{   }

FilterGroup::~FilterGroup() = default;

int FilterGroup::ID() const
{
    return m_ID;
}

void FilterGroup::setID( int ID)
{
    if (m_ID != ID)
    {
        m_ID = ID;
        emit IDChanged();
    }
}

QRectF FilterGroup::getItemGeometry() const
{
    return {getItem()->x() + m_additionalValue/2, getItem()->y() + m_additionalValue/2, getItem()->width() - m_additionalValue, getItem()->height() - m_additionalValue};
}

void FilterGroup::setItemGeometry ( const QRectF& geometry )
{
    if (geometry.isValid())
    {
        getItem()->setRect({geometry.x() - m_additionalValue/2, geometry.y() - m_additionalValue/2, geometry.width() + m_additionalValue, geometry.height() + m_additionalValue});
        auto groupItem = getGroupItem();
        groupItem->setPreferredGroupWidth(geometry.width() + m_additionalValue);
        groupItem->setPreferredGroupHeight(geometry.height() + m_additionalValue);
    }
    if (!m_connected)
    {
        m_connected = true;
        connect(_item, &qan::NodeItem::widthChanged, this, std::bind(&FilterGroup::groupSizeChanged, this, this));
        connect(_item, &qan::NodeItem::heightChanged, this, std::bind(&FilterGroup::groupSizeChanged, this, this));
    }
}

void FilterGroup::setItemGeometry(const QPointF& position, const QSize& size)
{
    const QRectF rectangle = {position.x(), position.y(), static_cast<qreal>(size.width()), static_cast<qreal>(size.height())};
    setItemGeometry(rectangle);
}

void FilterGroup::setItemGeometry(int x, int y, int width, int height)
{
    const QRectF rectangle = {static_cast<qreal>(x), static_cast<qreal>(y), static_cast<qreal>(width), static_cast<qreal>(height)};
    setItemGeometry(rectangle);
}

void FilterGroup::resetFirstNode()
{
    m_firstNode = true;
}

int FilterGroup::getDistance(int x1, int x2)
{
    return x2 - x1;
}

bool FilterGroup::checkNodeIsAlreadyInGroup(const qreal &groupValue, const qreal &nodeValue)
{
    if ( groupValue > nodeValue )
    {
        return true;
    }
    return false;
}

void FilterGroup::updateGroupGeometry(const QRectF &nodeRect)
{
    auto groupGeometry = getItemGeometry();
    if (m_firstNode)
    {
        groupGeometry = nodeRect;
        m_firstNode = false;
        setItemGeometry(groupGeometry);
        return;
    }
    auto deltaX = getDistance(groupGeometry.x(), nodeRect.x());
    if (deltaX < 0 )
    {
        //Update X and width
        groupGeometry.setX( nodeRect.x() );     //Changes implicit the width to the right size, like groupGeometry.setWidth( groupGeometry.size().width() + std::abs(deltaX) ) would do.
    }
    else if (deltaX > 0 && !checkNodeIsAlreadyInGroup(groupGeometry.right(), nodeRect.right()))
    {
        //Update width
        groupGeometry.setWidth( deltaX + nodeRect.width());
    }
    auto deltaY = getDistance(groupGeometry.y(), nodeRect.y());
    if (deltaY < 0 )
    {
        //Update Y and height
        groupGeometry.setY( nodeRect.y() );         //Changes implicit the height to the right size, like groupGeometry.setHeight( groupGeometry.size().height() + std::abs(deltaY) ) would do.
    }
    else if (deltaY > 0 && !checkNodeIsAlreadyInGroup(groupGeometry.bottom(), nodeRect.bottom()))
    {
        //Update height
        groupGeometry.setHeight( deltaY + nodeRect.height() );
    }
    setItemGeometry(groupGeometry);
}

QQmlComponent * FilterGroup::delegate(QQmlEngine& engine, QObject* parent)
{
    Q_UNUSED(parent)
    static std::unique_ptr<QQmlComponent> delegate;
    if (!delegate)
        delegate = std::make_unique<QQmlComponent>(&engine, "qrc:/grapheditor/FilterGroup.qml");
    return delegate.get();
}

QUrl FilterGroup::configurationInterface() const
{
    return QUrl{QStringLiteral("qrc:/grapheditor/FilterGroupAttributes.qml")};
}
