/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS, KIR
 * 	@date		2014
 *	@brief		Defines layer types.
 */

#ifndef LAYERTYPE_H_20140113_INCLUDED
#define LAYERTYPE_H_20140113_INCLUDED


namespace precitec {
namespace image {

/**
 *	@brief		Defines overlay layers.
 *	@detail		There are 11 layers. Some opaque and transparent layers a toggled together (see group) on GUI.
 *
 *	name			layer		group		transparancy		example
 *
 *	Image			10			6			popup window
 *	InfoBox			9			5			popup window
 *	Text			8			3			transparent
 *	Position		7			2			transparent
 *	Contour			6			1			transparent
 *	Line			5			0			transparent
 *	Grid			4			4			transparent
 *	Text			3			3			opaque				legend, caption
 *	Position		2			2			opaque				line maximum, seam position
 *	Contour			1			1			opaque				laser line, pore contour
 *	Line			0			0			opaque				roi, bounding boxes
*/
enum LayerType { 
	eLayerLine			= 0, // and following will be used as array indices
	eLayerContour,
	eLayerPosition,
	eLayerText,
	eLayerGridTransp,
	eLayerLineTransp,
	eLayerContourTransp,
	eLayerPositionTransp,
	eLayerTextTransp,
	eLayerInfoBox,
	eLayerImage,

	eLayerMin	=	eLayerLine,
	eLayerMax	=	eLayerImage, 
	eNbLayers	=	eLayerMax + 1, 
	};

} // namespace image
} // namespace precitec

#endif /*LAYERTYPE_H_20140113_INCLUDED*/
