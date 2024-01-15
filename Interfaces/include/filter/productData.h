/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			HS
 *  @date			2013
 *  @brief			ProductData contains inspection relevant product data like velocity or trigger distance.
 */

#ifndef PRODUCTDATA_20130715_INCLUDED_H
#define PRODUCTDATA_20130715_INCLUDED_H

#include "common/product1dParameter.h"

#include "fliplib/FilterControlInterface.h"


namespace precitec {
namespace analyzer {


	class ProductData : public fliplib::ExternalData {
	public:
		ProductData(
			int 								p_oSeamSeries = 0,
			int 								p_oSeam = 0,
			int 								p_oInspectionVelocity = 0,
			int 								p_oTriggerDelta = 0,
			int									p_oNumTrigger = 1000,
			const interface::id_refcurve_map_t*	p_pIdRefCurveMap = nullptr,
			int									p_oLength = 0,
			int									p_oDirection = 0,
			int									p_oThicknessLeft = 0,
			int									p_oThicknessRight = 0,
			int                                 p_oTargetDifference = 0,
			int                                 p_oRoiX = 0,
			int                                 p_oRoiY = 0,
			int                                 p_oRoiW = 0,
			int                                 p_oRoiH = 0
			)
			:
			m_oSeamSeries						{ p_oSeamSeries },
			m_oSeam								{ p_oSeam },
			m_oInspectionVelocity				{ p_oInspectionVelocity },
			m_oTriggerDelta						{ p_oTriggerDelta },
			m_oNumTrigger						{ p_oNumTrigger },
			m_pIdRefCurveMap					{ p_pIdRefCurveMap },
			m_oLength							{ p_oLength },
			m_oDirection						{ p_oDirection },
			m_oThicknessLeft					{ p_oThicknessLeft },
			m_oThicknessRight					{ p_oThicknessRight },
            m_oTargetDifference                 { p_oTargetDifference },
            m_oRoiX                             { p_oRoiX },
            m_oRoiY                             { p_oRoiY },
            m_oRoiW                             { p_oRoiW },
            m_oRoiH                             { p_oRoiH }
		{
		} // ProductData()

		int									m_oSeamSeries;					///< current seam series
		int									m_oSeam;						///< current seam
		int									m_oInspectionVelocity;			///< speed [um/ms]
		int									m_oTriggerDelta;				///< trigger distance [um]
		int									m_oNumTrigger;					///< total number of images in seam
		const interface::id_refcurve_map_t*	m_pIdRefCurveMap;				///< stores reference curves
		int									m_oLength;						///< defined seam length
		int									m_oDirection;					///< direction of inspection from upper/lower
		int									m_oThicknessLeft;				///< part thickness left
		int									m_oThicknessRight;				///< part thickness right
        int                                 m_oTargetDifference;
        int                                 m_oRoiX;
        int                                 m_oRoiY;
        int                                 m_oRoiW;
        int                                 m_oRoiH;
	}; // class ProductData


} // namespace analyzer
} // namespace precitec

#endif // PRODUCTDATA_20130715_INCLUDED_H
