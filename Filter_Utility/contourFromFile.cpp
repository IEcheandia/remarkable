#include "contourFromFile.h"

#include "module/moduleLogger.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "filter/algoArray.h"
#include "json.hpp"
#include <fstream>
#include "system/tools.h" //pwd
#include <algorithm>
#include <filter/armStates.h>
#include "ramping.h"

#include <fliplib/TypeToDataTypeImpl.h>

namespace
{
    using precitec::wmLog;
    using json = nlohmann::ordered_json;

    struct WeldingPoint
    {
        std::pair<double,double> endPosition;
        double power;
        double ringPower;
        double velocity;
    };
    struct Ramp
    {
        std::size_t startPointID;
        double length;
        double startPower;
        double endPower;
        double startPowerRing;
        double endPowerRing;
    };
    struct SeamCoordinates
    {
        std::string name;
        std::string ID;
        std::string description;
        std::vector<Ramp> ramps;
        std::vector<WeldingPoint> figure;
    };

    void from_json(const nlohmann::ordered_json &j, WeldingPoint& o)
    {
        j.at("EndPosition").get_to(o.endPosition);
        j.at("Power").get_to(o.power);
        if (j.contains("RingPower"))
        {
            j.at("RingPower").get_to(o.ringPower);
        }
        else
        {
            o.ringPower = -1;
        }
        if (j.contains("Velocity"))
        {
            j.at("Velocity").get_to(o.velocity);
        }
        else
        {
            o.velocity = -1;
        }
    };

    void from_json(const nlohmann::ordered_json &j, Ramp& o)
    {
        j.at("StartID").get_to(o.startPointID);
        j.at("Length").get_to(o.length);
        j.at("StartPower").get_to(o.startPower);
        j.at("EndPower").get_to(o.endPower);
        j.at("StartPowerRing").get_to(o.startPowerRing);
        j.at("EndPowerRing").get_to(o.endPowerRing);
    }

    void from_json(const nlohmann::ordered_json &j, SeamCoordinates& o)
    {
        j.at("Name").get_to(o.name);
        j.at("ID").get_to(o.ID);
        j.at("Description").get_to(o.description);
        if (j.contains("Ramps"))
        {
            j.at("Ramps").get_to(o.ramps);
        }
        j.at("Figure").get_to(o.figure);
    }


    bool readFromFile(SeamCoordinates & rSeamCoordinates, const std::string& filename)
    {
        std::ifstream file;
        file.open(filename);
        if (!file.is_open())
        {
            std::ostringstream oMsg;
            oMsg << "Could not open " << filename << " for reading \n";
            precitec::wmLog(precitec::eWarning, oMsg.str());
            return false;
        }
        nlohmann::ordered_json j;
        try
        {
            file >> j;
            rSeamCoordinates = j;
        }

        catch(nlohmann::json::exception & e)
        {
            file.close();
            std::ostringstream oMsg;
            oMsg << "Exception when reading " << filename << ": " << e.what()
                  << "(id: " << e.id << ") \n";
            precitec::wmLog(precitec::eWarning, oMsg.str());
            return false;
        }
        catch (...)
        {
            file.close();
            precitec::system::logExcpetion("Read " + filename, std::current_exception());
            return false;
        }
        file.close();
        return true;
    }

    const auto attributeLaserPower = precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPower;
    const auto attributeLaserPowerRing = precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPowerRing;
    const auto attributeLaserVelocity = precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity;

    bool fuzzyCompare(double d1, double d2)
    {
        return (std::abs(d1 - d2) * 100000.f <= std::min(std::abs(d1), std::abs(d2)));
    }

    bool fuzzyCompare(const precitec::geo2d::TPoint<double>& p1, const precitec::geo2d::TPoint<double>& p2)
    {
        return (fuzzyCompare(p1.x, p2.x) && fuzzyCompare(p1.y, p2.y));
    }

