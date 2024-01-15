/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS, CB
* 	@date		2016
* 	@brief 		This filter finds the seam texture based
*/

#ifndef TEXTUREBASED_H_
#define TEXTUREBASED_H_

#include <fliplib/Fliplib.h>			///< Fliplib image processing pipes and filter framework
#include <fliplib/TransformFilter.h>	///< Basisclass
#include <fliplib/PipeEventArgs.h>		///< Eventprocessing
#include <fliplib/SynchronePipe.h>		///< Inputs and outputs

#include <common/frame.h>				///< ImageFrame
#include <image/image.h>				///< BImage
#include <geo/geo.h>					///< Size2d, Intarray
#include <geo/array.h>					///< ByteArray
#define _USE_MATH_DEFINES
#include <math.h>


// to do
#define SFTB_NPIXX_MAX 1024
#define SFTB_NPIXY_MAX 1024



namespace precitec {
	using namespace image;
	using namespace geo2d;

	namespace filter {

		struct SFTB_ParametersT
		{
		public:
			//wie bei allen anderen Nahtsuchmethoden:
			int	priority;

			//Wie bei allen anderen Nahtsuchmethoden: Einfaches Zeug in viele Funktionen verteilt: sehr schoen ...
			int	getPriority(void)const
			{
				return priority;
			}

			bool	testPriorityOn()const
			{
				return (priority>0);
			}

			bool	testPriorityOff()const
			{
				return (priority == 0);
			}

			bool	testPriorityEqual(int iPriority)const
			{
				return (iPriority == priority);
			}
			int		getWidthDefault()const
			{
				return defaultWidth;
			}

			int defaultWidth;
			int nblines;

			// von der SFTB methode verwendet:
			int ImfWantedThreshold;
			int ImfWantedSinglevalueThreshold;
			int GridStepsizeX;
			int GridStepsizeY;
			int MaxGapBridge;
			int	MinSeamWidth;
			int RF_CalculatorArrayWantedSlot;
			int RF_Lx1;
			int RF_Ly1;
			int RF_Lx2;
			int RF_Ly2;


		};



		class FILTER_API SeamFindTextureBased : public fliplib::TransformFilter
		{

		public:

			SeamFindTextureBased();
			virtual ~SeamFindTextureBased();

			static const std::string m_oFilterName;		///< Name Filter
			static const std::string PIPENAME_SEAMPOS_OUT;		///< Name Out-Pipe
			static const std::string PIPENAME_SEAMLEFT_OUT;		///< Name Out-Pipe
			static const std::string PIPENAME_SEAMRIGHT_OUT;		///< Name Out-Pipe
			static const std::string PIPENAME_SEAMFINDINGS_OUT;		///< Name Out-Pipe

			/// Set filter parameters as defined in database / xml file.
			void setParameter();
			/// paints overerlay primitives
			void paint();

			//SFTB_ParametersT *param;
			
		protected:

			/// In-pipe registration.
			bool subscribe(fliplib::BasePipe& pipe, int group);
			/// In-pipe event processing.
			void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

			void searchSeamTextureBased(const byte *bImageNew, const Size2d size,
				geo2d::Doublearray & p_rSeamPosOut, geo2d::Doublearray & p_rSeamLeftOut, geo2d::Doublearray & p_rSeamRightOut, geo2d::SeamFindingarray & seamFindingArrayOut);

			int setRF_CalcSelect(SFTB_ParametersT & par);

		private:

			const fliplib::SynchronePipe< interface::ImageFrame >          * m_pPipeInImageFrame;	///< In pipe
			const fliplib::SynchronePipe< interface::GeoSeamFindingarray > * m_pPipeInSeamFindings;	///< In pipe

			fliplib::SynchronePipe< interface::GeoDoublearray >       * m_pPipeOutSeamPos;		///< Out pipe
			fliplib::SynchronePipe< interface::GeoDoublearray >       * m_pPipeOutSeamLeft;		///< Out pipe
			fliplib::SynchronePipe< interface::GeoDoublearray >       * m_pPipeOutSeamRight;	///< Out pipe
			fliplib::SynchronePipe< interface::GeoSeamFindingarray >  * m_pPipeOutSeamFindings;	///< Out pipe

			interface::SmpTrafo											m_oSpTrafo;				///< roi translation

