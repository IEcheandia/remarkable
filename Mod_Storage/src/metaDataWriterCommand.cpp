#include "metaDataWriterCommand.h"

#include <QJsonObject>

namespace precitec
{
namespace storage
{

MetaDataWriterCommand::MetaDataWriterCommand(const QDir &path, std::initializer_list<QPair<QString, QJsonValue>> args)
    : m_path(path)
    , m_json(QJsonObject{std::move(args)})
{
}

void MetaDataWriterCommand::execute()
{
    QFile file{m_path.absoluteFilePath(QStringLiteral("metadata.json"))};
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(m_json.toJson());
    }
}

}
}
