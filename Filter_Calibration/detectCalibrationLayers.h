/**
 * 	@file       detectCalibrationLayers.h
 * 	@copyright  Precitec Vision GmbH & Co. KG
 * 	@author     Andreas Beschorner (BA)
 * 	@date       2013
 * 	@brief      Tries to detect two calibration workpiece main layers by simple summed pixel intensity per line decision
 */

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"

#include "common/frame.h"
#include "geo/geo.h"


using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {


class FILTER_API DetectCalibrationLayers : public fliplib::TransformFilter
{
	static const std::string m_oFilterName;
	static const std::string m_oPipeNameRois;

	static const int m_oMinDist;

public:
	DetectCalibrationLayers();
	virtual ~DetectCalibrationLayers();

	void setParameter();
	void paint();

protected:
	/// in pipe registrastion
	bool subscribe(fliplib::BasePipe& pipe, int group);

	// Verarbeitungsmethode IN Pipe
	void proceed(const void* sender, fliplib::PipeEventArgs& e);
		// ordered list of 2 maximae where oValue[0] >= oValue[1]
	struct tMaximae
	{
		int oValue[2];
		int oIdx[2];
	};

private:
	const fliplib::SynchronePipe< ImageFrame >*	m_pPipeInImageFrame;    ///< in pipe
	fliplib::SynchronePipe< interface::GeoVecDoublearray >	m_oPipeOutRois; ///< Output PIN fuer Minimum

	// internal methods
	unsigned int guessThreshold(std::map<int, int> &p_rBins);
	/// Finds the two (first) lightest lines of the layers, using m_oThreshold as a separation threshold. Returns true on success.
	bool findLayerCenters(std::vector<int> &p_rIntensities, const ImageFrame &p_rFrame);
	/// Inserts value and its associate index into m_oMaximae if the is greater than any of the two members
	inline void insert(std::vector<int> &p_rMaximae, const int p_oIdxMaximae, const std::vector<int> &p_rIntensities, const int p_oIndex);
	void getHighestPeaks(tMaximae &p_rMaximae, const std::vector<int> &p_rIntensities, const std::vector<int> &p_rVecMaximae);
	bool validMaximae(); ///< both maximae must be set and their indices should have a distance of least m_oMinDist (pixel)
	void resetMaximae(); ///< reset values

	bool determineExtent(const std::vector<int> &p_rIntensities, int m_oIdxMax);
	/// Determines ROIs of layers based on both the parameter "Extend" and the results from DetectCalibrationLayers::findLayerCenters. Returns true if both have a mininum extend of m_oMinDist/2.
	bool determineROIs(const std::vector<int> &p_rIntensities);

	void signalSend(const ImageContext &p_rImgContext, const int p_oIO);

	// parameter variables
	int m_oExtent;          ///< Parameter Extend
	double m_oExtentFactor; ///< Equals parameter "Extend"/100.0 and is in [0.02, 0.2]. "Extend" % of most significant lines will be used for the ROIs
	int m_oParameterThreshold;       ///< The (average, over a line) pixel gray value that separates the two layers
	int m_oActualThreshold;

	// internal variables
	geo2d::VecDoublearray m_oRois; ///< upper left corner, delta x and delta y for each roi in a stream of ints
	int m_oRoiYTop[2];    ///< Data for ROI 1, self explaning. For Paint.
	int m_oRoiYBottom[2]; ///< Data for ROI 2, self explaning. For Paint.
	bool m_oPaint;
	interface::SmpTrafo m_oSpTrafo; ///< Roi translation
	int m_oWidth;

	tMaximae m_oMaximae;
};

} // namespaces
}
