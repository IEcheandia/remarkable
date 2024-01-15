/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief 		This filter creates a seamFindDummy
*/

#ifndef SEAMFINDINGDUMMY_H_
#define SEAMFINDINGDUMMY_H_

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

		class FILTER_API SeamFindingDummy : public fliplib::TransformFilter
		{
		public:
			SeamFindingDummy();
			virtual ~SeamFindingDummy();

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

			const fliplib::SynchronePipe< interface::ImageFrame >        * m_pPipeInImageFrame;	///< In pipe
			fliplib::SynchronePipe< interface::GeoSeamFindingarray >     * m_pPipeOutSeamFinding;	///< Out pipe

			interface::SmpTrafo											m_oSpTrafo;				///< roi translation
			int 														m_oSeamLeft;			 
			int															m_oSeamRight;		     
			int															m_oAlgoType;			
			int															m_oQuality;			

		};

	} // namespace precitec
} // namespace filter

#endif /* SEAMFINDINGDUMMY_H_ */
