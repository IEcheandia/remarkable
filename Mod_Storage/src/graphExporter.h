#pragma once

#include "fliplib/graphContainer.h"

#include <QSaveFile>
#include <QXmlStreamWriter>

namespace precitec
{
namespace storage
{

/**
 * Class to perform an export of a GraphContainer to xml.
 **/
class GraphExporter
{
public:
    explicit GraphExporter(const fliplib::GraphContainer &graph);

    /**
     * Filepath for export
     **/
    void setFileName(const QString &fileName);

    /**
     * Performs the actual export.
     **/
    void exportToXml();

private:
    void writeHeader(QXmlStreamWriter &stream);
    void writeHeaderFilterInfo(QXmlStreamWriter &stream, const fliplib::FilterDescription &filter);
    void writeParameterSet(QXmlStreamWriter &stream);
    void writeFilter(QXmlStreamWriter &stream, const fliplib::InstanceFilter &filter);
    void writePosition(QXmlStreamWriter &stream, const fliplib::Position &position);
    void writeGroup(QXmlStreamWriter &stream, int group);
    void writePipe(QXmlStreamWriter &stream, const fliplib::Pipe &pipe);
    void writeAttribute(QXmlStreamWriter &stream, const Poco::UUID &filter, const fliplib::InstanceFilter::Attribute &attribute);
    void writeVariant(QXmlStreamWriter &stream, const Poco::UUID &filter, const fliplib::InstanceFilter::Attribute &attribute);
    void writeError(QXmlStreamWriter &stream, const fliplib::ErrorType &errorType);
    void writeResult(QXmlStreamWriter &stream, const fliplib::ResultType &result);
    void writeFilterAttributes(QXmlStreamWriter &stream, const fliplib::InstanceFilter &filter)
    {
        for (const auto &attribute : filter.attributes)
        {
            writeAttribute(stream, filter.id, attribute);
        }
    }
    void writeFilterVariants(QXmlStreamWriter &stream, const fliplib::InstanceFilter &filter);
    void writeSensor(QXmlStreamWriter &stream, const fliplib::GenericType &sensor);
    void writeGenricTypeElements(QXmlStreamWriter &stream, const fliplib::GenericType &sensor);
    void writeErrorTypeElements(QXmlStreamWriter &stream, const fliplib::ErrorType &type);
    void writePort(QXmlStreamWriter &stream, const fliplib::Port &port);
    void writeMacro(QXmlStreamWriter &stream, const fliplib::Macro &macro);
    void writeConnector(QXmlStreamWriter &stream, const fliplib::Macro::Connector &macro);

    void writeGraphItemExtension(QXmlStreamWriter &stream, const Poco::UUID &reference, const fliplib::GraphItemExtension &extension);
    template <typename T>
    void writeGraphItemExtensions(QXmlStreamWriter &stream, const std::vector<T> &elements)
    {
        for (const T &element : elements)
        {
            for (const auto &graphItem : element.extensions)
            {
                writeGraphItemExtension(stream, element.id, graphItem);
            }
        }
    }
    template <typename T>
    void writeElements(QXmlStreamWriter &stream, const QString &name, const std::vector<T> &elements, void (GraphExporter::*function)(QXmlStreamWriter &, const T &))
    {
        stream.writeStartElement(name);
        for (const auto &element : elements)
        {
            (this->*(function))(stream, element);
        }
        stream.writeEndElement();
    }
    QString graphId() const;

    fliplib::GraphContainer m_graph;
    QSaveFile m_file;
    QString m_parameterSet;
};

}
}
