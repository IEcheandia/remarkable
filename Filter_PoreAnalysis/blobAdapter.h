/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		02 / 2015
 * 	@brief		Filter that allows the access to every value coming out of the pore analysis
 */

#ifndef BLOBADAPTER_H_INCLUDED
#define BLOBADAPTER_H_INCLUDED

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

class FILTER_API BlobAdapter : public fliplib::TransformFilter
{
public:

	/**
	 * @brief CTor.
	 */
	BlobAdapter();

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
	void proceed( const void* p_pSender, fliplib::PipeEventArgs& );

	typedef fliplib::SynchronePipe<interface::GeoBlobarray> blob_pipe_t;
	typedef fliplib::SynchronePipe<interface::GeoDoublearray> scalar_pipe_t;

	const blob_pipe_t*			m_pPipeInBlob;			///< in-pipe.
	scalar_pipe_t 				m_oPipeOutData;			///< out-pipe.

	interface::SmpTrafo			m_oSpTrafo;				///< roi translation
	int							m_oHwRoiY;				///< Y coordinate of hw roi.	

	geo2d::Doublearray			m_oArrayData;

	BlobAdapterType				m_oBlobAdapter;			///< type of adapter
}; // class BlobAdapter

} // namespace filter
} // namespace precitec

#endif /* BLOBADAPTER_H_INCLUDED */
