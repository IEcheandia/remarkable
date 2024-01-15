#include "idmZCorrection.h"

#include "module/moduleLogger.h"

#include "fliplib/TypeToDataTypeImpl.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

#include "util/calibDataSingleton.h"

#define FILTER_ID            "f73380bb-a0c5-4bb6-9f43-a2617fffde9b"
#define VARIANT_ID           "8c3bbb73-c65d-4bad-96ab-5a3d9705b3c3"

#define PIPE_ID_REFERENCEARM "c77629ae-6f65-49b1-9608-d8f16c4e57ce"
#define PIPE_ID_IDMZ         "c398fae0-b11c-4136-bf64-68fa6315ab84"
#define PIPE_ID_CORRECTZ     "6264f989-85d4-4309-b7de-c96c15eec4d3"

namespace precitec
{
namespace filter
{
IdmZCorrection::IdmZCorrection()
    : TransformFilter("IdmZCorrection", Poco::UUID(FILTER_ID))
    , m_pipeReferenceArm(nullptr)
    , m_pipeIdmZ(nullptr)
    , m_pipeCorrectZ(this, "CorrectZ")
    , m_mode(IdmZCorrectionMode::VirtualPlane)
{
    parameters_.add("CorrectionMode", fliplib::Parameter::TYPE_int, static_cast<int>(m_mode));
    setInPipeConnectors({
        {Poco::UUID(PIPE_ID_REFERENCEARM), m_pipeReferenceArm, "ReferenceArm", 1, "ReferenceArm"},
        {Poco::UUID(PIPE_ID_IDMZ), m_pipeIdmZ, "IdmZ", 1, "IdmZ"},
    });
    setOutPipeConnectors({
        {Poco::UUID(PIPE_ID_CORRECTZ), &m_pipeCorrectZ, "CorrectZ", 1, "CorrectZ"},
    });
    setVariantID(Poco::UUID(VARIANT_ID));
}

void IdmZCorrection::setParameter()
{
    TransformFilter::setParameter();

    m_mode = static_cast<IdmZCorrectionMode>(parameters_.getParameter("CorrectionMode").convert<int>());
}

bool IdmZCorrection::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == "ReferenceArm")
    {
        m_pipeReferenceArm = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&pipe);
    }
    else if (pipe.tag() == "IdmZ")
    {
        m_pipeIdmZ = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&pipe);
    }

    return BaseFilter::subscribe(pipe, group);
}

void IdmZCorrection::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    const auto& calibrationData = system::CalibDataSingleton::getCalibrationData(math::SensorId::eSensorId0);

    const auto model = calibrationData.idmModel();
    const auto& l0 = model.first;
    const auto& k = model.second;

    const auto& referenceArmArray = m_pipeReferenceArm->read(m_oCounter).ref();
    const auto& idmZGeo = m_pipeIdmZ->read(m_oCounter);
    const auto& idmZArray = idmZGeo.ref();

    const auto sx = idmZGeo.context().m_ScannerInfo.m_x;
    const auto sy = idmZGeo.context().m_ScannerInfo.m_y;
    int n = referenceArmArray.getData().empty() ? 0 : referenceArmArray.getData().front();
    const auto z = idmZArray.getData().empty() ? 0.0 : idmZArray.getData().front();

    double zc = z;

    switch (m_mode)
    {
        case IdmZCorrectionMode::None:
            break;
        case IdmZCorrectionMode::VirtualPlane:
        {
            const double offset = l0.at(n) + k[0] * sx * sx + k[1] * sy * sy;
            if (offset > 0.0)
            {
                zc = z - offset;
            }
            else
            {
                zc = -z - offset;
            }
            break;
        }
        case IdmZCorrectionMode::PhysicalCalibrationSurface:
        {
            const double offset = l0.at(n) + k[0] * sx * sx + k[1] * sy * sy + k[2] * sx + k[3] * sy;
            if (offset > 0.0)
            {
                zc = z - offset;
            }
            else
            {
                zc = -z - offset;
            }
            break;
        }
        default:
            break;
    }

    //Debug
    if (m_oVerbosity > eNone)
    {
        wmLog(eInfo, "[IdmZCorrection] zIdm: %f, zc: %f", z, zc);
    }
    if (m_oVerbosity == eMax)
    {
        std::ostringstream oss;
        oss << "[IdmZCorrection] l0:";
        for (std::size_t i = 0; i < l0.size(); ++i)
        {
            oss << " " << l0[i];
        }
        oss << ", k:";
        for (std::size_t i = 0; i < k.size(); ++i)
        {
            oss << " " << k[i];
        }

        oss << ", sx: " << sx;
        oss << ", sy: " << sy;
        oss << ", n: " << n;
        oss << ", CorrectionMode: " << m_mode;

        wmLog(eInfo, oss.str());
    }

    const auto geoRank = interface::Limit; //1.0
    const auto arrayRank = 255;
    const interface::GeoDoublearray geoOutZc(idmZGeo.context(),
        geo2d::TArray<double>(1, zc, arrayRank),
        interface::ResultType::AnalysisOK, geoRank);

    preSignalAction();
    m_pipeCorrectZ.signal(geoOutZc);
}

} //namespace filter
} //namespace precitec