    precitec::geo2d::AnnotatedDPointarray computeContourWithRamps(const precitec::geo2d::AnnotatedDPointarray& contour, const std::vector<Ramp>& ramps)
    {
        //Create ramps to add them to the contour later
        Ramping ramp;
        std::vector<precitec::geo2d::AnnotatedDPointarray> rampContours;
        rampContours.reserve(ramps.size());
        std::size_t additionalPointsToContour = 0;

        for (const auto& currentRamp : ramps)
        {
            ramp.setLength(currentRamp.length);
            ramp.setStartPower(currentRamp.startPower);
            ramp.setEndPower(currentRamp.endPower);
            ramp.setStartPowerRing(currentRamp.startPowerRing);
            ramp.setEndPowerRing(currentRamp.endPowerRing);

            additionalPointsToContour += static_cast<std::size_t>(std::round(ramp.length() / ramp.rampStep()));

            if (currentRamp.startPointID == contour.size() - 1)             //Important for ramp out!
            {
                rampContours.emplace_back(ramp.reversePoints(ramp.createRamp(ramp.reversePoints(contour), 0)));
                rampContours.back().getScalarData(attributeLaserVelocity) = ramp.changeOrderRampOutVelocity(contour, rampContours.back());
            }
            else
            {
                rampContours.emplace_back(ramp.createRamp(contour, currentRamp.startPointID));
            }
        }

        precitec::geo2d::AnnotatedDPointarray contourWithRamps;
        contourWithRamps.reserve(contour.size() + additionalPointsToContour);
        contourWithRamps.insertScalar(attributeLaserPower);
        contourWithRamps.insertScalar(attributeLaserPowerRing);
        contourWithRamps.insertScalar(attributeLaserVelocity);

        //Add ramps to contour
        std::size_t currentRampIndex = 0;
        for (std::size_t i = 0; i < contour.size(); i++)
        {
            const auto& contourPoint = contour.getData().at(i);
            const auto& contourPower = contour.getScalarData(attributeLaserPower).at(i);
            const auto& contourPowerRing = contour.getScalarData(attributeLaserPowerRing).at(i);
            const auto& contourVelocity = contour.getScalarData(attributeLaserVelocity).at(i);
            //Check if point is already in the contour!
            if (currentRampIndex != 0)
            {
                const auto& lastRampData = rampContours.at(currentRampIndex - 1).getData();
                auto isAlreadyInserted = std::any_of(lastRampData.begin(), lastRampData.end(), [contourPoint](const auto& alreadyInsertedPoint)
                {
                    return fuzzyCompare(contourPoint, alreadyInsertedPoint);
                });
                if (isAlreadyInserted)
                {
                    continue;
                }
            }

            if (currentRampIndex >= rampContours.size())
            {
                contourWithRamps.getData().emplace_back(contourPoint);
                contourWithRamps.getRank().push_back(precitec::filter::eRankMax);
                contourWithRamps.getScalarData(attributeLaserPower).emplace_back(contourPower);
                contourWithRamps.getScalarData(attributeLaserPowerRing).emplace_back(contourPowerRing);
                contourWithRamps.getScalarData(attributeLaserVelocity).emplace_back(contourVelocity);
                continue;
            }

            const auto& currentRamp = rampContours.at(currentRampIndex);
            auto contains = std::any_of(currentRamp.getData().begin(), currentRamp.getData().end(), [contourPoint](const auto& rampPoint)
            {
                return fuzzyCompare(contourPoint, rampPoint);
            });

            if (!contains)
            {
                contourWithRamps.getData().emplace_back(contourPoint);
                contourWithRamps.getRank().push_back(precitec::filter::eRankMax);
                contourWithRamps.getScalarData(attributeLaserPower).emplace_back(contourPower);
                contourWithRamps.getScalarData(attributeLaserPowerRing).emplace_back(contourPowerRing);
                contourWithRamps.getScalarData(attributeLaserVelocity).emplace_back(contourVelocity);
            }
            else
            {
                std::copy(currentRamp.getData().begin(), currentRamp.getData().end(), std::back_inserter(contourWithRamps.getData()));
                std::copy(currentRamp.getRank().begin(), currentRamp.getRank().end(), std::back_inserter(contourWithRamps.getRank()));
                std::copy(currentRamp.getScalarData(attributeLaserPower).begin(), currentRamp.getScalarData(attributeLaserPower).end(), std::back_inserter(contourWithRamps.getScalarData(attributeLaserPower)));
                std::copy(currentRamp.getScalarData(attributeLaserPowerRing).begin(), currentRamp.getScalarData(attributeLaserPowerRing).end(), std::back_inserter(contourWithRamps.getScalarData(attributeLaserPowerRing)));
                std::copy(currentRamp.getScalarData(attributeLaserVelocity).begin(), currentRamp.getScalarData(attributeLaserVelocity).end(), std::back_inserter(contourWithRamps.getScalarData(attributeLaserVelocity)));
                currentRampIndex++;
            }
        }

        return contourWithRamps;
    }
}