			int 														m_oThreshProj;
			int 														m_oThreshSingleVal;
			int 														m_oSubX;
			int 														m_oSubY;
			int 														m_oMaxSeamCenterWidth;
			int 														m_oMinSeamWidth;
			int 														m_oLx1;
			int 														m_oLy1;
			int 														m_oLx2;
			int 														m_oLy2;

			geo2d::Doublearray										    m_oSeamPosOut;			///< Output profile
			geo2d::Doublearray										    m_oSeamLeftOut;			///< Output profile
			geo2d::Doublearray										    m_oSeamRightOut;			///< Output profile
			geo2d::SeamFindingarray										m_oSeamFindingsOut;			///< Output profile

			int m_resultSeamLeft, m_resultSeamRight, m_resultSeamPosX, m_resultSeamPosY;
		
		};


		class SFTB_roiT
		{
		public:
			//unsigned char*	start;		///< Startadresse des Rois
			const byte		*start;	    ///< Startadresse des Rois
			unsigned int	pitch;		///< Zeilensprungzeiger
			//groesse des Bereichs:
			int				dx;
			int				dy;
		};

		class SeamBoundaryMeasurementT
		{
		public:
			int xL;
			int xR;
			int y;
			int errorflag;
			int rank; //Souvis Qualitaetsbewertung 0..255
			double rankL; // Qualitaetsbewertung 0.0 ... 1.0
			double rankR; // Qualitaetsbewertung 0.0 ... 1.0
		};

		struct MFLineDistributionParameterT
		{
			float GridMaxW;
			float GridAveW;
			float LineMaxW;
			float LineAveW;
			//float LineQAveW;

		};

		struct MFPeakSectionT
		{
			unsigned short xL;
			unsigned short xR;
			float spotMaxW;
			float spotSumW;
			unsigned short onspotlength;
			unsigned short spotlength;

		};

		class SFTB_RF_CalculatorBaseclassT
		{
		public:

			int Lx1;
			int Ly1;
			int Lx2;
			int Ly2;
			int templatesizeX;
			int templatesizeY;

			virtual void f(SFTB_roiT & roi, float &rf1, float &rf2) = 0;
			virtual  ~SFTB_RF_CalculatorBaseclassT();
			virtual int setCoefficients(int &inLx1, int &inLy1, int &inLx2, int &inLy2);

		};


		class SFTB_SeamFinderTextureBasedT
		{
		public:

			float linearray1[SFTB_NPIXX_MAX];
			float linearray2[SFTB_NPIXX_MAX];
			float MeritValueArray1[SFTB_NPIXX_MAX*SFTB_NPIXY_MAX];
			float MeritValueArray2[SFTB_NPIXX_MAX*SFTB_NPIXY_MAX];

			int NumberOfMeasurements;
			SeamBoundaryMeasurementT MeasurementArray[20];

			MFLineDistributionParameterT MF1LineDistributionParameter;
			MFLineDistributionParameterT MF2LineDistributionParameter;
			MFPeakSectionT spots1[500];
			MFPeakSectionT spots2[500];

			SFTB_SeamFinderTextureBasedT();

			void GetSeamPositions(SFTB_roiT & roi);

			SFTB_RF_CalculatorBaseclassT * RF_CalculatorArray[30];
			SFTB_RF_CalculatorBaseclassT * RF_Calc; //aus RF_CalculatorArray[param->RF_CalculatorArrayWantedSlot] wird hier eingetragen
			int RF_CalculatorArrayCurrentSlot; //derz. verwendeter Slot, stimmt der nicht mit param->RF_CalculatorArrayWantedSlot ueberein so muss RF_Calc neu gesetzt werden


			void printparams(void);

			//int init(void);
			//int setParameters(SFTB_ParametersT & par);
//			int setDefaultParameters(void);

			//int setRF_Calc(int index);
			//int setRF_CalcSelect(SFTB_ParametersT & par);
			//int RF_CalcNumberOfSlots;
			//******************************************************************
		private:


			// Threshold fuer Meritfunction (Einzel-)Wert:  threshold = average * fUsedmfSinglevalueThreshold
			//aus Parametern:  int iWantedmfSinglevalueThreshold;
			float fUsedmfSinglevalueThreshold;

