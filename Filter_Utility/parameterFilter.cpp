/**
 *  Filter_Utility::parameterFilter.cpp
 *	@file
 *	@copyright      Precitec Vision GmbH & Co. KG
 *	@author         Wolfgang Reichl (WoR)
 *	@date           25.10.2012
 *	@brief					<What's the purpose of the code in this file>
 */

#include "parameterFilter.h"
#include "geo/geo.h"
#include "module/moduleLogger.h"
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
namespace filter {

	using interface::ImageFrame;
	using fliplib::Parameter;

		ParameterFilter::ParameterFilter()
		: fliplib::TransformFilter("ParameterFilter", Poco::UUID{"1a7f9c78-ff0a-4574-ad2d-a587f0561e13"}),
			m_pPipeIn(NULL),
			m_oPipeOut(this, "Scalar"),
			m_oParam(1.0)
	  {
			// add parameters to parameter list
			parameters_.add( "scalar",	Parameter::TYPE_double,	static_cast<double>(m_oParam) );

            setInPipeConnectors({{Poco::UUID("b1fc92b7-be22-4765-b202-45fd88d6c2ce"), m_pPipeIn, "Image", 0, ""}});
            setOutPipeConnectors({{Poco::UUID("2495a1d1-6022-48c8-9b56-6e6ca2d67d88"), &m_oPipeOut, "Scalar", 0, ""}});
            setVariantID(Poco::UUID("7BC89CB7-48E0-4c94-9F6A-0F5F7FDBEEBA"));
		}

		void ParameterFilter::setParameter() {
			TransformFilter::setParameter();
			m_oParam = parameters_.getParameter("scalar").convert<double>();
		} // setParameter

		bool ParameterFilter::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
			m_pPipeIn = dynamic_cast< fliplib::SynchronePipe<ImageFrame>*>(&p_rPipe);
			return BaseFilter::subscribe( p_rPipe, p_oGroup );
		} // subscribe

        bool ParameterFilter::isValidConnected() const
        {
            if (m_pPipeIn == nullptr)
            {
                return false;
            }
            return true;
        }

		void ParameterFilter::proceed(const void* sender, fliplib::PipeEventArgs& e) 
		{
			poco_assert_dbg( m_pPipeIn != nullptr); // to be asserted by graph editor
			
			// here we check only for existence of the input, not its well-formedness;
			ImageFrame const& image( m_pPipeIn->read(m_oCounter) );
			// put parmeter into an array of length 1 with max rank
			geo2d::Doublearray output(1, m_oParam, filter::eRankMax);
			const auto oGeoOut  =   interface::GeoDoublearray(image.context(), output, image.analysisResult() , interface::Perfect);
			preSignalAction(); 
			m_oPipeOut.signal(oGeoOut);
		
		} // proceed

	} // namespace inspect
} // namespace precitec
