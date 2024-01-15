/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		CB
 * 	@date		2019
 * 	@brief		This filter produces a single constant value, which is a selectable seam parameter (length, trigger delta, speed, etc).
 */

#ifndef SEAMCONSTANT_H_
#define SEAMCONSTANT_H_

// WM includes
#include <fliplib/Fliplib.h>
#include <fliplib/TransformFilter.h>
#include <fliplib/SynchronePipe.h>
#include <geo/geo.h>
#include <common/frame.h>
#include <util/calibDataSingleton.h>
#include <filter/productData.h>
// Poco includes
#include "Poco/Mutex.h"
// stl includes
#include <queue>

namespace precitec {
namespace filter {

enum
{
	eSeamNull = 0,
	eLength,
	eVelocity,
	eTriggerDistance,
	eNumTrigger,
	eDirection,
	eThicknessLeft,
	eThicknessRight,
	eSeamNo,
	eSeamSeriesNo,
    eTargetDifference,
    eRoiX,
    eRoiY,
    eRoiW,
    eRoiH
};

/**
 * @brief This filter produces a single constant value, which is a selectable seam parameter (length, trigger delta, speed, etc).
 */
class FILTER_API SeamConstant : public fliplib::TransformFilter
{
public:

	/**
	 * CTor.
	 */
	SeamConstant();
	/**
	 * @brief DTor.
	 */
	virtual ~SeamConstant();

	// Declare constants
	static const std::string m_oFilterName;			///< Filter name.
	static const std::string m_oPipeOutName;		///< Pipe: Data out-pipe.
	static const std::string m_oParamConstant;		///< Parameter: Type of system constant.

	/**
	 * @brief Set filter parameters.
	 */
	void setParameter();

	/**
	* @brief Arm the filter. The filter will empty
	*/
	virtual void arm(const fliplib::ArmStateBase& state);

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
	void proceed( const void* p_pSender, fliplib::PipeEventArgs& p_rE );

protected:

	const fliplib::SynchronePipe< interface::ImageFrame >*		m_pPipeInImage;			///< Image in-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >			m_oPipeOutData;			///< Data out-pipe.

	int 														m_oConstant;			///< Parameter: Type of constant, e.g. length, etc.

	analyzer::ProductData									m_oProductData;			///< Product Info with seam details

	std::queue< interface::GeoDoublearray > 					m_oQueue;				///< The data queue.
	Poco::FastMutex m_oMutex;

}; // class SeamConstant

} // namespace filter
} // namespace precitec

#endif /* SEAMCONSTANT_H_ */
