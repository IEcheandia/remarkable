#include "contourCoordinateTransform.h"

#include "module/moduleLogger.h"
#include "common/bitmap.h"

#include "fliplib/TypeToDataTypeImpl.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

#include "util/calibDataSingleton.h"
#include "coordinates/fieldDistortionMapping.h"

#include <vector>
#include <utility>
#include <cmath>
#include <fstream>

#define FILTER_ID                 "57007ee8-edda-4833-b9be-bb9fc272bf77"
#define VARIANT_ID                "37757c2e-09ce-4f72-9b2f-aae05d5e3a71"

#define PIPE_ID_CONTOURIN         "8f00e2f8-e3e6-47d0-b7f3-85e3b81b8d3a"
#define PIPE_ID_CONTOUROUT        "a7a7740b-2ba6-4178-b50d-0a21aa37bf95"

namespace precitec
{
namespace filter
{

ContourCoordinateTransform::ContourCoordinateTransform()
    : TransformFilter("ContourCoordinateTransform", Poco::UUID(FILTER_ID))
    , m_contourIn(nullptr)
    , m_contourOut(this, "ContourOut")
    , m_conversionMode(Mode::PixelToWorld)
    , m_logToFile(false)
    , m_logFileName("cct.log")
    , m_contourOutArray()
{
    parameters_.add("ConversionMode", fliplib::Parameter::TYPE_int, static_cast<int>(m_conversionMode));
    parameters_.add("LogToFile", fliplib::Parameter::TYPE_bool, m_logToFile);
    parameters_.add("LogFileName", fliplib::Parameter::TYPE_string, m_logFileName);

    setInPipeConnectors({
        {Poco::UUID(PIPE_ID_CONTOURIN), m_contourIn, "ContourIn", 1, "ContourIn"},
    });

    setOutPipeConnectors({
        {Poco::UUID(PIPE_ID_CONTOUROUT), &m_contourOut, "ContourOut", 1, "ContourOut"},
    });

    setVariantID(Poco::UUID(VARIANT_ID));
}

void ContourCoordinateTransform::setParameter()
{
    TransformFilter::setParameter();

    m_conversionMode = static_cast<Mode>(parameters_.getParameter("ConversionMode").convert<int>());
    m_logToFile = parameters_.getParameter("LogToFile").convert<bool>();
    m_logFileName = parameters_.getParameter("LogFileName").convert<std::string>();
}

void ContourCoordinateTransform::paint()
{
    if (m_oVerbosity == eNone)
    {
        return;
    }

    image::OverlayCanvas &currentCanvas = canvas<image::OverlayCanvas>(m_oCounter);
    image::OverlayLayer &contourLayer = currentCanvas.getLayerContour();

    if (m_conversionMode == Mode::WorldToPixel)
    {
        for (auto &contour : m_contourOutArray.ref())
        {
            const auto &contourData = contour.getData();
            for (std::size_t i = 1; i < contourData.size(); ++i)
            {
                const double x1 = std::round(contourData[i-1].x);
                const double x2 = std::round(contourData[i].x);
                const double y1 = std::round(contourData[i-1].y);
                const double y2 = std::round(contourData[i].y);
                contourLayer.add<image::OverlayLine>(x1, y1, x2, y2, image::Color::Magenta());
            }
        }
    }
}

bool ContourCoordinateTransform::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == "ContourIn")
    {
        m_contourIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>*>(&pipe);
    }

    return BaseFilter::subscribe(pipe, group);
}

