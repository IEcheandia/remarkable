/***
*	@file			
*	@copyright		Precitec Vision GmbH & Co. KG
*	@author			LB
*	@date			2017
*	@brief			Arithmetic on Images Stacks
*/


#ifndef ImageArithmetic_H_
#define ImageArithmetic_H_


#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"
#include <filter/armStates.h>
#include "frameBuffer.h"
#include "operationsOnImageVector.h"

#include "common/frame.h"

class BenchmarkImageArithmetic;
class TestImageArithmetic;

namespace precitec {
	using namespace image;
	using namespace interface;
namespace filter {

/**
 * Arithmetic operations on image stack
 */
class FILTER_API ImageArithmetic : public fliplib::TransformFilter
{
public:
    friend BenchmarkImageArithmetic;
    friend TestImageArithmetic;

	ImageArithmetic();

	static const std::string m_oFilterName;
	static const std::string PIPENAME;
	void setParameter();
	void arm(const fliplib::ArmStateBase& p_rArmstate);


protected:
	/// in pipe registrastion
	bool subscribe(fliplib::BasePipe& pipe, int group);
	// Verarbeitungsmethode IN Pipe
	void proceed(const void* sender, fliplib::PipeEventArgs& e);
	/**
	 * @brief Paint overlay output.
	 */
	void paint();
private:

    
	const fliplib::SynchronePipe< ImageFrame >*		m_pPipeInImageFrame;	///< in pipe
	fliplib::SynchronePipe< ImageFrame >			m_oPipeImageFrame;		//<- Output PIN fuer verarbeitetes Graubild

	unsigned int			m_oWindow; 			///< number of frames for arithmetic operation
	bool					m_oRescalePixelIntensity; ///< True = linear lut (stretchContrast) , False= clamp intensities
	bool					m_oPassThroughBadRank; ///< pass through bad rank frames (not implemented))
	Operations				m_oOperation;
	byte					m_oMinIntensity;
	byte					m_oMaxIntensity;
	bool					m_oInvertLUT;
	bool					m_oProductMode;
	bool					m_oResampleOutput; ///< True: ensure that output has sampling 1,1, even if computed with downsampling
	int						m_oStartImage;
	FrameBuffer				m_oFrameBuffer;
	OperationsOnImageVector m_oOperationOnImageVector;
	
	interface::SmpTrafo			m_oSpTrafo;			///< roi translation
    std::array<image::BImage, g_oNbParMax> m_oImagesOut;			///< output image
    std::array<image::BImage, g_oNbParMax> m_oImagesOutCompressed;			///< output image before upsampling

    static void stretchContrast(image::BImage & p_rImage);
    void processRepeatImage(image::BImage & p_rOutputImage, interface::ImageContext & p_rOutputContext, 
                            FrameBuffer & rFrameBuffer,
                            const interface::ImageFrame & p_rInputFrame) const;
    
    void processPixelOperationOnImage(image::BImage & p_rOutputImage, 
                                      OperationsOnImageVector & rOperationsOnImageVector, FrameBuffer & rFrameBuffer,
                                      const interface::ImageFrame & p_rInputFrame) const;

    void applyLUT(image::BImage & p_rComputedImage) const;
    
};

}}

#endif /*ImageArithmetic_H_*/
