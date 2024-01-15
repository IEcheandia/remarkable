/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		LB
 * 	@date		08.2018
 * 	@brief		This filter change the value and context of an input GeoDouble, in order to be consistent with a reference context
 */


#include "contextNormalizeDouble.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"
#include <filter/armStates.h>
#include <fliplib/TypeToDataTypeImpl.h>
#include "filter/algoPoint.h"
#include "module/logType.h"
#include "module/moduleLogger.h"

#include "common/frame.h"


namespace precitec
{
using namespace image;
using namespace interface;
namespace filter
{

	const std::string ContextNormalizeDouble::m_oFilterName("ContextNormalizeDouble");
	const std::string ContextNormalizeDouble::m_oPipeInValue1Name("Value1");
	const std::string ContextNormalizeDouble::m_oPipeInValue2Name("Value2");
	const std::string ContextNormalizeDouble::m_oPipeOut1Name("Result1");
	const std::string ContextNormalizeDouble::m_oPipeOut2Name("Result2");

	ContextNormalizeDouble::ContextNormalizeDouble() :
		TransformFilter(ContextNormalizeDouble::m_oFilterName, Poco::UUID{"9561eeac-841a-473c-99d2-47f262876c6f"}),
		m_pPipeInImageFrame(nullptr),
		m_pPipeInValue1(nullptr),
		m_pPipeInValue2(nullptr),
		m_pipeAngle(nullptr),
		m_oPipeOut1(this, ContextNormalizeDouble::m_oPipeOut1Name),
		m_oPipeOut2(this, ContextNormalizeDouble::m_oPipeOut2Name),
		m_oParameterType1(ParameterType::eX),
		m_oParameterType2(ParameterType::eY),
        m_oHandleSampling(false)
	{
		parameters_.add("Type1", fliplib::Parameter::TYPE_int, int(m_oParameterType1));
		parameters_.add("Type2", fliplib::Parameter::TYPE_int, int(m_oParameterType2));
        parameters_.add("HandleSampling", fliplib::Parameter::TYPE_bool, m_oHandleSampling);

        setInPipeConnectors({{Poco::UUID("6ee6261e-88e6-4b57-a122-10f44ff86175"), m_pPipeInImageFrame, "ReferenceImage", 1, "referenceImage"},
        {Poco::UUID("ed005647-1b37-49cb-9d6c-2013dcb6bf15"), m_pPipeInValue1, "Value1", 1, "value1"},
        {Poco::UUID("65d6a428-4b23-4477-9f1b-6a0a42052f1d"), m_pPipeInValue2, "Value2", 1, "value2"},
        {Poco::UUID("b3818103-feaa-4ade-861b-687bc2a7506b"), m_pipeAngle, "OptionalAngle", 1, "OptionalAngle", fliplib::PipeConnector::ConnectionType::Optional}
        });
        setOutPipeConnectors({{Poco::UUID("d4387156-2af4-455e-9a17-45ebab5ca72a"), &m_oPipeOut1, "Result1", 0, "result1"},
        {Poco::UUID("3547a480-8eb5-46c6-809c-df6d9c218c58"), &m_oPipeOut2, "Result2", 0, "result2"}});
        setVariantID(Poco::UUID("8421d1cf-756b-4ff2-8557-d26ceb16c157"));
	}


	/*virtual*/ ContextNormalizeDouble::~ContextNormalizeDouble()
	{

	} // DTor

	void ContextNormalizeDouble::setParameter()
	{
		TransformFilter::setParameter();
		int oParameterType1 = parameters_.getParameter("Type1").convert<int>();
		m_oParameterType1 = (oParameterType1 >= 0 && oParameterType1 <= int(ParameterType::eMaxValid)) ? static_cast<ParameterType> (oParameterType1) : ParameterType::ePureNumber;
		int oParameterType2 = parameters_.getParameter("Type2").convert<int>();
		m_oParameterType2 = (oParameterType2 >= 0 && oParameterType2 <= int(ParameterType::eMaxValid)) ? static_cast<ParameterType> (oParameterType2) : ParameterType::ePureNumber;
        m_oHandleSampling = parameters_.getParameter("HandleSampling").convert<bool>();
	}

	bool ContextNormalizeDouble::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
	{
		if ( p_rPipe.tag() == "referenceImage" )
		{
			m_pPipeInImageFrame = dynamic_cast<fliplib::SynchronePipe<ImageFrame> *>(&p_rPipe);
		}
		if ( p_rPipe.tag() == "value1" )
		{
			m_pPipeInValue1 = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray> *>(&p_rPipe);
		}
		if ( p_rPipe.tag() == "value2" )
		{
			m_pPipeInValue2 = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray> *>(&p_rPipe);
		}
		if (p_rPipe.tag() == "OptionalAngle")
        {
            m_pipeAngle = dynamic_cast<fliplib::SynchronePipe<GeoDoublearray>*>(&p_rPipe);
        }
		return BaseFilter::subscribe(p_rPipe, p_oGroup);
	}

