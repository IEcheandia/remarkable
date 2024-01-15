
/**
 *	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2013
 *  @brief			algorithmic interface for images
 */



#ifndef ALGOIMAGE_H_20131023_INCLUDED	
#define ALGOIMAGE_H_20131023_INCLUDED

#include <common/geoContext.h> // for trafo
#include "math/3D/projectiveMathStructures.h"  // for Vec3D

#include "Analyzer_Interface.h"

#include "image/image.h"
#include "parameterEnums.h"


namespace precitec {
namespace filter {


/**
 * @brief      Apply trafo to Vec3D vector and return result as Vec3D vector
 * @param      p_oX      X Screen coord to be transformed
 * @param      p_oY      Y Screen coord to be transformed
 * @param      p_rTrafo  Trafo to apply
 * @return     Vec3D     Transformed vector
 */
ANALYZER_INTERFACE_API math::Vec3D applyTrafo(const int p_oX, const int p_oY, const interface::Trafo &p_rTrafo);

/**
 * @brief      Apply trafo to Vec3D vector and return result as Vec3D vector
 * @param      p_oSrc    Vector to apply trafo to
 * @param      p_rTrafo  Trafo to apply
 * @return     Vec3D     Transformed vector
 */
ANALYZER_INTERFACE_API math::Vec3D applyTrafo(const math::Vec3D &p_oSrc, const interface::Trafo &p_rTrafo);

/**
 *	@brief		Calculate variance of an image
 *	@param		p_rImageIn		Input data
 *	@return		double			Variance of input image
 */
ANALYZER_INTERFACE_API double calcVariance(const image::BImage& p_rImageIn);

/**
*	@brief		Calculate MinMaxDistance of an image
*	@param		p_rImageIn		Input data
*	@return		double			MinMaxDistance of input image
*/
ANALYZER_INTERFACE_API byte calcMinMaxDistance(const image::BImage& p_rImageIn);

/**
*	@brief		Calculate MinMaxDistance of an image
*	@param		p_rImageIn		Input data
*   @param      deletePerCent   Percent value of deleted highest and lowest values
*	@return		double			MinMaxDistance of input image
*/
ANALYZER_INTERFACE_API byte calcMinMaxDistanceDeleteHighLow(const image::BImage& p_rImageIn, int deletePerCent);

/**
*	@brief		Calculate texture (mean x-gradient) of input image
*	@return		double			Texture (mean x-gradient) of input image
*/
ANALYZER_INTERFACE_API double calcGradientSumX(const image::BImage& p_rImageIn);

/**
*	@brief		Calculate texture (mean y-gradient) of input image
*	@return		double			Texture (mean y-gradient) of input image
*/
ANALYZER_INTERFACE_API double calcGradientSumY(const image::BImage& p_rImageIn);

/**
*	@brief		Calculate texture (mean y-gradient) of input image
*	@return		double			Texture (mean y-gradient) of input image
*/
ANALYZER_INTERFACE_API double calcTexture(const image::BImage& p_rImageIn);
ANALYZER_INTERFACE_API double calcTexture(const image::BImage& p_rImageIn, image::BImage& p_rImageTmp);

/**
* @brief						Calculates mean intensity of an image.
* @param p_rImageIn				Input image to be read.
*/
ANALYZER_INTERFACE_API double calcMeanIntensity(
	const image::BImage&					p_rImageIn);


/**
* @brief						Calculates mean intensity of an image.
* @param p_rImageIn				Input image to be read.
* @param p_oStep				Subsampling step width [pixels].
*/
ANALYZER_INTERFACE_API double calcMeanIntensityWithStep(
	const image::BImage&					p_rImageIn,
	unsigned int							p_oStep);

/**
* @brief						Binarizes an image depending on dynamic threshold, which is calculated depending on mean image intensity.
* @param p_rImageIn				Input image to be read.
* @param p_oComparisonType		Comparison type 'smaller' / 'greater-equal'. Dark / bright regions will be set to 255.
* @param p_oDistToMeanIntensity	Distance to mean image intensity.
* @param p_rImageOut			Output binary image to be calculated. Contains only 0 and 255.
*/
ANALYZER_INTERFACE_API void calcBinarizeDynamic(
	const image::BImage&					p_rImageIn,
	ComparisonType 							p_oComparisonType,
	byte									p_oDistToMeanIntensity,
	image::BImage&							p_rImageOut);

/**
* @brief						Binarizes an image depending on static threshold.
* @param p_rImageIn				Input image to be read.
* @param p_oComparisonType		Comparison type 'smaller' / 'greater-equal'. Dark / bright regions will be set to 255.
* @param p_oThreshold			Binarize threshold.
* @param p_rImageOut			Output binary image to be calculated. Contains only 0 and 255.
*/
ANALYZER_INTERFACE_API void calcBinarizeStatic(
	const image::BImage&					p_rImageIn,
	ComparisonType 							p_oComparisonType,
	byte									p_oThreshold,
	image::BImage&							p_rImageOut);

/**
* @brief						Binarizes an image depending on threshold. No border processing.
* @param p_rImageIn				Input image to be read.
* @param p_oComparisonType		Comparison type 'smaller' / 'greater-equal'. Dark / bright regions will be set to 255.
* @param p_oDistToMeanIntensity	Distance to mean image intensity.
* @param p_rImageOut			Output binary image to be calculated. Contains only 0 and 255.
*/
ANALYZER_INTERFACE_API void calcBinarizeLocal(
	const image::BImage&					p_rImageIn,
	ComparisonType 							p_oComparisonType,
	byte									p_oDistToMeanIntensity,
	image::BImage&							p_rImageOut);

/**
* @brief						Eliminates single pixel > 0 if 5x5 neighboorhood is dark (0). Inplace.
* @param p_rImageIn				Input image to be filtered.
*								TODO not in use, code guidelines, write own filter or extend binarize filter
*/
ANALYZER_INTERFACE_API void filterBinNoise5x5(image::BImage& p_rImageIn);

ANALYZER_INTERFACE_API double binaerSumX(const image::BImage& p_rImageIn);

ANALYZER_INTERFACE_API void binaerItMean(const image::BImage & p_rImageIn, image::BImage& p_rImageTmp);

/**
* @brief						Calculates min intensity of an image.
* @param p_rImageIn				Input image to be read.
*/
ANALYZER_INTERFACE_API byte calcMinIntensity(
	const image::BImage&					p_rImageIn);

/**
* @brief						Calculates max intensity of an image.
* @param p_rImageIn				Input image to be read.
*/
ANALYZER_INTERFACE_API byte calcMaxIntensity(
	const image::BImage&					p_rImageIn);



//nearest neighbour resampling
ANALYZER_INTERFACE_API void resampleFrame(image::BImage& rOutputImage, interface::ImageContext& rOutputContext, const image::BImage& rInputImage, const interface::ImageContext& rInputContext);
ANALYZER_INTERFACE_API void downsampleImage(image::BImage & outImage, const image::BImage& rImage, unsigned int dx, unsigned int dy);
ANALYZER_INTERFACE_API void upsampleImage(image::BImage & outImage, const image::BImage& rImage, unsigned int dx, unsigned int dy);



/**
*	@brief		Calculate texture (mean y-gradient) of input image
*	@return		double			Texture (mean y-gradient) of input image
*/
ANALYZER_INTERFACE_API double calcStructure(const image::BImage & p_rImageIn);
ANALYZER_INTERFACE_API double calcStructure(const image::BImage & p_rImageIn, image::BImage & p_rImageTmp);

ANALYZER_INTERFACE_API enum class SummedAreaTableOperation {sumValues, sumSquaredValues};

//summed area table (see  Viola,  Jones(2002). "Robust Real-time Object Detection")
template <typename TInput, typename TTable>
struct SummedAreaTable
{
    SummedAreaTable(){};
    SummedAreaTable(const image::TLineImage<TInput>  & p_rImageIn, SummedAreaTableOperation p_oOperation );

