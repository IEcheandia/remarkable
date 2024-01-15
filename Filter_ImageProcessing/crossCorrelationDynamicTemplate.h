/***
*    @file
*    @copyright        Precitec Vision GmbH & Co. KG
*    @author           LB
*    @date             2018
*    @brief            Filter to compute the cross-correlation of an image with a pattern, both coming from input pipes
*/


#ifndef CROSSCORRELATIONDYNAMICTEMPLATE_H_
#define CROSSCORRELATIONDYNAMICTEMPLATE_H_

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"

#include "common/frame.h"

namespace precitec {
    using namespace image;
    using namespace interface;
    namespace filter {


        /**
        * Cross-correlation of an image with a pattern. The pattern can be provided as an input or generated
        */
        class FILTER_API CrossCorrelationDynamicTemplate : public fliplib::TransformFilter
        {
        public:
            CrossCorrelationDynamicTemplate();

            static const std::string m_oFilterName;   ///< Name Filter
            static const std::string m_oPipeOutImage; ///< Name Out-Pipe

        protected:
            /// Set filter parameters as defined in database / xml file.
            void setParameter();
            /// in pipe registration
            bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
            /// In-pipe event processing.
            void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);
            /// paints overerlay primitives
            void paint();


        private:
            const fliplib::SynchronePipe< ImageFrame >* m_pPipeInImageFrame;    ///< in pipe
            const fliplib::SynchronePipe< ImageFrame >* m_oPipeInTemplateFrame;    ///< in pipe
            fliplib::SynchronePipe< ImageFrame > m_oPipeImageFrame;        ///< out pipe
            int m_oMethod; ///< cross correlation method
            bool m_oFillOutputBorder;    ///< flag for fill output border(such that output image has same dimension as input)
            BImage m_oImageOut;        ///< output image
            interface::SmpTrafo m_oSpTrafo;            ///< roi translation
            DImage m_oKernel1;            ///< normalized template (component 1)
            DImage m_oKernel2;            ///< normalized template (component 2), size 0 if kernel could not separated
            geo2d::Size m_oKernelSize;            ///< byte rescaled template (for out pipe and for comparing dimensions)
            TLineImage<int>     m_oIntegralImage; ///< used for normalization
            double m_oKernelRootSquaredSum; ///< cached sum of squares of template elements
            std::vector<std::string> m_oResultInfo;
            double m_oRangeInputMinPerc;
            double m_oRangeInputMaxPerc;

            /**
            * @brief Update m_oKernel and m_oKernelRootSquaredSum
            *
            * @param pTemplateImage   Input template (byte image)
            * @param pNormalize   normalize the kernel (such that the sum of elements is 1)
            */
            void updateKernel(const BImage & pTemplateImage, bool pNormalize);
  
            /**
            * Compute the sum of squares of the kernel elements and precompute other helper variables
            */
            void updateKernelVariables();

        };

    }
}

#endif /*CROSSCORRELATIONDYNAMICTEMPLATE_H_*/
