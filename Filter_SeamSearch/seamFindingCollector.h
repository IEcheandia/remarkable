/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief 		This filter collects two seamFindings
*/

#ifndef SEAMFINDINGCOLLECTOR_H_
#define SEAMFINDINGCOLLECTOR_H_

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

		class FILTER_API SeamFindingCollector : public fliplib::TransformFilter
		{
		public:
			SeamFindingCollector();
			virtual ~SeamFindingCollector();

			static const std::string m_oFilterName;		///< Name Filter
			static const std::string PIPENAME_OUT;		///< Name Out-Pipe

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

			const fliplib::SynchronePipe< interface::GeoSeamFindingarray >        * m_pPipeInSeamFindings1;	///< In pipe
			const fliplib::SynchronePipe< interface::GeoSeamFindingarray >        * m_pPipeInSeamFindings2;	///< In pipe
			fliplib::SynchronePipe< interface::GeoSeamFindingarray >     * m_pPipeOutSeamFindings;	///< Out pipe

			interface::SmpTrafo											m_oSpTrafo;				///< roi translation
			int 														m_oMode;

		};

	} // namespace precitec
} // namespace filter

#endif /* SEAMFINDINGDUMMY_H_ */