void ContourCoordinateTransform::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    const auto &contourInArray = m_contourIn->read(m_oCounter);

    const auto context = contourInArray.context();

    const auto sx = context.m_ScannerInfo.m_x;
    const auto sy = context.m_ScannerInfo.m_y;

    const auto &calibrationData = system::CalibDataSingleton::getCalibrationData(math::SensorId::eSensorId0);
    const auto tcp = calibrationData.getTCPCoordinate(context.m_ScannerInfo, filter::LaserLine::FrontLaserLine);

    m_contourOutArray = contourInArray; //copy

    std::vector<double> K;
    const auto distortionCorrectionEnable = calibrationData.scanfieldDistortionCorrectionFactor(K);

    auto k = scannerPositionToDistortionCoefficient(sx, sy, K);

    if (!distortionCorrectionEnable)
    {
        const auto linearModel = calibrationData.getCoaxCalibrationData();
        const auto ax = linearModel.m_oBeta0 / linearModel.m_oDpixX * (linearModel.m_oInvertX ? -1 : 1);
        const auto ay = linearModel.m_oBeta0 / linearModel.m_oDpixY * (linearModel.m_oInvertY ? 1 : -1);

        k = {ax, 0, 0, 0, 0, 0, ay, 0, 0, 0, 0, 0}; // no distortion or rotation compensation
    }

    if (m_oVerbosity > eLow)
    {
        wmLog(eInfo, "-------------------------------------------[ContourCoordinateTransform]-------------------------------------------");
        wmLog(eInfo, "sx: %f mm, sy: %f mm, tcpx: %f px, tcpy: %f px", sx, sy, tcp.x, tcp.y);
        wmLog(eInfo, "ax: %f, bx: %f, cx: %f, dx: %f, ex: %f, fx: %f", k[0], k[1], k[2], k[3], k[4], k[5]);
        wmLog(eInfo, "ay: %f, by: %f, cy: %f, dy: %f, ey: %f, fy: %f", k[6], k[7], k[8], k[9], k[10], k[11]);
    }

    if (m_conversionMode == Mode::PixelToWorld)
    {
        for (auto &contour : m_contourOutArray.ref())
        {
            int i = 1;
            for (auto &point : contour.getData())
            {
                const auto pointPixel = point;
                const auto pointWorld = pixelToWorld(pointPixel.x - tcp.x, pointPixel.y - tcp.y, k);
                point = {pointWorld.first + sx, pointWorld.second + sy};

                if (m_oVerbosity > eLow)
                {
                    wmLog(eInfo, "[%d](%f px, %f px) --> (%f mm, %f mm)", i, pointPixel.x, pointPixel.y, point.x, point.y);
                }
                ++i;
            }
        }
    }
    else
    {
        for (auto &contour : m_contourOutArray.ref())
        {
            int i = 1;
            for (auto &point : contour.getData())
            {
                const auto pointWorld = point;
                const auto pointPixel = worldToPixel(pointWorld.x - sx, pointWorld.y -sy, k);
                point = {pointPixel.first + tcp.x, pointPixel.second + tcp.y};

                if (m_oVerbosity > eLow)
                {
                    wmLog(eInfo, "[%d](%f mm, %f mm) --> (%f px, %f px)", i, pointWorld.x, pointWorld.y, point.x, point.y);
                }
                ++i;
            }
        }
    }

    if (m_oVerbosity > eLow)
    {
        wmLog(eInfo, "-----------------------------------------[ContourCoordinateTransform End]-----------------------------------------");
    }

    if (m_logToFile)
    {
        std::string filePath;
        filePath = (getenv("WM_BASE_DIR") ? std::string(getenv("WM_BASE_DIR")) : "") +
            "/logfiles/" + m_logFileName;

        std::fstream logFile;
        logFile.open(filePath, std::ios_base::out | std::ios_base::app);

        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        long long now_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm* now_tm = std::localtime(&now_time_t);
        int ms = now_since_epoch % 1000;

        logFile <<"[" << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S.") << std::setfill('0') << std::setw(3) << ms << "]" << std::endl;
        logFile << std::fixed << std::setprecision(3)
                << "[sx: " << sx << " mm, sy: " << sy << " mm"
                << ", tcpx: " << tcp.x << " px, tcpy: " << tcp.y << " px]" << std::endl
                << std::scientific
                << "[mode: " << (m_conversionMode == Mode::PixelToWorld ? "Camera To Scanner" : "Scanner To Camera")
                << ", ax: " << k[0] << ", bx: " << k[1] << ", cx: " << k[2] << ", dx: " << k[3] << ", ex: " << k[4] << ", fx: " << k[5]
                << ", ay: " << k[6] << ", by: " << k[7] << ", cy: " << k[8] << ", dy: " << k[9] << ", ey: " << k[10] << ", fy: " << k[11]
                << "]" << std::endl;

        logFile << std::fixed << std::setprecision(3);
        for (std::size_t i = 0; i < m_contourOutArray.ref().size(); ++i)
        {
            const auto& contourIn = contourInArray.ref()[i].getData();
            const auto& contourOut = m_contourOutArray.ref()[i].getData();

            for (std::size_t j = 0; j < contourOut.size(); ++j)
            {
                logFile << std::left << std::setfill(' ') << std::setw(7) << std::to_string(j + 1)
                        << std::right << std::setfill(' ') << std::setw(10) << contourIn[j].x << " "
                        << std::right << std::setfill(' ') << std::setw(10) << contourIn[j].y << " "
                        << std::right << std::setfill(' ') << std::setw(10) << contourOut[j].x << " "
                        << std::right << std::setfill(' ') << std::setw(10) << contourOut[j].y << std::endl;
            }
        }

        logFile << std::endl;
        logFile.close();
    }

    preSignalAction();
    m_contourOut.signal(m_contourOutArray);
}


} //namespace filter
} //namespace precitec
