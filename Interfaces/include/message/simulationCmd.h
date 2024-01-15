#pragma once

#include "message/serializer.h"

namespace precitec
{
namespace interface
{

/**
 * @short Status information after simulating a frame.
 **/
class SimulationFrameStatus : public system::message::Serializable
{
public:
    SimulationFrameStatus() = default;

    /**
     * @returns whether a previous frame is available, @link{TSimulationCmd<AbstractInterface>::previousFrame} can be used.
     **/
    bool hasPreviousFrame() const
    {
        return m_hasPreviousFrame;
    }

    /**
     * @returns whether a next frame is available, @link{TSimulationCmd<AbstractInterface>::nextFrame} can be used.
     **/
    bool hasNextFrame() const
    {
        return m_hasNextFrame;
    }

    /**
     * @returns whether a previous seam is availabe, @link{TSimulationCmd<AbstractInterface>::previousSeam} can be used.
     **/
    bool hasPreviousSeam() const
    {
        return m_hasPreviousSeam;
    }

    /**
     * @returns whether a next seam is available, @link{TSimulationCmd<AbstractInterface>::nextSeam} can be used.
     **/
    bool hasNextSeam() const
    {
        return m_hasNextSeam;
    }

    /**
     * Marks whether there is a previous frame.
     * @param set Whether there is a previous frame
     **/
    void setHasPreviousFrame(bool set)
    {
        m_hasPreviousFrame = set;
    }

    /**
     * Marks whether there is a next frame.
     * @param set Whether there is a next frame
     **/
    void setHasNextFrame(bool set)
    {
        m_hasNextFrame = set;
    }

    /**
     * Marks whether there is a previous seam.
     * @param set Whether there is a previous seam
     **/
    void setHasPreviousSeam(bool set)
    {
        m_hasPreviousSeam = set;
    }

    /**
     * Marks whether there is a next seam.
     * @param set Whether there is a next seam
     **/
    void setHasNextSeam(bool set)
    {
        m_hasNextSeam = set;
    }

    /**
     * @returns The index of the just simulated image in the sequence of images to be simulated.
     **/
    uint32_t frameIndex() const
    {
        return m_frameIndex;
    }

    /**
     * Sets the index of the just simulated image in the sequence of images to be simulated.
     **/
    void setFrameIndex(uint32_t index)
    {
        m_frameIndex = index;
    }

    void serialize(system::message::MessageBuffer &p_buffer) const override
    {
        marshal(p_buffer, m_hasPreviousFrame);
        marshal(p_buffer, m_hasNextFrame);
        marshal(p_buffer, m_hasPreviousSeam);
        marshal(p_buffer, m_hasNextSeam);
        marshal(p_buffer, m_frameIndex);
    }

    void deserialize(system::message::MessageBuffer const &p_buffer) override
    {
        deMarshal(p_buffer, m_hasPreviousFrame);
        deMarshal(p_buffer, m_hasNextFrame);
        deMarshal(p_buffer, m_hasPreviousSeam);
        deMarshal(p_buffer, m_hasNextSeam);
        deMarshal(p_buffer, m_frameIndex);
    }

private:
    bool m_hasPreviousFrame = false;
    bool m_hasNextFrame = false;
    bool m_hasPreviousSeam = false;
    bool m_hasNextSeam = false;
    uint32_t m_frameIndex = 0;
};

/**
 * @short Status information after initializing the Simulation.
 **/
class  SimulationInitStatus : public system::message::Serializable
{
public:
    SimulationInitStatus() = default;

    /**
     * Struct describing information about an image participating in the simulation
     **/
    struct ImageData
    {
        /**
         * The seam series number of the image
         **/
        uint32_t seamSeries = 0;
        /**
         * The seam number of the image
         **/
        uint32_t seam = 0;
        /**
         * The image number
         **/
        uint32_t image = 0;
    };

    /**
     * @returns Information about all images in the current simulation.
     **/
    const std::vector<ImageData> &imageData() const
    {
        return m_imageData;
    }

    /**
     * Sets the @p data about all images available in the current simulation
     **/
    void setImageData(const std::vector<ImageData> &data)
    {
        m_imageData = data;
    }

    /**
     * @returns The simulation frame status at the start of the simulation.
     **/
    const SimulationFrameStatus &status() const
    {
        return m_status;
    }

    /**
     * Sets the simulation frame @p status at the start of the simulation.
     **/
    void setStatus(const SimulationFrameStatus &status)
    {
        m_status = status;
    }

    /**
     * @returns the absolute base path where the images of the productInsance are located in seamSeries/seam structure.
     **/
    const std::string &imageBasePath() const
    {
        return m_imageBasePath;
    }

    /**
     * Sets the absolute @path where the images are located.
     **/
    void setImageBasePath(const std::string &path)
    {
        m_imageBasePath = path;
    }

    void serialize(system::message::MessageBuffer &p_buffer) const override
    {
        marshal(p_buffer, m_status);
        marshal(p_buffer, m_imageBasePath);
        marshal(p_buffer, m_imageData.size());
        for (const auto &data : m_imageData)
        {
            marshal(p_buffer, data.seamSeries);
            marshal(p_buffer, data.seam);
            marshal(p_buffer, data.image);
        }
    }

    void deserialize(system::message::MessageBuffer const &p_buffer) override
    {
        deMarshal(p_buffer, m_status);
        deMarshal(p_buffer, m_imageBasePath);
        std::size_t size = 0;
        deMarshal(p_buffer, size);
        m_imageData.reserve(size);
        for (std::size_t i = 0; i < size; i++)
        {
            ImageData data;
            deMarshal(p_buffer, data.seamSeries);
            deMarshal(p_buffer, data.seam);
            deMarshal(p_buffer, data.image);
            m_imageData.push_back(data);
        }
    }

private:
    std::vector<ImageData> m_imageData;
    SimulationFrameStatus m_status;
    std::string m_imageBasePath;
};

}
}
