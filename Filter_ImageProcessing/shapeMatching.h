#pragma once

#include "shapeMatchingImpl.h"

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "geo/geo.h"
#include "common/frame.h"

#include <vector>

namespace precitec
{
namespace filter
{

class FILTER_API ShapeMatching : public fliplib::TransformFilter
{
public:
    ShapeMatching();

    void setParameter() override;
    void paint() override;

    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
    override;

private:
    const fliplib::SynchronePipe<interface::ImageFrame>* m_pipeInImageFrame;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_pipePosX;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_PipePosY;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_pipeAngle;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_pipeScore;

    enum class CandidateSortMethod
    {
        ScoreHighestToLowest = 0,
        ClosestToCenter = 1,
        LeftToRight = 2,
        TopToBottom = 3,
    };

    std::string m_templateFileName;
    double m_blur;
    unsigned int m_contrast;
    unsigned int m_pyramidLevels;
    double m_angleStart;
    double m_angleExtent;
    double m_minScore;
    double m_greediness;
    double m_maxOverlap;
    unsigned int m_maxMatches;
    CandidateSortMethod m_sortMethod;

    std::vector<ShapeModel> m_shapeModelScaled;

    std::vector<MatchCandidate> m_matchCandidate;
    interface::SmpTrafo m_trafo;

private:
    void generateShapeModel();
};

} //namespace filter
} //namespace precitec

