/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		HS
* 	@date		2017
* 	@brief		Filter 'SurfaceClassifier'. Gets a SurfaceInfo structure and checks the given limits
*/

// WM includes

#include "surfaceClassifier.h"

#include "fliplib/Parameter.h"
#include "overlay/overlayPrimitive.h"
#include "image/image.h"
#include "module/moduleLogger.h"
#include "filter/algoImage.h"
#include "filter/algoStl.h"
#include "system/typeTraits.h"
#include <fliplib/TypeToDataTypeImpl.h>

// stdlib includes

#include <algorithm>

using namespace fliplib;
namespace precitec
{
	using namespace geo2d;
	using namespace interface;
	using namespace image;
	namespace filter
{


const std::string SurfaceClassifier::m_oFilterName("SurfaceClassifier");		///< Filter name.
const std::string SurfaceClassifier::m_oPipeOutSizeConnectedName("SizeConnected");	///< Pipe: Scalar out-pipe.
const std::string SurfaceClassifier::m_oPipeOutWidthNioName("WidthNio");	///< Pipe: Scalar out-pipe.
const std::string SurfaceClassifier::m_oPipeOutHeightNioName("HeightNio");	///< Pipe: Scalar out-pipe.


SurfaceClassifier::SurfaceClassifier()
	:
	TransformFilter(m_oFilterName, Poco::UUID{"175cb7dd-149c-41a8-873f-8c9ced80b84a"}),
	m_pPipeInSurfaceInfo(nullptr),
	m_oPipeOutSizeConnected(this, m_oPipeOutSizeConnectedName),
	m_oPipeOutWidthNio(this, m_oPipeOutWidthNioName),
	m_oPipeOutHeightNio(this, m_oPipeOutHeightNioName),
	m_oDisplay(0),
	m_oUse4Neighborhood(true),
	m_oMinMean(0),
	m_oMaxMean(255),
	m_oMinRelInt(0),
	m_oMaxRelInt(1000),
	m_oMinVariation(0),
	m_oMaxVariation(1000),
	m_oMinMinMaxDistance(0),
	m_oMaxMinMaxDistance(255),
	m_oMinSurface(0),
	m_oMaxSurface(100),
	m_oMinSurfaceX(0),
	m_oMaxSurfaceX(100),
	m_oMinSurfaceY(0),
	m_oMaxSurfaceY(100),
	m_oMinTexture(0),
	m_oMaxTexture(100),
	m_oMinStructure(0),
	m_oMaxStructure(100)
	{
	parameters_.add("Display", Parameter::TYPE_int, m_oDisplay);
	parameters_.add("Use4Neighborhood", Parameter::TYPE_bool, m_oUse4Neighborhood);
	parameters_.add("MinMean", Parameter::TYPE_double, m_oMinMean);
	parameters_.add("MaxMean", Parameter::TYPE_double, m_oMaxMean);
	parameters_.add("MinRelInt", Parameter::TYPE_double, m_oMinRelInt);
	parameters_.add("MaxRelInt", Parameter::TYPE_double, m_oMaxRelInt);
	parameters_.add("MinVariation", Parameter::TYPE_double, m_oMinVariation);
	parameters_.add("MaxVariation", Parameter::TYPE_double, m_oMaxVariation);
	parameters_.add("MinMinMaxDistance", Parameter::TYPE_double, m_oMinMinMaxDistance);
	parameters_.add("MaxMinMaxDistance", Parameter::TYPE_double, m_oMaxMinMaxDistance);
	parameters_.add("MinSurface", Parameter::TYPE_double, m_oMinSurface);
	parameters_.add("MaxSurface", Parameter::TYPE_double, m_oMaxSurface);
	parameters_.add("MinSurfaceX", Parameter::TYPE_double, m_oMinSurfaceX);
	parameters_.add("MaxSurfaceX", Parameter::TYPE_double, m_oMaxSurfaceX);
	parameters_.add("MinSurfaceY", Parameter::TYPE_double, m_oMinSurfaceY);
	parameters_.add("MaxSurfaceY", Parameter::TYPE_double, m_oMaxSurfaceY);
	parameters_.add("MinTexture", Parameter::TYPE_double, m_oMinTexture);
	parameters_.add("MaxTexture", Parameter::TYPE_double, m_oMaxTexture);
	parameters_.add("MinStructure", Parameter::TYPE_double, m_oMinStructure);
	parameters_.add("MaxStructure", Parameter::TYPE_double, m_oMaxStructure);

    setInPipeConnectors({{Poco::UUID("50AEE76D-A5AF-4423-8206-19BB947D8E9E"), m_pPipeInSurfaceInfo, "SurfaceInfo", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("87DBE3CB-7E66-4DAB-8706-8E28C03C0590"), &m_oPipeOutSizeConnected, m_oPipeOutSizeConnectedName, 0, ""},
    {Poco::UUID("54CD3FD8-0DDD-4410-B1A8-0BDD429621C0"), &m_oPipeOutWidthNio, m_oPipeOutWidthNioName, 0, ""},
    {Poco::UUID("CC0C058A-5087-4605-AACC-A4F6CB0C3DAE"), &m_oPipeOutHeightNio, m_oPipeOutHeightNioName, 0, ""}});
    setVariantID(Poco::UUID("9dd11027-6150-45cc-aa04-a0ea254c22dd"));
}


void SurfaceClassifier::setParameter()
{
	using namespace std::placeholders;

	TransformFilter::setParameter();
	m_oDisplay = parameters_.getParameter("Display");
	m_oUse4Neighborhood = parameters_.getParameter("Use4Neighborhood");
	m_oMinMean = parameters_.getParameter("MinMean");
	m_oMaxMean = parameters_.getParameter("MaxMean");
	m_oMinRelInt = parameters_.getParameter("MinRelInt");
	m_oMaxRelInt = parameters_.getParameter("MaxRelInt");
	m_oMinVariation = parameters_.getParameter("MinVariation");
	m_oMaxVariation = parameters_.getParameter("MaxVariation");
	m_oMinMinMaxDistance = parameters_.getParameter("MinMinMaxDistance");
	m_oMaxMinMaxDistance = parameters_.getParameter("MaxMinMaxDistance");
	m_oMinSurface = parameters_.getParameter("MinSurface");
	m_oMaxSurface = parameters_.getParameter("MaxSurface");
	m_oMinSurfaceX = parameters_.getParameter("MinSurfaceX");
	m_oMaxSurfaceX = parameters_.getParameter("MaxSurfaceX");
	m_oMinSurfaceY = parameters_.getParameter("MinSurfaceY");
	m_oMaxSurfaceY = parameters_.getParameter("MaxSurfaceY");
	m_oMinTexture = parameters_.getParameter("MinTexture");
	m_oMaxTexture = parameters_.getParameter("MaxTexture");
	m_oMinStructure = parameters_.getParameter("MinStructure");
	m_oMaxStructure = parameters_.getParameter("MaxStructure");
} // setParameter.


bool SurfaceClassifier::subscribe(BasePipe& p_rPipe, int p_oGroup)
{ // Hat nur eine InPipe => SurfaceInfo wird ausgewertet
	m_pPipeInSurfaceInfo = dynamic_cast<const fliplib::SynchronePipe< interface::GeoSurfaceInfoarray >*>(&p_rPipe);

	return BaseFilter::subscribe(p_rPipe, p_oGroup);
} // subscribe


void SurfaceClassifier::proceed(const void* p_pSender, PipeEventArgs& p_rEvent)
{
	m_hasPainting = false;
	poco_assert_dbg(m_pPipeInSurfaceInfo != nullptr);

	const GeoSurfaceInfoarray&		rGeoSurfaceInfoIn(m_pPipeInSurfaceInfo->read(m_oCounter));
	geo2d::Doublearray oOutSizeConnected;
	geo2d::Doublearray oOutWidthNio;
	geo2d::Doublearray oOutHeightNio;
	_allInfoLines.clear();
	std::vector<SurfaceInfoLine> infoLinesPerSurface;

	m_oSpTrafo = rGeoSurfaceInfoIn.context().trafo();

	unsigned int oSizeOfArray = rGeoSurfaceInfoIn.ref().size();

	if ( (oSizeOfArray <= 0) || (rGeoSurfaceInfoIn.isValid() == false) ) // Probleme bei Eingangsdaten?
	{
		oOutSizeConnected.getData().push_back(0);
		oOutSizeConnected.getRank().push_back(0);

		oOutWidthNio.getData().push_back(0);
		oOutWidthNio.getRank().push_back(0);

		oOutHeightNio.getData().push_back(0);
		oOutHeightNio.getRank().push_back(0);

		const auto oSizeConnectedOut = GeoDoublearray(rGeoSurfaceInfoIn.context(), oOutSizeConnected, rGeoSurfaceInfoIn.analysisResult());
		const auto oWidthNioOut = GeoDoublearray(rGeoSurfaceInfoIn.context(), oOutWidthNio, rGeoSurfaceInfoIn.analysisResult());
		const auto oHeightNioOut = GeoDoublearray(rGeoSurfaceInfoIn.context(), oOutHeightNio, rGeoSurfaceInfoIn.analysisResult());
		preSignalAction();
		m_oPipeOutSizeConnected.signal(oSizeConnectedOut);
		m_oPipeOutWidthNio.signal(oWidthNioOut);
		m_oPipeOutHeightNio.signal(oHeightNioOut);

		return;
	}

	m_oInInfo = std::get<eData>(rGeoSurfaceInfoIn.ref()[0]);

	TileContainer tileContainer = m_oInInfo.getTileContainer();

	if (tileContainer.m_sizeY * tileContainer.m_sizeX > 0) m_hasPainting = true;

	image::Color col = Color::Black();
	int value;

	for (int j = 0; j < tileContainer.m_sizeY; j++)
	{
		for (int i = 0; i < tileContainer.m_sizeX; i++)
		{
			SingleTile tile = tileContainer.getSingleTile(i, j);
			infoLinesPerSurface.clear();

			infoLinesPerSurface.push_back(SurfaceInfoLine(1, tile.m_number, Color::Black()));

			col = Color::Black();
			value = 0;
			if (tile.m_isMeanValid) // Mittelwert wurde berechnet
			{
				value = (int)tile.m_MeanValue;
				if (m_oInInfo.usesMean)
				{
					col = Color::Green();
					if (tile.m_MeanValue < m_oMinMean || tile.m_MeanValue > m_oMaxMean) // Wert faellt aus Grenzen, fehlerhaft
					{
						tile.setMeanDefect();
						tile.setDefect();
						col = Color::Red();
					}
				}
			}
			infoLinesPerSurface.push_back(SurfaceInfoLine(2, value, col));

			if (tile.m_isMeanValid) // Mittelwert wurde berechnet
			{
				col = Color::Black();
				value = 0;
				if (m_oInInfo.usesRelBrightness)
				{
					value = (int)tile.m_relIntensity;
					col = Color::Green();
					if (tile.m_relIntensity < m_oMinRelInt || tile.m_relIntensity > m_oMaxRelInt) // Wert faellt aus Grenzen, fehlerhaft
					{
						tile.setRelIntDefect();
						tile.setDefect();
						col = Color::Red();
					}
				}
			}
			infoLinesPerSurface.push_back(SurfaceInfoLine(3, value, col));

			col = Color::Black();
			value = 0;
			if (m_oInInfo.usesVariation)
			{
				value = tile.m_Variation;
				col = Color::Green();
				if (tile.m_Variation < m_oMinVariation || tile.m_Variation > m_oMaxVariation) // Wert faellt aus Grenzen, fehlerhaft
				{
					tile.setVariationDefect();
					tile.setDefect();
					col = Color::Red();
				}
			}
			infoLinesPerSurface.push_back(SurfaceInfoLine(4, value, col));

			col = Color::Black();
			value = 0;
			if (m_oInInfo.usesMinMaxDistance)
			{
				value = (int)tile.m_MinMaxDistance;
				col = Color::Green();
				if (tile.m_MinMaxDistance < m_oMinMinMaxDistance || tile.m_MinMaxDistance > m_oMaxMinMaxDistance) // Wert faellt aus Grenzen, fehlerhaft
				{
					tile.setMinMaxDistanceDefect();
					tile.setDefect();
					col = Color::Red();
				}
			}
			infoLinesPerSurface.push_back(SurfaceInfoLine(5, value, col));

			col = Color::Black();
			value = 0;
			if (m_oInInfo.usesSurface)
			{
				value = tile.m_Surface;
				col = Color::Green();
				if (tile.m_Surface < m_oMinSurface || tile.m_Surface > m_oMaxSurface) // Wert faellt aus Grenzen, fehlerhaft
				{
					tile.setSurfaceDefect();
					tile.setDefect();
					col = Color::Red();
				}
			}
			infoLinesPerSurface.push_back(SurfaceInfoLine(6, value, col));

			col = Color::Black();
			value = 0;
			if (m_oInInfo.usesSurfaceX)
			{
				value = (int)tile.m_SurfaceX;
				col = Color::Green();
				if (tile.m_SurfaceX < m_oMinSurfaceX || tile.m_SurfaceX > m_oMaxSurfaceX) // Wert faellt aus Grenzen, fehlerhaft
				{
					tile.setSurfaceXDefect();
					tile.setDefect();
					col = Color::Red();
				}
			}
			infoLinesPerSurface.push_back(SurfaceInfoLine(7, value, col));

			col = Color::Black();
			value = 0;
			if (m_oInInfo.usesSurfaceY)
			{
				value = (int)tile.m_SurfaceY;
				col = Color::Green();
				if (tile.m_SurfaceY < m_oMinSurfaceY || tile.m_SurfaceY > m_oMaxSurfaceY) // Wert faellt aus Grenzen, fehlerhaft
				{
					tile.setSurfaceYDefect();
					tile.setDefect();
					col = Color::Red();				}
			}
			infoLinesPerSurface.push_back(SurfaceInfoLine(8, value, col));

			col = Color::Black();
			value = 0;
			if (m_oInInfo.usesTexture)
			{
				value = (int)tile.m_Texture;
				col = Color::Green();
				if (tile.m_Texture < m_oMinTexture || tile.m_Texture > m_oMaxTexture) // Wert faellt aus Grenzen, fehlerhaft
				{
					tile.setTextureDefect();
					tile.setDefect();
					col = Color::Red();
				}
			}
			infoLinesPerSurface.push_back(SurfaceInfoLine(9, value, col));

			col = Color::Black();
			value = 0;
			if (m_oInInfo.usesStructure)
			{
				value = (int)tile.m_Structure;
				col = Color::Green();
				if (tile.m_Structure < m_oMinStructure || tile.m_Structure > m_oMaxStructure) // Wert faellt aus Grenzen, fehlerhaft
				{
					tile.setStructureDefect();
					tile.setDefect();
					col = Color::Red();
				}
			}
			infoLinesPerSurface.push_back(SurfaceInfoLine(10, value, col));

			tileContainer.putSingleTile(i, j, tile);

			_allInfoLines.push_back(infoLinesPerSurface);
		}
	}


	// Ansammlungen von Fehlern taggen
	tagBlobs(tileContainer);

	// Groesste zusammenhaengende Fehler-Ansammlung suchen
	int biggestBlobNumber = findBiggestBlob(tileContainer);
	// alle Blobs loeschen, nur den groessten nicht
	deleteBlobNumbersExceptGiven(tileContainer, biggestBlobNumber);

	// Groesse des Groessten bestimmten (ist nur och uebrig)
	int blobWidth = 0, blobHeight = 0;
	int biggestBlobSize = getTotalBlobSize(tileContainer, blobWidth, blobHeight);

	m_tileContainer = tileContainer; // letzten TileContainer halten fuer Paint-Routine

	oOutSizeConnected.getData().push_back(biggestBlobSize);
	oOutSizeConnected.getRank().push_back(255);

	oOutWidthNio.getData().push_back(blobWidth);
	oOutWidthNio.getRank().push_back(255);

	oOutHeightNio.getData().push_back(blobHeight);
	oOutHeightNio.getRank().push_back(255);

	// update sampling factors in context
	const auto oSizeConnectedOut = GeoDoublearray(rGeoSurfaceInfoIn.context(), oOutSizeConnected, rGeoSurfaceInfoIn.analysisResult());
	const auto oWidthNioOut = GeoDoublearray(rGeoSurfaceInfoIn.context(), oOutWidthNio, rGeoSurfaceInfoIn.analysisResult());
	const auto oHeightNioOut = GeoDoublearray(rGeoSurfaceInfoIn.context(), oOutHeightNio, rGeoSurfaceInfoIn.analysisResult());
	preSignalAction();
	m_oPipeOutSizeConnected.signal(oSizeConnectedOut);			// invoke linked filter(s)
	m_oPipeOutWidthNio.signal(oWidthNioOut);			// invoke linked filter(s)
	m_oPipeOutHeightNio.signal(oHeightNioOut);			// invoke linked filter(s)

} // proceed


void SurfaceClassifier::paint()
{
	if (!m_hasPainting) return;
	m_hasPainting = false;

	try
	{

		if (m_oVerbosity < eLow || m_oSpTrafo.isNull())
		{
			return;
		} // if

		const Trafo		&rTrafo(*m_oSpTrafo);

		OverlayCanvas&			rOverlayCanvas = canvas<OverlayCanvas>(m_oCounter);
		OverlayLayer&			rLayerLine = rOverlayCanvas.getLayerLine();
		OverlayLayer&			rLayerText = rOverlayCanvas.getLayerText();
		OverlayLayer&           rLayerInfoBox(rOverlayCanvas.getLayerInfoBox());

		for (int j = 0; j < m_tileContainer.m_sizeY; j++)
		{
			for (int i = 0; i < m_tileContainer.m_sizeX; i++)
			{
				const SingleTile &tile = m_tileContainer.getSingleTile(i, j);
				const Rect			oTileRoi(tile.m_startX, tile.m_startY, tile.m_width, tile.m_height);
				const Rect			oTileRoi_1(tile.m_startX + 1, tile.m_startY + 1, tile.m_width, tile.m_height);

				const Rect			oTileRoi2(tile.m_startX, tile.m_startY + 11, tile.m_width, tile.m_height);
				const Rect			oTileRoi2_1(tile.m_startX + 1, tile.m_startY + 12, tile.m_width, tile.m_height);

				const Font			oFont(12);

				Color			oColor = Color::Green();
				if (tile.m_isDefect) oColor = Color::Yellow();
				if (tile.m_blobNumber != 0) oColor = Color::Red();

				rLayerLine.add<OverlayRectangle>(rTrafo(oTileRoi), oColor);

				int displayValue = 0;
				bool isValid = false;

				if (m_oDisplay == 0)
				{
					displayValue = (int)(0.5 + tile.m_number);
					isValid = true;
					oColor = Color::Green();
					if (tile.m_isMeanDefect) oColor = Color::Red();
				}

				if (m_oDisplay == 1)
				{
					if (tile.m_isMeanValid)
					{
						displayValue = (int)(0.5 + tile.m_MeanValue);
						isValid = true;
						oColor = Color::Green();
						if (tile.m_isMeanDefect) oColor = Color::Red();
					}
				}

				if (m_oDisplay == 2)
				{
					if (tile.m_isRelIntensityValid)
					{
						displayValue = (int)(0.5 + tile.m_relIntensity);
						isValid = true;
						oColor = Color::Green();
						if (tile.m_isRelIntDefect) oColor = Color::Red();
					}
				}

				if (m_oDisplay == 3)
				{
					if (tile.m_isVariationValid)
					{
						displayValue = (int)(0.5 + tile.m_Variation);
						isValid = true;
						oColor = Color::Green();
						if (tile.m_isVariationDefect) oColor = Color::Red();
					}
				}

				if (m_oDisplay == 4)
				{
					if (tile.m_isMinMaxDistanceValid)
					{
						displayValue = (int)(0.5 + tile.m_MinMaxDistance);
						isValid = true;
						oColor = Color::Green();
						if (tile.m_isMinMaxDistanceDefect) oColor = Color::Red();
					}
				}

				if (m_oDisplay == 5)
				{
					if (tile.m_isSurfaceValid)
					{
						displayValue = (int)(0.5 + tile.m_Surface);
						isValid = true;
						oColor = Color::Green();
						if (tile.m_isSurfaceDefect) oColor = Color::Red();
					}
				}

				if (m_oDisplay == 6)
				{
					if (tile.m_isSurfaceXValid)
					{
						displayValue = (int)(0.5 + tile.m_SurfaceX);
						isValid = true;
						oColor = Color::Green();
						if (tile.m_isSurfaceXDefect) oColor = Color::Red();
					}
				}

				if (m_oDisplay == 7)
				{
					if (tile.m_isSurfaceYValid)
					{
						displayValue = (int)(0.5 + tile.m_SurfaceY);
						isValid = true;
						oColor = Color::Green();
						if (tile.m_isSurfaceYDefect) oColor = Color::Red();
					}
				}

				if (m_oDisplay == 8)
				{
					if (tile.m_isTextureValid)
					{
						displayValue = (int)(0.5 + tile.m_Texture);
						isValid = true;
						oColor = Color::Green();
						if (tile.m_isTextureDefect) oColor = Color::Red();
					}
				}

				if (m_oDisplay == 9)
				{
					if (tile.m_isStructureValid)
					{
						displayValue = (int)(0.5 + tile.m_Structure);
						isValid = true;
						oColor = Color::Green();
						if (tile.m_isStructureDefect) oColor = Color::Red();
					}
				}

				if (isValid)
				{
					rLayerText.add<OverlayText>(std::to_string(displayValue), oFont, rTrafo(oTileRoi_1), Color::Black());
					rLayerText.add<OverlayText>(std::to_string(displayValue), oFont, rTrafo(oTileRoi), oColor);
				}

				int size = _allInfoLines.size();

				const auto &infoLinesPerSurface = (tile.m_number - 1 < size) ? _allInfoLines[tile.m_number - 1] : _allInfoLines[size - 1];

				const auto		oBoundingBoxStart = Point(tile.m_startX, tile.m_startY);
				const auto		oBoundingBoxEnd = Point(tile.m_startX + tile.m_width, tile.m_startY + tile.m_height);
				const auto		oBoundingBox = Rect(oBoundingBoxStart, oBoundingBoxEnd);

				std::vector<OverlayText> oFeatureLines = std::vector<OverlayText>(infoLinesPerSurface.size() + 1);

				oFeatureLines[0] = OverlayText(id().toString() + ":FILTERGUID:0", Font(12, true, false, "Courier New"), rTrafo(Rect(10, 10, 100, 200)), Color::Black(), 0);

				for (int i = 0; i < (int)infoLinesPerSurface.size(); i++)
				{
					oFeatureLines[i + 1] = OverlayText(infoLinesPerSurface[i].getLine(), Font(12, true, false, "Courier New"), rTrafo(Rect(10, 10, 100, 200)), infoLinesPerSurface[i]._color, i + 1);
				}

				rLayerInfoBox.add<OverlayInfoBox>(image::eSurface, tile.m_number - 1, std::move(oFeatureLines), rTrafo(oBoundingBox));




			} // for i = 0...SizeX
		}
	}
	catch (...)
	{
		// Paint ging schief => nur loggen
		wmLog(eWarning, "Exception during painting in surfaceClassifier");
	}
} // paint

// Helper Functions

void SurfaceClassifier::tagBlobs(TileContainer & tileContainer)
{
	bool use8Neighborhood = !m_oUse4Neighborhood;
	int curBlobNumber = 1 + getMaxBlobNumber(tileContainer);
	SingleTile otherTile;

	for (int j = 0; j < tileContainer.m_sizeY; j++)
	{
		for (int i = 0; i < tileContainer.m_sizeX; i++)
		{
			SingleTile currentTile = tileContainer.getSingleTile(i, j);

			if ((currentTile.m_isDefect) && (currentTile.m_blobNumber == 0)) // defektes Tile ohne Blobnummer
			{
				// links checken
				if (i - 1 >= 0) // es gibt eins links
				{
					otherTile = tileContainer.getSingleTile(i - 1, j);

					if (otherTile.m_isDefect) // links ist defekt, muss getagged sein...
					{
						currentTile.m_blobNumber = otherTile.m_blobNumber;
					}
				}

				// links-oben checken (bei entsprechender Nachbarschaft)
				if (use8Neighborhood && (i - 1 >= 0) && (j - 1 >= 0)) // es gibt eins links drueber
				{
					otherTile = tileContainer.getSingleTile(i - 1, j - 1);

					if (otherTile.m_isDefect) // linksoben ist defekt, muss getagged sein, akt. kann getagged sein
					{
						if (currentTile.m_blobNumber == 0) // ist noch ohne Tag (links war nicht)
						{
							currentTile.m_blobNumber = otherTile.m_blobNumber;
						}
						else // akt. hat Nummer, drueber auch, checken, ob die beide gleich sind. Wenn nicht dafuer sorgen, dass sie gleich werden
						{
							if (currentTile.m_blobNumber != otherTile.m_blobNumber) // zwei benachbarte gefunden, die verschiedene Nummern haben => aufraeumen
							{
								tileContainer.putSingleTile(i, j, currentTile);
								combineEqualBlobs(tileContainer, currentTile.m_blobNumber, otherTile.m_blobNumber);
								currentTile = tileContainer.getSingleTile(i, j);
								curBlobNumber = 1 + getMaxBlobNumber(tileContainer); // schauen, ob akt. Blobnummer noch aktuell
							}
						}
					}
				}

				// oben checken
				if (j - 1 >= 0) // es gibt eins drueber
				{
					otherTile = tileContainer.getSingleTile(i, j - 1);

					if (otherTile.m_isDefect) // oben ist defekt, muss getagged sein, akt. kann getagged sein
					{
						if (currentTile.m_blobNumber == 0) // ist noch ohne Tag (links war nicht)
						{
							currentTile.m_blobNumber = otherTile.m_blobNumber;
						}
						else // akt. hat Nummer, drueber auch, checken, ob die beide gleich sind. Wenn nicht dafuer sorgen, dass sie gleich werden
						{
							if (currentTile.m_blobNumber != otherTile.m_blobNumber) // zwei benachbarte gefunden, die verschiedene Nummern haben => aufraeumen
							{
								tileContainer.putSingleTile(i, j, currentTile);
								combineEqualBlobs(tileContainer, currentTile.m_blobNumber, otherTile.m_blobNumber);
								currentTile = tileContainer.getSingleTile(i, j);
								curBlobNumber = 1 + getMaxBlobNumber(tileContainer); // schauen, ob akt. Blobnummer noch aktuell
							}
						}
					}
				}

				// rechts-oben checken (bei entsprechender Nachbarschaft)
				if (use8Neighborhood && (i + 1 < tileContainer.m_sizeX) && (j - 1 >= 0)) // es gibt eins rechts drueber
				{
					otherTile = tileContainer.getSingleTile(i + 1, j - 1);

					if (otherTile.m_isDefect) // rechtsoben ist defekt, muss getagged sein, akt. kann getagged sein
					{
						if (currentTile.m_blobNumber == 0) // ist noch ohne Tag (links war nicht)
						{
							currentTile.m_blobNumber = otherTile.m_blobNumber;
						}
						else // akt. hat Nummer, drueber auch, checken, ob die beide gleich sind. Wenn nicht dafuer sorgen, dass sie gleich werden
						{
							if (currentTile.m_blobNumber != otherTile.m_blobNumber) // zwei benachbarte gefunden, die verschiedene Nummern haben => aufraeumen
							{
								tileContainer.putSingleTile(i, j, currentTile);
								combineEqualBlobs(tileContainer, currentTile.m_blobNumber, otherTile.m_blobNumber);
								currentTile = tileContainer.getSingleTile(i, j);
								curBlobNumber = 1 + getMaxBlobNumber(tileContainer); // schauen, ob akt. Blobnummer noch aktuell
							}
						}
					}
				}

				// alle Nachbarn durch, schauen, ob noch immer keine Nummer fuer aktuelles Tile vorliegt
				if (currentTile.m_blobNumber == 0)
				{
					currentTile.m_blobNumber = curBlobNumber;
					curBlobNumber++;
				}
			}
			tileContainer.putSingleTile(i, j, currentTile);
		}
	}
}

int SurfaceClassifier::findBiggestBlob(TileContainer & tileContainer)
{ // Summiert getrennt fuer jede Blobnummer die Anzahl Tiles. Gibt Blobnummer mit den meisten Tiles zurueck.
	int maxNumber = getMaxBlobNumber(tileContainer);

	std::vector<int> size(maxNumber+1);
	for (int i = 0; i < maxNumber + 1; i++) size[i] = 0;

	for (int j = 0; j < tileContainer.m_sizeY; j++)
	{
		for (int i = 0; i < tileContainer.m_sizeX; i++)
		{
			size[tileContainer.getSingleTile(i, j).m_blobNumber]++;
		}
	}

	int maxSize = 0;
	int maxIndex = 0;

	for (int i = 1; i < maxNumber + 1; i++) // bei 1 starten! 0 ist keine gueltige Blobnummer, sondern Initialisierung
	{
		if (size[i] > maxSize)
		{
			maxSize = size[i];
			maxIndex = i;
		}
	}
	return maxIndex;
}

void SurfaceClassifier::deleteBlobNumbersExceptGiven(TileContainer & tileContainer, int givenNumber)
{ // Setzt alle Blobummern der Tiles auf Null ausser es ist die gegebene Blobnummer. Entfernt quasi alle Blobs bis auf einen.
	for (int j = 0; j < tileContainer.m_sizeY; j++)
	{
		for (int i = 0; i < tileContainer.m_sizeX; i++)
		{
			SingleTile currentTile = tileContainer.getSingleTile(i, j);
			if (currentTile.m_blobNumber != givenNumber) currentTile.m_blobNumber = 0;
			tileContainer.putSingleTile(i, j, currentTile);
		}
	}
}

int SurfaceClassifier::getTotalBlobSize(TileContainer & tileContainer, int & blobWidth, int & blobHeight)
{ // bestimmt die gesamte Anzahl an Tiles, die eine Blobnummer ungleich Null haben und damit zu irgendeinem Blob gehoeren.
	int count = 0;
	int maxJ = -10000;
	int minJ = 10000;
	int maxI = -10000;
	int minI = 10000;

	for (int j = 0; j < tileContainer.m_sizeY; j++)
	{
		for (int i = 0; i < tileContainer.m_sizeX; i++)
		{
			const SingleTile &currentTile = tileContainer.getSingleTile(i, j);
			if (currentTile.m_blobNumber != 0)
			{
				if (i < minI) minI = i;
				if (i > maxI) maxI = i;
				if (j < minJ) minJ = j;
				if (j > maxJ) maxJ = j;
				count++;
			}
		}
	}
	blobWidth = 1 + maxI - minI;
	blobHeight = 1 + maxJ - minJ;

	if ((blobWidth<0) || (blobWidth>100) || (blobHeight<0) || (blobHeight>100))
	{
		blobWidth = 0;
		blobHeight = 0;
	}

	return count;
}

void SurfaceClassifier::combineEqualBlobs(TileContainer & tileContainer, int firstNumber, int secondNumber)
{ // geht den Container durch und ersetzt eine Blobnummer mit der anderen. Es sollte also woanders bereits geklaert worden sein, dass die auch gleich sind
	if (firstNumber == secondNumber) return; // nix zu veraendern, wenn beide gleich

	// es soll immer die groessere durch die kleinere ersetzt werden
	int smallerNumber = firstNumber < secondNumber ? firstNumber : secondNumber;
	int biggerNumber = firstNumber > secondNumber ? firstNumber : secondNumber;

	for (int j = 0; j < tileContainer.m_sizeY; j++)
	{
		for (int i = 0; i < tileContainer.m_sizeX; i++)
		{
			SingleTile tile = tileContainer.getSingleTile(i, j);
			if (tile.m_blobNumber == biggerNumber) tile.m_blobNumber = smallerNumber;
			tileContainer.putSingleTile(i, j, tile);
		}
	}
}

int SurfaceClassifier::getMaxBlobNumber(TileContainer & tileContainer)
{ // geht ueber alle Blobnummern und gibt die groesste zurueck
	int maxNumber = 0;
	for (int j = 0; j < tileContainer.m_sizeY; j++)
	{
		for (int i = 0; i < tileContainer.m_sizeX; i++)
		{
			const SingleTile &tile = tileContainer.getSingleTile(i, j);
			if (tile.m_blobNumber > maxNumber) maxNumber = tile.m_blobNumber;
		}
	}
	return maxNumber;
}






////////////////////////////////////////////////
// Klasse InfoLine
// Haelt eine Zeile einer Overlay-TextBox-Ausgabe
////////////////////////////////////////////////

SurfaceInfoLine::SurfaceInfoLine()
    : _number(0)
    , _value(0)
    , _color(Color::Black())
{
}

SurfaceInfoLine::SurfaceInfoLine(int number, int value, Color color)
    : _number(number)
    , _value(value)
    , _color(color)
{
}

std::string SurfaceInfoLine::getLine() const
{
	switch (_number)
	{
	case 0:
		return "Unit.None:Unit.None:0";
		break;
	case 1:
		return "TileNumber:Unit.None:" + std::to_string(_value);
		break;
	case 2:
		return "Mean:Unit.None:" + std::to_string(_value);
		break;
	case 3:
		return "RelBrightness:Unit.None:" + std::to_string(_value);
		break;
	case 4:
		return "Variation:Unit.None:" + std::to_string(_value);
		break;
	case 5:
		return "MinMaxDist:Unit.None:" + std::to_string(_value);
		break;
	case 6:
		return "SurfaceXY:Unit.None:" + std::to_string(_value);
		break;
	case 7:
		return "SurfaceX:Unit.None:" + std::to_string(_value);
		break;
	case 8:
		return "SurfaceY:Unit.None:" + std::to_string(_value);
		break;
	case 9:
		return "Texture:Unit.None:" + std::to_string(_value);
		break;
	case 10:
		return "Strukture:Unit.None:" + std::to_string(_value);
		break;
	}
	return "Unit.None:Unit.None:0";
}

//////////////////////////
// Overlay
//////////////////////////

SurfaceOverlay::SurfaceOverlay()
{
	reset();
}

void SurfaceOverlay::reset()
{
	_pointContainer.clear();
	_lineContainer.clear();
	_rectangleContainer.clear();
}

std::vector<SurfacePoint> SurfaceOverlay::getPointContainer()
{
	return _pointContainer;
}

std::vector<SurfaceLine> SurfaceOverlay::getLineContainer()
{
	return _lineContainer;
}

std::vector<SurfaceRectangle> SurfaceOverlay::getRectangleContainer()
{
	return _rectangleContainer;
}

void SurfaceOverlay::addPoint(int x, int y, Color color)
{
	_pointContainer.emplace_back(x, y, color);
}

void SurfaceOverlay::addLine(int x1, int y1, int x2, int y2, Color color)
{
	_lineContainer.emplace_back(x1, y1, x2, y2, color);
}

void SurfaceOverlay::addRectangle(int x, int y, int width, int height, Color color)
{
	_rectangleContainer.emplace_back(x, y, width, height, color);
}

SurfacePoint::SurfacePoint()
    : x(0)
    , y(0)
    , color(Color::Black())
{
}

SurfacePoint::SurfacePoint(int x, int y, Color color)
    : x(x)
    , y(y)
    , color(color)
{
}

SurfaceLine::SurfaceLine()
    : x1(0)
    , y1(0)
    , x2(0)
    , y2(0)
    , color(Color::Black())
{
}

SurfaceLine::SurfaceLine(int x1, int y1, int x2, int y2, Color color)
    : x1(x1)
    , y1(y1)
    , x2(x2)
    , y2(y2)
    , color(color)
{
}

SurfaceRectangle::SurfaceRectangle()
    : x(0)
    , y(0)
    , width(0)
    , height(0)
    , color(Color::Black())
{
}

SurfaceRectangle::SurfaceRectangle(int x, int y, int width, int height, Color color)
    : x(x)
    , y(y)
    , width(width)
    , height(height)
    , color(color)
{
}






} // namespace filter
} // namespace precitec