namespace precitec {
namespace filter {

using fliplib::Parameter;

ContourFromFile::ContourFromFile():
    TransformFilter("ContourFromFile", Poco::UUID{"a5ee6c8b-73ab-4904-91a8-abbff7557447"} ),
    m_pPipeInDataX(nullptr),
    m_pPipeInDataY(nullptr),
    m_oPipeOutData(this,"Contour"),
    m_oSeamShapeFilename("")
    {
        m_oSeamShape.clear();
        m_oOutPoints.clear();

        parameters_.add("name", fliplib::Parameter::TYPE_int, 0);

        setInPipeConnectors({
            {Poco::UUID("c963516e-e5fd-4cee-8bfb-6c3a59533e77"), m_pPipeInDataX, "X", 1, "X"},
            {Poco::UUID("5c0c5508-af53-4c96-8c58-1c7cda9064d2"), m_pPipeInDataY, "Y", 1, "Y"}
        });
        setOutPipeConnectors({{Poco::UUID("300e43d5-0da3-4398-bcd0-a2d266bc7bdd"), &m_oPipeOutData, "Contour", 0, ""}});
        // Product update on figure change depends on this uuid.
        // See precitec::storage::Module for more info
        setVariantID(Poco::UUID("1ecb49af-4407-4605-b90e-f09e0e2dd6b8"));
    }

void ContourFromFile::setParameter()
{
    TransformFilter::setParameter();
    auto nameIndex = parameters_.getParameter("name").convert<int>();
    std::string filename = precitec::system::wmBaseDir()
        + "/config/weld_figure/weldingSeam"
        + std::to_string(nameIndex)
        + ".json";

    if (filename != m_oSeamShapeFilename)
    {
        loadSeamShape(filename);
    }
}

bool ContourFromFile::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    if ( p_rPipe.tag() == "X" )
    {
        m_pPipeInDataX  = dynamic_cast<pipe_scalar_t*>(&p_rPipe);
    }
    else if ( p_rPipe.tag() == "Y" )
    {
        m_pPipeInDataY  = dynamic_cast<pipe_scalar_t*>(&p_rPipe);
    }
    else
    {
        poco_assert_dbg(false); // to be asserted by graph editor
    }

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe


void ContourFromFile::proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& e )
{
    using interface::GeoDoublearray;

    poco_assert_dbg(m_pPipeInDataX != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInDataX != nullptr); // to be asserted by graph editor

    const auto & rGeoDoubleArrayInX = m_pPipeInDataX->read(m_oCounter);
    const auto & rGeoDoubleArrayInY = m_pPipeInDataY->read(m_oCounter);

    const auto & rOutputContext = rGeoDoubleArrayInX.context();
    if ( m_oSeamShape.size() == 0
        || inputIsInvalid(rGeoDoubleArrayInX)
        || inputIsInvalid(rGeoDoubleArrayInY)
        || rGeoDoubleArrayInX.ref().getRank().front() == eRankMin
        || rGeoDoubleArrayInY.ref().getRank().front() == eRankMin)
    {
        auto oBadRankOutput = interface::GeoVecAnnotatedDPointarray{rOutputContext, std::vector<geo2d::AnnotatedDPointarray>(1), rGeoDoubleArrayInX.analysisResult(), 0.0 };
        preSignalAction();
        m_oPipeOutData.signal(oBadRankOutput);
        return;
    }

    if (rGeoDoubleArrayInX.ref().size() != 1)
    {
        wmLog(eDebug, "Filter '%s': Received %u X values. Can only process first element, rest will be discarded.\n", name().c_str(), rGeoDoubleArrayInX.ref().size());
    }
    if (rGeoDoubleArrayInY.ref().size() != 1)
    {
        wmLog(eDebug, "Filter '%s': Received %u Y values. Can only process first element, rest will be discarded.\n", name().c_str(), rGeoDoubleArrayInY.ref().size());
    }

    m_oSpTrafo = rOutputContext.trafo();
    interface::ResultType oGeoAnalysisResult = rGeoDoubleArrayInX.analysisResult();
    double oOffsetX = rGeoDoubleArrayInX.ref().getData().front();

    auto oContextDifferenceY = rGeoDoubleArrayInY.context().trafo()->dy() - m_oSpTrafo->dy();
    if (oContextDifferenceY != 0)
    {
        wmLog(eDebug, "Filter '%s': Applying context difference %d to Y value.\n", name().c_str(), oContextDifferenceY);
    }
    double oOffsetY = rGeoDoubleArrayInY.ref().getData().front() + double(oContextDifferenceY);

    computeOutPoints(oOffsetX, oOffsetY);

    const interface::GeoVecAnnotatedDPointarray oGeoOut( rOutputContext, std::vector<geo2d::AnnotatedDPointarray>{m_oOutPoints}, oGeoAnalysisResult, 1.0 );

    preSignalAction();
    m_oPipeOutData.signal(oGeoOut);
}

void ContourFromFile::paint()
{
    using namespace precitec::image;

    if (m_oVerbosity < VerbosityType::eMedium)
    {
        return;
    }

    if (m_oSpTrafo.isNull() || m_oOutPoints.size() == 0)
    {
        return;
    }

    OverlayCanvas	&rCanvas ( canvas<OverlayCanvas>(m_oCounter) );
    OverlayLayer	&rLayerPosition ( rCanvas.getLayerPosition());
    OverlayLayer	&rLayerContour ( rCanvas.getLayerContour());



    auto & rPower = m_oOutPoints.getScalarData(attributeLaserPower);
    assert(rPower.size() == m_oOutPoints.size());

    auto bounds = std::minmax_element(rPower.begin(), rPower.end());
    auto intensityRange = std::max(*bounds.second - *bounds.first, 1.0);
    auto intensityMin = *bounds.first;

    auto alphaFromPower = [intensityMin, intensityRange](double value)
    {
        return (value - intensityMin) * 128 / intensityRange;
    };

    auto & rData = m_oOutPoints.getData();
    auto & rRank = m_oOutPoints.getRank();
    auto oColor = Color::Green();
    for (int i = 0, n = m_oOutPoints.size(); i < n; i++)
    {
        auto & rPoint = rData[i];
        auto oCanvasPoint = m_oSpTrafo->apply(geo2d::Point( (int)std::round(rPoint.x), (int)std::round(rPoint.y)));
        oColor.alpha = alphaFromPower(rPower[i]) + 127;
        if (rRank[i] == eRankMax)
        {
            rLayerPosition.add<OverlayCross>(oCanvasPoint.x, oCanvasPoint.y, 3, oColor);
        }
        else
        {
            rLayerPosition.add<OverlayCircle>(oCanvasPoint.x, oCanvasPoint.y, 3, oColor);
        }
    }

    if (m_oVerbosity < eHigh)
    {
        return;
    }

    auto oColorContour = oColor;

    auto & firstPoint = rData[0];
    auto prevPoint = m_oSpTrafo->apply(geo2d::Point( (int)std::round(firstPoint.x), (int)std::round(firstPoint.y)));;

    for (int i = 1, n = m_oOutPoints.size(); i < n; i++)
    {
        auto & rPoint = rData[i];
        auto oCanvasPoint = m_oSpTrafo->apply(geo2d::Point( (int)std::round(rPoint.x), (int)std::round(rPoint.y)));
        oColor.alpha = alphaFromPower(rPower[i]) + 50;
        rLayerContour.add<OverlayLine>(prevPoint, oCanvasPoint, oColorContour);

        prevPoint = oCanvasPoint;

    }
}

void ContourFromFile::arm(const fliplib::ArmStateBase& p_rArmstate)
{
}

void ContourFromFile::loadSeamShape(std::string filename)
{
    m_oSeamShape.clear();
    m_oOutPoints.clear();
    m_oSeamShapeFilename = "";

    SeamCoordinates oSeamCoordinates;
    bool ok = readFromFile(oSeamCoordinates, filename);
    if (!ok)
    {
        wmLog(eDebug, "Error when loading seam shape from " + filename + "\n");
        return;
    }

    m_oSeamShape.insertScalar(attributeLaserPower);
    m_oSeamShape.insertScalar(attributeLaserPowerRing);
    m_oSeamShape.insertScalar(attributeLaserVelocity);
    m_oSeamShape.reserve(oSeamCoordinates.figure.size());
    auto & rData = m_oSeamShape.getData();
    auto & rRank = m_oSeamShape.getRank();
    auto & rPower = m_oSeamShape.getScalarData(attributeLaserPower);
    auto & rPowerRing = m_oSeamShape.getScalarData(attributeLaserPowerRing);
    auto & rVelocity = m_oSeamShape.getScalarData(attributeLaserVelocity);

    for (auto & rPoint : oSeamCoordinates.figure)
    {
        rData.push_back(geo2d::DPoint{std::get<0>(rPoint.endPosition), std::get<1>(rPoint.endPosition)});
        rRank.push_back(eRankMax);
        rPower.push_back(rPoint.power);
        rPowerRing.push_back(rPoint.ringPower);
        rVelocity.push_back(rPoint.velocity);
    }
    m_oOutPoints.reserve(m_oSeamShape.size());
    m_oSeamShapeFilename = filename;
    wmLog(eDebug, "Loading seam shape from " + filename + " successful, %d points read\n", m_oSeamShape.size());

    if (oSeamCoordinates.ramps.empty())           //Check if there are ramps to compute!
    {
        return;
    }

    const auto& ramps = oSeamCoordinates.ramps;
    const auto& seamShapeWithRamps = computeContourWithRamps(m_oSeamShape, ramps);

    m_oSeamShape.getData() = seamShapeWithRamps.getData();
    m_oSeamShape.getRank() = seamShapeWithRamps.getRank();
    m_oSeamShape.getScalarData(attributeLaserPower) = seamShapeWithRamps.getScalarData(attributeLaserPower);
    m_oSeamShape.getScalarData(attributeLaserPowerRing) = seamShapeWithRamps.getScalarData(attributeLaserPowerRing);
    m_oSeamShape.getScalarData(attributeLaserVelocity) = seamShapeWithRamps.getScalarData(attributeLaserVelocity);
    m_oOutPoints.reserve(m_oSeamShape.size());
    wmLog(eDebug, "Creating ramps successfully.\n");
}

void ContourFromFile::computeOutPoints(double offsetX, double offsetY)
{

    m_oOutPoints.resize(m_oSeamShape.size());
    std::transform(m_oSeamShape.getData().begin(), m_oSeamShape.getData().end(), m_oOutPoints.getData().begin(),
        [&](geo2d::DPoint point )
        {
            return geo2d::DPoint{point.x + offsetX, point.y + offsetY};
        }
    );
    m_oOutPoints.getRank() = m_oSeamShape.getRank();
    m_oOutPoints.getScalarData(attributeLaserPower) = m_oSeamShape.getScalarData(attributeLaserPower);
    m_oOutPoints.getScalarData(attributeLaserPowerRing) = m_oSeamShape.getScalarData(attributeLaserPowerRing);
    m_oOutPoints.getScalarData(attributeLaserVelocity) = m_oSeamShape.getScalarData(attributeLaserVelocity);

    if (m_oVerbosity > VerbosityType::eHigh)
    {
        wmLog(eDebug, "Filter ContourFromFile: %d points, offset = %f, %f\n", m_oOutPoints.size(), offsetX, offsetY);
        if (m_oOutPoints.size() > 0)
        {
            auto firstPoint = m_oOutPoints.getData().front();
            std::string power = "?";
            std::string ringPower = "?";
            std::string velocity = "?";
            if (m_oOutPoints.hasScalarData(attributeLaserPower))
            {
                power = std::to_string( m_oOutPoints.getScalarData(attributeLaserPower).front());
            }
            if (m_oOutPoints.hasScalarData(attributeLaserPowerRing))
            {
                ringPower = std::to_string(m_oOutPoints.getScalarData(attributeLaserPowerRing).front());
            }
            if (m_oOutPoints.hasScalarData(attributeLaserVelocity))
            {
                velocity = std::to_string( m_oOutPoints.getScalarData(attributeLaserVelocity).front());
            }
            wmLog(eDebug, "Filter ContourFromFile: first output point = %f, %f power %s ring power %s velocity %s\n", firstPoint.x, firstPoint.y, power.c_str(), ringPower.c_str(), velocity.c_str() );
        }
    }
}

}

}
