/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		LB
 * 	@date		08.2018
 * 	@brief		This filter change the value and context of an input GeoDouble, in order to be consistent with a reference context
 */


#ifndef CONTEXTNORMALIZEDOUBLE_H
#define CONTEXTNORMALIZEDOUBLE_H

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"

#include "common/frame.h"


namespace precitec
{
namespace filter {

class FILTER_API ContextNormalizeDouble : public fliplib::TransformFilter
{
public:

	/**
	 * CTor.
	 */
	ContextNormalizeDouble();
	/**
	 * @brief DTor.
	 */
	virtual ~ContextNormalizeDouble();

	/// Set filter parameters as defined in database / xml file.
	void setParameter();
    
	// Declare constants
	static const std::string m_oFilterName;			///< Filter name.
	static const std::string m_oPipeInValue1Name;		///< Out-pipe 
	static const std::string m_oPipeInValue2Name;		///< Out-pipe 
	static const std::string m_oPipeOut1Name;		///< Out-pipe 
	static const std::string m_oPipeOut2Name;		///< Out-pipe 



protected:
	/// in pipe registration
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	/// In-pipe event processing.
	void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);


private:
	enum class ParameterType
	{
		eX , eY, ePureNumber, eWidth, eHeight, eMaxValid=eHeight
	};
	static ParameterType parameterTypeTransposed(ParameterType param);
	static geo2d::TArray<double> transformDoubleArray(const geo2d::TArray<double>& inputArray, ParameterType & rParamType, 
        const interface::ImageContext & rContext, const interface::ImageContext & rContextReference, bool handleSampling);

	const fliplib::SynchronePipe< interface::ImageFrame >* m_pPipeInImageFrame;    ///< in pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >* m_pPipeInValue1;	///< In pipe
	const fliplib::SynchronePipe< interface::GeoDoublearray >* m_pPipeInValue2;	///< In pipe
	const fliplib::SynchronePipe<interface::GeoDoublearray>* m_pipeAngle;
	fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOut1;	///< Out pipe
	fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOut2;	///< Out pipe
	ParameterType m_oParameterType1;
	ParameterType m_oParameterType2;
    bool m_oHandleSampling;
}; 

} // namespace filter
} // namespace precitec

#endif /* CONTEXTNORMALIZEDOUBLE_H */
