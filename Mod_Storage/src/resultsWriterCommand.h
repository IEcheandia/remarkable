#pragma once
#include "videoRecorder/baseCommand.h"

#include <QDir>

#include <list>

namespace precitec
{

namespace interface
{
class ResultArgs;
}

namespace storage
{

class ResultsWriterCommand : public vdr::BaseCommand
{
public:
    explicit ResultsWriterCommand(const QDir &path, quint32 type, std::list<precitec::interface::ResultArgs> &&results);

    void execute() override;

private:
    QDir m_path;
    quint32 m_type;
    std::list<precitec::interface::ResultArgs> m_results;
};

}
}
