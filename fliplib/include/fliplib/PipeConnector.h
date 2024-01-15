#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/SynchronePipe.h"

#include <Poco/UUID.h>

namespace fliplib
{

template <typename T>
struct TypeToDataType
{
    typedef T value_type;
};

/**
 * The PipeConnector describes an in or out port of a Filter.
 **/
class FLIPLIB_API PipeConnector
{
public:
    /**
     * The data types the pipes can carry.
     * A PipeConnector is specific to one DataType
     **/
    enum class DataType
    {
        Image,
        Sample,
        Blobs,
        Line,
        Double,
        PointList,
        SeamFinding,
        PoorPenetrationCandidate,
        StartEndInfo,
        SurfaceInfo,
        HoughPPCandidate,
        LineModel
    };

    enum class ConnectionType
    {
        Mandatory,
        Optional,
        MandatoryForInOptionalForOut
    };

    PipeConnector(const Poco::UUID &id, DataType type, const std::string &name, const unsigned int group, const std::string &tag = {}, ConnectionType connectionType = ConnectionType::MandatoryForInOptionalForOut)
        : m_id(id)
        , m_type(type)
        , m_name(name)
        , m_group(group)
        , m_tag(tag)
        , m_connectionType(connectionType)
    {
    }

    template <typename T>
    PipeConnector(const Poco::UUID &id, SynchronePipe<T> *pipe, const std::string &fallbackName = {}, const unsigned int group = 1, const std::string &tag = {}, ConnectionType connectionType = ConnectionType::MandatoryForInOptionalForOut)
        : PipeConnector(id, TypeToDataType<T>::Type, pipe ? pipe->name() : fallbackName, group, tag, connectionType)
        {
        }

    template <typename T>
    PipeConnector(const Poco::UUID &id, const SynchronePipe<T> *pipe, const std::string &fallbackName = {}, const unsigned int group = 1, const std::string &tag = {}, ConnectionType connectionType = ConnectionType::MandatoryForInOptionalForOut)
        : PipeConnector(id, TypeToDataType<T>::Type, pipe ? pipe->name() : fallbackName, group, tag, connectionType)
        {
        }

    /**
     * The unique id of the PipeConnector
     **/
    const Poco::UUID &id() const
    {
        return m_id;
    }

    /**
     * The DataType of the PipeConnector
     **/
    DataType dataType() const
    {
        return m_type;
    }

    /**
     * The name of the PipeConnector
     **/
    const std::string &name() const
    {
        return m_name;
    }

    /**
     * The group of pipeConnector
     * Is used to show if the filter has the method proceed (0), proceedGroup(1) or a specific setup (1024)
     **/
    unsigned int group() const
    {
        return m_group;
    }

    /**
     * The tag of the pipeConnector
     * Is used to distinguish between multiple input pipes of the same type.
     **/
    const std::string &tag() const
    {
        return m_tag;
    }

    ConnectionType connectionType() const
    {
        return m_connectionType;
    }

private:
    Poco::UUID m_id;
    DataType m_type;
    std::string m_name;
    unsigned int m_group;
    std::string m_tag;
    ConnectionType m_connectionType;
};

}
