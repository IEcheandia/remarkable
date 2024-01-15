/***
*	@file
*	@copyright		Precitec Vision GmbH & Co. KG
*	@author			LB
*	@date			2017
*	@brief			Arithmetic on Images Stacks
*/

#include "imageArithmetic.h"

#include "filter/algoStl.h"			///< stl algo
#include "module/moduleLogger.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

#include "system/typeTraits.h"	///< byte, int max value

#include "image/image.h"
#include "common/bitmap.h"
#include "filter/algoImage.h"
#include <fstream>

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {


const std::string	ImageArithmetic::m_oFilterName 	= std::string("ImageArithmetic");
const std::string	ImageArithmetic::PIPENAME	= std::string("ImageFrame");


ImageArithmetic::ImageArithmetic() :
	TransformFilter		( ImageArithmetic::m_oFilterName, Poco::UUID{"41e3ff03-4d16-4033-a842-3d243f98228b"} ),
	m_pPipeInImageFrame	( nullptr ),
	m_oPipeImageFrame	( this, ImageArithmetic::PIPENAME ),
	m_oWindow(2),
	m_oRescalePixelIntensity(false),
	m_oPassThroughBadRank(true),
	m_oOperation(Operations::eInvalid),
	m_oMinIntensity(0),
	m_oMaxIntensity(255),
	m_oInvertLUT(false),
	m_oProductMode(true),
	m_oResampleOutput(true),
	m_oStartImage(0)
{
		m_oFrameBuffer.resetState(m_oWindow, FrameBuffer::ConstraintsOnInputSize::eResetStateOnDifferentSize, 1, 1);
		parameters_.add("ResolutionX", fliplib::Parameter::TYPE_UInt32, m_oFrameBuffer.getResolutionX());
		parameters_.add("ResolutionY", fliplib::Parameter::TYPE_UInt32, m_oFrameBuffer.getResolutionY());
		parameters_.add("TimeWindow", fliplib::Parameter::TYPE_UInt32, m_oWindow);
		parameters_.add("RescalePixelIntensity", fliplib::Parameter::TYPE_bool, m_oRescalePixelIntensity);
		parameters_.add("PassThroughBadRank", fliplib::Parameter::TYPE_bool, m_oPassThroughBadRank);
		parameters_.add("Operation", fliplib::Parameter::TYPE_int, int(m_oOperation));  //cast to int necessary! otherwise Parameter constructor fails
		parameters_.add("OutputMinValue", fliplib::Parameter::TYPE_UInt32, m_oMinIntensity);
		parameters_.add("OutputMaxValue", fliplib::Parameter::TYPE_UInt32, m_oMaxIntensity);
		parameters_.add("InvertLUT", fliplib::Parameter::TYPE_bool, m_oInvertLUT);
		parameters_.add("ResetOnSeamStart", fliplib::Parameter::TYPE_bool, m_oProductMode);
        parameters_.add("ResampleOutput", fliplib::Parameter::TYPE_bool, m_oResampleOutput);
        parameters_.add("StartImage", fliplib::Parameter::TYPE_int, int(m_oStartImage));


        setInPipeConnectors({{Poco::UUID("dc29a5fb-266e-4896-9633-26565ba0f172"), m_pPipeInImageFrame, "InputImage", 0, "InputImage"}});
        setOutPipeConnectors({{Poco::UUID("0a07efb5-2ca0-4daf-aea2-f28d2a703cb1"), &m_oPipeImageFrame, "ImageFrame", 0, "OutputImage"}});
        setVariantID(Poco::UUID("80a76f75-f055-412a-b92d-364f135ce8e3"));
}

void ImageArithmetic::setParameter()
{
	//wmLog(eInfo, "ImageArithmetic: setParameter (current buffer status: %d/%d\n", m_oFrameBuffer.getLastIndexes(m_oWindow+1).size(), m_oWindow);
	//store previous parameters, so that buffer is updated only when necessary
	auto prevWindow = m_oWindow;
	auto prevResolutionX = m_oFrameBuffer.getResolutionX();
	auto prevResolutionY = m_oFrameBuffer.getResolutionY();

	TransformFilter::setParameter();
	unsigned int oResolutionX = parameters_.getParameter("ResolutionX").convert<unsigned int>();
	unsigned int oResolutionY = parameters_.getParameter("ResolutionY").convert<unsigned int>();
	m_oWindow = parameters_.getParameter("TimeWindow").convert<unsigned int>();
	m_oRescalePixelIntensity = parameters_.getParameter("RescalePixelIntensity").convert<bool>();
	m_oPassThroughBadRank = parameters_.getParameter("PassThroughBadRank").convert<bool>();
	int oOperation = parameters_.getParameter("Operation").convert<int>();
	if ( (oOperation >= 0 && oOperation < int(Operations::NumberValidOperations))  //parameter is valid
		&& m_oWindow > 0 ) // a valid operation needs to work at least on one image
	{
		m_oOperation = static_cast<Operations>(oOperation);
	}
	else
	{
		m_oOperation = Operations::eInvalid;
		m_oFrameBuffer.resetState(1, FrameBuffer::ConstraintsOnInputSize::eResetStateOnDifferentSize,1,1);
		return;
	}
	m_oMinIntensity = parameters_.getParameter("OutputMinValue").convert<byte>();
	m_oMaxIntensity = parameters_.getParameter("OutputMaxValue").convert<byte>();
	m_oInvertLUT = parameters_.getParameter("InvertLUT").convert<bool>();
	m_oProductMode = parameters_.getParameter("ResetOnSeamStart").convert<bool>();

	if (m_oMinIntensity > m_oMaxIntensity)
	{
		wmLog(eWarning, "MinIntensity >MaxIntensity: swapping maximum and minimum pixel intensity and applying inverted LUT\n");
		std::swap(m_oMinIntensity, m_oMaxIntensity);
		m_oInvertLUT = true;
	}

	//limits for PixelType byte
	if (m_oMaxIntensity > 255)
	{
		m_oMaxIntensity = 255;
	}
	if (m_oMinIntensity < 0)
	{
		m_oMinIntensity = 0;
	}

	//sanity check on operations
	if (m_oOperation == Operations::ePixelDiff &&  m_oWindow > 2)
	{
		wmLog(eWarning, "Difference should be done with 2 images\n");
	}

    if ( m_oOperation == Operations::eRepeat)
    {
        if (m_oPassThroughBadRank )
        {
            //we can't skip any image, even if they are bad
            wmLog(eWarning, "Operation repeat requires PassThroughBadRank = false \n");
            m_oPassThroughBadRank = false;
        }
        if (oResolutionX != 1 || oResolutionY !=1)
        {
            wmLog(eWarning, "Operation repeat does not use sampling \n");
            oResolutionX = 1;
            oResolutionY = 1;
        }
	}

	if (!m_oProductMode)
	{
		wmLog(eWarning, "ResetOnSeamStart=False : the cached result will not be reset on seam start \n");
	}

	//reset the buffer if the size changed, otherwise keep it
	if ( (m_oWindow != prevWindow) || (oResolutionX != prevResolutionX) || (oResolutionY != prevResolutionY))
	{
		m_oFrameBuffer.resetState(m_oWindow,
			(m_oOperation == Operations::eRepeat || m_oWindow == 1) ? FrameBuffer::ConstraintsOnInputSize::eKeepEveryInput : FrameBuffer::ConstraintsOnInputSize::eResetStateOnDifferentSize,
			oResolutionX, oResolutionY
			);
	}

	m_oResampleOutput = parameters_.getParameter("ResampleOutput").convert<bool>();
	m_oStartImage = parameters_.getParameter("StartImage").convert<int>();
}


bool ImageArithmetic::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeInImageFrame  = dynamic_cast< fliplib::SynchronePipe < ImageFrame > * >(&p_rPipe);
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
}


