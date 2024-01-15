#pragma once

#include <QuickQanava.h>

#include "fliplib/PipeConnector.h"

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{

class FilterConnector : public qan::PortItem
{
    Q_OBJECT
    Q_PROPERTY( QString colorValue READ colorValue WRITE setColorValue NOTIFY colorValueChanged)
    Q_PROPERTY( int connectorType READ connectorType WRITE setConnectorType NOTIFY connectorTypeChanged)
    Q_PROPERTY( QUuid ID READ ID WRITE setID NOTIFY IDChanged)
    Q_PROPERTY( QString tag READ tag WRITE setTag NOTIFY tagChanged)
    Q_PROPERTY( unsigned int group READ group WRITE setGroup NOTIFY groupChanged)
    Q_PROPERTY( bool matchingInPipeConnection READ isMatchingInPipeConnection NOTIFY matchingInPipeConnectionChanged)

public:
    explicit FilterConnector (QQuickItem* parent = nullptr);
    ~FilterConnector() override;

    QString colorValue() const;
    void setColorValue(const QString &color);
    void setColorValue(int type);
    int connectorType() const;
    void setConnectorType(int type);
    QUuid ID() const;
    void setID(const QUuid &ID);
    QString tag() const;
    void setTag(const QString &tag);
    unsigned int group() const;
    void setGroup(unsigned int group);

    QRectF getItemGeometry() const;
    void setItemGeometry(const QRectF &geometry);

    static QQmlComponent* delegate(QQmlEngine &engine);

    void hoverEnterEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent * event) override;

    bool isMatchingInPipeConnection() const
    {
        return m_matching;
    }
    void setMatchingInPipeConnection(bool matching);


    fliplib::PipeConnector::ConnectionType connectionType() const
    {
        return m_connectionType;
    }
    void setConnectionType(fliplib::PipeConnector::ConnectionType type)
    {
        m_connectionType = type;
    }

    void setGeometryInputConnector(const QRectF& nodeGeometry, int actualConnector, int maxNumberConnectors);
    void setGeometryOutputConnector(const QRectF& nodeGeometry, int actualConnector, int maxNumberConnectors);

Q_SIGNALS:
    void colorValueChanged();
    void connectorTypeChanged();
    void IDChanged();
    void tagChanged();
    void groupChanged();
    void hoverEvent(FilterConnector* connector);
    void matchingInPipeConnectionChanged();

private:
    QString m_colorValue = QStringLiteral("black");
    int m_connectorType;
    QUuid m_ID;
    QString m_tag;
    unsigned int m_group;
    QRectF m_geometry;
    bool m_matching{true};
    fliplib::PipeConnector::ConnectionType m_connectionType{fliplib::PipeConnector::ConnectionType::MandatoryForInOptionalForOut};

};

}
}
}
}
QML_DECLARE_TYPE( precitec::gui::components::grapheditor::FilterConnector )
