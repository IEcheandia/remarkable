/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2017
* 	@brief 		This filter looks for the grey values on the tracked laser line. A gap is detected if several values are lower than a given threshold.
*/

#ifndef LINETRACKINGQUALITY_H_
#define LINETRACKINGQUALITY_H_

#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <common/frame.h>				///< ImageFrame
#include <image/image.h>				///< BImage
#include <geo/geo.h>					///< Size2d, Intarray
#include <geo/array.h>					///< ByteArray

namespace precitec {
	namespace filter {


		class FILTER_API LineTrackingQuality : public fliplib::TransformFilter
		{

		public:

			/// CTor.
			LineTrackingQuality();
			/// DTor.
			virtual ~LineTrackingQuality();

			static const std::string m_oFilterName;		///< Name Filter
			static const std::string PIPENAME_GAPWIDTH_OUT;		///< Name Out-Pipe
			static const std::string PIPENAME_GAPDETECTED_OUT;		///< Name Out-Pipe

			/// Set filter parameters as defined in database / xml file.
			void setParameter();
			/// paints overerlay primitives
			void paint();

			/**
			* @brief
			*
			* @param p_rImageIn       Input image.
			* @param p_rLaserLineIn   LaserLine input object.
			* @param p_oLineHeight    Height of the laser line object.
			* @param p_rProfileOut    Profile output object.
			* @param p_oProfileHeight Height of the output profile (for each of the upper and lower band).
			*/
			int calcGapWidth(const image::BImage &p_rImageIn, const geo2d::VecDoublearray &p_rLaserLineIn,
				geo2d::Doublearray & widthOut, geo2d::Doublearray & gapDetected);

		protected:

			/// In-pipe registration.
			bool subscribe(fliplib::BasePipe& pipe, int group);
			/// In-pipe event processing.
			void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

		private:

			const fliplib::SynchronePipe< interface::ImageFrame >        * m_pPipeInImageFrame;			///< In pipe
			const fliplib::SynchronePipe< interface::GeoVecDoublearray > * m_pPipeInLaserLine;			///< In pipe

			fliplib::SynchronePipe< interface::GeoDoublearray >       * m_pPipeOutGapWidth;	///< Out pipe
			fliplib::SynchronePipe< interface::GeoDoublearray >       * m_pPipeOutGapDetected;	///< Out pipe

			interface::SmpTrafo											m_oSpTrafo;				///< roi translation

			int															m_oMode;		///< Mode 
			int															m_oThreshold; ///< Threshold
			int															m_oSearchHeight;		///< height
			int															m_oMaxGapWidth;   ///< Threshold when a gap is detected

			int m_overlayMin, m_overlayMax;

			int m_oOverlayGapStart, m_oOverlayGapEnd, m_oOverlayGapY;

		};

	} // namespace precitec
} // namespace filter

#endif /* LINETRACKINGQUALITY_H_ */
