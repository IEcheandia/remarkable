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
#include "conditional.h"
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <event/resultType.h>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
namespace filter {

const std::string Conditional::m_oFilterName 					( "Conditional" );			///< FilterName
const std::string Conditional::m_oPipeOutDataName				( "Data_out");				///< Pipe: Data out-pipe.
const std::string Conditional::m_oPipeOutOperationResultName	( "OperationResult_out");	///< Pipe: Data out-pipe.
const std::string Conditional::m_oPipeOutOtherDataName			( "OtherData_out");	///< Pipe: Data out-pipe.
const std::string Conditional::m_oParamOperation				( "Operation" );			///< Parameter: Amount of delay [um].


Conditional::Conditional() :
	TransformFilter				( Conditional::m_oFilterName, Poco::UUID{"CB3E23AF-A9CB-4D9E-92CD-7238B51BB806"} ),
	m_pPipeInDataA				( nullptr ),
	m_pPipeInDataB				( nullptr ),
	m_pPipeInQualityA			( nullptr ),
	m_pPipeInQualityB			( nullptr ),
	m_oPipeOutDataOut			( this, Conditional::m_oPipeOutDataName ),
	m_oPipeOutOperationResult	( this, Conditional::m_oPipeOutOperationResultName ),
	m_oPipeOutOtherDataOut		(this, Conditional::m_oPipeOutOtherDataName),
	m_oOperation				( 0 ),
	m_lastValidValueDataA		( 0 )
{
	parameters_.add( m_oParamOperation, fliplib::Parameter::TYPE_int, m_oOperation );

    setInPipeConnectors({{Poco::UUID("39637C2D-59DE-4031-817B-1C935AB9C061"), m_pPipeInDataA, "DataA", 1, "data_a"},
    {Poco::UUID("684AD09A-62E9-47F1-9B57-7A0EB0924F1E"), m_pPipeInDataB, "DataB", 1, "data_b"},
    {Poco::UUID("EC6BC4DB-6ACD-4F30-B3B0-426DD126D293"), m_pPipeInQualityA, "QualityA", 1, "quality_a"},
    {Poco::UUID("D3D96C96-68ED-471E-A1E6-AE5AEC8F0535"), m_pPipeInQualityB, "QualityB", 1, "quality_b"}});
    setOutPipeConnectors({{Poco::UUID("9A3BBE66-EB81-4563-9C1E-4D0124EB8080"), &m_oPipeOutDataOut, m_oPipeOutDataName, 0, "data_out"},
    {Poco::UUID("605DADC7-1496-420B-8AF0-DDFDCA8E743E"), &m_oPipeOutOperationResult, m_oPipeOutOperationResultName, 0, "operationResult_out"},
    {Poco::UUID("1DED8F9D-533C-4B39-BC7C-BAA1BC073BDB"), &m_oPipeOutOtherDataOut, m_oPipeOutOtherDataName, 0, "otherdata_out"}
    });
    setVariantID(Poco::UUID("DB96341E-1593-429C-80E8-BFA2944844F5"));
} // CTor



/*virtual*/ Conditional::~Conditional()
{

} // DTor



void Conditional::setParameter()
{
	TransformFilter::setParameter();

	m_oOperation = parameters_.getParameter( Conditional::m_oParamOperation ).convert<int>();

} // setParameter.



bool Conditional::subscribe( fliplib::BasePipe& p_rPipe, int p_oGroup )
{
	if ( p_rPipe.tag() == "data_a" )
	{
		m_pPipeInDataA = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	}
	if ( p_rPipe.tag() == "data_b" )
	{
		m_pPipeInDataB = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
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



void Conditional::SignalOperationResultOut(double operationResult, const interface::GeoDoublearray &rGeoDoubleArrayInData)
{
	geo2d::Doublearray oOutOperationResult;
	oOutOperationResult.assign(1);
	double maxRank = eRankMax;
	oOutOperationResult[0] = std::tie(operationResult, maxRank);
	const interface::GeoDoublearray oGeoDoubleOperationResultOut(rGeoDoubleArrayInData.context(), oOutOperationResult, interface::AnalysisOK, eRankMax);
    m_oPipeOutOperationResult.signal(oGeoDoubleOperationResultOut);
}

void Conditional::SetLastValidValueOfDataA()
{
	const interface::GeoDoublearray & rGeoDoubleArrayInDataA = m_pPipeInDataA->read(m_oCounter);

	int lastIndexOfInDataA = rGeoDoubleArrayInDataA.ref().size();
	m_lastValidValueDataA = std::get<eData>(rGeoDoubleArrayInDataA.ref()[lastIndexOfInDataA]);
}

void Conditional::SignalLastValidValueOfDataA()
{
	const interface::GeoDoublearray & rGeoDoubleArrayInDataA = m_pPipeInDataA->read(m_oCounter);
	geo2d::Doublearray oOutDataArray;
	oOutDataArray.assign(1);
	double maxRank = eRankMax;
	oOutDataArray[0] = std::tie(m_lastValidValueDataA, maxRank);
	const interface::GeoDoublearray oGeoDoubleDataResultOut(rGeoDoubleArrayInDataA.context(), oOutDataArray, interface::AnalysisOK, eRankMax);

	preSignalAction();
    m_oPipeOutOperationResult.signal(oGeoDoubleDataResultOut);
}

void Conditional::proceedGroup(const void *p_pSender, fliplib::PipeGroupEventArgs & e)
{
	poco_assert_dbg( m_pPipeInDataA != nullptr); // to be asserted by graph editor
	poco_assert_dbg( m_pPipeInDataB != nullptr); // to be asserted by graph editor
	poco_assert_dbg( m_pPipeInQualityA != nullptr); // to be asserted by graph editor
	poco_assert_dbg( m_pPipeInQualityB != nullptr); // to be asserted by graph editor


	switch( m_oOperation )
	{

	default:
	case 0: //compare
	{
		ProceedCompare();
		break;
	}
	case 1: //encoder compare
	{
		ProceedEncoderCompare();
		break;
	}
	}
} // proceedGroup

void Conditional::ProceedCompare()
{
	// data
	const interface::GeoDoublearray & rGeoDoubleArrayInDataA = m_pPipeInDataA->read(m_oCounter);
	const interface::GeoDoublearray & rGeoDoubleArrayInDataB = m_pPipeInDataB->read(m_oCounter);
	const interface::GeoDoublearray & rGeoDoubleArrayInQualityA = m_pPipeInQualityA->read(m_oCounter);
	const interface::GeoDoublearray & rGeoDoubleArrayInQualityB = m_pPipeInQualityB->read(m_oCounter);
    if (rGeoDoubleArrayInQualityA.ref().size() == 0 || rGeoDoubleArrayInQualityB.ref().size() == 0)
    {
        wmLog(eWarning, "Empty Data in quality (a: %d elements, b: %d elements)\n",
              rGeoDoubleArrayInQualityA.ref().size(), rGeoDoubleArrayInQualityB.ref().size());
        preSignalAction();
        return;
    }

	const double quality_a = std::get<eData>(rGeoDoubleArrayInQualityA.ref()[0]);
	const double quality_b = std::get<eData>(rGeoDoubleArrayInQualityB.ref()[0]);

	const interface::GeoDoublearray* outData = NULL;
    const interface::GeoDoublearray* outOtherData = NULL;
	double operationResult = 0;

	if(quality_a >= quality_b)
	{
		outData = &rGeoDoubleArrayInDataA;
        outOtherData = &rGeoDoubleArrayInDataB;
		operationResult = 1;
	}
	else
	{
		outData = &rGeoDoubleArrayInDataB;
        outOtherData = &rGeoDoubleArrayInDataA;
		operationResult = -1;
	}

	if(outData != NULL && outOtherData != NULL)
	{
		preSignalAction();
		SignalOperationResultOut(operationResult, *outData);
		m_oPipeOutDataOut.signal(*outData);
        m_oPipeOutOtherDataOut.signal(*outOtherData);
	}
    else
    {
        preSignalAction();
    }
}

void Conditional::ProceedEncoderCompare()
{
	// data
	const interface::GeoDoublearray & rGeoDoubleArrayInDataA = m_pPipeInDataA->read(m_oCounter);
	const interface::GeoDoublearray & rGeoDoubleArrayInQualityA = m_pPipeInQualityA->read(m_oCounter);
	const interface::GeoDoublearray & rGeoDoubleArrayInQualityB = m_pPipeInQualityB->read(m_oCounter);
    if (rGeoDoubleArrayInQualityA.ref().size() == 0 || rGeoDoubleArrayInQualityB.ref().size() == 0)
    {
        wmLog(eWarning, "Empty Data in quality (a: %d elements, b: %d elements)\n",
              rGeoDoubleArrayInQualityA.ref().size(), rGeoDoubleArrayInQualityB.ref().size());
        preSignalAction();
        return;
    }
	const double quality_a = std::get<eData>(rGeoDoubleArrayInQualityA.ref()[0]);
	const double quality_b = std::get<eData>(rGeoDoubleArrayInQualityB.ref()[0]);
	double operationResult = 0;

    if (m_oPipeOutOtherDataOut.linked())
    {
        wmLog(eError, "Conditional Filter: out pipe otherdata_out not available in encoder compare mode \n");
    }

	if(quality_a < quality_b)
	{
		operationResult = 1;

		SetLastValidValueOfDataA();

		preSignalAction();
		SignalOperationResultOut(operationResult, rGeoDoubleArrayInDataA);
		m_oPipeOutDataOut.signal( rGeoDoubleArrayInDataA );
	}
	else
	{
		operationResult = 0;

		SignalLastValidValueOfDataA();  // calls preSignalAction();
		SignalOperationResultOut(operationResult, rGeoDoubleArrayInDataA);
	}
}



} // namespace filter
} // namespace precitec