	void ContextNormalizeDouble::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
	{
		poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor
		poco_assert_dbg(m_pPipeInValue1 != nullptr); // to be asserted by graph editor
		poco_assert_dbg(m_pPipeInValue2 != nullptr); // to be asserted by graph editor

		const ImageFrame & rFrame = m_pPipeInImageFrame->read(m_oCounter);
		const GeoDoublearray & rValue1 = m_pPipeInValue1->read(m_oCounter);
		const GeoDoublearray & rValue2 = m_pPipeInValue2->read(m_oCounter);

		auto & rContextReference = rFrame.context();
		auto & rContext1 = rValue1.context();
		auto & rContext2 = rValue2.context();

        auto contextOutput1 = rContextReference;
        auto contextOutput2 = contextOutput1;

        //check the transposed flad in the input context
        bool swapOutput = false;
        if ( (rContext1.m_transposed == rContext2.m_transposed))
        {
            if (rContextReference.m_transposed == rContext1.m_transposed)
            {
                //normal case , the 2 inputs and the reference have all the same transposed flag
            }
            else
            {
                if (m_oParameterType2 == parameterTypeTransposed(m_oParameterType1))
                {
                    if ( (m_oParameterType1 != ParameterType::ePureNumber))
                    {
                        //swap output only if the input is (x,y), (y,x), (w,h), (h,w)
                        swapOutput = true;
                    }
                    else
                    {
                        assert(m_oParameterType1 == ParameterType::ePureNumber && m_oParameterType2 == ParameterType::ePureNumber);
                        assert(rContext1.m_transposed != rContextReference.m_transposed);
                        assert(rContext2.m_transposed != rContextReference.m_transposed);

                        assert(contextOutput1.m_transposed == rContextReference.m_transposed);
                        assert(contextOutput2.m_transposed == rContextReference.m_transposed);
                    }
                }
                else
                {
                    wmLog(eInfo, "%s Incomplete input parameters %d %d, can't correct transposition \n", nameInGraph(), (int)m_oParameterType1, (int)m_oParameterType2);
                    contextOutput1.m_transposed = rContext1.m_transposed;
                    contextOutput2 = contextOutput1;
                    assert( contextOutput2.m_transposed == rContext2.m_transposed);
                }
            }
        }
        else
        {
            //this case cannot be correctly handled, the graph should avoid mixing transposed and not transposed inputs
            wmLog(eWarning, "%s Can't correct transpose \n", nameInGraph());
            contextOutput1.m_transposed = rContext1.m_transposed;
            contextOutput2.m_transposed = rContext2.m_transposed;
        }

        geo2d::TArray<double> result1;
        geo2d::TArray<double> result2;

        if (m_pipeAngle != nullptr && (rValue1.ref().size() != rValue2.ref().size() || m_oParameterType1 != ParameterType::eX || m_oParameterType2 != ParameterType::eY || rContext1 != rContext2))
        {
            wmLog(eWarning, "ContextNormalizeDouble angle pipe connected, but no angle transformation will be performed! Input must be X and Y and have the same size and context!\n");
        }

        if (m_pipeAngle != nullptr && rValue1.ref().size() == rValue2.ref().size() && m_oParameterType1 == ParameterType::eX && m_oParameterType2 == ParameterType::eY && rContext1 == rContext2)
        {
            result1 = rValue1.ref();
            result2 = rValue2.ref();
            auto& x = result1.getData();
            auto& y = result2.getData();

            const auto &geoAngle = m_pipeAngle->read(m_oCounter);

            double angle = 0.0;

            if (geoAngle.ref().getData().empty())
            {
                wmLog(eWarning, "ContextNormalizeDouble angle port has no data!");
            }
            else
            {
                angle = geoAngle.ref().getData().front();
            }

            const auto sx = m_oHandleSampling ? rContext1.SamplingX_ : 1.0;
            const auto sy = m_oHandleSampling ? rContext1.SamplingY_ : 1.0;
            const auto sxr = m_oHandleSampling ? rContextReference.SamplingX_ : 1.0;
            const auto syr = m_oHandleSampling ? rContextReference.SamplingY_ : 1.0;
            const auto x0 = rContext1.trafo()->dx();
            const auto y0 = rContext1.trafo()->dy();
            const auto xr = rContextReference.trafo()->dx();
            const auto yr = rContextReference.trafo()->dy();

            const auto c = std::cos(angle * M_PI / 180);
            const auto s = std::sin(angle * M_PI / 180);

            const int N = x.size();

            for (int i = 0; i < N; ++i)
            {
                const double xi = (x[i] * c - y[i] * s) / sx + x0;
                const double yi = (x[i] * s + y[i] * c) / sy + y0;
                x[i] = (xi - xr) * sxr;
                y[i] = (yi - yr) * syr;
            }

            if (m_oVerbosity == VerbosityType::eMax)
            {
                wmLog(eInfo, "Trafo x: %f, Trafo y: %f, angle: %f", x0, y0, angle);
                for (int i = 0; i < N; ++i)
                {
                    wmLog(eInfo, "x: %f, y: %f --> x: %f, y: %f\n", (rValue1.ref().getData())[i], (rValue2.ref().getData())[i], x[i], y[i]);
                }
            }
        }
        else
        {
            result1 = transformDoubleArray(swapOutput ? rValue2.ref() : rValue1.ref(), m_oParameterType1, rContext1, rContextReference, m_oHandleSampling);
            result2 = transformDoubleArray(swapOutput ? rValue1.ref() : rValue2.ref(), m_oParameterType2, rContext2, rContextReference, m_oHandleSampling);
        }

		const interface::GeoDoublearray oGeoDoubleOut1( contextOutput1, std::move(result1), rValue1.analysisResult(), rValue1.rank());
		const interface::GeoDoublearray oGeoDoubleOut2( contextOutput2, std::move(result2), rValue2.analysisResult(), rValue2.rank());

		// send the data out ...
		preSignalAction();
		m_oPipeOut1.signal(oGeoDoubleOut1);
		m_oPipeOut2.signal(oGeoDoubleOut2);
	}


