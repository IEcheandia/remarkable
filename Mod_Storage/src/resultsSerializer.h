#pragma once

#include "event/results.h"

#include <QDataStream>
#include <QDir>

namespace Poco
{
class UUID;
}

namespace precitec
{

namespace interface
{
class ImageContext;
class ResultArgs;
}

namespace storage
{

/**
 * Class which is able to (de)serialize results
 **/
class ResultsSerializer
{
public:
    ResultsSerializer();
    virtual ~ResultsSerializer();

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

    /**
     * Serializes the given @p results container to the file specified by setDirectory and setFileName.
     * @returns @c true if the results where serialized to the file, @c false on error.
     * @see setDirectory
     * @see setFileName
     **/
    template <template <typename T, typename Allocator> class List>
    bool serialize(const List<precitec::interface::ResultArgs, std::allocator<precitec::interface::ResultArgs>> &results) const
    {
        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        writeHeader(out);
        out << results;
        return writeToFile(data);
    }

    /**
     * Deserializes the file to a container which supports emplace_back and back.
     **/
    template <template<typename,typename> class List>
    List<precitec::interface::ResultArgs, std::allocator<precitec::interface::ResultArgs>> deserialize() const
    {
        List<precitec::interface::ResultArgs, std::allocator<precitec::interface::ResultArgs>> ret;
        QDataStream in(readFile());
        if (verifyHeader(in))
        {
            in >> ret;
        }
        return ret;
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
     * Writes @p data into the file.
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

QDataStream &operator<<(QDataStream &out, const precitec::interface::ResultArgs &result);
QDataStream &operator>>(QDataStream &in, precitec::interface::ResultArgs &result);

template <template <typename T, typename Allocator> class List>
QDataStream &operator<<(QDataStream &out, const List<precitec::interface::ResultArgs, std::allocator<precitec::interface::ResultArgs>> &results)
{
    out << quint64(results.size());
    for (const auto &result : results)
    {
        out << result;
    }
    return out;
}

template <template <typename T, typename Allocator> class List>
QDataStream &operator>>(QDataStream &in, List<precitec::interface::ResultArgs, std::allocator<precitec::interface::ResultArgs>> &results)
{
    quint64 size;
    in >> size;
    for (quint64 i = 0; i < size; i++)
    {
        precitec::interface::ResultArgs result;
        in >> result;
        results.emplace_back(std::move(result));
    }
    return in;
}

QDataStream &operator<<(QDataStream &out, const precitec::interface::ImageContext &context);
QDataStream &operator>>(QDataStream &in, precitec::interface::ImageContext &context);

QDataStream &operator<<(QDataStream &out, const Poco::UUID &id);
QDataStream &operator>>(QDataStream &in, Poco::UUID &id);
