#pragma once

#include "image/ipImage.h"
#include "geo/startEndInfo.h"
#include <algorithm>

using namespace  precitec::image;
typedef precitec::geo2d::StartEndInfo::FittedLine FittedLine;

namespace TestImage
{
    static const byte materialIntensity = 100;
    static const int xEdge = 200;
    Size2d imageSize() 
    {
        return {500,250};
    }
    
    BImage createBackgroundImage()
    {
        return BImage {imageSize()};
    };


    BImage createFullImage()
    {    
        BImage fullImage;
        fullImage.resizeFill( imageSize(), materialIntensity);
        return fullImage;
    }

    BImage createEdgesBackgroundOnBottom(int yLeft, int yRight, int edge = -1)
    {
        BImage imageWith2Edges{imageSize()};
        assert(imageWith2Edges.isValid());
        edge = edge >= 0 ? edge : xEdge;
        //black on bottom
        int yOverlap = std::min(yLeft, yRight);
        for (int y = 0; y < yOverlap+1; ++y)
        {
            std::fill(imageWith2Edges.rowBegin(y), imageWith2Edges.rowEnd(y), materialIntensity);            
        }
        //create leftEdge 
        
        for (int y = yOverlap+1; y < yLeft+1; ++y)
        {
            assert(imageWith2Edges.getValue(edge,y) == 0);
            std::fill_n(imageWith2Edges.rowBegin(y), edge, materialIntensity);
        }
        //create right Edge 
        for (int y = yOverlap+1; y < yRight+1; ++y)
        {
            assert(imageWith2Edges.getValue(imageSize().width-1,y) == 0);
            std::fill_n(imageWith2Edges.rowBegin(y)+edge, imageSize().width-edge, materialIntensity);
        }
        return imageWith2Edges;
    };


    BImage createEdgesBackgroundOnTop(int yLeft, int yRight)
    {
        const auto oSize = imageSize();
        BImage imageWith2Edges{oSize};
        //black on top
        int yOverlap = std::max(yLeft, yRight);
        for (int y = yOverlap; y < oSize.height; ++y)
        {
            std::fill(imageWith2Edges.rowBegin(y), imageWith2Edges.rowEnd(y), materialIntensity);            
        }
        //create leftEdge 
        for (int y = yLeft; y < yOverlap; ++y)
        {
            std::fill_n(imageWith2Edges.rowBegin(y), xEdge, materialIntensity);            
        }
        //create right Edge 
        for (int y = yRight; y < yOverlap+1; ++y)
        {
            std::fill_n(imageWith2Edges.rowBegin(y)+xEdge, oSize.width-xEdge, materialIntensity);            
        }
        return imageWith2Edges;
    };
};


enum class EdgePositionRespectROI
{
    Below, FullContained, Above, Undefined
};
struct TestCombinedImage
{
    typedef precitec::geo2d::StartEndInfo::ImageStateEvaluation ImageStateEvaluation;
    typedef precitec::geo2d::StartEndInfo::ImageState ImageState;

