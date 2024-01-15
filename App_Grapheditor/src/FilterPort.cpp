#include "FilterPort.h"
#include "FilterGroup.h"
#include "FilterNode.h"
#include "FilterConnector.h"

using namespace precitec::gui::components::grapheditor;

FilterPort::FilterPort(QObject* parent) : qan::Node(parent)
{ }

FilterPort::~FilterPort() = default;

int FilterPort::type() const
{
    return m_type;
}

QUuid FilterPort::ID() const
{
    return m_ID;
}

void FilterPort::setID(const QUuid& id)
{
    if (m_ID != id)
    {
        m_ID = id;
        emit IDChanged();
    }
}

void FilterPort::setType( int type)
{
    if (m_type != type)
    {
        m_type = type;
        emit typeChanged();
    }
}

int FilterPort::groupID() const
{
    return m_groupID;
}

void FilterPort::setGroupID( int groupID)
{
    if (m_groupID != groupID)
    {
        m_groupID = groupID;
        emit groupIDChanged();
    }
}

QRectF FilterPort::getItemGeometry() const
{
    return {getItem()->x(), getItem()->y(), getItem()->width(), getItem()->height()};
}

int FilterPort::dataType() const
{
    return m_dataType;
}

void FilterPort::setDataType(int dataType)
{
    if (m_dataType != dataType)
    {
        m_dataType = dataType;
        setRectColor(getRectColorFromDataType(dataType));
        emit dataTypeChanged();
    }
}

QString FilterPort::getRectColorFromDataType(int dataType)
{
    //Color = "#RRGGBB" and with transparency "#AARRGGBB"
    switch (dataType)
    {
        case 0 : return {"#000000"};
        case 1 : return {"#ffff00"};
        case 2 : return {"#8a2be2"};
        case 3 : return {"#ffa500"};
        case 4 : return {"#40e0d0"};
        case 5 : return {"#ff1493"};
        case 6 : return {"#00008b"};
        case 7 : return {"#90ee90"};
        case 8 : return {"#ffffff"};
        case 9 : return {"#d3d3d3"};
        case 10 : return {"#a9a9a9"};
        case 11 : return {"#dda0dd"};
        default: return {"#006400"};
    }
}

QString FilterPort::rectColor() const
{
    return m_rectColor;
}

void FilterPort::setRectColor(QString color)
{
    if (m_rectColor != color)
    {
        if (m_rectColor.isNull())
        {
            m_rectColor = QStringLiteral("#80006400");
            emit rectColorChanged();
        }
        else
        {
            m_rectColor = color;
            emit rectColorChanged();
        }
    }
}

int FilterPort::partnerSize() const
{
    return m_partnerSize;
}

bool FilterPort::gotPartner() const
{
    return m_gotPartner;
}

void FilterPort::setGotPartner(bool gotPartner)
{
    if (m_gotPartner != gotPartner)
    {
        m_gotPartner = gotPartner;
        emit gotPartnerChanged();
    }
}

void FilterPort::insertPartner(FilterPort* partnerPort)
{
    if (m_type == 3)
    {
        partnerPort->setDataType(m_dataType);
    }
    m_partnerPorts.push_back(partnerPort);
    setGotPartner(true);
    emit partnerChanged();
    m_partnerSize++;
    emit partnerSizeChanged();
}

FilterPort * FilterPort::searchFilterPortPartner(const QUuid& partnerID)
{
    auto searchedPort = std::find_if(m_partnerPorts.begin(), m_partnerPorts.end(), [partnerID](FilterPort* const &actualPort) {return actualPort->ID() == partnerID;});
    if (searchedPort !=  m_partnerPorts.end())
    {
        return m_partnerPorts.at(std::distance(m_partnerPorts.begin(), searchedPort));
    }
    else
    {
        return {};
    }
}

void FilterPort::setDataTypeOfFilterPortPartner()
{
    for (const auto partnerPorts: m_partnerPorts)
    {
        partnerPorts->setDataType(m_dataType);
    }
}

bool FilterPort::removeFilterPortPartner(const QUuid &partnerID)
{
    auto searchedPort = std::find_if(m_partnerPorts.begin(), m_partnerPorts.end(), [partnerID](FilterPort* const &actualPort) {return actualPort->ID() == partnerID;});
    if (searchedPort !=  m_partnerPorts.end())
    {
        auto partnerPort = m_partnerPorts.at(std::distance(m_partnerPorts.begin(), searchedPort));
        partnerPort->resetPartners();
        partnerPort->setDataType(-1);
        m_partnerPorts.erase(searchedPort);
        if (m_partnerPorts.size() == 0)
        {
            setGotPartner(false);
        }
        emit partnerChanged();
        m_partnerSize--;
        emit partnerSizeChanged();
        return true;
    }
    else
    {
        return false;
    }
}

void FilterPort::resetPartners()
{
    if (m_partnerPorts.size() > 0)
    {
        m_partnerPorts.clear();
        m_partnerPorts.shrink_to_fit();
        setGotPartner(false);
        emit partnerChanged();
        m_partnerSize = 0;
        emit partnerSizeChanged();
    }
}

FilterPort * FilterPort::getOneFilterPortPartner()
{
    if (m_partnerPorts.size() == 1)
    {
        return m_partnerPorts.at(0);
    }
    return {};
}

std::vector<FilterPort *> FilterPort::getFilterPortPartner()
{
    return m_partnerPorts;
}

