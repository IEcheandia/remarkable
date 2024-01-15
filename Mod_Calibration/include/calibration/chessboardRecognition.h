#ifndef CHESSBOARDRECOGNITIONALGORITHM_H
#define CHESSBOARDRECOGNITIONALGORITHM_H
/*
 * To be included in weldmaster and wmcalibration 
 */

#include <numeric>
#include <list>

//TODO rename namespace
#include "calibrationCornerGrid.h"
#include "module/moduleLogger.h"
#include "geo/point.h"
#include <image/image.h>
#include "chessboardFilteringAlgorithms.h"

namespace precitec {
namespace calibration_algorithm {


typedef std::list<precitec::geo2d::DPoint> point_list_t;

struct ClusterCenters
{
    public:
        typedef std::array<int,2> cluster_key_t;
        typedef std::vector<precitec::geo2d::DPoint> point_vector_t;
    
        ClusterCenters(const std::vector<precitec::geo2d::DPoint> & rThresholdedPoints, int pClusterRadius=15);
        const point_list_t & getRawCorners() const
        {
            return m_RawCorners;
        }
        const int m_oMinSquareSize = 20;  //needed for insert to grid
    private:
        std::map<cluster_key_t, point_list_t> estimateClusters (const std::vector<precitec::geo2d::DPoint> &  rThresholdedPoints, int pClusterRadius=15);
        int insertIntoGrid(double x, double y);  //insert into m_RawCorners, sorting according to minsquaresize
        void computeClusterCenters(const std::map<cluster_key_t, point_list_t>& rClusterCenters );
    
        static const unsigned int INDEXMAX = 2000;        
        point_vector_t m_invalidCorners;
        point_list_t m_RawCorners;
};

class ChessboardRecognitionAlgorithm
{
public:
    enum class PreviewType
    {
        AfterSmoothing,
        AfterBinarization,
        AfterDilation,
        AfterErosion,
        AfterCornerDetectionB2W,
        AfterCornerDetectionW2B,
        CompleteProcessing
    };
    ChessboardRecognitionAlgorithm(const precitec::image::BImage & rImgSource, int providedThreshold = -1,
                                   PreviewType previewType = PreviewType::CompleteProcessing);
    bool isValid() const
    {
        return m_validGridMap;
    }
    const precitec::math::CalibrationCornerGrid & getCornerGrid() const
    {
        return m_oCornerGrid;
    }
    const point_list_t & getRecognizedCorners() const
    {
        return m_oRecognizedCorners;
    }

    image::BImage getPreviewImage() const
    {
        return m_previewImage;
    }

    int getThreshold() const
    {
        return m_threshold;
    }

    
private: 
    
    static precitec::image::Size2d validImageSizeAfterFiltering(const precitec::image::Size2d & rImageSize, int border);
    static int guessThreshold(const precitec::image::BImage & preProcessedImage, int border);
    static precitec::image::BImage preprocessImage(const precitec::image::BImage & rImgSource);
    static precitec::image::BImage computeBinaryImage(const precitec::image::BImage & rImgSource , int oThreshold,
                                   PreviewType previewType = PreviewType::CompleteProcessing, image::BImage * p_previewImage = nullptr );
    static std::vector<precitec::geo2d::DPoint> thresholdCornerPoints(const precitec::image::BImage & rBinarizedImage, byte oThreshold = 200,
                                   PreviewType previewType = PreviewType::CompleteProcessing, image::BImage * p_previewImage = nullptr);
    static std::pair<bool, precitec::math::CalibrationCornerGrid> generateGridMap(const point_list_t & rRawCorners, const int p_oMinSquareSize,  geo2d::Size p_oValidArea);
    
    int m_threshold;
    bool m_validGridMap;
    precitec::math::CalibrationCornerGrid m_oCornerGrid;
    point_list_t m_oRecognizedCorners;
    PreviewType m_previewType;
    image::BImage m_previewImage;
    
};



}//end namespace
}


#endif