			// Threshold fuer summierte Meritfunction:  threshold = maximalwert * fUsedmfThreshold + average *(1-fUsedmfThreshold)
			//aus Parametern:   int iWantedmfThreshold;
			float fUsedmfThreshold; //


			//float thresholds aus parametern int thresholds berechnen
			void  CalculateThresholds(int iWantedmfSinglevalueThreshold, int iWantedmfThreshold);

			//Raumfrequenzwerte auf Grid (grobes Subsampling des Bildes) berechnen
			void  CalculateGridValues(SFTB_roiT & roi, signed short & ycenter);

			//Werte des Grid vertikal aufsummieren -> Wete einer horizontalen Zeile
			void CalculateLineValues(SFTB_roiT & roi);

			//Zeile in Abschnitte unterteilen:
			int CalculateLineSections(float *linearray, int npixx, MFLineDistributionParameterT & MFLDP, MFPeakSectionT * MFPeaksect, int & MFPeaksectAnz);

			//Zeilenabschitte zu Nahtrandpositionen verarbeiten
			int LineSectionEvaluation(MFPeakSectionT * MFPeaksect1, int & MFPeaksectAnz1, MFPeakSectionT * MFPeaksect2, int & MFPeaksectAnz2, int & xLraw, int & xRraw, unsigned char & quality);

		};

		//extern SFTB_SeamFinderTextureBasedT SFTB_SeamFinderTextureBased;

		class SFTB_RF_CalculatorT : public SFTB_RF_CalculatorBaseclassT
		{
		public:
			SFTB_RF_CalculatorT();
			~SFTB_RF_CalculatorT();

			virtual int setCoefficients(int &inLx1, int &inLy1, int &inLx2, int &inLy2);

			void RaumfrequenzVorbelegen(signed int Lx, signed int Ly, signed char *kernel);
			void RaumfrequenzVorbelegen(signed int inpLx1, signed int inpLy1, signed int inpLx2, signed int inpLy2);
			void RaumfrequenzFileoutput(char *fnpf, signed char *kernel);

			void f(SFTB_roiT & roi, float &rf1, float &rf2);

			signed char *kern1; //rf1: zwei kernel fuer sin und cos
			signed char *kern2; //rf2: zwei kernel fuer sin und cos


		};

		//extern SFTB_RF_CalculatorT  SFTB_RF_Calculator;

		class SFTB_SeamQuality_ParametersT
		{
		public:

			int debuglevel;

		};


		class SFTB_SeamQualityT
		{
		public:

			SFTB_SeamQualityT();

			SFTB_SeamQuality_ParametersT sd;
			SFTB_SeamQuality_ParametersT *pp;


			bool SeamQualityValidFlag;
			float SumMeritFunctionOnLeftSeam;
			float SumMeritFunctionOnRightSeam;
			float MaxSumMeritFunction;
			float MinSumMeritFunction;

			int   SumMeritFunctionNvaluesYdir;
			int SumMeritFunctionXSubsampling;
			double SeamWidth;
			double SeamCenter;
			int WidthLeftSeam;
			int WidthRightSeam;
			int TimeSeriesRankCenter;
			int TimeSeriesRankWidth;

			int calculateSeamQuality(void);

		};


		class SFTB_RF_CalculatorLx1M2_Ly1M1_Lx2P2_Ly2M1T : public SFTB_RF_CalculatorBaseclassT
		{
		public:
			SFTB_RF_CalculatorLx1M2_Ly1M1_Lx2P2_Ly2M1T();
			void f(SFTB_roiT & roi, float &rf1, float &rf2);

		};


		class SFTB_RF_CalculatorLx1M4_Ly1M4_Lx2P4_Ly2M4T : public SFTB_RF_CalculatorBaseclassT
		{
		public:
			SFTB_RF_CalculatorLx1M4_Ly1M4_Lx2P4_Ly2M4T();
			void f(SFTB_roiT & roi, float &rf1, float &rf2);

		};


		class SFTB_RF_CalculatorLx1M4_Ly1P4_Lx2P4_Ly2P4T : public SFTB_RF_CalculatorBaseclassT
		{
		public:
			SFTB_RF_CalculatorLx1M4_Ly1P4_Lx2P4_Ly2P4T();
			void f(SFTB_roiT & roi, float &rf1, float &rf2);

		};


	} // namespace precitec
} // namespace filter

#endif /* TEXTUREBASED_H_ */
