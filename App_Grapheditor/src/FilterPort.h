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

    enum class UUIDs
    {
        senderFilterInstanceUUID = 0,
        senderFilterConnectorUUID = 1,
        receiverFilterInstanceUUID = 2,
        receiverFilterConnectorUUID = 3,
    };

class FilterPort : public qan::Node
{
    Q_OBJECT
    Q_PROPERTY (QUuid ID READ ID WRITE setID NOTIFY IDChanged)
    Q_PROPERTY (int type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY (int groupID READ groupID WRITE setGroupID NOTIFY groupIDChanged)
    Q_PROPERTY (bool gotPartner READ gotPartner WRITE setGotPartner NOTIFY gotPartnerChanged)
    Q_PROPERTY (int dataType READ dataType WRITE setDataType NOTIFY dataTypeChanged)
    Q_PROPERTY (QString rectColor READ rectColor WRITE setRectColor NOTIFY rectColorChanged)
    Q_PROPERTY (int partnerSize READ partnerSize NOTIFY partnerSizeChanged)
    /**
     * Url to the qml file containing the configuration interface for the Port this node represents.
     **/
    Q_PROPERTY(QUrl configurationInterface READ configurationInterface NOTIFY typeChanged)

public:
    explicit FilterPort( QObject* parent = nullptr);
    ~FilterPort() override;

    QUuid ID() const;
    void setID(const QUuid &id);
    int type() const;
    void setType( int type);
    int groupID() const;
    void setGroupID( int groupID);
    Q_INVOKABLE QRectF getItemGeometry() const;
    int dataType() const;
    void setDataType(int dataType);
    QString getRectColorFromDataType(int dataType);
    QString rectColor() const;
    void setRectColor(QString color);
    int partnerSize() const;

    bool gotPartner() const;
    void setGotPartner( bool gotPartner);
    void insertPartner(FilterPort* partnerPort);
    void resetPartners();
    FilterPort* getOneFilterPortPartner();
    std::vector<FilterPort*> getFilterPortPartner();
    FilterPort* searchFilterPortPartner(const QUuid &partnerID);
    void setDataTypeOfFilterPortPartner();
    bool removeFilterPortPartner(const QUuid &partnerID);

    QUuid getUuid(const UUIDs &type) const;
    void setUuid(const UUIDs &type, const QUuid &uuid);
    void resetUuids(bool sender);

    //Group and port interaction
    void nodeGroupInteraction();
    Q_INVOKABLE QObject* getFilterPortPartnerQObject();

    static QQmlComponent* delegate(QQmlEngine &engine);

    void checkIfThereIsAnotherConnection();

    QUrl configurationInterface() const;

Q_SIGNALS:
    void IDChanged();
    void typeChanged();
    void groupIDChanged();
    void gotPartnerChanged();
    void partnerChanged();
    void dataTypeChanged();
    void rectColorChanged();
    void partnerSizeChanged();

private:

    QUuid m_ID;
    int m_type;
    int m_groupID;
    int m_dataType = -1;
    QString m_rectColor{"#006400"};

    bool m_gotPartner{false};
    int m_partnerSize = 0;
    std::vector<FilterPort*> m_partnerPorts;

    QUuid m_senderFilterInstanceID;
    QUuid m_senderFilterConnectorID;
    QUuid m_receiverFilterInstanceID;
    QUuid m_receiverFilterConnectorID;
};

}
}
}
}
QML_DECLARE_TYPE(precitec::gui::components::grapheditor::FilterPort*)
