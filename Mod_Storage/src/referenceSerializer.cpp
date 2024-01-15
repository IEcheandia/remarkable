#include "referenceSerializer.h"
#include "precitec/dataSet.h"

#include <QFile>
#include <QSaveFile>

namespace precitec
{
namespace storage
{

/*
 * The magic version is generated through:
 * sha256 on "weldmaster" and taking the first four bytes.
 **/
static const quint32 s_magicHeaderNumber = 0xCE983826;
static const quint32 s_versionNumber = 1;

ReferenceSerializer::ReferenceSerializer() = default;
ReferenceSerializer::~ReferenceSerializer() = default;

void ReferenceSerializer::writeHeader(QDataStream &out) const
{
    out.setVersion(QDataStream::Qt_5_10);
    out << s_magicHeaderNumber;
    out << s_versionNumber;
}

bool ReferenceSerializer::verifyHeader(QDataStream &in) const
{
    quint32 magic;
    in >> magic;
    if (magic != s_magicHeaderNumber)
    {
        return false;
    }
    quint32 version;
    in >> version;
    if (version != s_versionNumber)
    {
        return false;
    }
    in.setVersion(QDataStream::Qt_5_10);
    return true;
}

bool ReferenceSerializer::writeToFile(const QByteArray &data) const
{
    QSaveFile file(m_directory.absoluteFilePath(m_fileName));
    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }
    const QByteArray compressedData = qCompress(data, 5);
    if (file.write(compressedData) != compressedData.size())
    {
        return false;
    }
    return file.commit();
}

QByteArray ReferenceSerializer::readFile() const
{
    QFile file(m_directory.absoluteFilePath(m_fileName));
    if (!file.exists())
    {
        return QByteArray{};
    }
    if (!file.open(QIODevice::ReadOnly))
    {
        return QByteArray{};
    }
    return qUncompress(file.readAll());
}

}
}

