#include "pathValidator.h"
#include <QFileInfo>

namespace precitec
{
namespace gui
{
namespace components
{
namespace scheduler
{
PathValidator::PathValidator(QObject *parent)
    : QValidator(parent)
{
}

PathValidator::~PathValidator() = default;

QValidator::State PathValidator::validate(QString &input, [[maybe_unused]] int &pos) const
{
    if (input.isEmpty())
    {
        return QValidator::Intermediate;
    }
    if (!input.startsWith(QChar{'/'}))
    {
        return QValidator::Invalid;
    }
    if (!input.endsWith(QChar{'/'}))
    {
        return QValidator::Intermediate;
    }

    if (!m_localFileSystem)
    {
        return QValidator::Acceptable;
    }

    const QFileInfo path{input};
    return path.exists() && path.isDir() && path.isWritable() ? QValidator::Acceptable : QValidator::Intermediate;
}

void PathValidator::setLocalFileSystem(bool value)
{
    if (m_localFileSystem == value)
    {
        return;
    }
    m_localFileSystem = value;
    emit localFileSystemChanged();
}

}
}
}
}