void ImageArithmetic::arm(const fliplib::ArmStateBase& p_rArmstate)
{
	if ((m_oProductMode) && (p_rArmstate.getStateID() == eSeamStart))
	{
		auto bufferState = m_oFrameBuffer.getBufferState();
		assert(m_oOperation == Operations::eInvalid || bufferState.bufferSize == m_oWindow);
		wmLog(eDebug, "ImageArithmetic: resetting buffer at seam start (current buffer status: %d/%d)\n", bufferState.initializedElements, m_oWindow);
		m_oFrameBuffer.resetState(m_oWindow);
	}
}


void ImageArithmetic::proceed(const void* sender, PipeEventArgs& e)
{
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor
	m_oSpTrafo = nullptr;


	// Empfangenes Frame auslesen
	const ImageFrame	&rFrame = m_pPipeInImageFrame->read(m_oCounter);
	const BImage		&rImage = rFrame.data();
	ResultType	curAnalysisResult = rFrame.analysisResult();

	unsigned int	oInputWidth = rImage.size().width;
	unsigned int	oInputHeight = rImage.size().height;

	//similar to LowPass::process (in AlgoArray)
	if ( (!rImage.isValid())
		|| (m_oPassThroughBadRank && (curAnalysisResult != AnalysisOK)) //if passthroughbadrank is true, bad rank images are just ignored
		|| m_oOperation == Operations::eInvalid
		|| rFrame.context().imageNumber() < m_oStartImage
		)
	{
		ImageFrame oNewFrame( rFrame.context(), rImage, rFrame.analysisResult() );
		preSignalAction();
		m_oPipeImageFrame.signal( oNewFrame );
		return;
	}

	//check if frame should be skipped
	if ( m_oFrameBuffer.evaluateFrameInsertion(rFrame) == FrameBuffer::InsertInputFrameState::eFrameSkipped)
	{
		assert(m_oFrameBuffer.getLastCachedFrame().initialized() && "Empty buffer could not be initialized");

		const geo2d::Size oValidImageSize(m_oFrameBuffer.getLastCachedFrame().m_image.size());

		//the image is not compatible with the images saved in the buffer (for the selected operation)
		ImageFrame oNewFrame(rFrame.context(), BImage(), rFrame.analysisResult());
		wmLog(eWarning, "Image number %d (position %d): size  [%d, %d ] is not compatible with previous images (should be %d,%d)\n",
			m_oCounter, rFrame.context().position(), oInputWidth, oInputHeight, oValidImageSize.width, oValidImageSize.height);
		preSignalAction();

		m_oPipeImageFrame.signal(oNewFrame);
		return;

	}


	//prepare output image
	ImageContext oOutputContext = rFrame.context();

	//image used for computation
	auto & rComputedImage =  m_oFrameBuffer.hasSampling() ? m_oImagesOutCompressed[m_oCounter % g_oNbPar] :  m_oImagesOut[m_oCounter % g_oNbPar];

	//shortcut for when the operation is a simple delay
	if ( m_oOperation == Operations::eRepeat)
	{
        assert(!m_oFrameBuffer.hasSampling() && "sampling parameter should be ignored in case of repeat image");
        assert( &rComputedImage ==  &m_oImagesOut[m_oCounter % g_oNbPar]);
        processRepeatImage(rComputedImage, oOutputContext, m_oFrameBuffer, rFrame);
	}
	else
    {
        processPixelOperationOnImage(rComputedImage, m_oOperationOnImageVector, m_oFrameBuffer, rFrame);
    }


    assert(rComputedImage.isContiguos());

    //apply LUT operations and resample if necessary

    applyLUT(rComputedImage);

    auto & rImageOutFinal = m_oImagesOut[m_oCounter % g_oNbPar];

    if (m_oFrameBuffer.hasSampling())
    {
        assert (&rImageOutFinal != &rComputedImage);
        if (m_oResampleOutput)
        {
            rImageOutFinal.resize(Size2d(oInputWidth, oInputHeight));
            m_oFrameBuffer.upsample(rComputedImage, rImageOutFinal);
        }
        else
        {
            rImageOutFinal.resize(rComputedImage.size());
            rComputedImage.copyPixelsTo(rImageOutFinal);
            if ( (oOutputContext.SamplingX_ > 1) || (oOutputContext.SamplingY_ > 1) )
            {
                wmLog(eInfo, "Input Image was upsampled %d %% %d %%\n", 100 * rFrame.context().SamplingX_, 100 * rFrame.context().SamplingY_);
                //upsample not implemented because there aren't filters which upsample an image, output image will have the same context
            }
            else
            {
                oOutputContext.SamplingX_ = oOutputContext.SamplingX_ / m_oFrameBuffer.getResolutionX();
                oOutputContext.SamplingY_ = oOutputContext.SamplingY_ / m_oFrameBuffer.getResolutionY();
            }
        }
    }
    else
    {
        assert (&rImageOutFinal == &rComputedImage);
        //nothing to do
    }

	assert( (m_oOperation == Operations::eRepeat || !m_oResampleOutput) ||  rImageOutFinal.size() == rImage.size());

	if ( (oOutputContext.SamplingX_ > 1) || (oOutputContext.SamplingY_ > 1) )
	{
		wmLog(eInfo, "Input Image was upsampled %d %% %d %%\n", 100 * rFrame.context().SamplingX_, 100 * rFrame.context().SamplingY_);
		//upsample not implemented because there aren't filters which upsample an image, output image will have the same context
	}

	if ( ( (oOutputContext.SamplingX_ < 1) || (oOutputContext.SamplingY_ < 1) ) && m_oResampleOutput )
	{
		wmLog(eInfo, "Input image was downsampled %d perc %d perc \n", int(100*rFrame.context().SamplingX_), int(100*rFrame.context().SamplingY_));
        BImage oComputedImageCopy (rComputedImage.size());
        rComputedImage.copyPixelsTo(oComputedImageCopy);
        resampleFrame(rImageOutFinal, oOutputContext, oComputedImageCopy, rFrame.context());
	}


	// prepare output

	m_oSpTrafo = oOutputContext.trafo();
	ImageFrame oNewFrame(oOutputContext, rImageOutFinal, curAnalysisResult);

	preSignalAction();
	m_oPipeImageFrame.signal(oNewFrame);

}

