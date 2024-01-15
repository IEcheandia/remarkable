/**
*  @copyright:		Precitec Vision GmbH & Co. KG
*  @author			Simon Hilsenbeck (HS)
*  @date			2013
*  @file			
*  @brief			Performs morphology operations on a binary image. There are speed-optimized versions, which work on bit-packed image structures.
*  @details			Border treatment: Borders are assumed to be zero. First and last out image line is always filled with zeros.
*					NB:	Byte-image algorithms are optimized for 'black'-dominated images. Automatic detection of dominating color can be added, algorithms exist.
*					NB:	Optimized and non-optimized alogorithms should provide exactly same results (checked with memcmp).
*/

#ifndef MORPHOLOGYIMPL_20130321_INCLUDED_H
#define MORPHOLOGYIMPL_20130321_INCLUDED_H

// local includes
#include "system/types.h"				///< byte
#include "image/image.h"				///< BImage, U32Image


namespace precitec {
	namespace filter {


typedef void (*PMorphFun)(const image::BImage& , image::BImage& );

/**
  *  @brief					Dilates a binary image. optimized for 'white'-dominated images. Sets pixels to '0' or '255'.
  *  @param p_rSource		Binary input image.
  *  @param p_rDestin		Binary output image.
  */
void dilatationWhite  (const image::BImage& p_rSource, image::BImage& p_rDestin);

/**
  *  @brief					Dilates a binary image. optimized for 'black'-dominated images. Sets pixels to '0' or '255'.
  *  @param p_rSource		Binary input image.
  *  @param p_rDestin		Binary output image.
  */
void dilatationBlack  (const image::BImage& p_rSource, image::BImage& p_rDestin);

/**
  *  @brief					Erodes a binary image. optimized for 'black'-dominated images. Sets pixels to '0' or '255'.
  *  @param p_rSource		Binary input image.
  *  @param p_rDestin		Binary output image.
  */
void erosionBlack     (const image::BImage& p_rSource, image::BImage& p_rDestin);

/**
  *  @brief					Erodes a binary image. optimized for 'white'-dominated images. Sets pixels to '0' or '255'.
  *  @param p_rSource		Binary input image.
  *  @param p_rDestin		Binary output image.
  */
void erosionWhite     (const image::BImage& p_rSource, image::BImage& p_rDestin);

/**
  *  @brief					Applies N-times the specified morphological function on a binary image.
  *  @param p_rSource		Binary input image.
  *  @param p_rDestin		Binary output image.
  *  @param pMorphFun		Function pointer to morphological function to be applied.
  *  @param p_oNbIterations	Defines, how often the specified morphological function is applied.
  */
void morph          (const image::BImage& p_rSource, image::BImage& p_rDestin, PMorphFun pMorphFun, byte p_oNbIterations);

/**
  *  @brief					Opens a binary image. optimized for 'black'-dominated images. Sets pixels to '0' or '255'.
  *  @param p_rSource		Binary input image.
  *  @param p_rDestin		Binary output image.
  *  @param p_oNbIterations	Defines, how often the involved morphological functions are applied.
  */
void opening        (const image::BImage& p_rSource, image::BImage& p_rDestin, int p_oNbIterations);

/**
  *  @brief					Closes a binary image. optimized for 'black'-dominated images. Sets pixels to '0' or '255'.
  *  @param p_rSource		Binary input image.
  *  @param p_rDestin		Binary output image.
  *  @param p_oNbIterations	Defines, how often the involved morphological functions are applied.
  */
void closing        (const image::BImage& p_rSource, image::BImage& p_rDestin, int p_oNbIterations);

/**
  *  @brief					Types of morphological operation.
  */enum MorphOperation { Erosion, Dilation };

/**
  *  @brief					Applies the specified morphological function on a binary image.
  *  @param p_rSource		Binary input image.
  *  @param p_rDestin		Binary output image.
  *  @param p_oMorphOperation	Function pointer to morphological function to be applied.
  */
void dilationErosionS2(const image::BImage& p_rSource, image::BImage& p_rDestin, MorphOperation p_oMorphOperation);

/**
  *  @brief					Applies the specified morphological function on a binary image.
  *  @param p_rSource		Binary input image.
  *  @param p_rDestin		Binary output image.
  *  @param p_oMorphOperation	Function pointer to morphological function to be applied.
 */
void dilationErosionS3(const image::BImage& p_rSource, image::BImage& p_rDestin, MorphOperation p_oMorphOperation);

/**
  *  @brief				Calculates the maximum of a data array.
  *  @param p_pData		Input data.
  *  @param p_oSize		Size of the data array.
  */
byte getMax(const byte *const p_pData, byte p_oSize);

/**
  *  @brief				Calculates the minimum of a data array.
  *  @param p_pData		Input data.
  *  @param p_oSize		Size of the data array.
  */
byte getMin(const byte *const p_pData, byte p_oSize);

/**
  *  @brief				Calculates the maximum of a data array.
  *  @param p_pData		Input data.
  *  @param p_oSize		Size of the data array.
  */
byte getMaxBinaer(const byte *const p_pData, byte p_oSize);

/**
  *  @brief				Calculates the minimum of a data array.
  *  @param p_pData		Input data.
  *  @param p_oSize		Size of the data array.
  */
byte getMinBinaer(const byte *const p_pData, byte p_oSize);

/**
  *  @brief					Opens a binary image Sets bits to '0' or '1'.
  *  @param p_rSrcPacked	Binary input image.
  *  @param p_rDstPacked	Binary output image.
  *  @param p_oNbIterations	Defines, how often the involved morphological functions are applied.
  */
void opening32(const image::U32Image& p_rSrcPacked, image::U32Image& p_rDstPacked, unsigned int p_oNbIterations);	///< 32-Bit-Parallel-Variante von Open

/**
  *  @brief					Erodes a binary image Sets bits to '0' or '1'.
  *  @param p_rSrcPacked	Binary input image.
  *  @param p_rDstPacked	Binary output image.
  */
void erode32(const image::U32Image& p_rSrcPacked, image::U32Image& p_rDstPacked);					///< 32-Bit-Parallel-Variante von Close

/**
  *  @brief					Dilates a binary image Sets bits to '0' or '1'.
  *  @param p_rSrcPacked	Binary input image.
  *  @param p_rDstPacked	Binary output image.
  */
void dilate32(const image::U32Image& p_rSrcPacked, image::U32Image& p_rDilate);					///< 32-Bit-Parallel-Variante von Dilation

/**
  *  @brief					Closes a binary image Sets bits to '0' or '1'.
  *  @param p_rSrcPacked	Binary input image.
  *  @param p_rDstPacked	Binary output image.
  *  @param p_oNbIterations	Defines, how often the involved morphological functions are applied.
  */
void closing32(const image::U32Image& p_rSrcPacked, image::U32Image& p_rDst,  unsigned int p_oNbIterations);	///< 32-Bit-Parallel-Variante von Oeffnung

/**
  *  @brief					Dilates a binary image Sets bits to '0' or '1'.
  *  @param p_rSource		Binary input image.
  *  @param p_rDstPacked	Binary output image.
  */
void bin2ToBin32(const image::BImage& p_rSrc, image::U32Image& p_rDstPacked);					///< 1Byte/Pixel --> 32-Pixel/Doppelwort

/**
  *  @brief					Dilates a binary image Sets bits to '0' or '1'.
  *  @param p_rSrcPacked	Binary input image.
  *  @param p_rDst			Binary output image.
  */
void bin32ToBin2(const image::U32Image& p_rSrcPacked, image::BImage& p_rDst);					///< 32-Pixel/Doppelwort --> 1Byte/Pixel

	} // namespace filter
} // namespace precitec


#endif  // MORPHOLOGYIMPL_20130321_INCLUDED_H
