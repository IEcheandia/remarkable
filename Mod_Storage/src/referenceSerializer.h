#pragma once

#include "event/results.h"
#include "legacyReferenceCurve.h"
#include "product.h"

#include <QDataStream>
#include <QDir>

namespace precitec
{
namespace storage
{

/**
 * Class which is able to (de)serialize results
 **/
class ReferenceSerializer
{
public:
    ReferenceSerializer();
    virtual ~ReferenceSerializer();

    /**
     * Sets the directory in which to (de)serialize results.
     **/
    void setDirectory(const QDir &dir)
    {
        m_directory = dir;
    }

    /**
     * @returns The directory in which the filename is going to be (de)serialized.
     **/
    QDir directory() const
    {
        return m_directory;
    }

    /**
     * Sets the filename in which to (de)serialize results.
     **/
    void setFileName(const QString &fileName)
    {
        m_fileName = fileName;
    }

    /**
     * @returns The filename in the directory.
     * @see directory
     **/
    QString fileName() const
    {
        return m_fileName;
    }

    bool deserialize(LegacyReferenceCurve& curve) const
    {
        QDataStream in(readFile());
        if (verifyHeader(in))
        {
            in >> curve;
            return true;
        }
        return false;
    }

    bool serialize(Product* product) const
    {
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        writeHeader(out);
        out << product;
        return writeToFile(data);
    }

    bool deserialize(Product* product) const
    {
        QDataStream in(readFile());
        if (verifyHeader(in))
        {
            in >> product;
            return true;
        }
        return false;
    }

private:
    /**
     * Writes the header into the @p out data stream.
     **/
    void writeHeader(QDataStream &out) const;

    /**
     * Reads in the header and verifies that the magic version matches.
     **/
    bool verifyHeader(QDataStream &in) const;

    /**
     * Writes the uncompressed @p data into the file as compressed data.
     * @returns @c true on success, @c false if the file already exists, could not be opened for writing or if not all data was written.
     **/
    bool writeToFile(const QByteArray &data) const;
    /**
     * Reads the data from the file and returns the uncompressed data.
     * If the file could not be opened or not read, this method returns an empty byte array.
     **/
    QByteArray readFile() const;
    QDir m_directory;
    QString m_fileName;
};

}
}