    TestCombinedImage()
    {
        m_Image.clear();
    }
    bool init( FittedLine leftEdgeTop, 
                      FittedLine rightEdgeTop, 
                      FittedLine leftEdgeBottom, 
                      FittedLine rightEdgeBottom, 
                      int xLeft, 
                      int xRight,
                      int imageWidth,
                      int imageHeight,
                      byte intensityBackground,
                      byte intensityMaterial
                     )
    {
        m_leftEdgeTop = leftEdgeTop;
        m_rightEdgeTop = rightEdgeTop;
        m_leftEdgeBottom = leftEdgeBottom;
        m_rightEdgeBottom = rightEdgeBottom;
        m_xLeft = xLeft;
        m_xRight = xRight;    
        
        if (xRight < xLeft || xRight >= imageWidth)
        {
            m_Image.clear();
            return false;
        }
        
        std::array<double,4> yTop = { leftEdgeTop.getY(0), 
                                      leftEdgeTop.getY(xLeft),
                                      rightEdgeTop.getY(xRight),
                                      rightEdgeTop.getY(imageWidth-1)};
            
        
        std::array<double,4> yBottom = { leftEdgeBottom.getY(0), 
                                         leftEdgeBottom.getY(xLeft),
                                         rightEdgeBottom.getY(xRight),
                                         rightEdgeBottom.getY(imageWidth-1)};
        
        m_rangeFullMaterial.first = static_cast<int>(*std::max_element(yTop.begin(), yTop.end()));
        m_rangeFullMaterial.second = static_cast<int>(*std::min_element(yBottom.begin(), yBottom.end()));
        
        m_endBackgroundOnTop = static_cast<int>(*std::min_element(yTop.begin(), yTop.end()));
        m_startBackgroundOnBottom = static_cast<int>(*std::max_element(yBottom.begin(), yBottom.end()));
        
        if (m_startBackgroundOnBottom >= imageHeight) 
        {
            m_Image.clear();
            return false;
        }
        
        m_Image.resizeFill(Size2d{imageWidth, imageHeight}, intensityMaterial);
        
        //fill background stripe above
        assert(m_Image.isContiguos());
        
        
        std::fill(m_Image.rowBegin(0), m_Image.rowEnd(m_endBackgroundOnTop), intensityBackground);
        std::fill(m_Image.rowBegin(m_startBackgroundOnBottom), m_Image.rowEnd(m_Image.height()-1), intensityBackground);
        
        auto fillAbove = [this, &intensityBackground] (FittedLine line, int x1, int x2)
        {          
            if (line.m == 0)
            {
                int yMax = line.getY(x1);
                for (int y = m_endBackgroundOnTop; y < yMax; y ++)
                {
                    std::fill(m_Image.rowBegin(y)+x1, m_Image.rowBegin(y)+x2, intensityBackground);
                }
                
            }
            else
            {
                for (int y = m_endBackgroundOnTop, lastY = std::max(line.getY(x1), line.getY(x2));
                    y < lastY; y ++)
                {
                    int xEdge = double(y -line.q)/line.m;
                    xEdge = std::max(x1,std::min(xEdge,x2));
                    auto pRow = m_Image.rowBegin(y);
                    if ( line.m > 0)
                    {
                        std::fill(pRow + xEdge,pRow + x2, intensityBackground);
                    }
                    else
                    {
                        std::fill(pRow + x1, pRow + xEdge, intensityBackground);
                    }
                }
            }  
        };
        
        auto fillBelow = [this, &intensityBackground] (FittedLine line, int x1, int x2)
        {         
            if (line.m == 0)
            {
                int yMin = line.getY(x1);
                for (int y = yMin; y < m_startBackgroundOnBottom; y ++)
                {
                    std::fill(m_Image.rowBegin(y)+x1, m_Image.rowBegin(y)+x2, intensityBackground);
                }
                
            }
            else
            {
                for (int y = std::min(line.getY(x1), line.getY(x2)), lastY = m_startBackgroundOnBottom;
                    y < lastY; y ++)
                {
                    int xEdge = double(y -line.q)/line.m;
                    xEdge = std::max(x1,std::min(xEdge,x2));
                    auto pRow = m_Image.rowBegin(y);
                    if ( line.m < 0)
                    {
                        std::fill(pRow + xEdge,pRow + x2, intensityBackground);
                    }
                    else
                    {
                        std::fill(pRow + x1, pRow + xEdge, intensityBackground);
                    }
                }
            }  
        };
        
        fillAbove(leftEdgeTop, 0, xLeft+1);
        fillAbove(rightEdgeTop, xRight, imageWidth);
        fillAbove(FittedLine{0.0, std::max(yTop[1],yTop[2])}, xLeft+1, xRight);
        
        fillBelow(leftEdgeBottom, 0, xLeft+1);
        fillBelow(rightEdgeBottom, xRight, imageWidth);
        fillBelow(FittedLine{0.0, std::min(yBottom[1],yBottom[2])}, xLeft+1, xRight);
        return true;
    }
    
