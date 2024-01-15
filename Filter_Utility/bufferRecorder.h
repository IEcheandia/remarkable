/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		This filter stores data elements in a buffer. They can be accessed in a different filter graph using the buffer player filter. See also ContourBufferRecorder for the same functionality with contour elements
 */

#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>

namespace precitec
{
namespace filter
{

// This filter stores data elements in a buffer. They can be accessed in a different filter graph using the buffer player filter.
class FILTER_API BufferRecorder: public fliplib::TransformFilter
{
public:

    BufferRecorder();

    ~BufferRecorder() override;

    void setParameter() override;

    //Arm the filter. This means here, that the length of the seam is determined and memory is allocated for all the data elements.
    void arm(const fliplib::ArmStateBase& state) override;

private:

    bool subscribe(fliplib::BasePipe& pipe, int group) override;

    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event ) override;

    void setProductInformationParameter();

    int getDataSize();

    int getPosSize();

    static const std::string m_filterName;
    static const std::string m_paramSlot;

    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_pipeInPos; // Position in-pipe.
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_pipeInData; // Data in-pipe.

    std::shared_ptr<interface::GeoDoublearray> m_data; // map pointer to the actual data array. Is valid after arm.
    std::shared_ptr<interface::GeoDoublearray> m_pos; // Pointer to the actual pos array. Is valid after arm.

    unsigned int m_slot; // Slot, into which the data is written.
    unsigned int m_triggerDelta;
    unsigned int m_numTrigger;
    unsigned int m_seamSeries;
    unsigned int m_seam;
    int m_writeIndex;
};

} // namespace filter
} // namespace precitec

