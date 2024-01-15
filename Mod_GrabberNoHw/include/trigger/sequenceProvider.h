/**
*
* @defgroup Framegrabber Framegrabber
*
* \section sec Image Loading
*
*
* @file
* @brief  Load previously recorded images corresponding to product, seamseries and seam
* @copyright    Precitec GmbH & Co. KG
* @author GG
* @date   09.05.17
*
*
*/

#pragma once

#include "common/triggerContext.h"
#include "sequenceLoader.h"

namespace precitec
{
    using namespace interface;

namespace grabber
{
class SharedMemoryImageProvider;
}

class SequenceProvider
{
public:

    SequenceProvider();
    ~SequenceProvider();

    void init(grabber::SharedMemoryImageProvider *memory);
    void close(void);
    bool getImage(TriggerContext & _triggerContext, image::BImage & _image );
    bool getSamples(TriggerContext & _triggerContext, fileio::SampleDataHolder & _sampleDataHolder );
    void reload(uint32_t _productNumber);
    void setBasepath(std::string const & _path);
    void setTestImagesProductInstanceMode(bool set);
    void setTestProductInstance(const Poco::UUID &productInstance);
private:

    bool m_IsInitialized;
    std::string m_strBasepath;
    SequenceLoader m_SequenceLoader;

};
} // namespace precitec
