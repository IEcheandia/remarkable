#include "graphExporter.h"

#include <QDateTime>
#include <QFileInfo>
#include <QUuid>

namespace precitec
{
namespace storage
{

static const QString s_dateFormat{QStringLiteral("MM/dd/yyyy HH:mm:ss")};

GraphExporter::GraphExporter(const fliplib::GraphContainer& graph)
    : m_graph(graph)
{
}

void GraphExporter::setFileName(const QString& fileName)
{
    m_file.setFileName(fileName);
}

static QString idToString(const Poco::UUID &id)
{
    return QString::fromStdString(id.toString());
}

static QString boolToString(bool value)
{
    return value ? QStringLiteral("true") : QStringLiteral("false");
}

QString GraphExporter::graphId() const
{
    return idToString(m_graph.id);
}

void GraphExporter::exportToXml()
{
    if (!m_file.open(QIODevice::WriteOnly))
    {
        return;
    }
    m_parameterSet = idToString(m_graph.parameterSet);

    QXmlStreamWriter stream{&m_file};
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement(QStringLiteral("GraphData"));

    writeHeader(stream);
    writeParameterSet(stream);

    writeElements(stream, QStringLiteral("InstanzFilterList"), m_graph.instanceFilters, &GraphExporter::writeFilter);
    writeElements(stream, QStringLiteral("InstanzPipeList"), m_graph.pipes, &GraphExporter::writePipe);
    writeElements(stream, QStringLiteral("InstanzVariantList"), m_graph.instanceFilters, &GraphExporter::writeFilterVariants);
    writeElements(stream, QStringLiteral("InstanzAttributeList"), m_graph.instanceFilters, &GraphExporter::writeFilterAttributes);
    writeElements(stream, QStringLiteral("MesswerttypList"), m_graph.results, &GraphExporter::writeResult);
    writeElements(stream, QStringLiteral("FehlertypList"), m_graph.errors, &GraphExporter::writeError);
    writeElements(stream, QStringLiteral("SensorTypList"), m_graph.sensors, &GraphExporter::writeSensor);
    writeElements(stream, QStringLiteral("GraphPortList"), m_graph.ports, &GraphExporter::writePort);
    writeElements(stream, QStringLiteral("Macros"), m_graph.macros, &GraphExporter::writeMacro);
    writeElements(stream, QStringLiteral("InConnectors"), m_graph.inConnectors, &GraphExporter::writeConnector);
    writeElements(stream, QStringLiteral("OutConnectors"), m_graph.outConnectors, &GraphExporter::writeConnector);

    stream.writeStartElement(QStringLiteral("GraphItemsExtensionList"));
    writeGraphItemExtensions(stream, m_graph.instanceFilters);
    writeGraphItemExtensions(stream, m_graph.pipes);
    writeGraphItemExtensions(stream, m_graph.ports);
    stream.writeEndElement();

    stream.writeEndElement();
    stream.writeEndDocument();

    m_file.commit();
}

void GraphExporter::writeHeader(QXmlStreamWriter& stream)
{
    const QString date{QDateTime::currentDateTime().toString(s_dateFormat)};

    stream.writeTextElement(QStringLiteral("GraphID"), graphId());
    stream.writeTextElement(QStringLiteral("Name"), QString::fromStdString(m_graph.name));
    stream.writeTextElement(QStringLiteral("Kommentar"), QString::fromStdString(m_graph.comment));
    stream.writeTextElement(QStringLiteral("Erstellungsdatum"), date);
    stream.writeTextElement(QStringLiteral("DefaultGraph"), QStringLiteral("0"));
    stream.writeTextElement(QStringLiteral("Group"), QString::fromStdString(m_graph.group));

    stream.writeStartElement(QStringLiteral("Header"));
    stream.writeTextElement(QStringLiteral("InfoHeader"), QStringLiteral("Copyright © Precitec GmbH &amp; Co. KG  2013"));
    stream.writeEmptyElement(QStringLiteral("InfoUser"));
    stream.writeTextElement(QStringLiteral("GraphName"), QString::fromStdString(m_graph.name));
    stream.writeTextElement(QStringLiteral("GraphFileName"), QFileInfo{m_file.fileName()}.fileName());
    stream.writeEmptyElement(QStringLiteral("Version"));
    stream.writeAttribute(QStringLiteral("Major"), QStringLiteral("1"));
    stream.writeAttribute(QStringLiteral("Minor"), QStringLiteral("2"));
    stream.writeTextElement(QStringLiteral("SicherungsDatum"), date);

    writeElements(stream, QStringLiteral("HeaderFilterInfoList"), m_graph.filterDescriptions, &GraphExporter::writeHeaderFilterInfo);

    stream.writeEndElement();
}

void GraphExporter::writeHeaderFilterInfo(QXmlStreamWriter &stream, const fliplib::FilterDescription &filter)
{
    stream.writeStartElement(QStringLiteral("HeaderFilterInfo"));

    stream.writeTextElement(QStringLiteral("FilterID"), idToString(filter.id));
    stream.writeTextElement(QStringLiteral("FilterName"), QString::fromStdString(filter.name));
    stream.writeTextElement(QStringLiteral("Version"), QString::fromStdString(filter.version));
    stream.writeTextElement(QStringLiteral("KomponenteID"), idToString(filter.componentId));
    stream.writeTextElement(QStringLiteral("KomponenteAssemblyName"), QString::fromStdString(filter.component));

    //"KomponenteErstellungsDatum" not included, because it makes diff unusable (it's not required for importing the graph)

    stream.writeEndElement();
}

void GraphExporter::writeParameterSet(QXmlStreamWriter &stream)
{
    stream.writeStartElement(QStringLiteral("ParameterSatzList"));
    stream.writeStartElement(QStringLiteral("ParameterSatz"));
    stream.writeTextElement(QStringLiteral("ParametersatzID"), m_parameterSet);
    stream.writeTextElement(QStringLiteral("TypID"), graphId());
    stream.writeTextElement(QStringLiteral("ContentName"), QStringLiteral("parametersatz.ContentName"));
    stream.writeTextElement(QStringLiteral("ContentBeschreibung"), QStringLiteral("parametersatz.ContentBeschreibung"));
    stream.writeTextElement(QStringLiteral("ContentTooltip"), QStringLiteral("parametersatz.ContentTooltip"));
    stream.writeTextElement(QStringLiteral("TypName"), QStringLiteral("Graph"));
    stream.writeTextElement(QStringLiteral("Name"), QStringLiteral("Parameter set"));
    stream.writeTextElement(QStringLiteral("Beschreibung"), QStringLiteral("Comment"));
    stream.writeEndElement();
    stream.writeEndElement();
}

void GraphExporter::writeFilter(QXmlStreamWriter &stream, const fliplib::InstanceFilter &filter)
{
    stream.writeStartElement(QStringLiteral("InstanzFilter"));

    stream.writeTextElement(QStringLiteral("InstanzFilterID"), idToString(filter.id));
    stream.writeTextElement(QStringLiteral("FilterID"), idToString(filter.filterId));
    stream.writeTextElement(QStringLiteral("GraphID"), graphId());
    stream.writeTextElement(QStringLiteral("Name"), QString::fromStdString(filter.name));
    stream.writeEmptyElement(QStringLiteral("InstanzFilterDescription"));
    writePosition(stream, filter.position);
    stream.writeEmptyElement(QStringLiteral("InstanzFilterGrouping"));
    writeGroup(stream, filter.group);

    stream.writeEndElement();
}

void GraphExporter::writePosition(QXmlStreamWriter& stream, const fliplib::Position& position)
{
    stream.writeAttribute(QStringLiteral("PosX"), QString::number(position.x));
    stream.writeAttribute(QStringLiteral("PosY"), QString::number(position.y));
    stream.writeAttribute(QStringLiteral("Breite"), QString::number(position.width));
    stream.writeAttribute(QStringLiteral("Hoehe"), QString::number(position.height));
}

void GraphExporter::writeGroup(QXmlStreamWriter &stream, int group)
{
    stream.writeAttribute(QStringLiteral("Group"), QString::number(group));

    auto it = std::find_if(m_graph.filterGroups.begin(), m_graph.filterGroups.end(), [group] (const auto &fg) { return fg.number == group; });
    const int parent = (it != m_graph.filterGroups.end()) ? it->parent : -1;
    stream.writeAttribute(QStringLiteral("Parent"), QString::number(parent));

    const QString name = (it != m_graph.filterGroups.end()) ? QString::fromStdString(it->name) : QString{};
    stream.writeAttribute(QStringLiteral("GroupName"), name);
}

void GraphExporter::writePipe(QXmlStreamWriter &stream, const fliplib::Pipe &pipe)
{
    stream.writeStartElement(QStringLiteral("InstanzPipe"));
    stream.writeTextElement(QStringLiteral("InstanzPipeID"), idToString(pipe.id));
    stream.writeTextElement(QStringLiteral("PipePath"), QString::fromStdString(pipe.path));
    stream.writeTextElement(QStringLiteral("ReceiverConnectorID"), idToString(pipe.receiverConnectorId));
    stream.writeTextElement(QStringLiteral("ReceiverConnectorName"), QString::fromStdString(pipe.receiverConnectorName));
    stream.writeTextElement(QStringLiteral("ReceiverConnectorGroup"), QString::number(pipe.receiverConnectorGroup));
    stream.writeTextElement(QStringLiteral("ReceiverInstFilterID"), idToString(pipe.receiver));
    stream.writeTextElement(QStringLiteral("SenderConnectorID"), idToString(pipe.senderConnectorId));
    stream.writeTextElement(QStringLiteral("SenderConnectorName"), QString::fromStdString(pipe.senderConnectorName));
    stream.writeTextElement(QStringLiteral("SenderInstFilterID"), idToString(pipe.sender));
    stream.writeEndElement();
}

void GraphExporter::writeAttribute(QXmlStreamWriter &stream, const Poco::UUID &filter, const fliplib::InstanceFilter::Attribute &attribute)
{
    stream.writeStartElement(QStringLiteral("InstanzAttribute"));

    stream.writeTextElement(QStringLiteral("InstanzAttributeID"), idToString(attribute.instanceAttributeId));
    stream.writeTextElement(QStringLiteral("InstanzVariantID"), idToString(attribute.instanceVariantId));
    stream.writeTextElement(QStringLiteral("VariantID"), idToString(attribute.variantId));
    stream.writeTextElement(QStringLiteral("InstanceFilterID"), idToString(filter));
    stream.writeTextElement(QStringLiteral("AttributeID"), idToString(attribute.attributeId));
    stream.writeTextElement(QStringLiteral("AttributeName"), QString::fromStdString(attribute.name));
    stream.writeTextElement(QStringLiteral("UserLevel"), QString::number(attribute.userLevel));
    stream.writeTextElement(QStringLiteral("Visible"), boolToString(attribute.visible));
    stream.writeTextElement(QStringLiteral("Value"), QString::fromStdString(attribute.value.toString()));
    if (attribute.hasBlob)
    {
        stream.writeTextElement(QStringLiteral("Blob"), QString::number(attribute.blob));
    } else
    {
        stream.writeEmptyElement(QStringLiteral("Blob"));
    }
    stream.writeTextElement(QStringLiteral("Publicity"), boolToString(attribute.publicity));
    stream.writeTextElement(QStringLiteral("HelpFile"), QString::fromStdString(attribute.helpFile));

    stream.writeEndElement();
}

void GraphExporter::writeSensor(QXmlStreamWriter &stream, const fliplib::GenericType &sensor)
{
    stream.writeStartElement(QStringLiteral("sensor"));
    writeGenricTypeElements(stream, sensor);
    stream.writeEndElement();
}

void GraphExporter::writeGenricTypeElements(QXmlStreamWriter &stream, const fliplib::GenericType &type)
{
    stream.writeTextElement(QStringLiteral("UserTypID"), idToString(type.id));
    stream.writeTextElement(QStringLiteral("UserTypName"), QString::fromStdString(type.name));
    stream.writeTextElement(QStringLiteral("EnumTyp"), QString::number(type.enumType));
    stream.writeTextElement(QStringLiteral("OverviewVisibility"), boolToString(type.visibility));
    stream.writeTextElement(QStringLiteral("Plotable"), boolToString(type.plotable));
    stream.writeTextElement(QStringLiteral("Savetyp"), boolToString(type.saveType));
    stream.writeTextElement(QStringLiteral("Selectable"), boolToString(type.selectable));
}

void GraphExporter::writePort(QXmlStreamWriter &stream, const fliplib::Port &port)
{
    stream.writeStartElement(QStringLiteral("PortItem"));
    stream.writeTextElement(QStringLiteral("PortItemType"), QString::number(port.type));
    stream.writeTextElement(QStringLiteral("PortID"), idToString(port.id));
    stream.writeTextElement(QStringLiteral("GraphID"), graphId());
    stream.writeTextElement(QStringLiteral("ReceiverConnectorID"), idToString(port.receiverConnectorId));
    stream.writeTextElement(QStringLiteral("ReceiverInstFilterID"), idToString(port.receiverInstanceFilterId));
    stream.writeTextElement(QStringLiteral("ReceiverName"), QString::fromStdString(port.receiverName));
    stream.writeTextElement(QStringLiteral("SenderConnectorID"), idToString(port.senderConnectorId));
    stream.writeTextElement(QStringLiteral("SenderInstFilterID"), idToString(port.senderInstanceFilterId));
    stream.writeTextElement(QStringLiteral("SenderName"), QString::fromStdString(port.senderName));
    stream.writeTextElement(QStringLiteral("Text"), QString::fromStdString(port.text));

    stream.writeEmptyElement(QStringLiteral("PortDescription"));
    writePosition(stream, port.position);
    stream.writeEmptyElement(QStringLiteral("PortGrouping"));
    writeGroup(stream, port.group);
    stream.writeEndElement();
}

void GraphExporter::writeMacro(QXmlStreamWriter &stream, const fliplib::Macro &macro)
{
    stream.writeStartElement(QStringLiteral("Macro"));
    stream.writeTextElement(QStringLiteral("ID"), idToString(macro.macroId));
    stream.writeTextElement(QStringLiteral("InstanceID"), idToString(macro.id));

    writeElements(stream, QStringLiteral("InConnectors"), macro.inConnectors, &GraphExporter::writeConnector);
    writeElements(stream, QStringLiteral("OutConnectors"), macro.outConnectors, &GraphExporter::writeConnector);

    stream.writeEmptyElement(QStringLiteral("Position"));
    writePosition(stream, macro.position);
    stream.writeEmptyElement("Group");
    writeGroup(stream, macro.group);

    stream.writeEndElement();
}

void GraphExporter::writeConnector(QXmlStreamWriter &stream, const fliplib::Macro::Connector &connector)
{
    stream.writeStartElement(QStringLiteral("Connector"));
    stream.writeTextElement(QStringLiteral("ID"), idToString(connector.id));
    stream.writeTextElement(QStringLiteral("Name"), QString::fromStdString(connector.name));
    stream.writeTextElement(QStringLiteral("Type"), QString::number(int(connector.type)));
    if (connector.position.width != 0 && connector.position.height != 0)
    {
        stream.writeEmptyElement(QStringLiteral("Position"));
        writePosition(stream, connector.position);
    }
    stream.writeEndElement();
}

void GraphExporter::writeGraphItemExtension(QXmlStreamWriter &stream, const Poco::UUID &reference, const fliplib::GraphItemExtension &extension)
{
    stream.writeStartElement(QStringLiteral("GraphItemExtension"));
    stream.writeTextElement(QStringLiteral("ItemsExtensionID"), idToString(extension.id));
    stream.writeTextElement(QStringLiteral("GraphID"), graphId());
    stream.writeTextElement(QStringLiteral("GraphEditItemID"), idToString(reference));
    stream.writeTextElement(QStringLiteral("GraphEditItemType"), QString::number(extension.type));
    stream.writeTextElement(QStringLiteral("PipeInPortID"), idToString(extension.localScope));
    stream.writeEndElement();
}

void GraphExporter::writeFilterVariants(QXmlStreamWriter &stream, const fliplib::InstanceFilter &filter)
{
    std::list<Poco::UUID> variantIds;
    for (const auto &attribute : filter.attributes)
    {
        auto it = std::find(variantIds.begin(), variantIds.end(), attribute.instanceVariantId);
        if (it != variantIds.end())
        {
            continue;
        }
        variantIds.push_back(attribute.instanceVariantId);
        writeVariant(stream, filter.id, attribute);
    }
}

void GraphExporter::writeVariant(QXmlStreamWriter &stream, const Poco::UUID &filter, const fliplib::InstanceFilter::Attribute &attribute)
{
    const QString nullUuid{QUuid{}.toString(QUuid::WithoutBraces)};
    stream.writeStartElement(QStringLiteral("InstanzVariant"));
    stream.writeTextElement(QStringLiteral("InstanzVariantID"), idToString(attribute.instanceVariantId));
    stream.writeTextElement(QStringLiteral("ParentID"), nullUuid);
    stream.writeTextElement(QStringLiteral("VariantID"), idToString(attribute.variantId));
    stream.writeTextElement(QStringLiteral("ParametersatzID"), m_parameterSet);
    stream.writeTextElement(QStringLiteral("InstanceFilterID"), idToString(filter));
    stream.writeTextElement(QStringLiteral("HardwareKonfigID"), nullUuid);
    stream.writeTextElement(QStringLiteral("StationID"), nullUuid);
    stream.writeTextElement(QStringLiteral("ProduktID"), nullUuid);
    stream.writeEmptyElement(QStringLiteral("EditListOrder"));
    stream.writeEmptyElement(QStringLiteral("Name"));
    stream.writeEndElement();
}

void GraphExporter::writeErrorTypeElements(QXmlStreamWriter &stream, const fliplib::ErrorType &type)
{
    if (type.hasColor)
    {
        stream.writeTextElement(QStringLiteral("LineColor"), QString::number(type.color));
    } else
    {
        stream.writeEmptyElement(QStringLiteral("LineColor"));
    }
    if (type.hasHardwareFunction)
    {
        stream.writeTextElement(QStringLiteral("HASSPECIALHWFUNCTION"), QString::number(type.hardwareFunction));
    } else
    {
        stream.writeEmptyElement(QStringLiteral("HASSPECIALHWFUNCTION"));
    }
    if (type.hasListOrder)
    {
        stream.writeTextElement(QStringLiteral("LISTORDER"), QString::number(type.listOrder));
    } else
    {
        stream.writeEmptyElement(QStringLiteral("LISTORDER"));
    }
}

void GraphExporter::writeError(QXmlStreamWriter &stream, const fliplib::ErrorType &errorType)
{
    stream.writeStartElement(QStringLiteral("fehler"));
    writeGenricTypeElements(stream, errorType);
    writeErrorTypeElements(stream, errorType);
    stream.writeEndElement();
}

void GraphExporter::writeResult(QXmlStreamWriter &stream, const fliplib::ResultType &result)
{
    stream.writeStartElement(QStringLiteral("messwert"));
    writeGenricTypeElements(stream, result);
    writeErrorTypeElements(stream, result);
    stream.writeTextElement(QStringLiteral("BoundaryPlotable"), boolToString(result.boundaryPlotable));
    stream.writeTextElement(QStringLiteral("GlobalBoundaryPlotable"), boolToString(result.globalBoundaryPlotable));
    if (result.hasMax)
    {
        stream.writeTextElement(QStringLiteral("Max"), QString::number(result.max));
    } else
    {
        stream.writeEmptyElement(QStringLiteral("Max"));
    }
    if (result.hasMin)
    {
        stream.writeTextElement(QStringLiteral("Min"), QString::number(result.min));
    } else
    {
        stream.writeEmptyElement(QStringLiteral("Min"));
    }
    stream.writeTextElement(QStringLiteral("Unit"), QString::fromStdString(result.unit));
    stream.writeEndElement();
}

}
}
