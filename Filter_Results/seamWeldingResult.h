/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		LB
 * 	@date		2020
 * 	@brief		Result filter for double values. Performs a range check of the value.
 */

#ifndef SEAMWELDINGRESULT_H
#define SEAMWELDINGRESULT_H

#include "fliplib/Fliplib.h"
#include "fliplib/ResultFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"

#include "event/results.h"
#include "geo/range.h"

namespace precitec {
namespace filter {

class FILTER_API SeamWeldingResult : public fliplib::ResultFilter
{
public:
    enum class InputContourType { pixel = 0, mm_relative = 1};
    enum class InputAbsolutePositionUnit { mm = 0, um = 1};
    enum class ScanmasterResultType{SeamWelding = 0, ScannerMovingToFirstPoint = 1, Dummy = 2, SeamWeldingAndSendEndOfSeam = 3, PrepareContour = 4};

	SeamWeldingResult();
   	void setParameter();

private:
    typedef fliplib::SynchronePipe<interface::GeoDoublearray>	pipe_scalar_t;
	typedef fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>	pipe_contour_t;
	typedef fliplib::SynchronePipe<interface::ResultDoubleArray>	pipe_result_t;

	bool subscribe(fliplib::BasePipe&, int);
	void proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& e );
 	void paint();

    template<class UnaryFunction>
    static bool transformPointsToScanmasterResultArray(geo2d::Doublearray & rResultArray, const geo2d::AnnotatedDPointarray & points, UnaryFunction f, bool forceBadRank, double xMin, double xMax, double yMin, double yMax);

    geo2d::DPoint getTCPfromCameraPosition_pix (int HW_ROI_x0, int HW_ROI_y0) const;


    void fillResultWithRelativeCoords_mm(geo2d::AnnotatedDPointarray points);
    void fillResultWithTCPDistance(geo2d::AnnotatedDPointarray points);


	const pipe_contour_t *m_pPipeInContour_pix;
    const pipe_scalar_t * m_pPipeInAbsolutePositionX;
    const pipe_scalar_t * m_pPipeInAbsolutePositionY;
	pipe_result_t m_oPipeResultPath_mm; //special format x,y,x,y

	InputContourType m_inputContourType;
    InputAbsolutePositionUnit m_inputAbsolutePositionUnit;
    ScanmasterResultType m_outputType;
    double m_maxXCoordinate_mm;
    double m_maxYCoordinate_mm;


    geo2d::TArray<double> m_oResultArray;
    geo2d::DPoint m_scannerInputPosition_at_TCP; // scanner position mm (tcp)

    //copies of data for painting
    bool m_oPaint;
    geo2d::Point m_tcpInputPositionToPaint; //tcp position
    geo2d::Point m_tcpActualPositionToPaint;
    interface::SmpTrafo m_oSpTrafo;
    geo2d::AnnotatedDPointarray m_pointsToPaint; //contour points
    interface::ScannerContextInfo m_scannerPositionActual; // scanner position mm (tcp)

};

}
}
#endif