QUuid FilterPort::getUuid(const UUIDs& type) const
{
    if (type == UUIDs::senderFilterInstanceUUID)
    {
        return m_senderFilterInstanceID;
    }
    else if (type == UUIDs::senderFilterConnectorUUID)
    {
        return m_senderFilterConnectorID;
    }
    else if (type == UUIDs::receiverFilterInstanceUUID)
    {
        return m_receiverFilterInstanceID;
    }
    else
    {
        return m_receiverFilterConnectorID;
    }
}

void FilterPort::setUuid(const UUIDs& type, const QUuid& uuid)
{
    if (type == UUIDs::senderFilterInstanceUUID && uuid != m_senderFilterInstanceID)
    {
        m_senderFilterInstanceID = uuid;
    }
    else if (type == UUIDs::senderFilterConnectorUUID && uuid != m_senderFilterConnectorID)
    {
        m_senderFilterConnectorID = uuid;
    }
    else if (type == UUIDs::receiverFilterInstanceUUID && uuid != m_receiverFilterInstanceID)
    {
        m_receiverFilterInstanceID = uuid;
    }
    else if (type == UUIDs::receiverFilterConnectorUUID && uuid != m_receiverFilterConnectorID)
    {
        m_receiverFilterConnectorID = uuid;
    }
}

void FilterPort::resetUuids(bool sender)
{
    if (sender)
    {
        m_senderFilterInstanceID = QUuid{"00000000-0000-0000-0000-000000000000"};
        m_senderFilterConnectorID = QUuid{"00000000-0000-0000-0000-000000000000"};
    }
    else
    {
        m_receiverFilterInstanceID = QUuid{"00000000-0000-0000-0000-000000000000"};
        m_receiverFilterConnectorID = QUuid{"00000000-0000-0000-0000-000000000000"};
    }
}

void FilterPort::nodeGroupInteraction()
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

QObject* FilterPort::getFilterPortPartnerQObject()
{
    if (gotPartner())
    {
        auto portPartner = getOneFilterPortPartner();
        return portPartner;
    }
    return {};
}

QQmlComponent * FilterPort::delegate(QQmlEngine& engine)
{
    static std::unique_ptr<QQmlComponent> delegate;
    if (!delegate)
    {
        delegate = std::make_unique<QQmlComponent>(&engine, "qrc:/grapheditor/FilterPort.qml");
    }
    return delegate.get();
}

void FilterPort::checkIfThereIsAnotherConnection()
{
    if (m_type == 3)        //Inpipe
    {
        if (get_in_edges().size() == 0)
        {
            setUuid(UUIDs::senderFilterInstanceUUID, {});
            setUuid(UUIDs::senderFilterConnectorUUID, {});
            if (gotPartner())
            {
                for (const auto &portPartner : getFilterPortPartner())
                {
                    portPartner->setUuid(UUIDs::senderFilterInstanceUUID, {});
                    portPartner->setUuid(UUIDs::senderFilterConnectorUUID, {});
                }
            }
            return;
        }
        if (gotPartner())
        {
            for (const auto &portPartner : getFilterPortPartner())
            {
                portPartner->setUuid(UUIDs::senderFilterInstanceUUID, getUuid(UUIDs::senderFilterInstanceUUID));
                portPartner->setUuid(UUIDs::senderFilterConnectorUUID, getUuid(UUIDs::senderFilterConnectorUUID));
            }
        }
        return;
    }
    if (get_out_edges().size() == 0)
    {
        setUuid(UUIDs::receiverFilterInstanceUUID, {});
        setUuid(UUIDs::receiverFilterConnectorUUID, {});
        return;
    }
    //Check if in port stored pipe is still connected (Yes then exit, No then store a new pipe in port
    auto filterUUID = getUuid(UUIDs::receiverFilterInstanceUUID);
    auto connectorUUID = getUuid(UUIDs::receiverFilterConnectorUUID);

    for (const auto &pipe : get_out_edges())
    {
        auto edge = pipe;
        if (dynamic_cast<FilterNode*> (edge->getDestination()))
        {
            if (dynamic_cast<FilterNode*> (edge->getDestination())->ID() == filterUUID)
            {
                if (dynamic_cast<FilterConnector*> (edge->getItem()->getDestinationItem()))
                {
                    if (dynamic_cast<FilterConnector*> (edge->getItem()->getDestinationItem())->ID() == connectorUUID)
                    {
                        return;
                    }
                }
            }
        }
    }

    auto nextConnection = get_out_edges().at(0);
    auto destinationFilter = dynamic_cast<FilterNode*> (nextConnection->getDestination());
    if (!destinationFilter)
    {
        return;
    }
    setUuid(UUIDs::receiverFilterInstanceUUID, destinationFilter->ID());
    auto destinationFilterConnector = dynamic_cast<FilterConnector*> (nextConnection->getItem()->getDestinationItem());
    if (!destinationFilterConnector)
    {
        return;
    }
    setUuid(UUIDs::receiverFilterConnectorUUID, destinationFilterConnector->ID());
}

QUrl FilterPort::configurationInterface() const
{
    if (m_type == 3)
    {
        return QUrl{QStringLiteral("qrc:/grapheditor/FilterInPortAttributes.qml")};
    }
    else
    {
        return QUrl{QStringLiteral("qrc:/grapheditor/FilterPortAttributes.qml")};
    }
}

