/**
* 	@file
* 	@copyright	Precitec GmbH & Co. KG
* 	@author		LB
* 	@date		2023
* 	@brief		This filter computes basic subsampling with the same parameters for 2 inputs
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

class FILTER_API DataSubsampling2 : public fliplib::TransformFilter
{
public:
    DataSubsampling2();
    void setParameter() override;

    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;

private:
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_pPipeInData1;
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_pPipeInData2;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_oPipeOutData1;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_oPipeOutData2;

    int m_oOperation;           ///< Parameter: Operation, e.g. Addition, Subtraction, Multiplication, ...
    bool m_oPassThroughBadRank; ///< Parameter: If PassThroughBadRank is false, data with bad rank is ignored
};

}
}
