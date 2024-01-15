/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief 		This filter calculates the displacement of two blanks at the beginning and at the end.
*/

#ifndef EDGESKEW_H_
#define EDGESKEW_H_

#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <common/frame.h>				///< ImageFrame
#include <image/image.h>				///< BImage
#include <geo/geo.h>					///< Size2d, Intarray
#include <geo/array.h>					///< ByteArray

namespace precitec {
	namespace filter {

#define CHECK_START_AND_THROW 0

		struct paramBlockDef;
		struct internalResultsDef;
		class CImageInfo;
		class Project;
		class ROI;

#define STATUS_UNBEKANNT      0
#define STATUS_SCHWARZ_ANFANG 1
#define STATUS_KANTE_ANFANG   2
#define STATUS_ROHR           3
#define STATUS_KANTE_ENDE     4
#define STATUS_SCHWARZ_ENDE   5
#define STATUS_SCHWARZ        6

#define ABLAUF_OK             0
#define ABLAUF_WRN_BLACK      1
#define ABLAUF_WRN_NIX_ANFANG 2
#define ABLAUF_WRN_NIX_ENDE   3
#define ABLAUF_ERR_NOK        4

#define ORT_EGAL   0
#define ORT_VORNE  1
#define ORT_HINTEN 2

		//const int INT_HEIGHT_BEGIN_END_STRIPES = 50;
		//const int INT_GRAD_THRESH_DEFAULT = 40;
		//const int INT_HEIGHT_SEARCH_NOTCH = 80;

		//class PointContainer
		//{
		//public:
		//	PointContainer();
		//	void reset();
		//	void addPoint(int x, int y);
		//	int getPointX(int nr);
		//	int getPointY(int nr);
		//	int size();
		//private:
		//	std::vector<precitec::geo2d::Point> points;
		//};
		//
		//class Line
		//{
		//	public:
		//	  Line(double m, double b);
		//	  void draw(int xStart, int yStart, int xEnd, int yEnd, int color);
		//	  double getY(double x);
		//
		//	  double m_, b_;
		//};
		//
		//
		//
		//class ProjXStripe
		//{
		//	public:
		//		ProjXStripe();
		//		virtual ~ProjXStripe();
		//		void reset();
		//		//void draw(unsigned char * pImage, int iResX, int iResY, int c=0);
		//		int getMedian(int anz, int pInt[]);
		//		int getMedian(std::vector<int> intVec);
		//		void copyVal2Tmp();
		//		void copyTmp2Val();
		//		void smoothValues(int sizeMedian);
		//		int getMin();
		//		int getMax();
		//		int countChanges();
		//
		//		int iValue_[10];
		//		std::vector<int> _values;
		//		int iValTmp_[10];
		//		std::vector<int> _tempValues;
		//
		//		int iStart_, iEnd_;
		//		int x1_, x2_, y1_, y2_;
		//};
		//
		//class ProjAnalyser
		//{
		//	public:
		//	  ProjAnalyser(precitec::image::BImage image);
		//	  virtual ~ProjAnalyser();
		//	  int getBoxMean(int x1, int y1, int x2, int y2);
		//	  void start();
		//	  bool isBad();
		//
		//	private:
		//	  precitec::image::BImage _image;
		//	  int iResX_, iResY_;
		//	  ProjXStripe Stripe[10];
		//	  ProjXStripe TotalStripe;
		//	  bool isBad_;
		//};
		//
		//class DiagStripe
		//{
		//  public:
		//    DiagStripe();
		//	virtual ~DiagStripe();
		//	void setImage(precitec::image::BImage image, bool direction);
		//	void drawLine(int color);
		//	int getPixel(int x, int y);
		//	void setData(double m, double pointX, double pointY);
		//	void setData(double m, double n);
		//	void checkValid();
		//	void calcValues();
		//	void getValues();
		//	int getLineY(int x);
		//
		//	bool isTubeStripe;
		//	bool isValid_;
		//	//paramBlockDef * pPar_;
		//	bool bDirection_;
		//
		//
		////	private:
		//	precitec::image::BImage pImage_;
		//	int ResX_;
		//	int ResY_;
		//	double Steigung_;
		//	double AchsAbschnitt_;
		//	int mean_, min_, max_;
		//	int VRohr_, VBG_, HRohr_, HBG_;
		//	int i;
		//
		//	// Konstanten (setzt Constructor)
		//	int stripeheight;
		//	int speederX;
		//	int speederY;
		//
		//};
		//
		//struct SBlockInfo
		//{
		//	public:
		//	  SBlockInfo();
		//	  virtual ~SBlockInfo();
		//	  int startX, startY, sizeX, sizeY;
		//	  int mean;
		//	  int min, max;
		//	  bool NoTubeBlock;
		//
		//};
		//
		//class SoutubeResult1
		//{
		//	public:
		//		SoutubeResult1();
		//		void reset();
		//		
		//		bool ResultValid_;
		//		int BildStatus_;
		//		int BildStatusOrg_;
		//		int iLastStripeOnTube_;
		//
		//		geo2d::Range InspectionRange; // Ergebnis von wo bis wo auszuwerten ist
		//};
		//
		//
		//class SoutubeResult2
		//{
		//	public:
		//		SoutubeResult2();
		//		void reset();
		//		void copySR1(SoutubeResult1 SR1);
		//		
		//		bool ResultValid_;
		//		int BildStatus_;
		//		int iLastStripeOnTube_;
		//
		//		geo2d::Range InspectionRange; // Ergebnis von wo bis wo auszuwerten ist
		//		
		//		int linksNaht_;
		//		int rechtsNaht_;
		//		int notchSize_;
		//		int edgeSkew_;
		//		// Status = 0: weder Anfang noch Ende
		//		// Status = 1: Rohr-Anfang
		//		// Status = 2: Rohr-Ende
		//		int iRohrStatus_;
		//};
		//
		//

