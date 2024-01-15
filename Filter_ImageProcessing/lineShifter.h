/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief 		This filter is able to shift some lines in an image
*/

#ifndef LINESHIFTER_H_
#define LINESHIFTER_H_

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

		class FILTER_API LineShifter : public fliplib::TransformFilter
		{

		public:

			LineShifter();
			virtual ~LineShifter();

			static const std::string m_oFilterName;				///< Name Filter
			static const std::string PIPENAME_IMAGE_OUT;		///< Name Out-Pipe
			//static const std::string PIPENAME_LASERLINE_OUT;	///< Name Out-Pipe
			//static const std::string PIPENAME_DOUBLE1_OUT;		///< Name Out-Pipe
			//static const std::string PIPENAME_DOUBLE2_OUT;		///< Name Out-Pipe
			//static const std::string PIPENAME_DOUBLE3_OUT;		///< Name Out-Pipe

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

			const fliplib::SynchronePipe< interface::ImageFrame >        * m_pPipeInImageFrame;		///< In pipe
			//const fliplib::SynchronePipe< interface::GeoVecDoublearray > * m_pPipeInLaserLine;		///< In pipe
			//const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInDouble1;		///< In pipe
			//const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInDouble2;		///< In pipe
			//const fliplib::SynchronePipe< interface::GeoDoublearray >    * m_pPipeInDouble3;		///< In pipe

			fliplib::SynchronePipe< interface::ImageFrame >				 * m_pPipeOutImageFrame;	///< Out pipe
			//fliplib::SynchronePipe< interface::GeoVecDoublearray >       * m_pPipeOutLaserLine;		///< Out pipe
			//fliplib::SynchronePipe< interface::GeoDoublearray >			 * m_pPipeOutDouble1;		///< Out pipe
			//fliplib::SynchronePipe< interface::GeoDoublearray >			 * m_pPipeOutDouble2;		///< Out pipe
			//fliplib::SynchronePipe< interface::GeoDoublearray >			 * m_pPipeOutDouble3;		///< Out pipe

			interface::SmpTrafo											m_oSpTrafo;				///< roi translation

			int m_oMode;

			int m_oNoShiftLinePercent;
			int m_oShiftTop;
			int m_oShiftBottom;
			int m_oFillGreyValue;

			int m_oX1, m_oY1;
			int m_oX2, m_oY2;
			int m_oX3, m_oY3;

			//geo2d::VecDoublearray m_oLaserLineOut;
			//geo2d::Doublearray m_oDouble1Out;
			//geo2d::Doublearray m_oDouble2Out;
			//geo2d::Doublearray m_oDouble3Out;

			bool m_hasPainting;

			precitec::image::BImage m_imageOut;

		};

	} // namespace precitec
} // namespace filter

#endif /* LINESHIFTER_H_ */
