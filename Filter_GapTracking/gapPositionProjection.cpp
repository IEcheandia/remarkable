/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		KIH, Simon Hilsenbeck (HS)
* 	@date		2012
* 	@brief
*/

#include "gapPositionProjection.h"


#include <cmath>
#include <sstream>

#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "filter/algoArray.h"
#include "module/moduleLogger.h"

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;

namespace precitec {
	using namespace geo2d;
namespace filter {


const std::string GapPositionProjection::FILTERBESCHREIBUNG = std::string("GapPositionProjection Bondposition aus Hz- und V-Projektionsauswertung, gut fuer Ueberlappstoss\n ");
const std::string GapPositionProjection::m_oFilterName = std::string("GapPositionProjection");
const std::string GapPositionProjection::PIPENAME_POS1	= std::string("PositionX");
const std::string GapPositionProjection::PIPENAME_POS2	= std::string("PositionY");


GapPositionProjection::GapPositionProjection() : TransformFilter( GapPositionProjection::m_oFilterName, Poco::UUID{"D2E19E80-8AB0-45C3-B6D0-2D1E4BAD7C8F"} ),
	pipeInFrame_( nullptr ),
	pipeOutPosX_( new SynchronePipe< GeoDoublearray >( this, GapPositionProjection::PIPENAME_POS1 ) ),
	pipeOutPosY_( new SynchronePipe< GeoDoublearray >( this, GapPositionProjection::PIPENAME_POS2 ) )
{
	// postulate parameter

	par.MaxGapBridge=16;
	par.SinglevalueThreshold=150;
	par.HzProjektionMinDeltay=10;
	par.HzProjektionMinimalMax=255;
	par.VProjektionThreshold=255;

	parameters_.add("MaxGapBridge", "int", par.MaxGapBridge);
	parameters_.add("SinglevalueThreshold", "int", par.SinglevalueThreshold);
	parameters_.add("HzProjektionMinDeltay", "int", par.HzProjektionMinDeltay);
	parameters_.add("HzProjektionMinimalMax", "int", par.HzProjektionMinimalMax);
	parameters_.add("VProjektionThreshold", "int", par.VProjektionThreshold);

    setInPipeConnectors({{Poco::UUID("9A6B3872-125E-4912-89B9-73A326001502"), pipeInFrame_, "ImageFrame", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("A32C017C-2A93-4849-8EDC-70D7FB30C26C"), pipeOutPosX_, "PositionX", 0, ""},
    {Poco::UUID("58CEEA63-4C23-4A5B-B006-BE86233C4600"), pipeOutPosY_, "PositionY", 0, ""}});
    setVariantID(Poco::UUID("A4F772F8-D2B2-429E-B028-E36B4F29B2DA"));
}

GapPositionProjection::~GapPositionProjection()
{
	delete pipeOutPosX_;
	delete pipeOutPosY_;
}



void GapPositionProjection::paint() {
	if(m_oVerbosity < eLow || m_oSpTrafo.isNull()){
		return;
	} // if

	const Trafo		&rTrafo			( *m_oSpTrafo );
	OverlayCanvas	&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerPosition	( rCanvas.getLayerPosition());

	rLayerPosition.add( new	OverlayCross(rTrafo(m_oGapPosition), Color::Green())); // paint position in green

} // paint



void GapPositionProjection::setParameter()
{
	TransformFilter::setParameter();
	//wmLog(eDebug, "GapPositionProjection::setParameter()\n");

	par.MaxGapBridge =  parameters_.getParameter("MaxGapBridge").convert<int>();
	par.SinglevalueThreshold =  parameters_.getParameter("SinglevalueThreshold").convert<int>();
	par.HzProjektionMinDeltay =  parameters_.getParameter("HzProjektionMinDeltay").convert<int>();
	par.HzProjektionMinimalMax =  parameters_.getParameter("HzProjektionMinimalMax").convert<int>();
	par.VProjektionThreshold =  parameters_.getParameter("VProjektionThreshold").convert<int>();

	if(m_oVerbosity >= eMedium)
	{
		wmLog(eDebug, "%s", GapPositionProjection::FILTERBESCHREIBUNG.c_str());
	}

	//CurvatureGradRatio_= parameters_.getParameter("curvaturegradratio").convert<double>();

	//markcolor_=ovstring2color(parameters_.getParameter("markcolor").convert<std::string>());
	markcolor_=0x00ff00;

}



bool GapPositionProjection::subscribe(BasePipe& p_rPipe, int p_oGroup) {

	pipeInFrame_  = dynamic_cast< fliplib::SynchronePipe < ImageFrame > * >(&p_rPipe);
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void GapPositionProjection::proceed(const void* sender, PipeEventArgs& e)
{
	poco_assert_dbg(pipeInFrame_ != nullptr); // to be asserted by graph editor
	const ImageFrame	&rFrame = pipeInFrame_->read(m_oCounter);
	const BImage		&rBImage = rFrame.data();
	m_oSpTrafo	= rFrame.context().trafo();

	sgmima.img= rBImage.begin();
	sgmima.npixx = rBImage.width();
	sgmima.npixy= rBImage.height();
	sgmima.pitch= rBImage.rowBegin(1) -  rBImage.rowBegin(0);
	sgmima.roistart=sgmima.img;
	sgmima.roix0=0;
	sgmima.roiy0=0;
	sgmima.roidx=sgmima.npixx;
	sgmima.roidy=sgmima.npixy;


	if(m_oVerbosity >= eHigh)
	{
		wmLog(eDebug, "GapPositionProjection\n");
		wmLog(eDebug, "MaxGapBridge=%d\n",par.MaxGapBridge);
		wmLog(eDebug, "SinglevalueThreshold=%d\n",par.SinglevalueThreshold);
		wmLog(eDebug, "HzProjektionMinDeltay=%d\n",par.HzProjektionMinDeltay);
		wmLog(eDebug, "HzProjektionMinimalMax=%d\n",par.HzProjektionMinimalMax);
		wmLog(eDebug, "VProjektionThreshold=%d\n\n",par.VProjektionThreshold);
		wmLog(eDebug, "sgmima.npixx=%d\n\n",sgmima.npixx);
		wmLog(eDebug, "sgmima.npixy=%d\n\n",sgmima.npixy);
		wmLog(eDebug, "sgmima.pitch=%d\n\n",sgmima.pitch);
		wmLog(eDebug, "sgmima.roix0=%d\n\n",sgmima.roix0);
		wmLog(eDebug, "sgmima.roiy0=%d\n\n",sgmima.roiy0);
		wmLog(eDebug, "sgmima.roidx=%d\n\n",sgmima.roidx);
		wmLog(eDebug, "sgmima.roidy=%d\n\n",sgmima.roidy);
		//save_bmp_bw("testi.bmp",sgmima);
	}


	int ret=0;
	gapposx=-1.0;
	gapposy=-1.0;
	gapposrank=0.0;


	ret=BondPosCalculation(gapposx,gapposy,gapposrank);
	if(ret!=0) gapposrank=0.0;


	if(m_oVerbosity >= eMedium)
	{
		wmLog(eDebug, "GapPositionProjection::proceed  ->   x=%g   y=%g   rank=%g\n",gapposx,gapposy,gapposrank);
	}

	// Resultat eintragen:

	m_oGapPosition = Point(static_cast<int>(gapposx),static_cast<int>(gapposy));


	// Resultat eintragen:

	const GeoPoint			point		( rFrame.context(), m_oGapPosition, AnalysisOK, gapposrank );
	const int				oRank		( doubleToIntRank(point.rank()) );
	const GeoDoublearray	oGeoPosXOut	( point.context(), Doublearray(1, point.ref().x, oRank), point.analysisResult(), point.rank() );
	const GeoDoublearray	oGeoPosYOut	( point.context(), Doublearray(1, point.ref().y, oRank), point.analysisResult(), point.rank() );

	preSignalAction();
	pipeOutPosX_->signal(oGeoPosXOut);
	pipeOutPosY_->signal(oGeoPosYOut);

	if(m_oVerbosity >= eMedium)
	{
		wmLog(eDebug, "GapPositionProjection::proceed  -> Position: %i, %i Rank: %g", m_oGapPosition.x, m_oGapPosition.y, gapposrank);
	}


}



int GapPositionProjection::BondPosCalculation(double & x,double & y,double & rank)
{
	x=0.0;
	y=0.0;
	rank=-1.0;
	int ret;
	int projend1=0;
	int projend2=0;
	int xug, xog, yug, yog,seppos,Xseppos;

	xug=sgmima.roix0;
	xog=sgmima.roix0+sgmima.roidx;
	yug=sgmima.roiy0;
	yog=sgmima.roiy0+sgmima.roidy;

	HorizontalProjection(sgmima,xug, xog, yug, yog,par.SinglevalueThreshold,proj1,projend1);

	//DrawHorizontalprojection(proj1,projend1,xug,yug,100);

	ret=GetSeparationBetweenTwoMaxima(proj1,projend1,seppos);
	if(m_oVerbosity >= eHigh)
	{
		wmLog(eDebug, "seppos=%d\n",seppos);
	}

	if(ret==0)
	{
		//	ovcross(100,seppos+yug,10,0x00ff00);
		;
	}
	else return ret;


	y=seppos+yug;

	xug=sgmima.roix0;
	xog=sgmima.roix0+sgmima.roidx;
	yug=sgmima.roiy0;
	yog=seppos+yug;
	VerticalProjection(sgmima,xug, xog, yug, yog,par.SinglevalueThreshold,proj1,projend1);
	//DrawVerticalprojection(proj1,projend1,xug,yug,20);


	xug=sgmima.roix0;
	xog=sgmima.roix0+sgmima.roidx;
	yug=seppos+yug;
	yog=sgmima.roiy0+sgmima.roidy;
	VerticalProjection(sgmima,xug, xog, yug, yog,par.SinglevalueThreshold,proj2,projend2);
	//DrawVerticalprojection(proj2,projend2,xug,yug,20);

	ret=FindHorizontalSeparation(proj1,projend1,proj2,projend2,Xseppos);
	if(m_oVerbosity >= eHigh)
	{
		wmLog(eDebug, "Xseppos=%d\n",Xseppos);
	}
	if(ret==0)
	{
		//x=sgmima.roix0+Xseppos;
		x=Xseppos;
		rank=1.0;
		return 0;
	}
	else
	{
		x=0;
		return 1;
	}


	return 0;
}



void GapPositionProjection::HorizontalProjection(SSF_SF_InputStruct & img,int xug,int xog,int yug,int yog,int threshold,double *proj,int & projend)
{
	int x,y,i,w;

	projend=yog-yug;

	for(i=0,y=yug;y<yog;++y,++i)
	{
		proj[i]=0.0;
		for(x=xug;x<xog;++x)
		{
			w=*(img.img+img.pitch*y+x);
			if(w>=threshold) proj[i]+=w;
		}
	}

}





void GapPositionProjection::DrawHorizontalprojection(double * proj,int projend,int xug,int yug,int displaywidth)
{
	int UNUSED i,x,y;
	double pmin,pmax,scale;

	pmin=0.0;
	pmax=0.0;
	for(i=0;i<projend;++i)
	{
		if(proj[i]>pmax) pmax=proj[i];
		if(proj[i]<pmin) pmin=proj[i];
	}

	if(pmax-pmin>10.0)
	{
		scale=double(displaywidth)/(pmax-pmin);
	}
	else
	{
		scale=0.0;
	}

	for(i=0;i<projend;++i)
	{
		x=xug+ int(0.5+scale*proj[i]);
		y=i+yug;
		//		ovpoint(x,y,markcolor_);
	}

}



void GapPositionProjection::VerticalProjection(SSF_SF_InputStruct & img,int xug,int xog,int yug,int yog,int threshold,double *proj,int & projend)
{
	int x,y,i,w;

	projend=xog-xug;


	//wmLog(eDebug, "VerticalProjection  xug=%d  xog=%d yug=%d yog=%d threshold=%d projend=%d\n", xug, xog, yug, yog, threshold, projend);

	for(i=0,x=xug;x<xog;++x,++i)
	{
		proj[i]=0.0;
		for(y=yug;y<yog;++y)
		{
			w=*(img.img+img.pitch*y+x);
			if(w>=threshold) proj[i]+=w;
		}
		//wmLog(eDebug, "%d %g \n",i,proj[i]);
	}

	//wmLog(eDebug, "\n", xug, xog, yug, yog, threshold, projend);

}





void GapPositionProjection::DrawVerticalprojection(double * proj,int projend,int xug,int yug,int displayheight)
{
	int UNUSED i,x,y;
	double pmin,pmax,scale;

	pmin=0.0;
	pmax=0.0;
	for(i=0;i<projend;++i)
	{
		if(proj[i]>pmax) pmax=proj[i];
		if(proj[i]<pmin) pmin=proj[i];
	}

	if(pmax-pmin>10.0)
	{
		scale=double(displayheight)/(pmax-pmin);
	}
	else
	{
		scale=0.0;
	}

	for(i=0;i<projend;++i)
	{
		y=yug+ int(0.5+scale*proj[i]);
		x=i+xug;
		// ovpoint(x,y,markcolor_);
	}

}


int GapPositionProjection::GetSeparationBetweenTwoMaxima(double * proj,int projend,int & separationpos)
{
	int i,ret,w;
	//int deltaimin=10; par.HzProjektionMinDeltay=10
	int max1,imax1,max2,imax2,state;
	ret=0;


	state=0;
	imax1=imax2=0;
	max1=max2=0;
	for(i=0;i<projend;++i)
	{
		w=(int)proj[i]; // BAUSTELLE: fuer Projektion reicht long ; double rausschmeissen

		switch(state)
		{
		case 0:
			//erstes Maximum noch nicht gefunden
			if(w>max1)
			{
				max1=w;
				imax1=i;
			}
			if(i>imax1+par.HzProjektionMinDeltay && max1>par.HzProjektionMinimalMax) state=1; //par.HzProjektionMinimalMax=255;
			break;

		case 1:
			//zweites Maximum noch nicht gefunden
			if(w>max2)
			{
				max2=w;
				imax2=i;
			}
			if(i>imax1+par.HzProjektionMinDeltay  && max2>par.HzProjektionMinimalMax) state=2;
			break;

		default :
			goto endloop;

		}
	}
endloop: ;

	if(m_oVerbosity >= eMedium)
	{
		wmLog(eDebug, "> %d %d\n",max1,imax1);
		wmLog(eDebug, "> %d %d\n",max2,imax2);
	}

	if(max1==0 || max2==0) return 1;

	separationpos=(imax1+imax2)/2;
	return ret;
}




int  GapPositionProjection::FindHorizontalSeparation(double *proj1,int projend1,double *proj2,int projend2,int & Xseppos)
{
	int xL1,xR1,xL2,xR2;
	Xseppos=0;

	//threshold=255;
	//Find_XL_XR_1(proj1,projend1,threshold,xL1,xR1);
	//Find_XL_XR_1(proj2,projend2,threshold,xL2,xR2);


	Find_XL_XR_2(proj1,projend1,par.VProjektionThreshold,xL1,xR1); //par.VProjektionThreshold=255;
	Find_XL_XR_2(proj2,projend2,par.VProjektionThreshold,xL2,xR2);
	if(m_oVerbosity >= eMedium)
	{
		wmLog(eDebug, "xL1=%d  xR1=%d  xL2=%d  xR2=%d\n",xL1,xR1,xL2,xR2);
	}
	// jetzt: bestimme ob xL1 naeher an xR2   oder xR1 naeher an xL2

	if(std::abs(xL1-xR2) < std::abs(xR1-xL2))
	{
		Xseppos=(xL1+xR2)/2;
	}
	else
	{
		Xseppos=(xR1+xL2)/2;
	}

	return 0;
}



int GapPositionProjection::Find_XL_XR_1(double *proj,int projend,int threshold,int & xL, int & xR)
{
	int i;
	int w;
	xL=-1;
	for(i=0;i<projend;++i)
	{
		w=(int) proj[i]; // BAUSTELLE: fuer Projektion reicht long ; double rausschmeissen
		if(w>threshold)
		{
			if(xL<0) xL=i;
			xR=i;
		}
	}

	return 0;
}



int GapPositionProjection::Find_XL_XR_2(double *proj,int projend,int threshold,int & xL, int & xR)
{
	int i			= 0;
	int ret			= 0;
	int	maxspotlen	= 0;
	int ibestspot	= 0;

	PeakSectionT Peaksect[1000];
	int PeaksectAnz=0;

	ret=CalculateLineSections(proj,projend,threshold,Peaksect,PeaksectAnz);
	if(ret!=0) return ret;

	maxspotlen=-1;
	for(i=0;i<PeaksectAnz;++i)
	{
		if(Peaksect[i].onspotlength>maxspotlen)
		{
			maxspotlen=Peaksect[i].onspotlength;
			ibestspot=i;
		}
	}

	if(maxspotlen<0)
	{
		xL=0;
		xR=0;
		return 1;
	}

	xL=Peaksect[ibestspot].xL;
	xR=Peaksect[ibestspot].xR;

	return 0;
}



int GapPositionProjection::CalculateLineSections(double *linearray,int npixx,int threshold, PeakSectionT * Peaksect, int & PeaksectAnz)
{
	int i,x1,x2,spotlength,gapsize,onspotlength;
	bool onspot,ongap;
	double UNUSED MaxW, spotMaxW;
	double w,spotSumW;

	PeaksectAnz=0;
	onspotlength=gapsize=0;
	spotSumW=spotMaxW=0.0;
	x1=x2=0;

	onspot=ongap=false;

	MaxW=0.0;
	for(i=0;i<npixx;++i)
	{
		w=linearray[i];
		if(w<0.0) break;

		if(w>=threshold)
		{
			if(onspot)
			{
				//Spot weiter auffuellen
				if (m_oVerbosity >= eMax)
				{
					wmLog(eDebug, "Spot weiter auffuellen: i=%d w=%g \n",i,w);
				}

				x2=i;
				if(w>spotMaxW) spotMaxW=w;
				spotSumW+=w;
				onspotlength++;
				continue;
			}
			else
			{
				if(!ongap)
				{
					//start a new spot
					if (m_oVerbosity >= eHigh)
					{
						wmLog(eDebug, "start a new spot: i=%d w=%g \n",i,w);
					}
					x1=x2=i;
					spotMaxW=w;
					spotSumW=w;
					onspotlength=1;
				}
				else
				{
					//nach ueberbruecktem Gap: Spot weiter auffuellen
					if (m_oVerbosity >= eHigh)
					{
						wmLog(eDebug, "bridged gap fill spot: i=%d w=%g \n",i,w);
					}
					x2=i;
					if(w>spotMaxW) spotMaxW=w;
					spotSumW+=w;
					onspotlength++;
				}
				onspot=true;
				ongap=false;
				continue;
			}
		}
		else
		{ // w<threshold
			if(onspot)
			{

				if (m_oVerbosity >= eHigh)
				{
					wmLog(eDebug, "off spot: i=%d w=%g \n",i,w);
				}

				onspot=false;
				ongap=true;
				gapsize=1;
				continue;
			}
			else
			{
				if(!ongap) continue;
				// so we are ongap:
				++gapsize;

				if (m_oVerbosity >= eMax)
				{
					wmLog(eDebug, "off spot on gap: i=%d w=%g gapsize=%d onspotlength=%d\n",i,w,gapsize,onspotlength);
				}

				spotlength=1+x2-x1;
				if(gapsize<par.MaxGapBridge) continue; //bridge a normal gap

				//spot fertig
				ongap=false;

				if (m_oVerbosity >= eMedium)
				{
					wmLog(eDebug, "spot %d fertig: i=%d x1=%d x2=%d    spotlength=%d  spotMaxW=%g  spotSumW=%g\n",PeaksectAnz+1,i,x1,x2,spotlength,spotMaxW,spotSumW);
				}

				Peaksect[PeaksectAnz].xL=x1;
				Peaksect[PeaksectAnz].xR=x2;
				Peaksect[PeaksectAnz].spotMaxW=(float)spotMaxW;  // BAUSTELLE: fuer Projektion reicht long ; double rausschmeissen
				Peaksect[PeaksectAnz].onspotlength=onspotlength;
				Peaksect[PeaksectAnz].spotlength=spotlength;
				Peaksect[PeaksectAnz].spotSumW=(float)spotSumW; // BAUSTELLE: fuer Projektion reicht long ; double rausschmeissen
				PeaksectAnz++;
			}
		}
	}//end for i

	if(ongap || onspot)
	{
		Peaksect[PeaksectAnz].xL=x1;
		Peaksect[PeaksectAnz].xR=x2;
		Peaksect[PeaksectAnz].spotMaxW=(float)spotMaxW; // BAUSTELLE: fuer Projektion reicht long ; double rausschmeissen
		Peaksect[PeaksectAnz].onspotlength=onspotlength;
		spotlength=1+x2-x1;
		Peaksect[PeaksectAnz].spotlength=spotlength;
		Peaksect[PeaksectAnz].spotSumW=(float)spotSumW; // BAUSTELLE: fuer Projektion reicht long ; double rausschmeissen
		PeaksectAnz++;
	}

	if(PeaksectAnz==0)
	{
		return 1;
	}

	return 0;
}





	}} //Namespaces
