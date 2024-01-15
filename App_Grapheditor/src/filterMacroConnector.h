#pragma once

#include <QuickQanava.h>
#include <QUuid>

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{

class FilterMacroConnector : public qan::Node
{
    Q_OBJECT
    Q_PROPERTY (QUuid ID READ ID WRITE setID NOTIFY IDChanged)
    Q_PROPERTY( bool matchingInPipeConnection READ isMatchingInPipeConnection NOTIFY matchingInPipeConnectionChanged)

    /**
     * Url to the qml file containing the configuration interface for the InstanceFilter this node represents.
     **/
    Q_PROPERTY(QUrl configurationInterface READ configurationInterface CONSTANT)

public:
    explicit FilterMacroConnector( QObject* parent = nullptr);
    ~FilterMacroConnector() override;

    QUuid ID() const
    {
        return m_ID;
    }
    void setID(const QUuid &ID);

    Q_INVOKABLE QRectF getItemGeometry() const;
    Q_INVOKABLE void setItemGeometry(int x, int y, int width = 160, int height = 160);

    bool isMatchingInPipeConnection() const
    {
        return m_matching;
    }
    void setMatchingInPipeConnection(bool matching);

    QUrl configurationInterface() const;

    static QQmlComponent* delegate(QQmlEngine &engine);

Q_SIGNALS:
    void IDChanged();
    void matchingInPipeConnectionChanged();

private:
    QUuid m_ID;
    bool m_matching{true};

};

}
}
}
}
QML_DECLARE_TYPE(precitec::gui::components::grapheditor::FilterMacroConnector)
