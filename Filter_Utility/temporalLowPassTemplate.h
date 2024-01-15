#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/ParameterContainer.h"
#include "geo/geo.h"
#include "geo/array.h"
#include "filter/algoArray.h"

#include <optional>

namespace precitec
{
namespace filter
{

template<int slotCount>
class TemporalLowPassTemplate
{
public:
    TemporalLowPassTemplate()
    {
        for (int slot = 0; slot < slotCount; slot++)
        {
            m_oUpLowPass[slot].reset(new LowPass<double>(m_oFilterLenght, &calcMean<double>, m_oPassThroughBadRank));
        }
        clear();
    }

    void addToParameterContainer(fliplib::ParameterContainer& parameters_)
    {
        using fliplib::Parameter;
        // Defaultwerte der Parameter setzen
        parameters_.add("LowPassType", Parameter::TYPE_int, static_cast<int>(m_oLowPassType));
        parameters_.add("FilterLength", Parameter::TYPE_UInt32, m_oFilterLenght);
        parameters_.add("StartImage", Parameter::TYPE_UInt32, m_oParamStartImage);
        parameters_.add("MaxJump", Parameter::TYPE_double, m_oMaxJump);
        parameters_.add("MaxJumpAddOn", Parameter::TYPE_bool, m_oMaxJumpAdditionOn);
        parameters_.add("PassThroughBadRank", Parameter::TYPE_bool, m_oPassThroughBadRank);
    }

    void setParameter(const fliplib::ParameterContainer& parameters_)
    {
        auto oPrevLowPassType = m_oLowPassType;
        auto oPrevFilterLength = m_oFilterLenght;
        auto oPrevStartImage = m_oParamStartImage;
        auto oPrevPassThrough = m_oPassThroughBadRank;

        m_oLowPassType = static_cast<FilterAlgorithmType>(parameters_.getParameter("LowPassType").convert<int>());
        m_oFilterLenght = parameters_.getParameter("FilterLength");
        m_oPassThroughBadRank = parameters_.getParameter("PassThroughBadRank");
        m_oParamStartImage = parameters_.getParameter("StartImage");
        m_oMaxJump = parameters_.getParameter("MaxJump");
        m_oMaxJumpAdditionOn = parameters_.getParameter("MaxJumpAddOn");

        poco_assert_dbg(m_oLowPassType >= FilterAlgorithmType::eTypeMin && m_oLowPassType <= FilterAlgorithmType::eTypeMax); // Parameter assertion. Should be pre-checked by UI / MMI / GUI.
        poco_assert_dbg(m_oFilterLenght > 0);                                                                                // Parameter assertion. Should be pre-checked by UI / MMI / GUI.
        poco_assert_dbg(m_oParamStartImage > 0);                                                                             // Parameter assertion. Should be pre-checked by UI / MMI / GUI.

        for (int slot = 0; slot < slotCount; slot++)
        {
            // only reset filter if the algorithm or the window size was changed ...
            if (oPrevLowPassType != m_oLowPassType || oPrevFilterLength != m_oFilterLenght || oPrevStartImage != m_oParamStartImage || oPrevPassThrough != m_oPassThroughBadRank)
            {
                switch (m_oLowPassType)
                {
                case FilterAlgorithmType::eMean:
                {
                    // make boxcar filter with mean as actual filter function.
                    m_oUpLowPass[slot].reset(new LowPass<double>(m_oFilterLenght, &calcMean<double>, m_oPassThroughBadRank));
                }
                break;
                case FilterAlgorithmType::eMedian:
                {
                    // make boxcar filter with median as actual filter function
                    m_oUpLowPass[slot].reset(new LowPass<double>(m_oFilterLenght, &calcMedian1d<double>, m_oPassThroughBadRank));
                }
                break;
                case FilterAlgorithmType::eMinLowPass:
                {
                    // make boxcar filter with minimum as actual filter function
                    m_oUpLowPass[slot].reset(new LowPass<double>(m_oFilterLenght, &calcDataMinimum<double>, m_oPassThroughBadRank));
                }
                break;
                case FilterAlgorithmType::eMaxLowPass:
                {
                    // make boxcar filter with maximum as actual filter function
                    m_oUpLowPass[slot].reset(new LowPass<double>(m_oFilterLenght, &calcDataMaximum<double>, m_oPassThroughBadRank));
                }
                break;
                default:
                {
                    std::ostringstream oMsg;
                    oMsg << "No case for switch argument: " << static_cast<int>(m_oLowPassType);
                    wmLog(eError, oMsg.str().c_str());

                    // make boxcar filter with mean as actual filter function
                    m_oUpLowPass[slot].reset(new LowPass<double>(m_oFilterLenght, &calcMean<double>));
                }
                break;
                }
            }
        }
    }

