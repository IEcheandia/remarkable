/**
 *  @file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		LB
 *  @date		2022
 *  @brief		Computes maximum / minimum of a laserline.
 */

#pragma once


#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <image/image.h>				///< BImage
#include "filter/parameterEnums.h"		///< enum ExtremumType

#include <common/frame.h>				///< ImageFrame
#include <geo/geo.h>					///< Size2d, VecDoublearray
#include <geo/array.h>					///< ByteArray


namespace precitec {
namespace filter {

/**
* @ingroup Filter_LineGeometry
* @brief This filter finds the maximum or minimum of a laser line and returns it as a point output pipe.
*/
class FILTER_API LineSelectLocalExtremum  : public fliplib::TransformFilter
{
public:

    LineSelectLocalExtremum();

	static const std::string m_oFilterName;		///< Name Filter
	static const std::string PIPENAME_OUT1;		///< Name Out-Pipe 1
	static const std::string PIPENAME_OUT2;		///< Name Out-Pipe 2

    void setParameter();
    void paint();


protected:

    bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
    void proceedGroup ( const void * p_pSender, fliplib::PipeGroupEventArgs & p_rEventArg );

private:
    static int indexMaxUnassignedArrayElement (const std::vector<double> & rLine, std::vector<int> mask );
    static int indexMinUnassignedArrayElement (const std::vector<double> & rLine, std::vector<int> mask );

    template<ExtremumType extremumType>
    std::tuple<std::vector<geo2d::DPoint>, double, double> computePeaks(const std::vector<double> & rLine, std::vector<int> & mask, bool maskCurrentPeak ) const;


    const fliplib::SynchronePipe< interface::GeoVecDoublearray >* m_pPipeLineIn;///< In pipe
    fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipePositionXOut;	///< Out pipe
    fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipePositionYOut;	///< Out pipe
    fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeStartPeakX;
    fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeEndPeakX;

    // parameters
    ExtremumType m_oExtremumType;		///< Searching for the maximum or minimum?
    unsigned int m_oExtremumNumber;
    unsigned int m_oExtremumDistance;    ///< Distance between the points
    double m_oExtremumDifference; ///< Difference between the points

    // internal variables
    std::vector<int> m_maskCache;


    std::vector<int> m_paintMask;
    std::vector<double> m_paintLine;
    std::vector<geo2d::Point> m_paintPeaks;
    bool m_paintValid;
    interface::SmpTrafo m_oSpTrafo;				///< roi translation
};

} // namespace precitec
} // namespace filter
