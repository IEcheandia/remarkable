#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "geo/geo.h"
#include "common/frame.h"

enum PeakType
{
    NONPEAK,
    MAX_POSITIVE_PEAK, // dark to bright
    MIN_NEGATIVE_PEAK, // bright to dark
    MAX_NEGATIVE_PEAK,
    MIN_POSITIVE_PEAK,
};

enum PeakSearchType
{
    MAX_P_TO_MIN_N, // dark -> bright -> dark
    MIN_N_TO_MAX_P, // bright -> dark -> bright
    MAX_P_TO_MAX_P, // dark -> gray -> bright
    MIN_N_TO_MIN_N, // bright -> gray -> dark
    MAX_P,          // dark -> bright
    MIN_N,          // bright -> dark
    MAX_P_OR_MIN_N, // dark -> bright or bright -> dark
};

struct PeakCandidate
{
    int first;
    int second;
    double score;
};

namespace precitec
{
namespace filter
{

class FILTER_API Caliper : public fliplib::TransformFilter
{
private:
    const fliplib::SynchronePipe<interface::ImageFrame>* m_imageIn;
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_centerXIn;
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_centerYIn;
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_widthIn;
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_heightIn;
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_angleIn;

    fliplib::SynchronePipe<interface::GeoDoublearray> m_x1Out;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_y1Out;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_x2Out;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_y2Out;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_angleOut;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_scoreOut;

    double m_angleRange;
    int m_angleSteps;
    bool m_normalizeProfileMinMax;
    double m_sigma;
    double m_minContrast;
    PeakSearchType m_peakSearchType;
    std::string m_scoreFunction;

    // paint info
    interface::SmpTrafo m_trafo;
    double m_centerX;
    double m_centerY;
    int m_width;
    int m_height;
    double m_angle;
    std::vector<double> m_profile;
    std::vector<double> m_convolvedProfile;
    std::vector<double> m_sumProfile;

    std::vector<PeakType> m_peak;
    std::vector<PeakCandidate> m_candidate;

public:
    Caliper();

    void setParameter() override;
    void paint() override;

    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;

private:
    void evaluateCandidateScore(); // calculate scores of all candidates in m_candidate
};

} //namespace filter
} //namespace precitec
