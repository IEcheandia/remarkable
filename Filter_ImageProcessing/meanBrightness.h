/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief 		This filter calculates the mean brightness in the whole image.
*/

#ifndef MEANBRIGHTNESS_H_
#define MEANBRIGHTNESS_H_

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

		class FILTER_API MeanBrightness : public fliplib::TransformFilter
		{

		public:

			MeanBrightness();
			virtual ~MeanBrightness();

			static const std::string m_oFilterName;				///< Name Filter
			static const std::string PIPENAME_MEAN_OUT;		    ///< Name Out-Pipe

			void setParameter();
			void paint();

		protected:

			/// In-pipe registration.
			bool subscribe(fliplib::BasePipe& pipe, int group);
			/// In-pipe event processing.
			void proceed(const void* sender, fliplib::PipeEventArgs& e);

		private:

			const fliplib::SynchronePipe< interface::ImageFrame >        * m_pPipeInImageFrame;		///< In pipe
			fliplib::SynchronePipe< interface::GeoDoublearray >			 * m_pPipeOutMean;		    ///< Out pipe

			interface::SmpTrafo											m_oSpTrafo;				///< roi translation

			int m_oResolutionX;
			int m_oResolutionY;

			geo2d::Doublearray m_oMeanOut;

			bool m_hasPainting;
		};

	} // namespace precitec
} // namespace filter

#endif /* MEANBRIGHTNESS_H_ */
