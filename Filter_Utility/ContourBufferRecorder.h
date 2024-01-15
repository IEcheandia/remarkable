/**
*  @copyright:		Precitec Vision GmbH & Co. KG
*  @author			LB
*  @date			05/2020
*  @file
*  @brief			This filter stores data elements of type Contour (point list) in a buffer (similar to the filter BufferPlayer, but with contour elements ). They can be accessed in a different filter graph using the contour buffer player filter.
*/

#pragma once

#include <fliplib/Fliplib.h>
#include <fliplib/TransformFilter.h>
#include <fliplib/SynchronePipe.h>
#include <geo/geo.h>

namespace precitec
{
namespace filter
{

//This filter stores data elements of type Contour (point list) in a buffer. They can be accessed in a different filter graph using the contour buffer player filter.
class FILTER_API ContourBufferRecorder : public fliplib::TransformFilter
{
public:

    ContourBufferRecorder();

    virtual ~ContourBufferRecorder();

    void setParameter() override;

    void arm(const fliplib::ArmStateBase& state) override;

private:

    bool subscribe(fliplib::BasePipe& pipe, int group) override;

    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;

    void setProductInformationParameter();

    static const std::string m_filterName; //Filter name.
    static const std::string m_paramSlot; //Slot, into which the data is written.

    typedef fliplib::SynchronePipe<interface::GeoDoublearray> pipe_scalar_t;
    typedef fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray> pipe_contour_t;
    const pipe_contour_t* m_pipeInData;
    const pipe_scalar_t* m_pipeInPos;

    unsigned int m_slot; //Slot, into which the data is written.
    unsigned int m_numTrigger;
    unsigned int m_triggerDelta; //Trigger distance [um]
    unsigned int m_seamSeries;
    unsigned int m_seam;
    int m_writeIndex;
    const int m_maxNumTriggers = 100000;

    std::shared_ptr< interface::GeoVecAnnotatedDPointarray> m_data; // Pointer to the actual data array. Is valid after arm.
    std::shared_ptr< interface::GeoDoublearray> m_pos; // Pointer to the actual pos array. Is valid after arm.
};

} // namespace filter
} // namespace precitec