    void clear()
    {
        for (int slot = 0; slot < slotCount; slot++)
        {
            m_imageCounter[slot] = 0;
            (m_oUpLowPass[slot].get())->resetBuffer();
            m_lastValue[slot] = {};
            m_oValueOut[slot] = geo2d::Doublearray(1, 0);
        }
    }

    template<unsigned int slot>
    interface::GeoDoublearray temporalLowPass(const interface::GeoDoublearray& rGeoDoubleArrayIn)
    {
        using interface::GeoDoublearray;
        using geo2d::Doublearray;
        static_assert(slot < slotCount);

        m_imageCounter[slot]++;

        // input validity check
        if ((!rGeoDoubleArrayIn.isValid() && m_oPassThroughBadRank) || (rGeoDoubleArrayIn.ref().size() == 0))
        {
            return GeoDoublearray(rGeoDoubleArrayIn.context(), m_oValueOut[slot], rGeoDoubleArrayIn.analysisResult(), interface::NotPresent); // bad rank
        }

        // process all values in value vector
        auto& p_rDataIn = rGeoDoubleArrayIn.ref();

        const unsigned int oNbValues = p_rDataIn.size();
        m_oValueOut[slot].resize(oNbValues);

        for (auto oValueN = 0U; oValueN != oNbValues; ++oValueN)
        {
            if ((std::get<eRank>(p_rDataIn[oValueN]) == eRankMin) && !m_oPassThroughBadRank)
            {
                // Invalid value  =>  value out = 0, rank = eRankMin

                m_oValueOut[slot].getData()[oValueN] = m_lastValue[slot].value_or(0.0);
                m_oValueOut[slot].getRank()[oValueN] = eRankMin;
               //here lastValue is not updated (it would be an invalid value)
            }
            else if ( m_imageCounter[slot] < m_oParamStartImage)
            {
                // too early for filtering, send current input values
                m_oValueOut[slot][oValueN] = p_rDataIn[oValueN];
                m_lastValue[slot] = m_oValueOut[slot].getData()[oValueN];
            }
            else
            {
                // apply actual signal processing filter

                // process data with our boxcar filter
                m_oValueOut[slot][oValueN] = m_oUpLowPass[slot]->process(p_rDataIn[oValueN]);

                // limit jumps
                if (m_lastValue[slot].has_value())
                {
                    auto& rCurrentValue = m_oValueOut[slot].getData()[oValueN];
                    const double oDelta = rCurrentValue - m_lastValue[slot].value();

                    if ((std::abs(oDelta) > m_oMaxJump))
                    {
                        if (m_oMaxJumpAdditionOn)
                        {
                            rCurrentValue = oDelta > 0 ? m_lastValue[slot].value() + m_oMaxJump : m_lastValue[slot].value() - m_oMaxJump; // limit jump
                        }
                        else
                        {
                            rCurrentValue = m_lastValue[slot].value(); // without limit jump
                        }
                        m_oValueOut[slot].getRank()[oValueN] /= 2; // reduce rank
                        // Wo wird "reduced Rank" gesetzt/benutzt???
                    }
                }
                m_lastValue[slot] = m_oValueOut[slot].getData()[oValueN];
            }

        } // for
        return GeoDoublearray(rGeoDoubleArrayIn.context(), m_oValueOut[slot], rGeoDoubleArrayIn.analysisResult(), rGeoDoubleArrayIn.rank());

    } // temporalLowPass
private:
    typedef std::unique_ptr<LowPass<double>> upLowPass_t;

    FilterAlgorithmType m_oLowPassType = FilterAlgorithmType::eMean; ///< type of low pass algorithm
    unsigned int m_oFilterLenght = 3u;                               ///< filter length
    unsigned int m_oParamStartImage = 1u;                            ///< Image number where to start with filter function
    double m_oMaxJump = 20u;                                         ///< Max jump in
    bool m_oPassThroughBadRank = true;                               ///< if bad ranked values are always passed through and not eliminated
    bool m_oMaxJumpAdditionOn = true;                                ///< If jump is too big.. False: take last value, True: Add maxJump to last value
    std::array<upLowPass_t, slotCount> m_oUpLowPass;                 ///< Low pass object with functor
    std::array<std::optional<double>, slotCount> m_lastValue;        ///< last value
    std::array<unsigned int, slotCount> m_imageCounter;              ///< internal counter for the start of the filtering process
    std::array<geo2d::Doublearray, slotCount> m_oValueOut;           ///< output values
};

}
}
