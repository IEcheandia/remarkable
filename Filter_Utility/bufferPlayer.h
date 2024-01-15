/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		This filter retrieves data elements from a global a buffer. The data elements were stored in the buffer by the BufferRecorder filter. See also ContourBufferPlayer for the same functionality with contour elements
 */

#pragma once

#include <fliplib/Fliplib.h>
#include <fliplib/TransformFilter.h>
#include <fliplib/SynchronePipe.h>
#include <geo/geo.h>
#include "common/frame.h"

namespace precitec
{
namespace filter
{

// This filter retrieves data elements from a global a buffer. The data elements were stored in the buffer by the BufferRecorder filter.
class FILTER_API BufferPlayer: public fliplib::TransformFilter
{
public:

    BufferPlayer();

    ~BufferPlayer() override;

    void setParameter() override;

    void arm(const fliplib::ArmStateBase& state) override;

private:

    bool subscribe(fliplib::BasePipe& pipe, int group) override;

    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;

    void setProductInformationParameter();

    const fliplib::SynchronePipe<interface::ImageFrame>* m_pipeInImage;
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_pipeInPos;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_pipeOutData;

    // Declare constants
    static const std::string m_filterName;
    static const std::string m_pipeOutName;
    static const std::string m_paramSlot; // Slot, into which the data is written.
    static const std::string m_paramDataOffset; // To compensate for the time it takes to read the image, read encoder values, etc.
    static const std::string m_paramSeamSeriesOffset; //SeamSeries offset - should the data be taken from a previous or later seamSeries?
    static const std::string m_paramSeamOffset; //Seam offset - should the data be taken from a previous or later seam?

    unsigned int m_slot;
    int m_dataOffset;
    int m_seamSeriesOffset;
    int m_seamOffset;
    int m_requestedSeamSeries;
    int m_requestedSeam;
    int m_image;
    unsigned int m_count; // Current read-index into the buffer.
    bool m_ready; // Is the buffer ready to be read?

    std::shared_ptr<interface::GeoDoublearray> m_data;
    std::shared_ptr<interface::GeoDoublearray> m_pos;

    int m_triggerDelta; // Trigger distance [um]
    int m_seamSeries;
    int m_seam;

};


} // namespace filter
} // namespace precitec