    precitec::geo2d::Rect getROI(bool fromAbove, int number, Size2d size, int pixelOffsetBetweenImages) const
    {
        int y0 = number*pixelOffsetBetweenImages;
        if (!fromAbove)
        {
            y0 = m_Image.height() - size.height- y0;
        }
        return {0,y0,m_Image.width(), size.height+1};
    }
    
    
    std::pair<int,int> getImageNumbersOnlyMaterial(bool fromAbove, Size2d size, int pixelOffsetBetweenImages) const
    {
        int yRoiTop = m_rangeFullMaterial.first + 1;
        int yRoiBottom =  m_rangeFullMaterial.second - size.height -1;
        
        if (fromAbove)
        {
            return { std::ceil( yRoiTop / double(pixelOffsetBetweenImages)),
                                                std::floor(yRoiBottom/ double(pixelOffsetBetweenImages))};
        }
        else
        {
            int distRoiTop = m_Image.height() - (yRoiTop + size.height);
            int distRoiBottom = m_Image.height() - (yRoiBottom + size.height);
            return { std::ceil(distRoiBottom/ double(pixelOffsetBetweenImages)),
                                                std::floor(distRoiTop/ double(pixelOffsetBetweenImages))};
        }
                
    }
    
    bool isValidROI(precitec::geo2d::Rect roi) const
    {
        if (roi.y().end() > m_Image.height() || roi.y().start() < 0)
        {
            return false;
        }
        return true;
    }
    
