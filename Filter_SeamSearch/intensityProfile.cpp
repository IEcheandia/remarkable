/*!
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2011
 *  @file
 *  @brief			Fliplib filter 'IntensityProfile' in component 'Filter_SeamSearch'. Calculates grey level profile on image.
 */


#include "intensityProfile.h"

#include <system/platform.h>					///< global and platform specific defines
#include <system/tools.h>						///< poco bugcheck
#include "module/moduleLogger.h"

#include "overlay/overlayPrimitive.h"
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
	using namespace interface;
	using namespace geo2d;
namespace filter {
	using fliplib::SynchronePipe;
	using fliplib::PipeEventArgs;
	using fliplib::PipeGroupEventArgs;
	using fliplib::Parameter;

const std::string IntensityProfile::m_oFilterName 	= std::string("IntensityProfile");
const std::string IntensityProfile::m_oPipeOut1Name	= std::string("Line");
const std::string IntensityProfile::m_oPipeOut2Name	= std::string("ImageSize");


IntensityProfile::IntensityProfile() :
	TransformFilter( IntensityProfile::m_oFilterName, Poco::UUID{"ae6c985e-3eaf-452c-beb0-de33c08632b9"} ),
	m_pPipeInImageFrame		( nullptr ),
	m_oPipeOutProfile		( this, m_oPipeOut1Name ),
	m_oPipeOutImgSize		( this, m_oPipeOut2Name ),
	m_oResX					( 1 ),
	m_oResY					( 1 ),
	m_oNSlices				( 3 )
{
	// Defaultwerte der Parameter setzen
	parameters_.add("ResX", Parameter::TYPE_int, m_oResX);
	parameters_.add("ResY", Parameter::TYPE_int, m_oResY);
	parameters_.add("NSlices", Parameter::TYPE_int, m_oNSlices);

    setInPipeConnectors({{Poco::UUID("ff5e0db8-fade-42b8-aaa6-fb47ac0be883"), m_pPipeInImageFrame, "ImageFrame", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("88ac9a62-9915-4c5f-8ac9-42f9b8500741"), &m_oPipeOutProfile, m_oPipeOut1Name, 0, ""},
    {Poco::UUID("09EA6AF5-2786-4e12-BD24-35205F869301"), &m_oPipeOutImgSize, m_oPipeOut2Name, 0, ""}});
    setVariantID(Poco::UUID("ffb791f9-f4b0-42fe-a337-18e656b25252"));
} // IntensityProfile



void IntensityProfile::setParameter() {
	TransformFilter::setParameter();
	m_oResX			= parameters_.getParameter("ResX").convert<int>();
	m_oResY			= parameters_.getParameter("ResY").convert<int>();
	m_oNSlices		= parameters_.getParameter("NSlices").convert<int>();

	poco_assert_dbg(m_oResX		>= 1);	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.
	poco_assert_dbg(m_oResY		>= 1);	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.
	poco_assert_dbg(m_oNSlices	>= 1);	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.
} // setParameter


void IntensityProfile::paint() {
} // paint


bool IntensityProfile::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
	m_pPipeInImageFrame  = dynamic_cast<image_pipe_t*>(&p_rPipe);
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void IntensityProfile::proceed(const void* sender, PipeEventArgs& e) {
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

	// get data from frame

	const auto oFrameIn					= ( m_pPipeInImageFrame->read(m_oCounter) );
	const BImage	&rImageIn	= oFrameIn.data();

	// (re)initialization of output structure
	reinitialize(rImageIn);

	// input validity check

	const bool oTooManySlices	( [&, this]()->bool{
		if (m_oNSlices > rImageIn.size().height / 2) {
			wmLog(eDebug, "Too many slices.\n");
			return true;
		}
		return false; }() );

	const bool oResolutionTooHigh	( [&, this]()->bool{
		if (m_oResX >= rImageIn.size().width / 10 || m_oResY >= rImageIn.size().height / m_oNSlices) {
			wmLog(eDebug, "Resolution is too high.\n");
			return true;
		}
		return false; }() );

	if ( inputIsInvalid(rImageIn) || oTooManySlices || oResolutionTooHigh ) {
		const GeoVecDoublearray		oGeoProfileOut			( oFrameIn.context(), m_oProfileOut, oFrameIn.analysisResult(), 0.0 ); // bad rank
		const GeoDoublearray		oGeoImgSizeOut			( oFrameIn.context(), Doublearray(2, 0, eRankMin), oFrameIn.analysisResult(), 0.0 ); // bad rank

		preSignalAction();
		m_oPipeOutProfile.signal( oGeoProfileOut );			// invoke linked filter(s)
		m_oPipeOutImgSize.signal( oGeoImgSizeOut );
		return; // RETURN
	}

	int error = calcIntensityProfile( rImageIn, m_oResX, m_oResY, m_oNSlices, m_oProfileOut ); // image processing
	if (error < 0)
	{
		const GeoVecDoublearray		oGeoProfileOut(oFrameIn.context(), m_oProfileOut, oFrameIn.analysisResult(), 0.0); // bad rank
		const GeoDoublearray		oGeoImgSizeOut(oFrameIn.context(), Doublearray(2, 0, eRankMin), oFrameIn.analysisResult(), 0.0); // bad rank

		preSignalAction();
		m_oPipeOutProfile.signal(oGeoProfileOut);			// invoke linked filter(s)
		m_oPipeOutImgSize.signal(oGeoImgSizeOut);
		return; // RETURN

	}



	Doublearray		oImgSizeOut(2, 0, eRankMax);
	oImgSizeOut.getData()[0] = rImageIn.size().width;
	oImgSizeOut.getData()[1] = rImageIn.size().height;

	const GeoVecDoublearray		oGeoProfileOut		( oFrameIn.context(), m_oProfileOut, oFrameIn.analysisResult(),  1.0 ); // full rank, detailed rank in Profile
	const GeoDoublearray		oGeoImgSizeOut		( oFrameIn.context(), oImgSizeOut, oFrameIn.analysisResult(), 1.0 );

	preSignalAction();
	m_oPipeOutProfile.signal( oGeoProfileOut );			// invoke linked filter(s)
	m_oPipeOutImgSize.signal( oGeoImgSizeOut );			// invoke linked filter(s)

} // proceed


bool IntensityProfile::inputIsInvalid(const BImage &p_rImageIn) {
	const bool imgIsInvalid = ! p_rImageIn.isValid();
	if (imgIsInvalid) {
		wmLog(eDebug, "Input image invalid.\n");
	}

	return imgIsInvalid;
}



void IntensityProfile::reinitialize(const BImage &p_rImageIn) {
	const int oProfileOutWidth = p_rImageIn.size().width / m_oResX;
	m_oProfileOut.assign( m_oNSlices, Doublearray( oProfileOutWidth ) );

} // reinitialize


// actual signal processing

/*static*/ int IntensityProfile::calcIntensityProfile(
	const BImage &p_rImageIn,
	unsigned int p_oResX,
	unsigned int p_oResY,
	unsigned int p_oNSlices,
	std::vector< Doublearray > &p_rProfileOut
)
{
	const int oImgWidth = p_rImageIn.size().width;
	const int oLinesPerSlice = p_rImageIn.size().height / p_oNSlices;

	if (oLinesPerSlice <= 0)
	{
		return -1;
	}

	poco_assert_dbg(oLinesPerSlice > 0);	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.


	if (p_oNSlices <= 0)
	{
		return -1;
	}

	std::vector<int> sumProfile(oImgWidth, 0);
	for (unsigned int sliceN = 0; sliceN < p_oNSlices; ++sliceN) { // loop over N slices

		sumProfile.assign(oImgWidth, 0);

		auto &rProfileVectorOut = p_rProfileOut[sliceN].getData();
		auto &rRankVectorOut = p_rProfileOut[sliceN].getRank();
        assert(rProfileVectorOut.size() == oImgWidth / p_oResX );
        assert(rRankVectorOut.size() == oImgWidth / p_oResX );

		const int oOffsetY = sliceN * oLinesPerSlice;
		//std::cout << "\nSlice " << sliceN << std::endl;
        int y = oOffsetY;
        const int yMax = (oOffsetY + oLinesPerSlice);

        if (p_oResX == 1)
        {
            if (p_oResY == 1)
            {
                for (; y < yMax ; ++y)
                {
                    std::transform(p_rImageIn.rowBegin(y) , p_rImageIn.rowEnd(y), sumProfile.begin(),
                            sumProfile.begin(), [](byte curPixel, int curSum){return curSum+curPixel;} );
                }
            }
            else
            {
                for (; y < yMax ; y+=p_oResY)
                {
                    std::transform(p_rImageIn.rowBegin(y) , p_rImageIn.rowEnd(y), sumProfile.begin(),
                            sumProfile.begin(), [](byte curPixel, int curSum){return curSum+curPixel;} );
                }
            }
            //compute mean and save it to the output
            std::transform(sumProfile.begin(), sumProfile.end(),rProfileVectorOut.begin(), [&oLinesPerSlice](int pixelSum){return pixelSum/oLinesPerSlice;} );
            std::fill(rRankVectorOut.begin(), rRankVectorOut.end(), eRankMax);
        }
        else
        {
            // loop over lines within a slice
            for (; y < yMax ; y += p_oResY)
            {
                // loop over img width
                int		oXOut	( 0 );
                for (int x = 0; x < oImgWidth; x += p_oResX)
                {
                        sumProfile[oXOut] += p_rImageIn[y][x];
                        //std::cout << "p_rImageIn" << '(' << y << ',' << x << "): " << (int)p_rImageIn[y][x] << std::endl;
                        ++oXOut;
                } // for x

            } //for y

            //compute mean and save it to the output
            int oXOut = 0;
            for (int x = 0; x < oImgWidth; x+= p_oResX)
            {

                // rProfileVectorOut ist Array mit den Spaltenmittelwerten
                rProfileVectorOut[oXOut] = sumProfile[oXOut] / oLinesPerSlice;
                //std::cout << "\nrProfileVectorOut[x] " << rProfileVectorOut[x] << std::endl;
                rRankVectorOut[oXOut] = eRankMax;

                ++oXOut;
            } // for
        }

		//std::cout << "\nrProfileVectorOut " << p_rProfileOut[sliceN] << std::endl;
	} // for

	return 1;
} // calcIntensityProfile



} // namespace filter
} // namespace precitec
