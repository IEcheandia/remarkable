/**
* 	@file
* 	@copyright	Precitec GmbH & Co. KG
* 	@author		AL
* 	@date		2015
* 	@brief		This filter computes basic subsampling
*/

// todo: operation enum

// project includes
#include "dataSubsampling.h"
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <event/resultType.h>
#include <fliplib/TypeToDataTypeImpl.h>
#include <filter/algoStl.h>
#include <filter/algoArray.h>

namespace precitec {
namespace filter {

const std::string DataSubsampling::m_oFilterName("DataSubsampling");
const std::string DataSubsampling::m_oPipeOutName("Data");
const std::string DataSubsampling::m_oParamOperation("Operation");


DataSubsampling::DataSubsampling() :
    TransformFilter(DataSubsampling::m_oFilterName, Poco::UUID{"9142B694-CD28-4933-A573-75CAA0CD9555"}),
    m_pPipeInDataA(nullptr),
    m_oPipeOutData(this, DataSubsampling::m_oPipeOutName),
    m_oOperation(0),
    m_oPassThroughBadRank(false)
{
    parameters_.add(m_oParamOperation, fliplib::Parameter::TYPE_int, m_oOperation);
    parameters_.add("PassThroughBadRank", fliplib::Parameter::TYPE_bool, m_oPassThroughBadRank);

    setInPipeConnectors({{Poco::UUID("92532D22-CBB6-436A-9723-64F4CC170FC4"), m_pPipeInDataA, "DataA", 0, "data_a"}});
    setOutPipeConnectors({{Poco::UUID("87BA85F2-8F1C-425F-B863-0797407B5BF5"), &m_oPipeOutData, m_oPipeOutName, 0, "data"}});
    setVariantID(Poco::UUID("8065126F-C2B7-4F74-B56C-413F36A150A3"));
} // CTor


/*virtual*/
DataSubsampling::~DataSubsampling()
{

} // DTor


void DataSubsampling::setParameter()
{
    TransformFilter::setParameter();

    m_oOperation = parameters_.getParameter(DataSubsampling::m_oParamOperation).convert<int> ();
    m_oPassThroughBadRank = parameters_.getParameter("PassThroughBadRank").convert<bool>();

} // setParameter.


bool DataSubsampling::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    if (p_rPipe.tag() == "data_a")
        m_pPipeInDataA = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray> *> (&p_rPipe);

