/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS
 * 	@date		2013
 * 	@brief		Filter that classifies pore candidates into pores and non-pores. Number of pores is the result. A NIO is thrown if a pore was classified.
 */

// WM includes
#include "poreClassifier.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/Parameter.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "geo/array.h"
#include "module/moduleLogger.h"
#include "common/defines.h"
// stdlib includes
#include <algorithm>
#include <functional>
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace geo2d;
	using namespace image;
	namespace filter {


const std::string PoreClassifier::m_oFilterName 			= "PoreClassifier";
const std::string PoreClassifier::m_oPipeResultName 		= "ImageIO";			///< Pipe: Result out-pipe..
const std::string PoreClassifier::m_oParamSizeMin			= "SizeMin";			///< Parameter name
const std::string PoreClassifier::m_oParamSizeMax			= "SizeMax";			///< Parameter name
const std::string PoreClassifier::m_oParamBoundingBoxDXMin	= "BoundingBoxDXMin";	///< Parameter name
const std::string PoreClassifier::m_oParamBoundingBoxDXMax	= "BoundingBoxDXMax";	///< Parameter name
const std::string PoreClassifier::m_oParamBoundingBoxDYMin	= "BoundingBoxDYMin";	///< Parameter name
const std::string PoreClassifier::m_oParamBoundingBoxDYMax	= "BoundingBoxDYMax";	///< Parameter name
const std::string PoreClassifier::m_oParamPcRatioMin		= "PcRatioMin";			///< Parameter name
const std::string PoreClassifier::m_oParamPcRatioMax		= "PcRatioMax";			///< Parameter name
const std::string PoreClassifier::m_oParamGradientMin		= "GradientMin";		///< Parameter name
const std::string PoreClassifier::m_oParamGradientMax		= "GradientMax";		///< Parameter name
const std::string PoreClassifier::m_oParamSurfaceMin		= "SurfaceMin";			///< Parameter name
const std::string PoreClassifier::m_oParamSurfaceMax		= "SurfaceMax";			///< Parameter name
const std::string PoreClassifier::m_oParamParamScaling		= "ParamScaling";		///< Parameter name
const std::string PoreClassifier::m_oParamResultType		= "ResultType";			///< Parameter: User-defined nio type.
const std::string PoreClassifier::m_oParamNioType 			= "NioType";			///< Parameter: User-defined nio type.

const PoreClassifier::featuretypes_strings_t PoreClassifier::m_oFeatureTypeString = {{"Size", "DX", "DY", "PcRatio", "Contrast", "Surface"}};
int PoreClassifier::m_oPoreId	=	0;

PoreClassifier::PoreClassifier() :
	ResultFilter			( m_oFilterName, Poco::UUID{"8218F3E8-0461-4575-B56E-1564CF3EEEFE"} ),
	m_pPipeInBoundingBoxDX	( nullptr ),
	m_pPipeInBoundingBoxDY	( nullptr ),
	m_pPipeInPcRatio		( nullptr ),
	m_pPipeInGradient		( nullptr ),
	m_pPipeInSurface		( nullptr ),
	m_pPipeInBlob			( nullptr ),
	m_oPipeResult			( this, m_oPipeResultName ),
	m_oImgNb				( 0 ),
	m_oParamScaling			( 0 ),
	m_oUserResultType		( Value ),
	m_oUserNioType			( ValueOutOfLimits )
{
	parameters_.add(m_oParamSizeMin,			Parameter::TYPE_double,	m_oFeatureRange.m_oArea.start());
	parameters_.add(m_oParamSizeMax,			Parameter::TYPE_double,	m_oFeatureRange.m_oArea.end());
	parameters_.add(m_oParamBoundingBoxDXMin,	Parameter::TYPE_double,	m_oFeatureRange.m_oBoundingBoxDX.start());
	parameters_.add(m_oParamBoundingBoxDXMax,	Parameter::TYPE_double,	m_oFeatureRange.m_oBoundingBoxDX.end());
	parameters_.add(m_oParamBoundingBoxDYMin,	Parameter::TYPE_double,	m_oFeatureRange.m_oBoundingBoxDY.start());
	parameters_.add(m_oParamBoundingBoxDYMax,	Parameter::TYPE_double,	m_oFeatureRange.m_oBoundingBoxDY.end());
	parameters_.add(m_oParamPcRatioMin,			Parameter::TYPE_double,	m_oFeatureRange.m_oPcRatio.start());
	parameters_.add(m_oParamPcRatioMax,			Parameter::TYPE_double,	m_oFeatureRange.m_oPcRatio.end());
	parameters_.add(m_oParamGradientMin,		Parameter::TYPE_double,	m_oFeatureRange.m_oGradient.start());
	parameters_.add(m_oParamGradientMax,		Parameter::TYPE_double,	m_oFeatureRange.m_oGradient.end());
	parameters_.add(m_oParamSurfaceMin,			Parameter::TYPE_double,	m_oFeatureRange.m_oSurface.start());
	parameters_.add(m_oParamSurfaceMax,			Parameter::TYPE_double,	m_oFeatureRange.m_oSurface.end());

	parameters_.add(m_oParamParamScaling,		Parameter::TYPE_int,	m_oParamScaling);
	parameters_.add(m_oParamResultType, 		Parameter::TYPE_int,	static_cast<int>( m_oUserResultType ));
	parameters_.add(m_oParamNioType, 			Parameter::TYPE_int,	static_cast<int>( m_oUserNioType ));

    setInPipeConnectors({{Poco::UUID("50A3B98A-8DA1-49d8-8C52-2CD0FE95F1BC"), m_pPipeInBoundingBoxDX, "PoreClassifierBoundingBoxDXIn", 1, "bounding_box_dx"},
    {Poco::UUID("73B93492-5028-49b1-9677-16975BAA9707"), m_pPipeInBoundingBoxDY, "PoreClassifierBoundingBoxDYIn", 1, "bounding_box_dy"},
    {Poco::UUID("753227EA-4298-4a7f-AC56-C0C3A013C142"), m_pPipeInPcRatio, "PoreClassifierPcRatioIn", 1, "pc_ratio"},
    {Poco::UUID("D2092D1A-C513-4629-8D82-42403D6BAF4A"), m_pPipeInGradient, "PoreClassifierGradientIn", 1, "gradient"},
    {Poco::UUID("F6DC7994-8F2C-4d73-BE3D-61704763D104"), m_pPipeInSurface, "PoreClassifierSurfaceIn", 1, "surface"},
    {Poco::UUID("9169AF8A-0083-4fe4-9598-9EC767AC1E0E"), m_pPipeInBlob, "PoreClassifierBlobIn", 1, "contour"}});
    setVariantID(Poco::UUID("CE298AFA-E55C-4363-B856-2B3EDA7CD730"));
} // CTor.



void PoreClassifier::setParameter() {
	ResultFilter::setParameter();

	m_oFeatureRange.m_oArea.start()				= parameters_.getParameter(m_oParamSizeMin);
	m_oFeatureRange.m_oArea.end()				= parameters_.getParameter(m_oParamSizeMax);
	m_oFeatureRange.m_oBoundingBoxDX.start()	= parameters_.getParameter(m_oParamBoundingBoxDXMin);
	m_oFeatureRange.m_oBoundingBoxDX.end()		= parameters_.getParameter(m_oParamBoundingBoxDXMax);
	m_oFeatureRange.m_oBoundingBoxDY.start()	= parameters_.getParameter(m_oParamBoundingBoxDYMin);
	m_oFeatureRange.m_oBoundingBoxDY.end()		= parameters_.getParameter(m_oParamBoundingBoxDYMax);
	m_oFeatureRange.m_oPcRatio.start()			= parameters_.getParameter(m_oParamPcRatioMin);
	m_oFeatureRange.m_oPcRatio.end()			= parameters_.getParameter(m_oParamPcRatioMax);
	m_oFeatureRange.m_oGradient.start()			= parameters_.getParameter(m_oParamGradientMin);
	m_oFeatureRange.m_oGradient.end()			= parameters_.getParameter(m_oParamGradientMax);
	m_oFeatureRange.m_oSurface.start()			= parameters_.getParameter(m_oParamSurfaceMin);
	m_oFeatureRange.m_oSurface.end()			= parameters_.getParameter(m_oParamSurfaceMax);

	m_oParamScaling								= parameters_.getParameter(m_oParamParamScaling);
	m_oUserResultType 							= ResultType( parameters_.getParameter(m_oParamResultType).convert<int>() );
	m_oUserNioType 								= ResultType( parameters_.getParameter(m_oParamNioType).convert<int>() );
} // setParameter



bool PoreClassifier::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
	{
	if		(p_rPipe.tag() == "bounding_box_dx") {
		m_pPipeInBoundingBoxDX	= dynamic_cast<scalar_pipe_t*>(&p_rPipe);
	} // if
	else if (p_rPipe.tag() == "bounding_box_dy") {
		m_pPipeInBoundingBoxDY	= dynamic_cast<scalar_pipe_t*>(&p_rPipe);
	} // else if
	else if	(p_rPipe.tag() == "pc_ratio") {
		m_pPipeInPcRatio		= dynamic_cast<scalar_pipe_t*>(&p_rPipe);
	} // else if
	else if (p_rPipe.tag() == "gradient") {
		m_pPipeInGradient		= dynamic_cast<scalar_pipe_t*>(&p_rPipe);
	} // else if
	else if	(p_rPipe.tag() == "surface") {
		m_pPipeInSurface		= dynamic_cast<scalar_pipe_t*>(&p_rPipe);
	} // else if
	else if	(p_rPipe.tag() == "contour") {
		m_pPipeInBlob		= dynamic_cast<blob_pipe_t*>(&p_rPipe);
	} // else

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



/*virtual*/ void PoreClassifier::arm(const fliplib::ArmStateBase& p_rArmState)
{
	if (p_rArmState.getStateID() == eSeamStart)
	{
		m_oPoreId = 0; // additionally reset pore ids on seam start for simulation
	}
} // arm


void PoreClassifier::proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArgs ) {
	// assert connected pipes

	poco_assert_dbg( m_pPipeInBoundingBoxDX	!= nullptr );
	poco_assert_dbg( m_pPipeInBoundingBoxDY	!= nullptr );
	poco_assert_dbg( m_pPipeInPcRatio		!= nullptr );
	poco_assert_dbg( m_pPipeInGradient		!= nullptr );
	poco_assert_dbg( m_pPipeInSurface		!= nullptr );
	poco_assert_dbg( m_pPipeInBlob			!= nullptr );

    m_oInputBoundingBoxDX   =   m_pPipeInBoundingBoxDX->read(m_oCounter);
    m_oInputBoundingBoxDY   =   m_pPipeInBoundingBoxDY->read(m_oCounter);
    m_oInputPcRatio         =   m_pPipeInPcRatio->read(m_oCounter);
    m_oInputGradient        =   m_pPipeInGradient->read(m_oCounter);
    m_oInputSurface         =   m_pPipeInSurface->read(m_oCounter);
    m_oInputBlob            =   m_pPipeInBlob->read(m_oCounter);

    poco_assert_dbg( m_oInputBoundingBoxDX.context() == m_oInputBoundingBoxDY.context() );
    poco_assert_dbg( m_oInputBoundingBoxDX.context() == m_oInputPcRatio.context() );
    poco_assert_dbg( m_oInputBoundingBoxDX.context() == m_oInputGradient.context() );
    poco_assert_dbg( m_oInputBoundingBoxDX.context() == m_oInputSurface.context() );
    poco_assert_dbg( m_oInputBoundingBoxDX.context() == m_oInputBlob.context() );

	const auto&					rContext			= m_oInputBoundingBoxDX.context();
	const auto					oAnalysisResult		= m_oInputBoundingBoxDX.analysisResult();
	const auto&					rBlobsIn			= m_oInputBlob.ref().getData();
	const auto&					rDataBoundingBoxDX	= m_oInputBoundingBoxDX.ref().getData();
	const auto&					rDataBoundingBoxDY	= m_oInputBoundingBoxDY.ref().getData();

	m_oSpTrafo	= rContext.trafo();

	// reset pore id if image has changed

	if(rContext.imageNumber() != m_oImgNb)
	{
		m_oImgNb	=	rContext.imageNumber();
		m_oPoreId	=	0;
	}// if

	// calculate area in sq mm from bounding box and size from blob vector

	m_oArea.resize(rBlobsIn.size());
	auto	oItBlob		= std::begin(rBlobsIn);
	auto	oItSizeX	= std::begin(rDataBoundingBoxDX);
	auto	oItSizeY	= std::begin(rDataBoundingBoxDY);

	for(auto& rArea : m_oArea)
	{
		const auto oNbPixPerBox		=	(oItBlob->xmax - oItBlob->xmin) * (oItBlob->ymax - oItBlob->ymin);
		const auto oAreaPPixSqMm	=	double{ *oItSizeX * *oItSizeY } / oNbPixPerBox;	// [mm^2]	area of pixel
		const auto oAreaBblobSqMm	=	oAreaPPixSqMm * oItBlob->si;	                // [mm^2]	area of blob
		rArea = oAreaBblobSqMm;

		++oItBlob;
		++oItSizeX;
		++oItSizeY;
	} // for

	// classify with unchanged feature range

	const auto	oNbPoresNotBadRank	= classify(m_oFeatureRange);
	const auto	oIsNio				= oNbPoresNotBadRank != 0;
	const auto	oResultValue		= Doublearray{1, double(oNbPoresNotBadRank), eRankMax};

	// send result now. result vectors coud be changed after due to scaled classification.
	// So, result does not include results from  scaled classification, which only served as an indication to the user.

	const GeoDoublearray	oGeoValueOut		( rContext, oResultValue, oAnalysisResult, Limit );
	const Range1d			oResultRange		( 0, 1000 );
	ResultDoubleArray oResultDoubleOut( id(), m_oUserResultType, m_oUserNioType, rContext, oGeoValueOut, oResultRange, oIsNio);


	// skip if scaled classification is not desired (scale factor equals zero)

	if (m_oParamScaling != 0)
	{
	    // classify with scaled feature range

	    /**
	     * @brief	Dilates or erodes a range by a given percentage. Eg [50, 100] and 10 percent -> [45, 110]; -10 percent -> [55, 90].
	     * @param	p_oRange	Percentage [-100, +100] Negative value means erosion, positive value dilation.
	     * @return	Dilated or eroded range.
	     */
	    const auto	oDilateOrErodeByPercent = [this](Range1d p_oRange) {
		    const auto	oScaleFactorStart			= 1. - m_oParamScaling * 0.01;
		    const auto	oScaleFactorEnd				= 1. + m_oParamScaling * 0.01;

		    return Range1d{p_oRange.start() * oScaleFactorStart, p_oRange.end() * oScaleFactorEnd};
	    };

	    m_oFeatureRangeScaled.m_oBoundingBoxDX	=	oDilateOrErodeByPercent(m_oFeatureRange.m_oBoundingBoxDX);
	    m_oFeatureRangeScaled.m_oBoundingBoxDY	=	oDilateOrErodeByPercent(m_oFeatureRange.m_oBoundingBoxDY);
	    m_oFeatureRangeScaled.m_oGradient		=	oDilateOrErodeByPercent(m_oFeatureRange.m_oGradient);
	    m_oFeatureRangeScaled.m_oPcRatio		=	oDilateOrErodeByPercent(m_oFeatureRange.m_oPcRatio);
	    m_oFeatureRangeScaled.m_oArea			=	oDilateOrErodeByPercent(m_oFeatureRange.m_oArea);
	    m_oFeatureRangeScaled.m_oSurface		=	oDilateOrErodeByPercent(m_oFeatureRange.m_oSurface);

	    const auto	oClassByFeatureNotScaled	=	m_oClassByFeature;
	    const auto	oClassMergedNotScaled		=	m_oClassMerged;

	    classify(m_oFeatureRangeScaled);

	    // mark if classification result per feature has changed

	    auto	oItClassNotScaled	=	std::begin(oClassByFeatureNotScaled);
	    for (auto& rClass : m_oClassByFeature) {
		    auto	oItResultNotScaled	=	std::begin(*oItClassNotScaled);
		    for (auto& rResult : rClass) {
			    if (*oItResultNotScaled != rResult) {
				    rResult	= rResult == ePore ? ePoreIfScaled : eNoPoreIfScaled;
			    } // if

			    ++oItResultNotScaled;
		    } // for

		    ++oItClassNotScaled;
	    } // for

	    // mark if merged classification result has changed

	    auto	oItResultMergedNotScaled	=	std::begin(oClassMergedNotScaled);
	    for (auto& rResultMerged : m_oClassMerged) {
		    if (*oItResultMergedNotScaled != rResultMerged) {
			    rResultMerged	= rResultMerged == ePore ? ePoreIfScaled : eNoPoreIfScaled;
		    } // if

		    ++oItResultMergedNotScaled;
	    } // for

	} // if

    preSignalAction();
    m_oPipeResult.signal( oResultDoubleOut );
		} // proceedGroup



unsigned int PoreClassifier::classify(const FeatureRange&	p_rFeatureRange) {
	using namespace std::placeholders;

	// get data

	const unsigned int			oNbCandidates		= m_oArea.size();	// could be any other feature
	const Doublearray&			rBoundingBoxDX		= m_oInputBoundingBoxDX.ref();
	const Doublearray&			rBoundingBoxDY		= m_oInputBoundingBoxDY.ref();
	const Doublearray&			rPcRatio			= m_oInputPcRatio.ref();
	const Doublearray&			rGradient			= m_oInputGradient.ref();
	const Doublearray&			rSurface			= m_oInputSurface.ref();

	const std::vector<double>&	rDataBoundingBoxDX	= rBoundingBoxDX.getData();
	const std::vector<double>&	rDataBoundingBoxDY	= rBoundingBoxDY.getData();
	const std::vector<double>&	rDataPcRatio		= rPcRatio.getData();
	const std::vector<double>&	rDataGradient		= rGradient.getData();
	const std::vector<double>&	rDataSurface		= rSurface.getData();

	const std::vector<int>&		rRankBoundingBoxDX	= rBoundingBoxDX.getRank();
	const std::vector<int>&		rRankBoundingBoxDY	= rBoundingBoxDY.getRank();
	const std::vector<int>&		rRankPcRatio		= rPcRatio.getRank();
	const std::vector<int>&		rRankGradient		= rGradient.getRank();
	const std::vector<int>&		rRankSurface		= rSurface.getRank();

	// assert equal number of features

	poco_assert_dbg( rBoundingBoxDX.size()	== oNbCandidates );
	poco_assert_dbg( rBoundingBoxDY.size()	== oNbCandidates );
	poco_assert_dbg( rPcRatio.size()		== oNbCandidates );
	poco_assert_dbg( rGradient.size()		== oNbCandidates );
	poco_assert_dbg( rSurface.size()		== oNbCandidates );

	const auto oResetVector		( [oNbCandidates, this](pore_class_vec_t& p_oVec) { p_oVec.assign(oNbCandidates, ePore); } ); // reset to ePore

	std::for_each(std::begin(m_oClassByFeature), std::end(m_oClassByFeature), oResetVector); // reset feature classification result, nb candidates may have changed
	m_oClassMerged.assign(oNbCandidates, ePore);		// reset classification result, nb candidates may have changed
	m_oClassMergedRank.assign(oNbCandidates, eRankMax);	// reset classification result rank, nb candidates may have changed. eRankMax!

	const auto oClassifyByRange = [&, this](double oFeature, Range1d p_oFeatureRange)->PoreClassType {
		return p_oFeatureRange.contains(oFeature)	? ePore : eNoPore; }; // if a feature value lies in range, it is a pore

	// calculate feature Area
	const auto oClassifySize			( std::bind(oClassifyByRange, _1, p_rFeatureRange.m_oArea) );			// bind appropriate feature range
	std::transform(std::begin(m_oArea), std::end(m_oArea), std::begin(m_oClassByFeature[eSize]), oClassifySize);
	// calculate feature BoundingBoxDX
	const auto oClassifyBoundingBoxDX	( std::bind(oClassifyByRange, _1, p_rFeatureRange.m_oBoundingBoxDX) );	// bind appropriate feature range
	std::transform(std::begin(rDataBoundingBoxDX), std::end(rDataBoundingBoxDX), std::begin(m_oClassByFeature[eBoundingBoxDX]), oClassifyBoundingBoxDX);
	// calculate feature BoundingBoxDY
	const auto oClassifyBoundingBoxDY	( std::bind(oClassifyByRange, _1, p_rFeatureRange.m_oBoundingBoxDY) );	// bind appropriate feature range
	std::transform(std::begin(rDataBoundingBoxDY), std::end(rDataBoundingBoxDY), std::begin(m_oClassByFeature[eBoundingBoxDY]), oClassifyBoundingBoxDY);
	// calculate feature PcRatio
	const auto oClassifyPcRatio			( std::bind(oClassifyByRange, _1, p_rFeatureRange.m_oPcRatio) );		// bind appropriate feature range
	std::transform(std::begin(rDataPcRatio), std::end(rDataPcRatio), std::begin(m_oClassByFeature[ePcRatio]), oClassifyPcRatio);
	// calculate feature Gradient
	const auto oClassifyGradient		( std::bind(oClassifyByRange, _1, p_rFeatureRange.m_oGradient) );		// bind appropriate feature range
	std::transform(std::begin(rDataGradient), std::end(rDataGradient), std::begin(m_oClassByFeature[eGradient]), oClassifyGradient);
	// calculate feature Surface
	const auto oClassifySurface			( std::bind(oClassifyByRange, _1, p_rFeatureRange.m_oSurface) );		// bind appropriate feature range
	std::transform(std::begin(rDataSurface), std::end(rDataSurface), std::begin(m_oClassByFeature[eSurface]), oClassifySurface);

	// calculate final classification by ANDing individual classification

	const auto oMergeClassification		( [this](PoreClassType oFirst, PoreClassType oSecond)->PoreClassType { return (oFirst == ePore && oSecond == ePore) ? ePore : eNoPore; } );

	std::transform(std::begin(m_oClassByFeature[eSize]),			std::end(m_oClassByFeature[eSize]),				std::begin(m_oClassMerged),		std::begin(m_oClassMerged), oMergeClassification);
	std::transform(std::begin(m_oClassByFeature[eBoundingBoxDX]),	std::end(m_oClassByFeature[eBoundingBoxDX]),	std::begin(m_oClassMerged),		std::begin(m_oClassMerged), oMergeClassification);
	std::transform(std::begin(m_oClassByFeature[eBoundingBoxDY]),	std::end(m_oClassByFeature[eBoundingBoxDY]),	std::begin(m_oClassMerged),		std::begin(m_oClassMerged), oMergeClassification);
	std::transform(std::begin(m_oClassByFeature[ePcRatio]),			std::end(m_oClassByFeature[ePcRatio]),			std::begin(m_oClassMerged),		std::begin(m_oClassMerged), oMergeClassification);
	std::transform(std::begin(m_oClassByFeature[eGradient]),		std::end(m_oClassByFeature[eGradient]),			std::begin(m_oClassMerged),		std::begin(m_oClassMerged), oMergeClassification);
	std::transform(std::begin(m_oClassByFeature[eSurface]),			std::end(m_oClassByFeature[eSurface]),			std::begin(m_oClassMerged),		std::begin(m_oClassMerged), oMergeClassification);

	// calculate rank by taking min rank of all features

	// std::min<int> does not work with std::transform in gcc 4.6:no matching function for call to 'transform(
	//	std::vector<int>::const_iterator, std::vector<int>::const_iterator, std::vector<int>::iterator, std::vector<int>::iterator, <unresolved overloaded function type>)'
	const auto oMin ( [](int a, int b)->int {return a < b ? a : b;} );	// min lambad equivalent to std::min<int>

	// rank for size not included
	std::transform(std::begin(rRankBoundingBoxDX),	std::end(rRankBoundingBoxDX),	std::begin(m_oClassMergedRank), std::begin(m_oClassMergedRank), oMin);
	std::transform(std::begin(rRankBoundingBoxDY),	std::end(rRankBoundingBoxDY),	std::begin(m_oClassMergedRank), std::begin(m_oClassMergedRank), oMin);
	std::transform(std::begin(rRankPcRatio),		std::end(rRankPcRatio),			std::begin(m_oClassMergedRank), std::begin(m_oClassMergedRank), oMin);
	std::transform(std::begin(rRankGradient),		std::end(rRankGradient),		std::begin(m_oClassMergedRank), std::begin(m_oClassMergedRank), oMin);
	std::transform(std::begin(rRankSurface),		std::end(rRankSurface),			std::begin(m_oClassMergedRank), std::begin(m_oClassMergedRank), oMin);

	// signal number of pores as result - and nio if at leat one pore classified

	unsigned int	oNbPoresNotBadRank		= 0;
	auto oClassMergedRankIt		= std::begin(m_oClassMergedRank);
	for (auto oClassMergedIt = std::begin(m_oClassMerged); oClassMergedIt != std::end(m_oClassMerged); ++oClassMergedIt, ++oClassMergedRankIt) {
		if (*oClassMergedIt == ePore && *oClassMergedRankIt != eRankMin) {
			++oNbPoresNotBadRank;
		} // if
	} // for

	return oNbPoresNotBadRank;
} // classify



void PoreClassifier::paint() {
	if(m_oVerbosity < eLow){
		return;
	} // if

    if (m_oSpTrafo.isNull())
    {
        return;
    }


	const Trafo		&rTrafo				( *m_oSpTrafo );
	OverlayCanvas	&rCanvas			( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerContour		( rCanvas.getLayerContour());
	OverlayLayer	&rLayerText			( rCanvas.getLayerText());
	OverlayLayer	&rLayerInfoBox		( rCanvas.getLayerInfoBox());

	typedef std::array<std::vector<double>::const_iterator, eNbFeatureTypes> feature_cits_array_t;
	const feature_cits_array_t oFeatureCIts =  {{
		m_oArea.cbegin(),
		m_oInputBoundingBoxDX.ref().getData().cbegin(),
		m_oInputBoundingBoxDY.ref().getData().cbegin(),
		m_oInputPcRatio.ref().getData().cbegin(),
		m_oInputGradient.ref().getData().cbegin(),
		m_oInputSurface.ref().getData().cbegin() }};

	typedef std::array<const std::string, eNbFeatureTypes> string_cits_array_t;
	static const string_cits_array_t oUnitsByFeature =  {{
		g_oLangKeyUnitSqMm,		// area in sq mm
		g_oLangKeyUnitMm, 		// width in mm
		g_oLangKeyUnitMm, 		// height in mm
		g_oLangKeyUnitNone, 	// no unit for ratio
		g_oLangKeyUnitNone}};	// surface has no unit

	// prepare one info box per candidate

	// these colors are used to paint the blob contours and the feature strings depending on classification result, see oPoreClassType
	static const auto	oPoreClassTypeToColor = std::array<Color, ePoreClassTypeMax + 1>{{
		Color::m_oScarletRed, Color::m_oChameleonDark, Color::m_oSkyBlue, Color::m_oMagenta }};

	const auto	oBlobsInBegin	= std::begin(m_oInputBlob.ref().getData());
	const auto	oBlobsInEnd		= std::end(m_oInputBlob.ref().getData());
	auto		oClassMergedIt	= std::begin(m_oClassMerged);
	auto		oItArea			= std::begin(m_oArea);

	poco_assert_dbg(m_oInputBlob.ref().size() ==  m_oClassMerged.size());

	for (auto oBlobIt	= oBlobsInBegin; oBlobIt != oBlobsInEnd; ++oBlobIt) {
		// get the bounding box

		const auto		oBoundingBoxStart	=	Point	( oBlobIt->xmin, oBlobIt->ymin );
		const auto		oBoundingBoxEnd		=	Point	( oBlobIt->xmax, oBlobIt->ymax );
		const auto		oBoundingBox		=	Rect	( oBoundingBoxStart, oBoundingBoxEnd );
		const auto		oIncCandidate		=	std::distance( oBlobsInBegin, oBlobIt );

		auto			oFeatureLines		=	std::vector<OverlayText>( eNbFeatureTypes );
		auto			oClassByFeatureIt	=	std::begin(m_oClassByFeature);
		auto			oUnitByFeatureIt	=	std::begin(oUnitsByFeature);

		for (auto oFeatureLinesIt = std::begin(oFeatureLines); oFeatureLinesIt != std::end(oFeatureLines); ++oFeatureLinesIt) {
			const auto				oIncFeature		=	std::distance( std::begin(oFeatureLines), oFeatureLinesIt );
			const auto				oTextColor		=	oPoreClassTypeToColor[(*oClassByFeatureIt)[oIncCandidate]];
			std::ostringstream		oMsg;

			oMsg << m_oFeatureTypeString[oIncFeature] << ":" <<  *oUnitByFeatureIt << ":" << // colon is key-key-value delimiter
				std::setprecision(2) << std::fixed << oFeatureCIts[oIncFeature][oIncCandidate];
			*oFeatureLinesIt	= OverlayText(oMsg.str(), Font(12), Rect(), oTextColor, oIncFeature);

			++oClassByFeatureIt;
			++oUnitByFeatureIt;
		} // for

		static const auto	oXDistToPore	= 6;
		static const auto	oFontSize		= 16;
		// paint pore id close to pore. For GUI: ID + 1
		rLayerText.add<OverlayText>(std::to_string(m_oPoreId + 1), Font(oFontSize), rTrafo(Rect(oBlobIt->xmax + oXDistToPore, oBlobIt->ymin - 1*15, 100, 20)), Color::Yellow()); // - 1*15 because zeroth

		// area overlay text

		if(m_oVerbosity > eLow){
			std::ostringstream	oMsg;
			oMsg << "Size:" << g_oLangKeyUnitSqMm << ":" << std::setprecision(2) << std::fixed << *oItArea;
			rLayerText.add<OverlayText>(oMsg.str(), Font(14), rTrafo(Rect(oBlobIt->xmax, oBlobIt->ymin + 0*15, 200, 20)), Color::Yellow());  // +0*15 because 1st
		} // if

		// paint corresponding info box
		rLayerInfoBox.add<OverlayInfoBox>(image::ePore, m_oPoreId, std::move(oFeatureLines), rTrafo(oBoundingBox) );

		// paint contour in green or red

		auto	oContourColor		= oPoreClassTypeToColor[*oClassMergedIt];
		for(auto oContourPosIt = std::begin(oBlobIt->m_oContour); oContourPosIt != std::end(oBlobIt->m_oContour); ++oContourPosIt) {
			rLayerContour.add<OverlayPoint>(rTrafo(*oContourPosIt), oContourColor );
		} // for

		++oClassMergedIt;
		++oItArea;
		++m_oPoreId; // counts for current image starting from zero - counts for all instances
	} // for
} // paint


	} // namespace filter
} // namespace precitec
