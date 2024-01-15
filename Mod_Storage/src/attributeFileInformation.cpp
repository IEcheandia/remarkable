#include "attributeFileInformation.h"
#include "jsonSupport.h"
#include <QJsonObject>

namespace precitec
{
namespace storage
{

AttributeFileInformation::AttributeFileInformation() = default;

AttributeFileInformation::~AttributeFileInformation() = default;

std::optional<AttributeFileInformation> AttributeFileInformation::fromJson(const QJsonObject& object)
{
    if (object.empty())
    {
        return {};
    }
    AttributeFileInformation file{};
    file.setSuffixes(json::parseFileSuffixes(object));
    file.setLocation(json::parseFileLocation(object));
    return file;
}

}
}
