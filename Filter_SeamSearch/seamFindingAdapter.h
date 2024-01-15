/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief		Filter that allows the access to every value coming out of the seam finding struct
*/

#ifndef SEAMFINDINGADAPTER_H_INCLUDED
#define SEAMFINDINGADAPTER_H_INCLUDED

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include "geo/geo.h"
#include "geo/array.h"
#include "math/3D/projectiveMathStructures.h"
#include "filter/parameterEnums.h"


namespace precitec {
	namespace filter {

		class FILTER_API SeamFindingAdapter : public fliplib::TransformFilter
		{
		public:

			/**
			* @brief CTor.
			*/
			SeamFindingAdapter();

		private:

			/**
			* @brief Set filter parameters.
			*/
			void setParameter();

			/**
			* @brief Paint overlay output.
			*/
			void paint();

			// Declare constants
			static const std::string m_oFilterName;
			static const std::string m_oPipeOutDataName;

			/**
			* @brief In-pipe registration.
			*/
			bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

			/**
			* @brief Processing routine.
			*/
			void proceed(const void* p_pSender, fliplib::PipeEventArgs&);

			typedef fliplib::SynchronePipe<interface::GeoSeamFindingarray> seamfinding_pipe_t;
			typedef fliplib::SynchronePipe<interface::GeoDoublearray> scalar_pipe_t;

			const seamfinding_pipe_t*	m_pPipeInSeamFinding;	///< in-pipe.
			scalar_pipe_t 				m_oPipeOutData;			///< out-pipe.

			interface::SmpTrafo			m_oSpTrafo;				///< roi translation
			int							m_oHwRoiY;				///< Y coordinate of hw roi.	

			geo2d::Doublearray			m_oArrayData;

			//math::CalibrationGrid		m_oCalibGrid;			///< Calibration data object.

			SeamFindingAdapterType		m_oSeamFindingAdapter;			///< type of adapter
		}; // class BlobAdapter

	} // namespace filter
} // namespace precitec

#endif /* SEAMFINDINGADAPTER_H_INCLUDED */
