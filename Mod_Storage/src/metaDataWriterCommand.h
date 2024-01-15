#pragma once
#include "videoRecorder/baseCommand.h"

#include <QDir>
#include <QJsonDocument>


namespace precitec
{

namespace storage
{

/**
 * Command object which stores metadata as json file in the provided path.
 **/
class MetaDataWriterCommand : public vdr::BaseCommand
{
public:
    /**
     * @param path The path where to create the metadata.json file
     * @param args The json object to write
     **/
    explicit MetaDataWriterCommand(const QDir &path, std::initializer_list<QPair<QString, QJsonValue>> args);

    void execute() override;

private:
    QDir m_path;
    QJsonDocument m_json;
};

}
}

