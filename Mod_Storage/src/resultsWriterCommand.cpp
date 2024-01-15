#include "resultsWriterCommand.h"
#include "resultsSerializer.h"

namespace precitec
{
namespace storage
{

ResultsWriterCommand::ResultsWriterCommand(const QDir &path, quint32 type, std::list<precitec::interface::ResultArgs> &&results)
    : m_path(path)
    , m_type(type)
    , m_results(std::move(results))
{
}

void ResultsWriterCommand::execute()
{
    ResultsSerializer serializer;
    serializer.setDirectory(m_path);
    serializer.setFileName(QString::number(m_type) + QStringLiteral(".result"));
    serializer.serialize(m_results);
}

}
}