    template <SummedAreaTableOperation t_operation>
    void init(const image::TLineImage<TInput>  & p_rImageIn);
    
    TTable calcSum( int xMin, int xMax, int yMin, int yMax) const;
    image::TLineImage<TTable> m_IntegralImage;
  
};

//explicit instantation 
#ifdef __linux__
template struct SummedAreaTable<byte,double>;
template struct SummedAreaTable<byte, int>; 
#else
template ANALYZER_INTERFACE_API SummedAreaTable<byte, int>::SummedAreaTable();
template ANALYZER_INTERFACE_API SummedAreaTable<byte, int>::SummedAreaTable(const image::TLineImage<byte>  & p_rImageIn, SummedAreaTableOperation p_oOperation);
template ANALYZER_INTERFACE_API void SummedAreaTable<byte, int>::init<SummedAreaTableOperation::sumValues>(const image::TLineImage<byte>  & p_rImageIn);
template ANALYZER_INTERFACE_API int SummedAreaTable<byte, int>::calcSum(int xMin, int xMax, int yMin, int yMax) const;
#endif

ANALYZER_INTERFACE_API image::BImage genNoiseImage( image::Size2d imageSize, int seed );



} // namespace filter
} // namespace precitec


#endif // ALGOIMAGE_H_20131023_INCLUDED
