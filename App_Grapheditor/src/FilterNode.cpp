#include "FilterNode.h"
#include "FilterConnector.h"
#include "FilterGroup.h"
#include <QQuickItemGrabResult>

using namespace precitec::gui::components::grapheditor;

FilterNode::FilterNode(QObject* parent) : qan::Node(parent)
{   }

FilterNode::~FilterNode() = default;

QUrl FilterNode::image() const
{
    return m_image;
}

void FilterNode::setImage(const QUrl &image)
{
    if (m_image != image && image.isValid())
    {
        m_image = image;
        emit imageChanged();
    }
}

QString FilterNode::type() const
{
    return m_type;
}

void FilterNode::setType(const QString &type)
{
    if (m_type != type && !type.isEmpty())
    {
        m_type = type;
        emit typeChanged();
    }
}

QUuid FilterNode::typeID() const
{
    return m_typeID;
}

void FilterNode::setTypeID(const QUuid& ID)
{
    if (m_typeID != ID)
    {
        m_typeID = ID;
        emit typeIDChanged();
    }
}

QUuid FilterNode::ID() const
{
    return m_ID;
}

void FilterNode::setID(const QUuid& ID)
{
    if (m_ID != ID)
    {
        m_ID = ID;
        emit IDChanged();
    }
}

int FilterNode::groupID() const
{
    return m_groupID;
}

void FilterNode::setGroupID( int groupID)
{
    if (m_groupID != groupID)
    {
        m_groupID = groupID;
        emit groupIDChanged();
    }
}

QString FilterNode::contentName() const
{
    return m_contentName;
}

void FilterNode::setContentName(const QString& content)
{
    if (m_contentName != content)
    {
        m_contentName = content;
        emit contentNameChanged();
    }
}

QRectF FilterNode::getItemGeometry() const
{
    return {getItem()->x(), getItem()->y(), getItem()->width(), getItem()->height()};
}

void FilterNode::setItemGeometry(int x, int y, int width, int height)
{
    getItem()->setRect({static_cast<qreal>(x),static_cast<qreal>(y),static_cast<qreal>(width),static_cast<qreal>(height)});
}

void FilterNode::nodeGroupInteraction()
{
    auto group = getGroup();
    if (!group)
    {
        setGroupID(-1);
        return;
    }
    setGroupID(dynamic_cast<FilterGroup*>(group)->ID());
}

QQmlComponent * FilterNode::delegate(QQmlEngine& engine)
{
    static std::unique_ptr<QQmlComponent> delegate;
    if (!delegate)
    {
        delegate = std::make_unique<QQmlComponent>(&engine, "qrc:/grapheditor/FilterNode.qml");
    }
    return delegate.get();
}

void FilterNode::setMatchingInPipeConnection(bool matching)
{
    if (m_matching == matching)
    {
        return;
    }
    m_matching = matching;
    emit matchingInPipeConnectionChanged();
}

QUrl FilterNode::configurationInterface() const
{
    return QUrl{QStringLiteral("qrc:/grapheditor/FilterNodeAttributes.qml")};
}
