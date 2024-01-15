/***
*	@file
*	@copyright		Precitec Vision GmbH & Co. KG
*	@author			LB
*	@date			2017
*	@brief			Fliplib filter 'Cross-correlation' in component 'Filter_ImageProcessing'. Computes the cross-correlation of an image with a pattern
*/


#ifndef CrossCorrelation_H_
#define CrossCorrelation_H_

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"

#include "sparseKernel.h"
#include "common/frame.h"

namespace precitec {
	using namespace image;
	using namespace interface;
	namespace filter {


		/**
		* Cross-correlation of an image with a pattern. The pattern can be provided as an input or generated
		*/
		class FILTER_API CrossCorrelation : public fliplib::TransformFilter
		{
		public:
			CrossCorrelation();

			static const std::string m_oFilterName;   ///< Name Filter
			static const std::string m_oPipeOutImage; ///< Name Out-Pipe
			static const std::string m_oPipeOutTemplate; ///< Name Out-Pipe

		protected:
			/// Set filter parameters as defined in database / xml file.
			void setParameter();
			/// in pipe registration
			bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
			/// In-pipe event processing.
			void proceed(const void* sender, fliplib::PipeEventArgs & e);
			/// paints overerlay primitives
			void paint();


		private:
            class TemplateFromFileInfo
            {
            public:
                bool update(int source, int templateNumber);
                bool needsToResampleTemplate(double samplingX, double samplingY) const;
                const BImage & getTemplate(double samplingX, double samplingY);
                const SparseKernel & getSparseKernel(double samplingX, double samplingY);
            private:
                void updateTemplates(double samplingX, double samplingY);
                int m_source = -1;
                int m_templateNumber = -1;
                BImage m_originalTemplate;
                BImage m_resampledTemplate;
                bool m_resampleImage;
                double m_lastUsedSamplingX;
                double m_lastUsedSamplingY;
                SparseKernel m_oSparseKernel;


            };

			const fliplib::SynchronePipe< ImageFrame >*		m_pPipeInImageFrame;	///< in pipe
			fliplib::SynchronePipe< ImageFrame >			m_oPipeImageFrame;		///< out pipe
			fliplib::SynchronePipe< ImageFrame >			m_oPipeTemplateFrame;	///< out pipe
			int												m_oSource;				///< template source 
			int												m_oMethod;				///< cross correlation method
			unsigned int									m_oTemplateNumber;		///< filename pattern of template image 
			unsigned int									m_oTemplateWidth;
			unsigned int									m_oTemplateHeight;
			double                                          m_oSigma;
			byte											m_oIntensityUpperLeftCorner;
			byte											m_oIntensityLowerRightCorner;
			unsigned int									m_oSquareLength;
			double											m_oKernelParameter1;	///< first parameter for kernel computation
			double											m_oKernelParameter2;    ///< second parameter for kernel computation
			bool											m_oFillOutputBorder;	///< flag for fill output border(such that output image has same dimension as input)
			BImage											m_oImageOut;		///< output image
			interface::SmpTrafo								m_oSpTrafo;			///< roi translation
			TemplateFromFileInfo							m_oTemplateFromFileInfo;  /// kernel without sampling
			DImage										    m_oKernel1;			///< normalized template (component 1)
			DImage										    m_oKernel2;			///< normalized template (component 2), size 0 if kernel could not separated
			BImage											m_oBKernel;			///< byte rescaled template (for out pipe and for comparing dimensions)
			TLineImage<int>                          m_oIntegralImage; ///< used for normalization
			double											m_oKernelRootSquaredSum; ///< cached sum of squares of template elements
			std::vector<std::string>						m_oResultInfo;
			double m_oRangeInputMinPerc;
			double m_oRangeInputMaxPerc;
			bool m_skipPaint;

			/**
			* @brief Update m_oKernel and m_oKernelRootSquaredSum
			*
			* @param pTemplateImage   Input template (byte image)
			* @param pNormalize   normalize the kernel (such that the sum of elements is 1)
			*/
			void updateKernel(const BImage & pTemplateImage, bool pNormalize);
			/**
			* @brief Update m_oKernel and m_oKernelRootSquaredSum
			*
			* @param pTemplateImage   Input template (double image)
			* @param pNormalizationFactor  precomputation normalization factor (such that the sum of the kernel elements is 1)
			*/
			void updateKernel(const DImage & pTemplateImage, double pNormalizationFactor);

			void updateKernel(const DImage & pTemplateHorizontalComponent, const DImage & pTemplateVerticalComponent, double pNormalizationFactor);


			/**
			* Compute the sum of squares of the kernel elements and precompute other helper variables
			*/
			void updateKernelVariables(const bool isSeparableKernel);

		};

	}
}

#endif /*CrossCorrelation_H_*/
