/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		JS
* 	@date		2017
* 	@brief 		This filter tries to detect a poor penetration by computing minimum positions along aline
*/

#ifndef POORPENETRATIONMULTI_H_
#define POORPENETRATIONMULTI_H_

#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <common/frame.h>				///< ImageFrame
#include <image/image.h>				///< BImage
#include <geo/geo.h>					///< Size2d, Intarray
#include <geo/array.h>					///< ByteArray

#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

using precitec::image::BImage;
using precitec::image::OverlayPoint;
using precitec::image::OverlayLine;
using precitec::image::OverlayRectangle;

//using precitec::filter::BoxSummer;

namespace precitec {
	using namespace interface;
	using namespace image;

	namespace filter {

		class PointGrey
		{
		public:
			int x;
			int y;
			int grey;
			precitec::image::Color color;

		};

		class PointOverlay
		{
		public:
			int x;
			int y;
			precitec::image::Color color;
		};

		class SimpleRect
		{
		public:
			int x;
			int y;
			int width;
			int height;
			precitec::image::Color color;
		};


		typedef std::vector<PointGrey> overLayArray;    ///storage of all overlay points over all lines
		

		class FILTER_API PoorPenetrationMulti : public fliplib::TransformFilter
		{

		public:

			PoorPenetrationMulti();
			virtual ~PoorPenetrationMulti();

			static const std::string m_oFilterName;				///< Name Filter
			static const std::string PIPENAME_RESULT_OUT;		///< Name Out-Pipe

			/// Set filter parameters as defined in database / xml file.
			void setParameter();
			/// paints overerlay primitives
			void paint();

		protected:

			/// In-pipe registration.
			bool subscribe(fliplib::BasePipe& pipe, int group);
			/// In-pipe event processing.
			void proceed(const void* sender, fliplib::PipeEventArgs& e);

		private:

			const fliplib::SynchronePipe< interface::ImageFrame >   * m_pPipeInImageFrame;		///< In pipe
			

			fliplib::SynchronePipe< interface::GeoPoorPenetrationCandidatearray > * m_pPipeOutResult;		///< Out pipe 

			interface::SmpTrafo		m_oSpTrafo;				///< roi translation

			int m_oMode;
			int m_oDisplay;
			int m_oThresholdUpper;
			int	m_oThresholdLower;
			int m_oGreyDistance;
			int	m_oCountOverThreshold;
			int	m_oCountUnderThreshold;
			int m_oFilterSizeX;
			int m_oFilterSizeY;
			int m_oNumberOfMinima;
			int m_oResolution;
			int m_oColumnDeviation;
			int m_oMaxInterruption;
			
					

			geo2d::PoorPenetrationCandidatearray m_oResultOut;

			bool m_hasPainting;
			overLayArray m_oDrawPoints;
			overLayArray m_oProfilePoints;
			SimpleRect  m_oRectangle;

			
		};

		class FdFind
		{
		public:
			
			FdFind();


			FdFind(BImage image, int sizeX, int sizeY, int numberOfMinima, int lowerThreshold,int upperThreshold, int greyDistance, int ctrUpper, int ctrUnder,int resolution,int deviation);
			~FdFind();


			/**
			*   @brief compute dark positions in an image, by looking for minima under a given threshold in a grey scale profile
			*   histogrm sorting will be done inside of the function
			*   \param display parameter to decide which overlay ae displayed in the image
			*   \param overlay array overlay points
			*   \param resultPositions resulting gap positions
			*   \return length of the biggest chain in y direction
			*/
			int CheckForNoSeam(int display, int mode,overLayArray &overlay,std::vector<PointGrey> &resultPosition);
			

			/**
			*   @brief compute the gradient positions beneath the gap positions
			*   \param resultPositions extinguished from these positions the gradient positioins will be computed
			*   \param gradPosLeft gradient positions at the left side of the gap
			*   \param gradPosRight gradient positions at the right side of the gap
			*   \return length of the biggest chain in y direction
			*/
			int CheckForGradient(std::vector<PointGrey> &resultPosition, std::vector<PointGrey> &gradPosLeft, std::vector<PointGrey> &gradPosRight,int &meanGrad, int &meanWidth);