void ImageArithmetic::paint()
{
	if ((m_oVerbosity < eMedium) || m_oSpTrafo.isNull())
	{
		return;
	}

	const Trafo					&rTrafo(*m_oSpTrafo);
	OverlayCanvas				&rCanvas				( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer				&rLayerImage			( rCanvas.getLayerImage());

	const auto		oPosition	=	rTrafo(Point(0, 0));
	const auto		oTitle		=	OverlayText("ImageArithmetic", Font(), Rect(150, 18), Color::Black());

	rLayerImage.add<OverlayImage>(oPosition, m_oImagesOut[m_oCounter % g_oNbPar ], oTitle);

} // paint


void ImageArithmetic::stretchContrast(image::BImage & p_rImage)
{
    assert(p_rImage.isContiguos());
    auto bounds = std::minmax_element(p_rImage.begin(), p_rImage.end());
    byte minValue = *(bounds.first);
    byte maxValue = *(bounds.second);
    const double range(maxValue - minValue);
    std::transform(p_rImage.begin(), p_rImage.end(), p_rImage.begin(), [&range, &minValue](byte v) {return (v-minValue)/range*255;});
}

void ImageArithmetic::processRepeatImage(image::BImage & rComputedImage, interface::ImageContext & p_rOutputContext, FrameBuffer & rFrameBuffer,
                                         const interface::ImageFrame & rFrame) const
{
    assert(m_oOperation == Operations::eRepeat && "This method is to be used for the repeat image case");

    //the current image has not been inserted yet, retrieve oldest image until (m_oWindow -1)
    std::vector<FrameBuffer::index> oPreviousValidBufferIndexes = rFrameBuffer.getLastIndexes(m_oWindow-1, 0);

    if (oPreviousValidBufferIndexes.empty())
    {
        p_rOutputContext = rFrame.context();
        rComputedImage.resize(rFrame.data().size());
        rFrame.data().copyPixelsTo(rComputedImage);
    }
	else
    {
        auto oCachedElement = rFrameBuffer.getCachedElement(oPreviousValidBufferIndexes.back());

        const auto & rCachedImage = oCachedElement.m_image;
        p_rOutputContext = ImageContext(rFrame.context(), oCachedElement.m_trafo);

        rComputedImage.resize(rCachedImage.size());
        rCachedImage.copyPixelsTo(rComputedImage);
    }

    if (m_oRescalePixelIntensity)
    {
        stretchContrast(rComputedImage);
    }

    if (m_oWindow > 1)
    {
        auto frameInserted = rFrameBuffer.insertInputFrame(rFrame);
        (void)(frameInserted); assert(frameInserted == FrameBuffer::InsertInputFrameState::eFrameAdded && "in the repeat case, all frames should be accepted independently of size");
    }
}


void ImageArithmetic::processPixelOperationOnImage(image::BImage & p_rOutputImage,
                                    OperationsOnImageVector & rOperationsOnImageVector, FrameBuffer & rFrameBuffer,
                                    const interface::ImageFrame & p_rInputFrame) const
{

    const auto & rInputImage = p_rInputFrame.data();
    const auto oInputSize = rInputImage.size();

    if (m_oWindow == 1)
    {
        if (rFrameBuffer.hasSampling())
        {

            auto frameInserted = rFrameBuffer.insertInputFrame(p_rInputFrame);
            (void)(frameInserted); assert(frameInserted != FrameBuffer::InsertInputFrameState::eFrameSkipped);

            assert (rFrameBuffer.getBufferState().initializedElements > 0 && "buffer can't be empty after insertion, has frame to skip  been checked?");

            // the size of the cached image (after resampling)
            const auto oCachedImage = rFrameBuffer.getLastCachedFrame().m_image;
            const auto oCachedSize = oCachedImage.size();
            rOperationsOnImageVector.clear(oCachedSize.width, oCachedSize.height);
            rOperationsOnImageVector.insertImage(oCachedImage);
            p_rOutputImage.resize(oCachedSize);
            assert(rOperationsOnImageVector.w() == oCachedSize.width);
            assert(rOperationsOnImageVector.h() == oCachedSize.height);
        }
        else
        {
            //shortcut: we do not need to save the image for the next iteration or to resample it
            rOperationsOnImageVector.clear(oInputSize.width, oInputSize.height);
            rOperationsOnImageVector.insertImage(rInputImage);

        }
        p_rOutputImage.resize(Size2d( rOperationsOnImageVector.w(), rOperationsOnImageVector.h()));
        rOperationsOnImageVector.computeResultImage(p_rOutputImage, m_oOperation, m_oRescalePixelIntensity);
        return;
    }

    const auto oInitialBufferState = rFrameBuffer.getBufferState();
    const auto oInitialFilledPositions = std::min(oInitialBufferState.initializedElements, m_oWindow); //we should not rely on the fact that FrameBuffer.size() == m_oWindow

    bool trytoReuseIntermediateResult = m_oWindow >= 3 && rOperationsOnImageVector.resultWithMovingWindowAvailable(
        m_oOperation, m_oRescalePixelIntensity, oInitialFilledPositions);

    //save the image that is going to be removed in the buffer
    BImage removedImage = (trytoReuseIntermediateResult && oInitialFilledPositions == m_oWindow)?
                rFrameBuffer.getCachedElement(rFrameBuffer.getLastIndexes(m_oWindow, m_oWindow - 1).front()).m_image:
                BImage(Size2d(0,0));


    auto frameInserted = rFrameBuffer.insertInputFrame(p_rInputFrame);
    (void)(frameInserted); assert(frameInserted != FrameBuffer::InsertInputFrameState::eFrameSkipped);

    const auto oBufferState = rFrameBuffer.getBufferState();
    assert (oBufferState.initializedElements> 0 && "buffer can't be empty after insertion, has frame to skip  been checked?");

    // the size of the cached image (after resampling)
    const auto oCachedSize = rFrameBuffer.getLastCachedFrame().m_image.size();
    p_rOutputImage.resize(oCachedSize);

    if (oBufferState.initializedElements == 1)
    {
        //buffer has only one element: it has been reset (for any reson: change of input size, seamStart...)
        //previous results can't be used
        removedImage.resize(Size2d(0,0));
        trytoReuseIntermediateResult = false;
        rOperationsOnImageVector.clear(oCachedSize.width, oCachedSize.height);
        assert( !rOperationsOnImageVector.resultWithMovingWindowAvailable(m_oOperation, m_oRescalePixelIntensity, oBufferState.initializedElements- 1));
    }

    if (trytoReuseIntermediateResult)
    assert((frameInserted != FrameBuffer::InsertInputFrameState::eBufferReset
        || (oBufferState.initializedElements== 1) ) && "buffer has not been actually reset, intermediate results could also be wrong");

    if (trytoReuseIntermediateResult)
    {

#ifndef NDEBUG
        assert(frameInserted == FrameBuffer::InsertInputFrameState::eFrameAdded);
        const auto oFilledPositions = std::min( rFrameBuffer.getBufferState().initializedElements, m_oWindow);
        if (removedImage.isValid())
        {
            assert(oInitialFilledPositions == m_oWindow);
            assert(oFilledPositions == m_oWindow);
        }
        else
        {
            assert(oInitialFilledPositions < m_oWindow);
            assert(oFilledPositions > 1);
            assert(oFilledPositions == (oInitialFilledPositions + 1));
        }
#endif

        bool resultUpdated = removedImage.isValid() ?
            rOperationsOnImageVector.tryUpdateResultWithMovingWindow(p_rOutputImage, removedImage, rInputImage,
                                                                            m_oOperation, m_oRescalePixelIntensity, oInitialFilledPositions):
            rOperationsOnImageVector.tryUpdateResultWithMovingWindow(p_rOutputImage, rInputImage,
                                                                            m_oOperation, m_oRescalePixelIntensity, oInitialFilledPositions);

        assert(resultUpdated && "not all conditions have been checked");
        if (resultUpdated)
        {
            return;
        }

    }


    auto oValidBufferIndexes = rFrameBuffer.getLastIndexes(m_oWindow,0);

    #ifndef NDEBUG
    // copy buffer size, to use later in assertions
    auto validValues = oValidBufferIndexes.size();
    #endif



    //initialize all pointers to pixels
    rOperationsOnImageVector.clear(oCachedSize.width, oCachedSize.height);

    for ( unsigned int i = 0; i < oValidBufferIndexes.size(); ++i )
    {
        auto buffer_ind = oValidBufferIndexes[i];
        bool ok = rOperationsOnImageVector.insertImage(rFrameBuffer.getCachedElement(buffer_ind).m_image);
        (void)(ok); assert(ok && "trying to iterate images of different size");
    }

    assert( rOperationsOnImageVector.numberOfImages() == validValues);
    assert(int( rOperationsOnImageVector.w()) == oCachedSize.width);
    assert(int( rOperationsOnImageVector.h()) == oCachedSize.height);

    rOperationsOnImageVector.computeResultImage(p_rOutputImage, m_oOperation, m_oRescalePixelIntensity);

}


void ImageArithmetic::applyLUT(image::BImage & p_rComputedImage) const
{
    if (m_oMaxIntensity != 255 || m_oMinIntensity != 0)
    {

        //here the result is approximate (2 consecutive rounding)
        assert(p_rComputedImage.isContiguos());
        auto bounds = std::minmax_element(p_rComputedImage.begin(), p_rComputedImage.end());
        byte minValue = *(bounds.first);
        byte maxValue = *(bounds.second);
        const double rangeIn =  maxValue > minValue ? double(maxValue - minValue) : 1.0;
        const double rangeOut = double(m_oMaxIntensity - m_oMinIntensity);

        if (!m_oInvertLUT)
        {
            std::transform(p_rComputedImage.begin(),p_rComputedImage.end(),
                        p_rComputedImage.begin(),
                            [&rangeIn, & rangeOut, &minValue, this](byte v)
                        {
                            return (v-minValue)*rangeOut/rangeIn + m_oMinIntensity;
                        });
        }
        else
        {
            std::transform(p_rComputedImage.begin(), p_rComputedImage.end(),
                    p_rComputedImage.begin(),
                           [&rangeIn, &rangeOut, &minValue, this](byte v)
                    {
                        return m_oMaxIntensity - ((v-minValue)*rangeOut/rangeIn);
                    });
        }

    }
    else
    {
        if (m_oInvertLUT)
        {
            assert(m_oMaxIntensity == 255 && m_oMinIntensity == 0);
            std::transform(p_rComputedImage.begin(), p_rComputedImage.end(), p_rComputedImage.begin(),
                           [](byte p)
                        {
                            return  255 - p;
                        });
        }
    }

}



} // namespace filter
} // namespace precitec