	geo2d::TArray<double> ContextNormalizeDouble::transformDoubleArray(const geo2d::TArray<double>& inputArray, ParameterType & rParamType,
	const interface::ImageContext & rContext, const interface::ImageContext & rContextReference, bool handleSampling)
	{

		geo2d::TArray<double> oResult(inputArray.size());
		oResult.getRank() = inputArray.getRank();

        const auto & rTrafo = *(rContext.trafo());
        const auto & rTrafoReference = *(rContextReference.trafo());
        switch ( rParamType )
        {
            case ParameterType::eX:
                if (handleSampling)
                {
                    std::transform(inputArray.getData().begin(),inputArray.getData().end(), oResult.getData().begin(),
                    [&] (double inputValue){return transformX(inputValue, rContext.SamplingX_, rTrafo.dx(), rContextReference.SamplingX_, rTrafoReference.dx());});
                }
                else
                {
                    std::transform(inputArray.getData().begin(),inputArray.getData().end(), oResult.getData().begin(),
                        [&] (double inputValue){return transformX(inputValue, rTrafo.dx(), rTrafoReference.dx());});
                }
                break;
            case ParameterType::eY:
                if (handleSampling)
                {
                    std::transform(inputArray.getData().begin(),inputArray.getData().end(), oResult.getData().begin(),
                        [&] (double inputValue){return transformY(inputValue, rContext.SamplingY_, rTrafo.dy(), rContextReference.SamplingY_, rTrafoReference.dy());});
                }
                else
                {
                    std::transform(inputArray.getData().begin(),inputArray.getData().end(), oResult.getData().begin(),
                        [&] (double inputValue){return transformY(inputValue, rTrafo.dy(), rTrafoReference.dy());});
                }
                break;
            case ParameterType::ePureNumber:
            default:
                oResult.getData() = inputArray.getData();
                break;
            case ParameterType::eWidth:
                if (handleSampling)
                {
                    std::transform(inputArray.getData().begin(),inputArray.getData().end(), oResult.getData().begin(),
                        [&] (double inputValue){return transformX(inputValue, rContext.SamplingX_, 0, rContextReference.SamplingX_, 0);});
                }
                else
                {
                    oResult.getData() = inputArray.getData();
                }
                break;
            case ParameterType::eHeight:
                if (handleSampling)
                {
                    std::transform(inputArray.getData().begin(),inputArray.getData().end(), oResult.getData().begin(),
                        [&] (double inputValue){return transformY(inputValue, rContext.SamplingY_, 0, rContextReference.SamplingY_, 0);});
                }
                else
                {
                    oResult.getData() = inputArray.getData();
                }
                break;

		}
		return oResult;
	}




ContextNormalizeDouble::ParameterType ContextNormalizeDouble::parameterTypeTransposed ( ParameterType param )
{
    switch ( param ) {
    case ParameterType::eX:
        return ParameterType::eY;
    case ParameterType::eY:
        return ParameterType::eX;
    case ParameterType::ePureNumber:
        return ParameterType::ePureNumber;
    case ParameterType::eWidth:
        return ParameterType::eHeight;
    case ParameterType::eHeight:
        return ParameterType::eWidth;
    }
    assert ( false );
    return ParameterType::ePureNumber;
}

} // namespace filter
} // namespace precitec

