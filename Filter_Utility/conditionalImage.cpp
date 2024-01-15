/**
 * 	@file
 * 	@copyright	Precitec GmbH & Co. KG
 * 	@author		AL
 * 	@date		2015
 * 	@brief		This is a conditional filter.
 * 				Inputs: data_a, data_b, quality_a, quality_b
 * 				Outputs: data_out, compareResult
 * 				Compare Mode:
 * 								if quality_a > quality_b then data_out == data_a and operationResult = 1
 * 								if quality_a < quality_b then data_out == data_b and operationResult = -1
 * 								if quality_a == quality_b then data_out == data_a and Result = 1
 *
 * 	 	 	 	Encoder-Compare Mode:
 * 								if quality_a < quality_b then data_out == data_a and operationResult = 1
 * 								if quality_a >= quality_b then data_out == lastValid value of data_a and operationResult = 0
 *
 */

// todo: operation enum

// project includes
#include "conditionalImage.h"
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <event/resultType.h>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
namespace filter {

	const std::string ConditionalImage::m_oFilterName("ConditionalImage");			///< FilterName
	const std::string ConditionalImage::m_oPipeOutDataName("ImageFrame");				///< Pipe: Data out-pipe.
	const std::string ConditionalImage::m_oPipeOutOperationResultName("OperationResult_out");	///< Pipe: Data out-pipe.
	//const std::string ConditionalImage::m_oParamOperation("Operation");			///< Parameter: Amount of delay [um].


	ConditionalImage::ConditionalImage() :
		TransformFilter(ConditionalImage::m_oFilterName, Poco::UUID{"0986C7B1-D740-4A64-BE33-7207720833D0"}),
	m_pPipeInImageA				( NULL ),
	m_pPipeInImageB				( NULL ),
	m_pPipeInQualityA			( nullptr ),
	m_pPipeInQualityB			( nullptr ),
	m_oPipeOutDataOut(this, ConditionalImage::m_oPipeOutDataName),
	m_oPipeOutOperationResult(this, ConditionalImage::m_oPipeOutOperationResultName),
	//m_oOperation				( 0 ),
	m_lastValidValueDataA		( 0 )
{
	//parameters_.add( m_oParamOperation, fliplib::Parameter::TYPE_int, m_oOperation );

    setInPipeConnectors({{Poco::UUID("F237E707-39D8-4B78-94A6-5E487B1A3039"), m_pPipeInImageA, "ImageA", 1, "image_a"},
    {Poco::UUID("E3C06580-5A2E-406D-9F27-0B6683441125"), m_pPipeInImageB, "ImageB", 1, "image_b"},
    {Poco::UUID("8219AC89-6F58-462B-B337-C6D27279F09A"), m_pPipeInQualityA, "QualityA", 1, "quality_a"},
    {Poco::UUID("55ACCD00-70B9-492A-A2D1-4D1A0CE11C01"), m_pPipeInQualityB, "QualityB", 1, "quality_b"}});
    setOutPipeConnectors({{Poco::UUID("CD051893-C3C8-407D-9E01-79CCF1B4C2ED"), &m_oPipeOutDataOut, m_oPipeOutDataName, 0, "ImageFrame"},
    {Poco::UUID("2E485F5E-232C-4FE4-B037-5131DA27E13E"), &m_oPipeOutOperationResult, m_oPipeOutOperationResultName, 0, "operationResult_out"}});
    setVariantID(Poco::UUID("4E857EE3-D1C7-4F6D-AC59-A7FBDCE4FEA8"));
} // CTor



	/*virtual*/ ConditionalImage::~ConditionalImage()
{

} // DTor



	void ConditionalImage::setParameter()
{
	TransformFilter::setParameter();

	//m_oOperation = parameters_.getParameter(ConditionalImage::m_oParamOperation).convert<int>();

} // setParameter.



	bool ConditionalImage::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.tag() == "image_a" )
	{
		m_pPipeInImageA = dynamic_cast< fliplib::SynchronePipe < interface::ImageFrame > * >(&p_rPipe);
	}
	if ( p_rPipe.tag() == "image_b" )
	{
		m_pPipeInImageB = dynamic_cast< fliplib::SynchronePipe < interface::ImageFrame > * >(&p_rPipe);
	}
	if ( p_rPipe.tag() == "quality_a" )
	{
		m_pPipeInQualityA = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	}
	if ( p_rPipe.tag() == "quality_b" )
	{
		m_pPipeInQualityB = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	}

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



	void ConditionalImage::SignalOperationResultOut(double operationResult, const interface::ImageFrame &rImageInData)
{
	geo2d::Doublearray oOutOperationResult;
	oOutOperationResult.assign(1);
	double maxRank = eRankMax;
	oOutOperationResult[0] = std::tie(operationResult, maxRank);
	const interface::GeoDoublearray oGeoDoubleOperationResultOut(rImageInData.context(), oOutOperationResult, interface::AnalysisOK, eRankMax);
    m_oPipeOutOperationResult.signal(oGeoDoubleOperationResultOut);
}

	void ConditionalImage::proceedGroup(const void *p_pSender, fliplib::PipeGroupEventArgs & e)
{
		poco_assert_dbg(m_pPipeInImageA != nullptr); // to be asserted by graph editor
		poco_assert_dbg(m_pPipeInImageB != nullptr); // to be asserted by graph editor
	poco_assert_dbg( m_pPipeInQualityA != nullptr); // to be asserted by graph editor
	poco_assert_dbg( m_pPipeInQualityB != nullptr); // to be asserted by graph editor


	ProceedCompare();
//		break;
//	}
//	case 1: //encoder compare
//	{
//		ProceedEncoderCompare();
//		break;
//	}
//	}
} // proceedGroup

	void ConditionalImage::ProceedCompare()
{
	// data
		const interface::ImageFrame & rImageFrameInDataA = m_pPipeInImageA->read(m_oCounter);
		const interface::ImageFrame & rImageFrameInDataB = m_pPipeInImageB->read(m_oCounter);
	const interface::GeoDoublearray & rGeoDoubleArrayInQualityA = m_pPipeInQualityA->read(m_oCounter);
	const interface::GeoDoublearray & rGeoDoubleArrayInQualityB = m_pPipeInQualityB->read(m_oCounter);

	const double quality_a = std::get<eData>(rGeoDoubleArrayInQualityA.ref()[0]);
	const double quality_b = std::get<eData>(rGeoDoubleArrayInQualityB.ref()[0]);

	const interface::ImageFrame* outData = NULL;
	double operationResult = 0;

	if(quality_a >= quality_b)
	{
		outData = &rImageFrameInDataA;
		operationResult = 1;
	}
	else
	{
		outData = &rImageFrameInDataB;
		operationResult = -1;
	}

	if(outData != NULL)
	{
		preSignalAction();
		SignalOperationResultOut(operationResult, *outData);
		m_oPipeOutDataOut.signal(*outData);

	}
    else
    {
        preSignalAction();
    }
}



} // namespace filter
} // namespace precitec
