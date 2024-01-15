/**
 *	@file
 *	@copyright      Precitec Vision GmbH & Co. KG
 *	@author         Wolfgang Reichl (WoR)
 *	@date           25.10.2012
 */

#ifndef PARAMETERFILTER_DOUBLE_H_
#define PARAMETERFILTER_DOUBLE_H_
#pragma once

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>
// stl includes
#include <vector>


namespace precitec {
namespace filter {

    
/**
    * @brief Parameterfilter has a dummy pipe input, so the filter can be triggered. It has a double parameter, which is piped out every time the filter is triggered.
    */
class FILTER_API ParameterFilterDouble : public fliplib::TransformFilter  
{
    
public:
    
    ParameterFilterDouble();
    virtual ~ParameterFilterDouble() {}
    
public:
    
    /// initialize local parameters from database
    virtual void setParameter();
private:
    
    /// registration input pipe
    virtual bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
    /// do the actual processing; N inputs so simple proceedGroup()
    virtual void proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rEventArg);
    
private:
    
    /// image in-pipe (needed for trigger)
    const fliplib::SynchronePipe< interface::GeoDoublearray >*            m_pPipeIn;
    /// out pipe
    fliplib::SynchronePipe< interface::GeoDoublearray >                   m_oPipeOut;
    /// scale factor
    double	m_oParam;

};


} // namespace filter
} // namespace precitec

#endif // PARAMETERFILTER_DOUBLE_H_