    precitec::geo2d::StartEndInfo roiContent(bool fromAbove, precitec::geo2d::Rect roi) const
    {
        using precitec::geo2d::StartEndInfo ;
        
        StartEndInfo oInfo;
        
        if (!m_Image.isValid() 
            || roi.x().start() < 0 || roi.x().end() >= m_Image.width()
            || roi.y().start() < 0 || roi.y().end() >= m_Image.height()
        )
        {
            oInfo.m_oImageState = StartEndInfo::ImageState::Invalid;
            oInfo.m_oImageStateEvaluation = StartEndInfo::ImageStateEvaluation::Unknown;
            return oInfo;
        }
        
        if (roi.y().end() <= m_endBackgroundOnTop || roi.y().start() >= m_startBackgroundOnBottom)
        {
            oInfo.m_oImageState = StartEndInfo::ImageState::OnlyBackground;
            if (roi.y().end() <= m_endBackgroundOnTop )
            {
                oInfo.m_oImageStateEvaluation = fromAbove ? StartEndInfo::ImageStateEvaluation::BackgroundBeforeStart 
                                                            : StartEndInfo::ImageStateEvaluation::BackgroundAfterEnd;
            }
            else
            {                
                oInfo.m_oImageStateEvaluation = fromAbove ? StartEndInfo::ImageStateEvaluation::BackgroundAfterEnd 
                                                            : StartEndInfo::ImageStateEvaluation::BackgroundBeforeStart;
            }
            oInfo.isTopDark = true;
            oInfo.isBottomDark = true;
            oInfo.borderBgStripeY = -1;
            return oInfo;
        }
        
        if (roi.y().start() > m_rangeFullMaterial.first && roi.y().end() <= m_rangeFullMaterial.second)
        {
            oInfo.m_oImageState = StartEndInfo::ImageState::OnlyMaterial;
            oInfo.m_oImageStateEvaluation = StartEndInfo::ImageStateEvaluation::OnlyMaterial;
            oInfo.isTopDark = false;
            oInfo.isBottomDark = false;
            oInfo.borderBgStripeY = -1;
            oInfo.m_oStartValidRangeY = 0;
            oInfo.m_oEndValidRangeY = roi.height()-1;
            return oInfo;
        }
                
        std::array<double,4> yTop = { m_leftEdgeTop.getY(0), 
            m_leftEdgeTop.getY(m_xLeft - roi.x().start()),
            m_rightEdgeTop.getY(m_xRight - roi.x().start()),
            m_rightEdgeTop.getY(roi.width()-1)};
            
        
        std::array<double,4> yBottom = { m_leftEdgeBottom.getY(0), 
            m_leftEdgeBottom.getY(m_xLeft - roi.x().start()),
            m_rightEdgeBottom.getY(m_xRight - roi.x().start()),
            m_rightEdgeBottom.getY(roi.width()-1)};
        
        auto isInROI = [&roi](double y){return y>= roi.y().start() && y <= roi.y().end();};
        
        bool topEdgeVisible = false;
        bool bottomEdgeVisible = false;
        int maxTopY = 0;
        int minBottomY = m_Image.height();
        
        for (auto & y : yTop)
        {
            if (isInROI(y))
            {
                topEdgeVisible = true;
                maxTopY = maxTopY > y ?  maxTopY: y; 
            }            
        }
        for (auto & y : yBottom)
        {
            if (isInROI(y))
            {
                bottomEdgeVisible = true;
                minBottomY = minBottomY < y ?  minBottomY: y; 
            }
            
        }
        
        
        if (topEdgeVisible && bottomEdgeVisible)
        {            
            oInfo.m_oImageState = StartEndInfo::ImageState::Invalid;
            oInfo.m_oImageStateEvaluation = StartEndInfo::ImageStateEvaluation::Unknown;
            return oInfo;
        }
        
        bool hasLeftEdge = topEdgeVisible ? (isInROI(yTop[0]) || isInROI(yTop[1])) : (isInROI(yBottom[0]) || isInROI(yBottom[1]));
        bool hasRightEdge = topEdgeVisible ? (isInROI(yTop[2]) || isInROI(yTop[3])) : (isInROI(yBottom[2]) || isInROI(yBottom[3]));
        
        assert(hasLeftEdge || hasRightEdge);
        if (hasLeftEdge && hasRightEdge)
        {
            oInfo.m_oImageState = StartEndInfo::ImageState::FullEdgeVisible;
        }
        else
        {
            oInfo.m_oImageState = hasLeftEdge ? StartEndInfo::ImageState::OnlyLeftEdgeVisible : StartEndInfo::ImageState::OnlyRightEdgeVisible;
        }
        oInfo.m_oImageStateEvaluation = (topEdgeVisible == fromAbove) ? StartEndInfo::ImageStateEvaluation::StartEdge : StartEndInfo::ImageStateEvaluation::EndEdge ;
        oInfo.isTopDark = topEdgeVisible;
        oInfo.isBottomDark = bottomEdgeVisible;
        oInfo.isCropped = true;
        
        if (topEdgeVisible)
        {
            oInfo.m_oStartValidRangeY = int(maxTopY) - roi.y().start();
            oInfo.m_oEndValidRangeY = roi.height()-1;
            oInfo.leftEdge = FittedLine::applyTranslation(m_leftEdgeTop,roi.x().start(),roi.y().start());
            oInfo.rightEdge = FittedLine::applyTranslation(m_rightEdgeTop,roi.x().start(),roi.y().start());
            oInfo.borderBgStripeY = oInfo.m_oStartValidRangeY - 10; //approximated value, not to be tested
            assert(oInfo.leftEdge.getY(0) == ( m_leftEdgeTop.getY(roi.x().start()) - roi.y().start()));
            assert(oInfo.rightEdge.getY(0) == ( m_rightEdgeTop.getY(roi.x().start()) - roi.y().start()));
        }
        else
        {
            assert(bottomEdgeVisible);
            oInfo.m_oStartValidRangeY = 0;
            oInfo.m_oEndValidRangeY = int(minBottomY) - roi.y().start();
            oInfo.leftEdge = FittedLine::applyTranslation(m_leftEdgeBottom,roi.x().start(),roi.y().start());
            oInfo.rightEdge = FittedLine::applyTranslation(m_rightEdgeBottom, roi.x().start(),roi.y().start());
            oInfo.borderBgStripeY = oInfo.m_oEndValidRangeY + 10; //approximated value, not to be tested
            assert(oInfo.leftEdge.getY(0) == ( m_leftEdgeBottom.getY(roi.x().start()) - roi.y().start()));
            assert(oInfo.rightEdge.getY(0) == ( m_rightEdgeBottom.getY(roi.x().start()) - roi.y().start()));
        }
        assert(oInfo.m_oStartValidRangeY >= 0 && oInfo.m_oEndValidRangeY < roi.height());
        assert(!hasLeftEdge || (oInfo.leftEdge.getY(50) >= 0 && oInfo.leftEdge.getY(50) < roi.height()));
        assert(!hasRightEdge || (oInfo.rightEdge.getY(50) >= 0 && oInfo.rightEdge.getY(50) < roi.height()));
        return oInfo;
        
        
    }
    
