/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2016
 * 	@brief		This filter produces a single constant value, which is a selectable system parameter (tcp position, hw roi size, etc).
 */

#ifndef SYSTEMCONSTANT_H_
#define SYSTEMCONSTANT_H_

// WM includes
#include <fliplib/Fliplib.h>
#include <fliplib/TransformFilter.h>
#include <fliplib/SynchronePipe.h>
#include <geo/geo.h>
#include <common/frame.h>
#include <util/calibDataSingleton.h>

namespace precitec {
namespace filter {

enum
{
	eNull = 0,
	eTCP_X,
	eTCP_Y,
	eHWROI_X,
	eHWROI_Y,
	eImageNumber,
	ePosition,
	eInput_W, 
	eInput_H,
	eUpper,
	eLower,
    eDpixX,
    eDpixY,
    eBeta0,
    eBetaZ,
    eBetaZ_2,
    eBetaZ_TCP,
    eRatio_pix_mm_horizontal,
    eRatio_pix_mm_Z1,
    eRatio_pix_mm_Z2,
    eRatio_pix_mm_Z3,
    eContextTCP_X,
    eContextTCP_Y,
    eContextTCP_Y2,
    eContextTCP_Y3
};

/**
 * @brief This filter produces a single constant value, which is a selectable system parameter (tcp position, hw roi size, etc).
 */
class FILTER_API SystemConstant : public fliplib::TransformFilter
{
public:

	/**
	 * CTor.
	 */
	SystemConstant();
	/**
	 * @brief DTor.
	 */
	virtual ~SystemConstant();

	// Declare constants
	static const std::string m_oFilterName;			///< Filter name.
	static const std::string m_oPipeOutName;		///< Pipe: Data out-pipe.
	static const std::string m_oParamConstant;		///< Parameter: Type of system constant.

	/**
	 * @brief Set filter parameters.
	 */
	void setParameter();

protected:

    enum class coord {x,y};
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
    double getTCP(coord p_coord, LaserLine p_laserline);
    double getCalibrationParameter(std::string key);
    double getContextTCP(const interface::ImageContext & rContext, coord p_coord, LaserLine p_laserline);
    double getRatio_pix_mm_horizontal(const interface::ImageContext & rContext, image::Size2d imageSize);
    double getRatio_pix_mm_Z(const interface::ImageContext & rContext, image::Size2d imageSize, filter::LaserLine line);

protected:

	const fliplib::SynchronePipe< interface::ImageFrame >*		m_pPipeInImage;			///< Image in-pipe.
	fliplib::SynchronePipe< interface::GeoDoublearray >			m_oPipeOutData;			///< Data out-pipe.

	int 														m_oConstant;			///< Parameter: Type of constant, e.g. tcp_x, etc.

}; // class SystemConstant

} // namespace filter
} // namespace precitec

#endif /* SYSTEMCONSTANT_H_ */
