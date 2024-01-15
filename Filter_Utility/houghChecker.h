/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2017
* 	@brief		This filter checks if a hough poor penetration candidate is a real poor penetration
*/

#ifndef HOUGHCHECKER_H_
#define HOUGHCHECKER_H_

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>

#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"


namespace precitec {
	using namespace image;

	namespace filter {

		struct HoughPPPoint
		{
			HoughPPPoint();
			HoughPPPoint(int x, int y, Color color);
			int x;
			int y;
			Color color;
		};

		struct HoughPPLine
		{
			HoughPPLine();
			HoughPPLine(int x1, int y1, int x2, int y2, Color color);
			int x1;
			int y1;
			int x2;
			int y2;
			Color color;
		};

		struct HoughPPRectangle
		{
			HoughPPRectangle();
			HoughPPRectangle(int x, int y, int width, int height, Color color);
			int x;
			int y;
			int width;
			int height;
			Color color;
		};

		class HoughPPOverlay
		{
		public:
			HoughPPOverlay();

			void reset();

			std::vector<HoughPPPoint> getPointContainer();
			std::vector<HoughPPLine> getLineContainer();
			std::vector<HoughPPRectangle> getRectangleContainer();

			void addPoint(int x, int y, Color color);
			void addLine(int x1, int y1, int x2, int y2, Color color);
			void addRectangle(int x, int y, int width, int height, Color color);

		private:
			std::vector<HoughPPPoint> _pointContainer;
			std::vector<HoughPPLine> _lineContainer;
			std::vector<HoughPPRectangle> _rectangleContainer;
		};

		class HoughPPInfoLine
		{
		public:
			HoughPPInfoLine();
			HoughPPInfoLine(int number, int value, Color color, bool hasValue = true);
			std::string getLine();

			int _number, _value;
			Color _color;

		private:
            std::string getLabel();
			std::string spaces(int i);
			std::string convertIntToString(int i);

            bool m_hasValue = true; ///< false: value of _value is just a dummy.
		};

		class FILTER_API HoughChecker : public fliplib::TransformFilter
		{
		public:
			/**
			* CTor.
			*/
			HoughChecker();
			/**
			* @brief DTor.
			*/
			virtual ~HoughChecker();

			// Declare constants
			static const std::string m_oFilterName;			///< Filter name.
			static const std::string m_oPipeOutName;		///< Pipe: Data out-pipe.

			static const std::string m_oParamMinimumLength1;
			static const std::string m_oParamMinimumLength2;
			static const std::string m_oParamMaximumLineDistance;
			static const std::string m_oParamMaximumDistanceRoiMiddle;
			static const std::string m_oParamMaximumJump;			
			static const std::string m_oParamMinimumSumPixelOnLine;				
			static const std::string m_oParamMaximumLineInterruption;				
			static const std::string m_oParamMaximumBrightness;	
			static const std::string m_oParamCheckIntersection;


			/**
			* @brief Set filter parameters.
			*/
			void setParameter();

			void paint();

		protected:

			/**
			* @brief In-pipe registration.
			* @param p_rPipe Reference to pipe that is getting connected to the filter.
			* @param p_oGroup Group number (0 - proceed is called whenever a pipe has a data element, 1 - proceed is only called when all pipes have a data element).
			*/
			bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

			/**
			* @brief Processing routine.
			* @param p_pSender pointer to
			* @param p_rEvent
			*/
			void proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rE);

		protected:

			interface::SmpTrafo			m_oSpTrafo;					///< roi translation

			const fliplib::SynchronePipe< interface::GeoHoughPPCandidatearray >* m_pPipeInData; ///< Data in-pipe.
			fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutData; ///< Data out-pipe.

			double m_oMinimumLength1;
			double m_oMinimumLength2;
			int m_oMaximumLineDistance;
			int m_oMaximumDistanceRoiMiddle;
			int m_oMaximumJump;			
			double m_oMinimumSumPixelOnLine;
			int m_oMaximumLineInterruption;
			int m_oMaximumBrightness;
			bool m_oCheckIntersection;

			std::vector<std::vector<HoughPPInfoLine>> _allInfoLines;

			std::string convertIntToString(int i);
			std::string convertUuidToString(Poco::UUID id);

			static std::vector<geo2d::HoughPPCandidate> _allPP;
			static int _currentImage;

			static Poco::Mutex m_oMutex;

			int getNumberOfStoredPP(geo2d::HoughPPCandidate cand);

			HoughPPOverlay _overlay;
			int m_oNoErrorNr4Paint;
			std::string m_oOverlayString;

			bool m_hasCandidates;

		}; // class HoughChecker

	} // namespace filter
} // namespace precitec

#endif /* HOUGHCHECKER_H_ */
