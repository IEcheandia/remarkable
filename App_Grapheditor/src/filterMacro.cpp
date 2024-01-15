#include "filterMacro.h"

namespace precitec
{
namespace gui
{
namespace components
{
namespace grapheditor
{

FilterMacro::FilterMacro(QObject* parent)
    : qan::Node(parent)
{
}

FilterMacro::~FilterMacro() = default;

QUrl FilterMacro::image() const
{
    return m_image;
}

void FilterMacro::setImage(const QUrl &image)
{
    if (m_image != image && image.isValid())
    {
        m_image = image;
        emit imageChanged();
    }
}

QString FilterMacro::type() const
{
    return m_type;
}

void FilterMacro::setType(const QString &type)
{
    if (m_type != type && !type.isEmpty())
    {
        m_type = type;
        emit typeChanged();
    }
}

QUuid FilterMacro::typeID() const
{
    return m_typeID;
}

void FilterMacro::setTypeID(const QUuid& ID)
{
    if (m_typeID != ID)
    {
        m_typeID = ID;
        emit typeIDChanged();
    }
}

QUuid FilterMacro::ID() const
{
    return m_ID;
}

void FilterMacro::setID(const QUuid& ID)
{
    if (m_ID != ID)
    {
        m_ID = ID;
        emit IDChanged();
    }
}

int FilterMacro::groupID() const
{
    return m_groupID;
}

void FilterMacro::setGroupID( int groupID)
{
    if (m_groupID != groupID)
    {
        m_groupID = groupID;
        emit groupIDChanged();
    }
}

QString FilterMacro::contentName() const
{
    return m_contentName;
}

void FilterMacro::setContentName(const QString& content)
{
    if (m_contentName != content)
    {
        m_contentName = content;
        emit contentNameChanged();
    }
}

QRectF FilterMacro::getItemGeometry() const
{
    return {getItem()->x(), getItem()->y(), getItem()->width(), getItem()->height()};
}

void FilterMacro::setItemGeometry(int x, int y, int width, int height)
{
    getItem()->setRect({static_cast<qreal>(x),static_cast<qreal>(y),static_cast<qreal>(width),static_cast<qreal>(height)});
}

QQmlComponent * FilterMacro::delegate(QQmlEngine& engine)
{
    static std::unique_ptr<QQmlComponent> delegate;
    if (!delegate)
    {
        delegate = std::make_unique<QQmlComponent>(&engine, "qrc:/grapheditor/FilterNode.qml");
    }
    return delegate.get();
}

void FilterMacro::setMatchingInPipeConnection(bool matching)
{
    if (m_matching == matching)
    {
        return;
    }
    m_matching = matching;
    emit matchingInPipeConnectionChanged();
}

QUrl FilterMacro::configurationInterface() const
{
    return QUrl{QStringLiteral("qrc:/grapheditor/FilterMacroAttributes.qml")};
}

void FilterMacro::setDescription(const QString &description)
{
    if (m_description == description)
    {
        return;
    }
    m_description = description;
    emit descriptionChanged();
}

}
}
}
}
