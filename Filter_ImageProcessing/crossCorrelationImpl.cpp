/***
*	@file
*	@copyright		Precitec Vision GmbH & Co. KG
*	@author			LB
*	@date			2017
*	@brief			Performs operations useful for cross correlation (convolution, template computation)
*/

#include "crossCorrelationImpl.h"

#include "system/tools.h"	
#include "module/moduleLogger.h"
#include "overlay/overlayPrimitive.h"
#include "common/bitmap.h"


namespace precitec {
	using namespace image;
	using namespace geo2d;
namespace filter {
namespace crosscorrelation
		{
		void calcGaussianTemplate(const int radius, const double sigma, DImage & newTemplate, double & kernelSum){
			newTemplate.resizeFill(Size(2 * radius + 1, 2 * radius + 1), 0);
			kernelSum = 0;
				for (int i = 0; i < newTemplate.width(); ++i){
					for (int j = 0; j < newTemplate.height(); ++j){
					double x = radius - i;
					double y = radius - j;
					double value = exp(-((x*x) + (y*y)) / (2 * sigma*sigma));
					kernelSum += value;
					newTemplate.setValue(i, j, value);
				}
			}
		}// calcGaussianTemplate

		void calcGaussianTemplate(const int radius, const double sigma, DImage & rHorizontalComponent, DImage & rVerticalComponent, double & kernelSum){
			int size = 2 * radius + 1;
			rHorizontalComponent.resizeFill(Size(size, 1), 0);
			rVerticalComponent.resizeFill(Size(1, size), 0);
			double elementSum = 0;
			for (int i = 0;  i < size; ++i){
				double x = radius - i;
				double value = exp(-((x*x)) / (2 * sigma));
				rHorizontalComponent.setValue(i, 0, value);
				rVerticalComponent.setValue(0, i, value);
				elementSum += value;
			}
			kernelSum = elementSum * elementSum;
		}

        void calcSobelKernel(bool horizontal, DImage & rHorizontalComponent, DImage & rVerticalComponent, double & kernelSum)
        {
            int size = 3;
            rHorizontalComponent.resizeFill(Size(size, 1), 0);
            rVerticalComponent.resizeFill(Size(1, size), 0);
            assert(rHorizontalComponent.isContiguos());
            assert(rVerticalComponent.isContiguos());
            //TODO check other kernel size

            //3x3

            static const std::vector<double> differentiationKernel {1,0,-1};
            static const std::vector<double> averagingKernel{1,2,1};
            if (horizontal)
            {
                std::copy(differentiationKernel.begin(), differentiationKernel.end(), rHorizontalComponent.begin());
                std::copy(averagingKernel.begin(), averagingKernel.end(), rVerticalComponent.begin());
            }
            else
            {
                std::copy(differentiationKernel.begin(), differentiationKernel.end(), rVerticalComponent.begin());
                std::copy(averagingKernel.begin(), averagingKernel.end(), rHorizontalComponent.begin());
            }
            kernelSum = 0.0;
        }




		void calcCheckeredTemplate(const unsigned int pTileWidth, const unsigned int pTileHeight,
			const unsigned int pNumberHorizontalRepetitions, const unsigned int pNumberVerticalRepetitions,
			const double pLowIntensity, const double pHighIntensity,
			DImage & rHorizontalComponent, DImage & rVerticalComponent,
			double & rKernelSum){

			unsigned int heigth = 2 * pTileHeight*pNumberVerticalRepetitions;
			unsigned int width = 2 * pTileWidth*pNumberHorizontalRepetitions;

			double oIntensityElement1 = sqrt(pLowIntensity);
			double oIntensityElement2 = sqrt(pHighIntensity);

			double oIntermediateIntensity = oIntensityElement1 * oIntensityElement2;
			double oTileArea = pTileWidth * pTileHeight;
			rKernelSum = (pNumberHorizontalRepetitions * pNumberVerticalRepetitions * oTileArea) * (2 * oIntermediateIntensity + pLowIntensity + pHighIntensity);

			rVerticalComponent.resizeFill(Size(1, heigth), oIntensityElement2);
			rHorizontalComponent.resizeFill(Size(width, 1), oIntensityElement2);

			//fill elements
			for (unsigned int n = 0; n < pNumberHorizontalRepetitions; ++n){
				for (unsigned int i = 0; i < pTileWidth; ++i){
					int x = (2 * pTileWidth)*n + i;
					rHorizontalComponent.setValue(x, 0, oIntensityElement1);
				}
			}

			for (unsigned int n = 0; n < pNumberVerticalRepetitions; ++n){
				for (unsigned int i = 0; i < pTileHeight; ++i){
					int y = (2 * pTileHeight)*n + i;
					rVerticalComponent.setValue(0, y, oIntensityElement1);
				}
			}
		}// calcCheckerBoardTemplate

		void multiplyHorizontalAndVerticalMatrix(const DImage & pHorizontalElement, const DImage & pVerticalElement, DImage & rResultMatrix){
			//dot product
			if ((pHorizontalElement.height() * pVerticalElement.width()) != 1){
				wmLog(eWarning, "the inputs are not vectors");
			}
			int width = pHorizontalElement.width();
			int height = pVerticalElement.height();
			rResultMatrix.resizeFill(Size(width, height), 0);
			for (int x = 0; x < width; ++x){
				for (int y = 0; y < height; ++y){
					const double v1 = pHorizontalElement.getValue(x, 0);
					const double v2 = pVerticalElement.getValue(0, y);
					const double value = v1 * v2;
					rResultMatrix.setValue(x, y, value);
				}
			}
		}

		void calcBoxTemplate(const unsigned int width, const unsigned int height, DImage & newTemplate, double & kernelSum)
		{
			newTemplate.resizeFill(Size(width, height), 0);
			newTemplate.fill(1 / double(newTemplate.size().area()));
			kernelSum = 1;
		}


		BImage readTemplateFromDisk(const std::string & pFilename){
			BImage oImage(Size(0, 0));
			wmLog(eInfo, "Reading template from "+ pFilename + "\n");
			Poco::File	oFile(pFilename);
			if (!oFile.exists()){
				wmLog(eError, "Requested file template doesn't exist " + pFilename + "\n");
				return oImage;
			}
			fileio::Bitmap bmp(pFilename);
			oImage.resizeFill(Size(bmp.width(), bmp.height()), 0);
			bool success = bmp.load(oImage.begin());
			if (!success){
				std::ostringstream oMsg;
				oMsg << "Error in loading bitmap file " << pFilename << "\n Bitmap header: " << bmp << "\n";
				wmLog(eError, oMsg.str());
				return oImage;
			}
			return oImage;
		}


	}
}
}
