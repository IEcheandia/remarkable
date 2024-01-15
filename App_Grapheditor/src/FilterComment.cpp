#include "FilterComment.h"
#include "FilterGroup.h"

using namespace precitec::gui::components::grapheditor;

FilterComment::FilterComment::FilterComment(QObject* parent) : qan::Node(parent)
{ }

FilterComment::~FilterComment() = default;

QUuid FilterComment::ID() const
{
    return m_ID;
}

void FilterComment::setID(const QUuid& id)
{
    if (m_ID != id)
    {
        m_ID = id;
        emit IDChanged();
    }
}

QString FilterComment::text() const
{
    return m_text;
}

void FilterComment::setText(const QString& comment)
{
    if (m_text != comment)
    {
        m_text = comment;
        emit textChanged();
    }
}

int FilterComment::groupID() const
{
    return m_groupID;
}

void FilterComment::setGroupID(int groupID)
{
    if (m_groupID != groupID)
    {
        m_groupID = groupID;
        emit groupIDChanged();
    }
}

QRectF FilterComment::getItemGeometry() const
{
    return {getItem()->x(), getItem()->y(), getItem()->width(), getItem()->height()};
}

void FilterComment::setItemGeometry(int x, int y, int width, int height)
{
    getItem()->setRect({static_cast<qreal>(x),static_cast<qreal>(y),static_cast<qreal>(width),static_cast<qreal>(height)});
}

void FilterComment::nodeGroupInteraction()
{
    auto group = getGroup();

    if (group)
    {
        setGroupID(dynamic_cast<FilterGroup*>(group)->ID());
    }
    else
    {
        setGroupID(-1);
    }
}

QQmlComponent * FilterComment::delegate(QQmlEngine& engine)
{
    static std::unique_ptr<QQmlComponent> delegate;
    if (!delegate)
    {
        delegate = std::make_unique<QQmlComponent>(&engine, "qrc:/grapheditor/FilterComment.qml");
    }
    return delegate.get();
}


QUrl FilterComment::configurationInterface() const
{
    return QUrl{QStringLiteral("qrc:/grapheditor/FilterCommentAttributes.qml")};
}
