#pragma once

#include <QuickQanava.h>

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{

class FilterGroup : public qan::Group
{
    Q_OBJECT
    Q_PROPERTY( int ID READ ID WRITE setID NOTIFY IDChanged)

    /**
     * Url to the qml file containing the configuration interface for the group this node represents.
     **/
    Q_PROPERTY(QUrl configurationInterface READ configurationInterface CONSTANT)
public:
    explicit FilterGroup( QQuickItem* parent = nullptr);
    ~FilterGroup() override;

    int ID() const;
    void setID( int ID);

    Q_INVOKABLE QRectF getItemGeometry() const;
    void setItemGeometry(const QRectF &geometry);
    void setItemGeometry(const QPointF &position, const QSize &size);
    Q_INVOKABLE void setItemGeometry(int x, int y, int width, int height);

    //Calculate Size of filterGroup
    void resetFirstNode();
    int getDistance( int x1, int x2 );
    bool checkNodeIsAlreadyInGroup(const qreal &groupValue, const qreal &nodeValue);
    void updateGroupGeometry(const QRectF &nodeRect);

    QUrl configurationInterface() const;

    //FilterGraph function
    static QQmlComponent* delegate(QQmlEngine &engine, QObject* parent = nullptr);

Q_SIGNALS:
    void IDChanged();
    void groupSizeChanged(FilterGroup* group);

private:
    int m_ID;
    bool m_firstNode = true;

    int m_additionalValue = 50;

    bool m_connected = false;
};

}
}
}
}
QML_DECLARE_TYPE(precitec::gui::components::grapheditor::FilterGroup)
