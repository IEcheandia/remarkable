#pragma once



#include "system/types.h"				// byte
#include "common/frame.h"				// ImageFrame
#include "image/image.h"				///< BImage
#include "circleFitDefinitions.h"

// std lib
#include <string>



namespace precitec {
	namespace filter {


        
/**
 * @brief The CircleHough class
 */
class CircleHoughImpl
{
public:
    
    
    CircleHoughImpl();
    ~CircleHoughImpl();

    /**
     * @brief DoCircleFit
     */
    std::vector<hough_circle_t> DoCircleHough(const precitec::image::BImage& p_rImageIn, byte threshold, SearchType searchOutsideROI, const CircleHoughParameters & parameters);
    std::vector<hough_circle_t> DoCircleHough(const std::vector<geo2d::DPoint> & p_rPointList, const CircleHoughParameters & parameters);
    std::vector<hough_circle_t> DoCircleHough(const std::vector<geo2d::DPoint> & p_rPointList, geo2d::Point minAllowedCenterPosition, geo2d::Point maxAllowedCenterPosition, const CircleHoughParameters & parameters);

    Circle GetResult();

	static void DrawCircleToImage(precitec::image::BImage& p_rImageOut, int middleX, int middleY, double radius);

private:
    static const int NUMBER_OF_SAMPLES_COARSE = 180;
    static const int NUMBER_OF_SAMPLES_FINE = 720;
    struct PositionInAccuMa
    {
        double xImage;
        double yImage;
        int valueAccuMa;
        int xAccuMa;
        int yAccuMa;
    };
    class AccumulationMatrix
    {
    public:
        AccumulationMatrix();
        ~AccumulationMatrix();
        void update(double radius, geo2d::Point minAllowedCenterPosition, geo2d::Point maxAllowedCenterPosition);

        template<bool coarse>
        void increaseScore(double imageX, double imageY);

        PositionInAccuMa extractMax() const;
        geo2d::DPoint extractMeanImagePosition(const PositionInAccuMa & startPosition) const;

        void deleteArea(int x, int y, int size);
        void exportInput(std::string filename, image::BImage image) const;
        void exportInput(std::string filename, std::vector<geo2d::DPoint> pointList) const;
        void exportMatrix(std::string filename, hough_circle_t chosenCandidate) const;
        bool empty() const {return _maxX * _maxY == 0; }

    private:
        int * _pAccuMa; // Die verwendete Akkumulator-Matrix
        int _maxX, _maxY; // size of accumulation matrix
        int m_accumulatorTrafoX; // offset between accumulator matrix coordinates and image coordinates
        int m_accumulatorTrafoY;
        unsigned int _oAccuMaSize;

        PositionInAccuMa extractPosition(unsigned int index) const;
        unsigned int fromImageCoordToAccumulatorIndex(int xImage, int yImage) const;
        void resizeArray(geo2d::Size newSize);

        template <int NUMBER_OF_SAMPLES>
        class OffsetLookUpTable
        {
            public:
                void update(double radius, int offsetX, int offsetY);
                const std::array< std::pair< double, double >, NUMBER_OF_SAMPLES > & getOffsets() const {return m_data;};
                double getRadius() const {return m_radius;}
            private:
                std::array< std::pair< double, double >, NUMBER_OF_SAMPLES > m_data;
                double m_radius;
        };
        OffsetLookUpTable<NUMBER_OF_SAMPLES_COARSE> m_accumulatorOffsetsLookUpTableCoarse;
        OffsetLookUpTable<NUMBER_OF_SAMPLES_FINE> m_accumulatorOffsetsLookUpTableFine;
    };

    //helper class for checking neighborhood in a point list (supports negative numbers that can come from MergeContours)
    struct PointMatrix
    {
        PointMatrix();
        PointMatrix( image::BImage image, byte threshold);
        PointMatrix( const std::vector<geo2d::DPoint> & pointList);

        image::BImage m_image;
        geo2d::DPoint m_origin;
        byte m_threshold;
        bool valid(geo2d::DPoint point) const;
        geo2d::DPoint topLeftCorner() const {
            return m_origin;
        }
        geo2d::DPoint bottomRightCorner() const {
            return {m_origin.x + m_image.width() - 1,
                m_origin.y + m_image.height() -1 };
        }
    };

    template<int NUMBER_OF_VERIFICATION_SAMPLES>
    struct VerificationLookUpTable
    {
        VerificationLookUpTable();
        void update(double radius);
        std::array< geo2d::DPoint, NUMBER_OF_VERIFICATION_SAMPLES > m_table;
        double computeScore(const PointMatrix & p_rMatrix, geo2d::DPoint center, double tolerance_degrees);
    };

	bool DoSingleCircleHough(const precitec::image::BImage & p_rImageIn, double radius,  byte threshold, SearchType searchOutsideROI, bool coarse);
	bool DoSingleCircleHough(std::vector<geo2d::DPoint> p_rPointList, double radius, geo2d::Point minAllowedCenterPosition, geo2d::Point maxAllowedCenterPosition, bool coarse);
    void extractCandidates(std::vector<hough_circle_t> & p_rResult, const PointMatrix & p_rMatrix, int numberMax, double curRadius, CircleHoughParameters::ScoreType scoreType, double tolerance_degrees, bool coarse);

	Circle _resultCircle;
    AccumulationMatrix m_accumulationMatrix;
    VerificationLookUpTable<720> m_verificationOffsetsLookUpTable;


};

/**
 * @brief The CircleFitImpl class
 * Diese Klasse setzt die Berechnung des optimalen Kreises analog zu
 * http://www.dtcenter.org/met/users/docs/write_ups/circle_fit.pdf
 * um.
 */
class CircleFitImpl
{
public:
    CircleFitImpl();
    ~CircleFitImpl();

    /**
     * @brief DoCircleFit
     * Diese Funktion sucht nach einem Kreis mit Mittelpunkt (x/y) und Radius r, der moeglichst optimal ausgleichend ueber alle Punkte geht. Siehe dazu
     * auch: http://www.dtcenter.org/met/users/docs/write_ups/circle_fit.pdf
     */
    void DoCircleFit(std::vector<geo2d::DPoint> data, int partStart, int partEnd, int iMinRadius);

    /**
     * @brief GetResult
     * Fragt die Result-Struktur ab, nachdem alles initialisiert und berechnet wurde.
     */
    Circle GetResult();

	void DrawCircleToImage(precitec::image::BImage& p_rImageOut, int middleX, int middleY, double radius);

	int getX();
	int getY();
	int getR();

private:

/**
 * @brief The LookUpTables class
 * Diese Klasse haelt alle notwendigen Lookup-Tabellen. Bisher sind es Sin- und Cos-Werte fuer 1 bis 360 Grad.
 */
    class LookUpTables
    {
    public:
        LookUpTables();

        /**
        * @brief cosTable
        * Cosinus-LookUp-Tabelle fuer 1-360 Grad.
        */
        double cosTable[360];

        /**
        * @brief cosTable
        * Sinus-LookUp-Tabelle fuer 1-360 Grad.
        */
        double sinTable[360];
    };

    LookUpTables lookUpTables;
	Circle _resultCircle;

	int m_resultX, m_resultY, m_resultR;

};



} // namespace filter
} // namespace precitec