		class DisplacementPoint
		{
		public:
			DisplacementPoint(int inX, int inY);
			int x;
			int y;
		};

		class FILTER_API EdgeSkew : public fliplib::TransformFilter
		{

		public:

			EdgeSkew();
			virtual ~EdgeSkew();

			static const std::string m_oFilterName;				///< Name Filter
			static const std::string PIPENAME_DISPLACEMENT_OUT;		///< Name Out-Pipe

			/// Set filter parameters as defined in database / xml file.
			void setParameter();
			/// paints overerlay primitives
			void paint();

		protected:

			/// In-pipe registration.
			bool subscribe(fliplib::BasePipe& pipe, int group);
			/// In-pipe event processing.
			void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

			std::vector<DisplacementPoint> calcEdgeMeanTopLeft(int minY, int maxY, int width, int resolution);
			std::vector<DisplacementPoint> calcEdgeMeanTopRight(int minY, int maxY, int width, int resolution);
			std::vector<DisplacementPoint> calcEdgeMeanBottomLeft(int minY, int maxY, int width, int resolution);
			std::vector<DisplacementPoint> calcEdgeMeanBottomRight(int minY, int maxY, int width, int resolution);

			int getMeanInArea(int x1, int y1, int x2, int y2);

			void killSmallest(std::vector<DisplacementPoint> & points);
			void killBiggest(std::vector<DisplacementPoint> & points);
			int getMean(std::vector<DisplacementPoint> & points);

		private:

			const fliplib::SynchronePipe< interface::ImageFrame >        * m_pPipeInImageFrame;		///< In pipe
			const fliplib::SynchronePipe< interface::GeoStartEndInfoarray > * m_pPipeInStartEndInfo;	///< In pipe

			fliplib::SynchronePipe< interface::GeoDoublearray >			 * m_pPipeOutDisplacement;		///< Out pipe

			interface::SmpTrafo											m_oSpTrafo;				///< roi translation

			int m_oWidth;
			int m_oResolution;

			int m_threshMaterial;
			int m_threshBackground;

			int meanLeft_, meanRight_;

			//geo2d::VecDoublearray m_oLaserLineOut;
			geo2d::Doublearray m_oDisplacementOut;
			//geo2d::Doublearray m_oDouble2Out;		
			//geo2d::Doublearray m_oDouble3Out;	

			std::vector<DisplacementPoint> pointsLeft, pointsRight;

			bool m_hasPainting;
			precitec::image::BImage m_image;
			int imageWidth, imageHeight;
		};

	} // namespace precitec
} // namespace filter

#endif /* EDGESKEW_H_ */