    return BaseFilter::subscribe(p_rPipe, p_oGroup);

} // subscribe

geo2d::Doublearray DataSubsampling::subsample(const geo2d::Doublearray& arrayIn, SubsamplingOperation operation, bool passThroughBadRank)
{
    unsigned int sizeOfArray = arrayIn.size();
    if (sizeOfArray == 0)
    {
        return geo2d::Doublearray(1, 0.0, eRankMin);
    }

    //helper functions
    double oResult = 0.0;
    int oRank = eRankMax;
    int accumulationCounter = 0;
    auto accumulateData = [&oResult, &accumulationCounter](double elementValue)
    {
        oResult += elementValue;
        accumulationCounter++;
    };
    auto minimumRank = [&oRank](int elementRank)
    {
        oRank = std::min(oRank, elementRank);
    };
    auto ignoreData = [](double) {};
    auto ignoreRank = [](int) {};

    switch (operation)
    {
    case SubsamplingOperation::NoneOperation:
    {
        return arrayIn;
        break;
    }

    default:
    case SubsamplingOperation::LatestValue:
    {
        if (passThroughBadRank)
        {
            oResult = arrayIn.getData().back();
            oRank = arrayIn.getRank().back();
        }
        else
        {
            const auto& rRankVector = arrayIn.getRank();
            auto itLatestGoodValue = std::find_if(rRankVector.rbegin(), rRankVector.rend(), [](auto rank)
                                                    { return rank != eRankMin; });
            if (itLatestGoodValue != rRankVector.rend())
            {
                auto latestIndex = rRankVector.size() - 1 - std::distance(rRankVector.rbegin(), itLatestGoodValue);
                oResult = arrayIn.getData()[latestIndex];
                oRank = *itLatestGoodValue;
            }
            else
            {
                //all values have bad rank, the result will have bad rank as well
                oResult = arrayIn.getData().back();
                oRank = rRankVector.back();
            }
        }
        return geo2d::Doublearray(1, oResult, oRank);
        break;
    }

    case SubsamplingOperation::ArithmeticMean:
    {
        accumulationCounter = 0;
        oResult = 0;
        bool allBadRankValues = false;
        if (!passThroughBadRank)
        {
            forEachInArray(arrayIn,
                            accumulateData, minimumRank,
                            ignoreData, ignoreRank);
            allBadRankValues = (accumulationCounter == 0);
        }
        if (passThroughBadRank || allBadRankValues)
        {
            forEachInArray_RankIndepent(arrayIn,
                                        accumulateData, minimumRank);
        }

        if (accumulationCounter > 0)
        {
            oResult = oResult / accumulationCounter;
        }
        else
        {
            oRank = 0;
        }

        assert(!allBadRankValues || oRank == 0);
        return geo2d::Doublearray(1, oResult, oRank);
        break;
    }

    case SubsamplingOperation::ReductionMeanFactor10:
    {
        const int NUM_SAMPLES_FOR_MEAN = 10;

        unsigned int sizeOfOutputArray = sizeOfArray / NUM_SAMPLES_FOR_MEAN;
        if (sizeOfArray % NUM_SAMPLES_FOR_MEAN != 0)
        {
            sizeOfOutputArray += 1;
        }
        geo2d::Doublearray oOut;
        oOut.reserve(sizeOfOutputArray);

        for (unsigned int i = 0; i < sizeOfArray; i += NUM_SAMPLES_FOR_MEAN)
        {
            accumulationCounter = 0;
            oResult = 0;
            oRank = eRankMax;
            bool allBadRankValues = false;

            if (!passThroughBadRank)
            {
                forEachInArrayRange(arrayIn,
                                    i, std::min(i + NUM_SAMPLES_FOR_MEAN, sizeOfArray),
                                    accumulateData, minimumRank,
                                    ignoreData, ignoreRank);
            }
            if (passThroughBadRank || allBadRankValues)
            {
                forEachInArrayRange_RankIndepent(arrayIn,
                                                    i, std::min(i + NUM_SAMPLES_FOR_MEAN, sizeOfArray),
                                                    accumulateData, minimumRank);
            }
            if (accumulationCounter > 0)
            {
                oResult = oResult / accumulationCounter;
            }
            else
            {
                oRank = eRankMin;
            }
            assert(!allBadRankValues || oRank == 0);
            oOut.push_back(oResult, oRank);

        } // for
        assert(oOut.size() == sizeOfOutputArray);
        return oOut;
        break;
    }

    case SubsamplingOperation::Minimum:
    {
        int counter = 0;
        bool allBadRankValues = false;
        auto minimumData = [&oResult, &counter](double elementValue)
        {
            oResult = (counter == 0) ? elementValue : std::min(oResult, elementValue);
            counter++;
        };
        if (!passThroughBadRank)
        {
            forEachInArray(arrayIn,
                            minimumData, minimumRank,
                            ignoreData, ignoreRank);
            allBadRankValues = (counter == 0);
        }
        if (passThroughBadRank || allBadRankValues)
        {
            forEachInArray_RankIndepent(arrayIn,
                                        minimumData, minimumRank);
        }

        if (counter == 0 || allBadRankValues)
        {
            oRank = 0;
        }

        return geo2d::Doublearray(1, oResult, oRank);
        break;
    }
    case SubsamplingOperation::Maximum:
    {
        int counter = 0;
        bool allBadRankValues = false;
        auto maximumData = [&oResult, &counter](double elementValue)
        {
            oResult = (counter == 0) ? elementValue : std::max(oResult, elementValue);
            counter++;
        };
        if (!passThroughBadRank)
        {
            forEachInArray(arrayIn,
                            maximumData, minimumRank,
                            ignoreData, ignoreRank);
            allBadRankValues = (counter == 0);
        }
        if (passThroughBadRank || allBadRankValues)
        {
            forEachInArray_RankIndepent(arrayIn,
                                        maximumData, minimumRank);
        }
        if (counter == 0 || allBadRankValues)
        {
            oRank = 0;
        }

        return geo2d::Doublearray(1, oResult, oRank);
        break;
    }
    case SubsamplingOperation::Median:
    {
        std::vector<double> oDataCopy;
        bool allBadRankValues = false;
        if (!passThroughBadRank)
        {
            oDataCopy.reserve(arrayIn.size());
            forEachInArray(
                arrayIn,
                [&oDataCopy](double elementValue)
                { oDataCopy.push_back(elementValue); },
                minimumRank,
                ignoreData, ignoreRank);
            allBadRankValues = oDataCopy.empty();
        }
        if (passThroughBadRank || allBadRankValues)
        {
            oDataCopy = arrayIn.getData();
            oRank = *std::min_element(arrayIn.getRank().begin(), arrayIn.getRank().end());
        }
        if (!oDataCopy.empty())
        {
            oResult = *filter::calcMedian(oDataCopy.begin(), oDataCopy.end());
        }

        assert(!allBadRankValues || oRank == 0);
        return geo2d::Doublearray(1, oResult, oRank);
        break;
    }
    }
    return geo2d::Doublearray{};
};

interface::GeoDoublearray DataSubsampling::subsample(const interface::GeoDoublearray& geoArrayIn, SubsamplingOperation operation, bool passThroughBadRank)
{
    return {geoArrayIn.context(),
            subsample(geoArrayIn.ref(), operation, passThroughBadRank),
            geoArrayIn.analysisResult(),
            geoArrayIn.rank()};
}

void DataSubsampling::proceed(const void* p_pSender, fliplib::PipeEventArgs& e)
{
    poco_assert_dbg( m_pPipeInDataA != nullptr); // to be asserted by graph editor

    const interface::GeoDoublearray &rGeoDoubleArrayInA = m_pPipeInDataA->read(m_oCounter);

    const interface::GeoDoublearray oGeoDoubleOut = subsample(rGeoDoubleArrayInA, static_cast<SubsamplingOperation>(m_oOperation), m_oPassThroughBadRank);

    // send the data out ...
    preSignalAction();
    m_oPipeOutData.signal(oGeoDoubleOut);

}// proceedGroup


} // namespace filter
} // namespace precitec
