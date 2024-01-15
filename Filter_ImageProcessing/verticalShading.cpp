/**
 *  Filter_PoreAnalysis::verticalShading.cpp
 *	@file
 *	@copyright      Precitec Vision GmbH & Co. KG
 *	@author         Wolfgang Reichl (WoR)
 *	@date           23.10.2012
 *	@brief					<What's the purpose of the code in this file>
 */

#include "verticalShading.h"

#include "common/frame.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "common/geoContext.h"
#include "system/templates.h" // iMin
#include <fliplib/TypeToDataTypeImpl.h>

using fliplib::Parameter;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {


	/**
	 * Filter and outpipe are name consitent with database scripts
	 */
		VerticalShading::VerticalShading()
		: TransformFilter("VerticalShading", Poco::UUID{"83ad32e1-b9a9-4f47-8b20-6b7f20fe445f"}),
			m_pPipeIn(NULL),
			m_oPipeOut(this, "ImageFrame")
		{
            setInPipeConnectors({{Poco::UUID("28A3A9CA-F5BA-471F-9FF3-2CC05B64AEEF"), m_pPipeIn, "ImageFrame", 0, ""}});
            setOutPipeConnectors({{Poco::UUID("7131E130-5318-4C1B-83E0-C6948281A607"), &m_oPipeOut, "ImageFrame", 0, ""}});
            setVariantID(Poco::UUID("3aebc7d0-5599-4c50-ac2a-0a878802e95a"));
		}


		/**
		 * we don't have parameters, so just call parent class for common parameters
		 */
		void VerticalShading::setParameter() {
			// here BaseFilter sets the common parameters (currently verbosity)
			TransformFilter::setParameter();
			// now we do our own parameters
		} // setParameter

		void VerticalShading::paint() {
			if ( (m_oVerbosity < eMedium || m_oSpTrafo.isNull()) ) {
				return;
			}

			const Trafo					&rTrafo					( *m_oSpTrafo );
			OverlayCanvas				&rCanvas				( canvas<OverlayCanvas>(m_oCounter) );
			OverlayLayer				&rLayerImage			( rCanvas.getLayerImage());

			const auto		oPosition	=	rTrafo(Point(0, 0));
			const auto		oTitle		=	OverlayText("Shaded image", Font(), Rect(150, 18), Color::Black());

			rLayerImage.add(new OverlayImage(oPosition, m_oShadedImageOut[m_oCounter % g_oNbPar], oTitle));
		} // paint

		/**
		 * casting the inpipe to a typed local var could be done later
		 * the main work is done by the parent
		 * @param p_rPipe the single input pipe
		 * @param p_oGroup 0..1 single parameter of multiple (actually its more complicated)
		 * @return
		 */
		bool VerticalShading::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
			m_pPipeIn = dynamic_cast< fliplib::SynchronePipe<ImageFrame>*>(&p_rPipe);
			// herewith we set the proceed as callback (return is always true)
			return BaseFilter::subscribe( p_rPipe, p_oGroup );
		} // subscribe

		/**
		 * called to do processing per image
		 * does some consistency checks and delegates actual work to 'shade'
		 * signals the result pipe
		 * @param sender ? some crazy parameter this routine really doesn't need
		 * @param e (here empty) array of parameters
		 */
		void	VerticalShading::proceed(const void* sender, fliplib::PipeEventArgs& e) {
			// here we check only for existence of the input, not its well-formedness
			if (m_pPipeIn==nullptr) {
				// signal error output == empty array
				preSignalAction(); m_oPipeOut.signal(ImageFrame());
			} else {
				// connect input variable to pipe content
				/// scalar input value(s)
				ImageFrame const& oIn( m_pPipeIn->read(m_oCounter) );
				m_oSpTrafo	= oIn.context().trafo();

				// do the actual processing
                auto& rShadedImageOut = m_oShadedImageOut[m_oCounter % g_oNbPar];
                rShadedImageOut.resize(oIn.data().size());
                eliminatePattern(oIn.data(), detectPattern(oIn.data()), rShadedImageOut);

			    auto oOut               =   ImageFrame(oIn.context(), rShadedImageOut, oIn.analysisResult());

				// signal output -> allow other filters to process result
				preSignalAction(); m_oPipeOut.signal(oOut);
			}
		} // proceed


		/**
		 * a wrapper routine that organizes the actual work
		 * detectPattern: analyse vertical structure of image -> minimum + average per column
		 * eliminatePattern: correct it: subtract minimum, scale average of col to 128
		 * wrap everything into a frame (same context as input)
		 * @param p_rIn unscaled image
		 * @return frame with scaled image
		 */
		ImageFrame VerticalShading::shade(ImageFrame const& p_rIn) {
			//m_oShadedImageOut	=	eliminatePattern(p_rIn.data(), detectPattern(p_rIn.data()));
            auto& rShadedImageOut = m_oShadedImageOut[m_oCounter % g_oNbPar];
            eliminatePattern(p_rIn.data(), detectPattern(p_rIn.data()), rShadedImageOut);
			return ImageFrame(p_rIn.context(), rShadedImageOut, p_rIn.analysisResult());
		} // shade

		/// detect column patterns in image
		Pattern	VerticalShading::detectPattern(BImage const& p_rIn) {
			uInt oWidth = p_rIn.width();
			uInt oHeight = p_rIn.height();
			// this algorithm evolved to make the offset redundant
			// so Pattern might be replaced by rFactor-vector
			Pattern oColumnNoise(oWidth);
			//std::vector<uInt> &rOffset(oColumnNoise.offset);
			std::vector<uInt> &rFactor(oColumnNoise.factor);

			// init vector
			for (uInt col=0; col<oWidth; ++col) {	rFactor[col]  = 0; }
			// do row sum for all columns
			for (uInt row=0; row<oHeight; ++row) {
				const byte *pIn(p_rIn[row]);
				for (uInt col=0; col<oWidth; ++col) {
					rFactor[col] += pIn[col];
				}
			}
			return oColumnNoise;
		}

		/// eliminate column patterns in image
		void VerticalShading::eliminatePattern(BImage const& p_rIn, Pattern const& p_rPattern, BImage& p_rOut) {
			//std::vector<uInt> const& rOffset(p_rPattern.offset);
			// factor contains imageHeight * averageFactor
			std::vector<uInt> const& rFactor(p_rPattern.factor);
			uInt oYSize(p_rIn.size().height);
			uInt oXSize(p_rIn.size().width);
			// this check if only fail if big shit happened
			if (uInt(rFactor.size())!=oXSize)
            {
                p_rOut.resize(Size2D{0, 0});
                return;
            }

			// we will calculate the fixed point inverse of the factors
			// so: inverseFactor = (1<<24) / factor;
			// later: correctedPixel = 128*((pixel * inverseFactor) >> 24); == 128 * pixel / factor
			// ... but without a division for every pixel. We use a shift of 24 so we can still mutliply
			// by 255 wihtout generating overflow. 128 is the Norm value;
			// we want the average to be mapped to 128
			uInt o128 = 128*256*256; // 2^7 = 128
			// calculate: 1/(avgColumnSum) == 1/(columnSum/imageHeight) == imageHeight/columnSum
			std::vector<uInt> oInverseFactor(oXSize);
			std::vector<uInt> oNormOffset(oXSize);
			for (uInt col=0; col<oXSize; ++col) {
				// inverse denoise in oOne times our intended result (we will need to correct that later)
				oInverseFactor[col] = (o128*oYSize) / (rFactor[col]==0 ? 1 : rFactor[col]);
				oNormOffset[col] = ((32*rFactor[col]) / oYSize)>>5;
			}
			//BImage oOut(p_rIn.size());
            p_rOut.resize(p_rIn.size());
			// now correct image
			for (uInt row=0; row<oYSize; ++row) {
				const byte *pIn(p_rIn[row]);
				byte *pOut(p_rOut[row]);
				for (uInt col=0; col<oXSize; ++col) {
					// inverse noise is o128 times our intended result (we will need to correct that later)
					// additive correction only
					//int val = (((uInt(pIn[col])-oNormOffset[col]+0x80))); // * oInverseFactor[col])>>8);
					// multiplicative correction only
					//int val = (((uInt(pIn[col])) * oInverseFactor[col])>>16);
					// combined correction
					int val = pIn[col]-oNormOffset[col];
					val = (val<0)? -int(((-val)* oInverseFactor[col])>>16) :  (val* oInverseFactor[col])>>16;
					val = 0x80+val;
					pOut[col] = val<0 ? 0 : (val>255 ? 255 : val);
				}
			}
		}

} // namespace filter
} // namespace precitec