			/**
			*   @brief compute the histogram maximum of the x positions of the gap points
			*   \param minimapos minimum positions
			*   \return histogram maximum
			*/
			int HistogramSort(std::vector<PointGrey> &minimapos);


			/**
			*   @brief erase all points with a distance over a threshold from a given position
			*   \param minimapos
			*   \param xMaxPos position to which the distance will be computed
			*   \return remaining points
			*/
			std::vector<PointGrey>  ErasePoints(std::vector<PointGrey> &minimapos, int xMaxPos);


			/**
			*   @brief compute the longest chain of connected points; points are connected ifthe gap between two points is closer than a threshold
			*   \param pointVec gap positions
			*   \param lastPointVec remaining points in the longest chain
			*   \param maxInterrupt biggest gap size betwenn two points regarding only the y position
			*   \return length of the biggest chain in y direction
			*/
			int InterruptPoints(std::vector<PointGrey> &pointVec, std::vector<PointGrey> &lastPointVec, int &maxInterrupt);


			/**
			*   @brief compute mean position an dmean grey value at the gap position
			*   \param mean mean x position  of the gap
			*   \param meanGrey mean grey value at the gap position
			*   \param greyValOutside mean grey value over 3 pixels beneath the gap
			*   \param lastPointVec gap positions
			*   \return error  < 0 if error
			*/
			int CheckForMean(int &mean,int &meanGrey,std::vector<PointGrey> & lastPointVec);


			/**
			*   @brief compute features of the gradient points
			*   \param gradPosLeft gradient points left side of gap
			*   \param gradPosRight gradient points right side of gap
			*   \param greyValInside mean grey value inside the gap, i.e. between the gradient points
			*   \param greyValOutside mean grey value over 3 pixels beneath the gap
			*   \param meanDev standard deviation of the gap edge points left and right side 
			*   \param lenL curve length of the left edge 
			*   \param lenR curve length of the right edge 
			*   \return error  < 0 if error 
			*/
			int CheckForGradientFeatures(std::vector<PointGrey> &gradPosLeft, std::vector<PointGrey> &gradPosRight, int &greyValInside, int &greyValOutside, double &meanDev, double &lenL, double &lenR);



		private:
			/**
			*   @brief looking for minima positions in a roi 
			*   \param mode controls additional computations
			*   \param x1 start x position in the roi
			*   \param x2 end x position in the roi
			*   \param y1 start y position in the roi
			*   \param y2 end y position in the roi
			*   \param minimapos resultig points
			*   \return length of the biggest chain in y direction
			*/
			int FindKontur(int mode,int x1, int x2, int y1, int y2, std::vector<PointGrey> &minimapos);

			void setImage(BImage image);

			/**
			*   @brief coputes the minim positions in a grey level line profile 
			*   \param line line number in a roi
			*   \param sizeX filterwidth for computing the line profile
			*   \param sizeY filterheight for processing the line profile
			*   \param startx start of the grey level line in the roi
			*   \param endx endpos of the grey level line in the roi
			*   \param mode controls additional computations
			*   \return minimapos positions in the grey level line profile
			*/
			std::vector<PointGrey> ScanLine(int line, int sizeX, int sizeY, int startx, int endx,int mode);

			BImage m_oIimage;
			int    m_oWidth;
			int    m_oHeight;
			int    m_oSizeX;
			int    m_oSizeY;
			int    m_oMaxNumberMinima;
			int    m_oLowerThreshold;
			int    m_oUpperThreshold;
			int    m_oDistance;
			int    m_oCtrUpper;
			int    m_oCtrUnder;
			int    m_oLineResolution;
			int    m_oLineDeviation;
			

		};

		


	} // namespace precitec
} // namespace filter

#endif /* PoorPenetration_H_ */
