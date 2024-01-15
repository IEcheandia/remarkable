/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		KIH, Simon Hilsenbeck (HS)
* 	@date		2012
* 	@brief		
*/

#ifndef GAP_POSITION_PROJECTION_20120711_INCLUDED
#define GAP_POSITION_PROJECTION_20120711_INCLUDED

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"

#include "geo/geo.h"
#include "common/frame.h"
#include "souvisSourceExportedTypes.h"

namespace precitec {
	using namespace image;
	using namespace interface;
	namespace filter {



class GapPositionProjectionparT
{
public:
	//int debuglevel;
	int MaxGapBridge;
	int SinglevalueThreshold;
	int HzProjektionMinDeltay;  //=10
	int HzProjektionMinimalMax; //=255;
	int VProjektionThreshold; //=255;
};


class FILTER_API GapPositionProjection : public fliplib::TransformFilter
{
public:

	GapPositionProjection();
	virtual ~GapPositionProjection();

	static const std::string m_oFilterName;
	static const std::string FILTERBESCHREIBUNG;
	static const std::string PIPENAME_POS1;
	static const std::string PIPENAME_POS2;

	void setParameter();
	void paint();

	double gapposx,gapposy,gapposrank;

	SSF_SF_InputStruct sgmima;
	double proj1[2000];
	double proj2[2000];

	// Parameter
	GapPositionProjectionparT par;

	//display
	int markcolor_;

	bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
	// Verarbeitungsmethode IN Pipe
	void proceed(const void* sender, fliplib::PipeEventArgs& e);

	const fliplib::SynchronePipe< interface::ImageFrame >* pipeInFrame_;
	//output
	fliplib::SynchronePipe< GeoDoublearray >* pipeOutPosX_; //<- Out
	fliplib::SynchronePipe< GeoDoublearray >* pipeOutPosY_; //<- Out
	interface::SmpTrafo						m_oSpTrafo;				///< roi translation
	geo2d::Point m_oGapPosition;

	int BondPosCalculation(double & x,double & y,double & rank);
	void HorizontalProjection(SSF_SF_InputStruct & img,int xug,int xog,int yug,int yog,int threshold,double *proj,int & projend);
	void DrawHorizontalprojection(double * proj,int projend,int xug,int yug,int displaywidth);
	int GetSeparationBetweenTwoMaxima(double * proj,int projend,int & separationpos);

	void VerticalProjection(SSF_SF_InputStruct & img,int xug,int xog,int yug,int yog,int threshold,double *proj,int & projend);
	void DrawVerticalprojection(double * proj,int projend,int xug,int yug,int displayheight);
	int  FindHorizontalSeparation(double *proj1,int projend1,double *proj2,int projend2,int & Xseppos);
	int Find_XL_XR_1(double *proj,int projend,int threshold,int & xL, int & xR);
	int Find_XL_XR_2(double *proj,int projend,int threshold,int & xL, int & xR);


	struct PeakSectionT
	{
		unsigned short xL;
		unsigned short xR;
		float spotMaxW;
		float spotSumW;
		unsigned short onspotlength;
		unsigned short spotlength;
	};

	int CalculateLineSections(double *linearray,int npixx,int threshold, PeakSectionT * Peaksect, int & PeaksectAnz);


}; // GapPositionProjection

	}} // namespaces


#endif 
