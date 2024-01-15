/*
 * calibrationResult.h
 *
 *  Created on: Jun 19, 2013
 *      Author: abeschorner
 */

#ifndef CALIBRATIONRESULT_H_
#define CALIBRATIONRESULT_H_

#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/ResultFilter.h>	    ///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include "event/results.h"
#include <common/frame.h>				///< ImageFrame
#include <image/image.h>				///< BImage
#include <geo/geo.h>					///< Size2d, VecDoublearray
#include <geo/array.h>					///< ByteArray
#include "math/2D/avgAndRegression.h"
#include "math/2D/LineEquation.h"

namespace precitec {
namespace filter {

enum FILTER_API tEndpoint{eEndpointNone=0, eEndpointLeft, eEndpointRight};

struct FILTER_API Endpoint
{
	Endpoint() : m_oXPos(0), m_oYPos(0.0), m_oType(eEndpointNone) {};
	Endpoint(const unsigned int oPos, const double oValue, const tEndpoint oType) : m_oXPos(oPos), m_oYPos(oValue), m_oType(oType){};

	unsigned int m_oXPos;
	double m_oYPos;
	tEndpoint m_oType;
};

class FILTER_API CalibrationResult: public fliplib::ResultFilter {
public:
	static const std::string m_oFilterName;
	static const std::string m_oResultName;

	CalibrationResult();
	virtual ~CalibrationResult();

	/// Set filter parameters as defined in database / xml file.
	void setParameter();
	void paint();

protected:
	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

	/// In-pipe event processing.
	void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg);

	// internal methods
	auto testForEndpoint(const std::vector<int> &p_rRank, const unsigned int p_oPos) -> tEndpoint;
	void findEndPointsHigherLayer(std::vector<Endpoint> &p_rEndpoints, const geo2d::VecDoublearray &p_rLine);
	bool findPointsBothLayers( const geo2d::VecDoublearray &rTopLayer, const geo2d::VecDoublearray &rBotLayer );
	bool findSegment(std::vector<Endpoint> &p_rEndpoints, const unsigned int p_oLeft, const unsigned int p_oRight, const geo2d::VecDoublearray &p_rLine);
	bool collectLayerPoints(const geo2d::VecDoublearray &p_rLineHigher, const geo2d::VecDoublearray &p_rLineLower);
	void invalidateResult( std::vector<double> &p_rResHeader, std::vector<double> &p_rResElements );
	void createResultVector( std::vector<double> &p_rResHeader, std::vector<double> &p_rResElements );

	int binarizeRank(const int oRank);
	void signalSend(const std::vector<double> &p_rResHeader, const std::vector<double> &p_rResElements, const interface::ResultType p_oType,
			const interface::ImageContext &p_rImgContextHigher, const interface::ImageContext &p_rImgContextLower);

private:
	enum ELayer{
		eHigherLayer, eLowerLayer
	};
	//utility methods for evaluation of correction factor
	void evaluateGapWidth(std::ostringstream & oInfoStream, double & gapWidth3D ) const;
	void evaluateGapHeight(std::ostringstream & oInfoStream, int & gapHeightPixel, double & gapHeightZ, double & gapHeight3D, const std::array<math::LineEquation, 2> & slopes) const;
	void evaluateSlopes(std::ostringstream & oInfoStream, std::array<math::LineEquation, 2> & slopes) const;
	void evaluateCurrentCalibration(double & gapHeight3D, double & gapHeightZ) ;
	geo2d::Point getImageCoords(const int & i,  const int & j, const ELayer pLayer) const;
	geo2d::Point getSensorCoords(const int & i, const int & j, const ELayer pLayer) const;
	geo2d::Point m_oHighLeft_SensorCoords;
	geo2d::Point m_oHighRight_SensorCoords;
	geo2d::Point m_oLowLeft_SensorCoords;
	geo2d::Point m_oLowLeftProjected_SensorCoords;
	geo2d::Point m_oLowRight_SensorCoords;
	std::map< std::array<int, 2>, std::vector<std::array<double, 2> > > m_oEvaluationStats;

	const fliplib::SynchronePipe< interface::GeoVecDoublearray >* m_pPipeInTopLayer;  ///< in pipe higher calibration workpiece layer laser line
	const fliplib::SynchronePipe< interface::GeoVecDoublearray >* m_pPipeInBotLayer;  ///< in pipe lower calibration workpiece layer laser line

	fliplib::SynchronePipe< interface::ResultDoubleArray > m_oPipeOutResults;          ///< out pipe combined results vector

	// parameters
	unsigned int m_oLayerSize;            ///< Amount of pixel that needs to be of constant rank for method testForEndPoints. Min 5, Max 100. Default 50
	bool m_ShowCoordinates;

	bool m_oIsTopLayerHigher;  //this is not a parameter set by the user, it's computed by the filter


	std::vector<Endpoint> m_oEndpointsHigher;  ///< End point of higher layer of calibration workpiece
	std::vector<Endpoint> m_oEndpointsLower;  ///< Start- and endpoint of lower layer of calibration workpiece
	std::vector<double> m_oHigherLayerPoints;  ///< Adjacent points of higher layer for laser plane computation
	std::vector<double> m_oLowerLayerPoints;  ///< Adjacent points of lower layer for laser plane computation
	std::vector<double> m_oResultHeader;
	std::vector<double> m_oResultElements;
	filter::LaserLine m_oTypeOfLaserLine;	///< used only with intensity max, to verify current calibration

	//used only for evaluateCurrentCalibration
	double m_oGapWidth; //read from calibrationdata
	double m_oGapHeight; //read from calibrationdata
	std::string m_oInfo;

	// internal variables
	interface::SmpTrafo m_oSpTopTrafo;      ///< roi translation higher layer
	interface::SmpTrafo m_oSpBotTrafo;      ///< roi translation lower layer
	geo2d::Point	m_oHwRoi;

};

} /* namespace filter */
} /* namespace precitec */
#endif /* CALIBRATIONRESULT_H_ */
