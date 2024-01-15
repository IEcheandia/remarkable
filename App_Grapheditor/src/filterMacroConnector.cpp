#include "filterMacroConnector.h"

namespace precitec
{
namespace gui
{
namespace components
{
namespace grapheditor
{

FilterMacroConnector::FilterMacroConnector(QObject* parent)
    : qan::Node(parent)
{
}

FilterMacroConnector::~FilterMacroConnector() = default;

void FilterMacroConnector::setID(const QUuid& ID)
{
    if (m_ID != ID)
    {
        m_ID = ID;
        emit IDChanged();
    }
}

QRectF FilterMacroConnector::getItemGeometry() const
{
    return {getItem()->x(), getItem()->y(), getItem()->width(), getItem()->height()};
}

void FilterMacroConnector::setItemGeometry(int x, int y, int width, int height)
{
    getItem()->setRect({static_cast<qreal>(x),static_cast<qreal>(y),static_cast<qreal>(width),static_cast<qreal>(height)});
}

QQmlComponent * FilterMacroConnector::delegate(QQmlEngine& engine)
{
    static std::unique_ptr<QQmlComponent> delegate;
    if (!delegate)
    {
        delegate = std::make_unique<QQmlComponent>(&engine, "qrc:/grapheditor/FilterNode.qml");
    }
    return delegate.get();
}

void FilterMacroConnector::setMatchingInPipeConnection(bool matching)
{
    if (m_matching == matching)
    {
        return;
    }
    m_matching = matching;
    emit matchingInPipeConnectionChanged();
}

QUrl FilterMacroConnector::configurationInterface() const
{
    return QUrl{QStringLiteral("qrc:/grapheditor/FilterMacroAttributes.qml")};
}

}
}
}
}