    std::vector<ImageStateEvaluation> acceptableImageStateEvaluation(bool fromAbove, precitec::geo2d::Rect roi, int tolerance, bool alwaysAcceptPartialEdges) const
    {
        std::vector<ImageStateEvaluation> result;
        
        auto expectedStartEndInfo = roiContent(fromAbove, roi);
        result.push_back(expectedStartEndInfo.m_oImageStateEvaluation);
            
        bool expectedEdgeImage = expectedStartEndInfo.m_oImageStateEvaluation == ImageStateEvaluation::StartEdge 
                    || expectedStartEndInfo.m_oImageStateEvaluation == ImageStateEvaluation::EndEdge;

        if (expectedEdgeImage)
        {
            assert(expectedStartEndInfo.isTopDark != expectedStartEndInfo.isBottomDark && "error in function roiContent");
            
            // Y in the coordinates of the roi
            auto YLeft = expectedStartEndInfo.leftEdge.getY(m_xLeft);
            auto YRight = expectedStartEndInfo.rightEdge.getY(m_xRight);
            
            bool leftEdgeWellVisibile = YLeft > tolerance && YLeft < (roi.height() - tolerance);
            bool rightEdgeWellVisibile = YRight > tolerance && YRight < (roi.height() - tolerance);
            
#ifndef NDEBUG            
            std::cout << "expected y left " << YLeft << "  expected y right " << YRight << " well visible " << leftEdgeWellVisibile << " " << rightEdgeWellVisibile << std::endl;
#endif
            
            {
  
                if (expectedStartEndInfo.m_oImageStateEvaluation == ImageStateEvaluation::StartEdge)
                {
                    if (!alwaysAcceptPartialEdges && (!leftEdgeWellVisibile || !rightEdgeWellVisibile))
                    {
                        result.push_back(ImageStateEvaluation::PartialStartEdgeIgnored);
                    }
                    if (!leftEdgeWellVisibile && !rightEdgeWellVisibile)
                    {                
                        result.push_back(ImageStateEvaluation::BackgroundBeforeStart);
                        result.push_back(ImageStateEvaluation::OnlyMaterial);
                    }
                
                }
                if (expectedStartEndInfo.m_oImageStateEvaluation == ImageStateEvaluation::EndEdge)
                {
                    if (!alwaysAcceptPartialEdges && (!leftEdgeWellVisibile || !rightEdgeWellVisibile))
                    {
                        result.push_back(ImageStateEvaluation::PartialEndEdgeIgnored);
                    }
                    if (!leftEdgeWellVisibile && !rightEdgeWellVisibile)
                    {
                        result.push_back(ImageStateEvaluation::BackgroundAfterEnd);
                        result.push_back(ImageStateEvaluation::OnlyMaterial);
                    }
                    
                }
            }
        
        }
        return result;
        
        
    }

    
    
    FittedLine m_leftEdgeTop;
    FittedLine m_rightEdgeTop;
    FittedLine m_leftEdgeBottom;
    FittedLine m_rightEdgeBottom;
    int m_xLeft;
    int m_xRight;
    int m_endBackgroundOnTop;
    std::pair<int,int> m_rangeFullMaterial;
    int m_startBackgroundOnBottom;
    BImage m_Image;
    
    
};

