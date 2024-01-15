/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		LB
 * 	@date		2020
 * 	@brief		This filter retrieves contour elements from a global buffer(similar to the filter BufferPlayer, but with contour elements ). The data elements were stored in the buffer by the ContourBufferRecorder filter.
 */

#pragma once

#include <fliplib/Fliplib.h>
#include <fliplib/TransformFilter.h>
#include <fliplib/SynchronePipe.h>
#include <geo/geo.h>
#include <common/frame.h>

namespace precitec
{
namespace filter
{
// This filter retrieves data elements from a global buffer. The data elements were stored in the buffer by the BufferRecorder filter.
class FILTER_API ContourBufferPlayer: public fliplib::TransformFilter
{
public:

    ContourBufferPlayer();

    virtual ~ContourBufferPlayer();

    void setParameter() override;

    void arm(const fliplib::ArmStateBase& state) override;

private:

    bool subscribe(fliplib::BasePipe& pipe, int group) override;

    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;

    void setProductInformationParameter();

    static const std::string m_filterName;
    static const std::string m_pipeOutName;
    static const std::string m_paramSlot; // Slot, into which the data is written.
    static const std::string m_paramDataOffset; // To compensate for the time it takes to read the image, read encoder values, etc.
    static const std::string m_paramSeamSeriesOffset; // SeamSerie offset - should the data be taken from a previous or later series?
    static const std::string m_paramSeamOffset; // Seam offset - should the data be taken from a previous or later seam?

    typedef fliplib::SynchronePipe<interface::ImageFrame> pipe_image_t;
    typedef fliplib::SynchronePipe<interface::GeoDoublearray> pipe_scalar_t;
    typedef fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray> pipe_contour_t;

    const pipe_image_t* m_pipeInImage;
    const pipe_scalar_t* m_pipeInPos;
    pipe_contour_t m_pipeOutData;
    unsigned int m_slot; // Slot, into which the data is written.
    int m_dataOffset; // To compensate for the time it takes to read the image, read encoder values, etc.
    int m_seamSeriesOffset; // seamSeries offset - should the data be taken from a previous or later seamSeries?
    int m_seamOffset; // seam offset - should the data be taken from a previous or later seam?
    unsigned int m_seamSeries;
    unsigned int m_seam;
    int m_requestedSeamSeries;
    int m_requestedSeam;
    bool m_ready; // Is the buffer ready to be read?

    std::shared_ptr<interface::GeoVecAnnotatedDPointarray> m_data;
    std::shared_ptr<interface::GeoDoublearray> m_pos;
};

}
}
