#include "scanfieldSeamModel.h"
#include "scanfieldModule.h"
#include "seamSeries.h"
#include "seam.h"
#include "hardwareParametersModule.h"
#include "hardwareParameters.h"
#include "parameterSet.h"
#include "parameter.h"

using precitec::storage::SeamSeries;
using precitec::storage::Seam;

namespace precitec
{
namespace gui
{
namespace
{

template <class InputIt, class OutputIt, class UnaryOperation1, class UnaryOperation2>
OutputIt transform_if(InputIt first1, InputIt last1, OutputIt d_first, UnaryOperation1 unary_op1, UnaryOperation2 unary_op2)
{
    while (first1 != last1)
    {
        if (unary_op1(*first1))
        {
            *d_first++ = unary_op2(*first1);
        }
        ++first1;
    }
    return d_first;
}

}

ScanfieldSeamModel::ScanfieldSeamModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_scanfieldModule(new ScanfieldModule{this})
    , m_hardwareParametersModule(new HardwareParametersModule{this})
{
    connect(this, &ScanfieldSeamModel::seamChanged, this, &ScanfieldSeamModel::currentCenterChanged);
    connect(this, &ScanfieldSeamModel::seamChanged, this, &ScanfieldSeamModel::currentDriveWithOCTReferenceChanged);
    connect(this, &ScanfieldSeamModel::seamSeriesChanged, this, &ScanfieldSeamModel::updateModel);

    connect(m_scanfieldModule, &ScanfieldModule::calibrationCoordinatesRequestProxyChanged, this, &ScanfieldSeamModel::currentCenterChanged);
    connect(m_scanfieldModule, &ScanfieldModule::configurationValidChanged, this, &ScanfieldSeamModel::currentCenterChanged);
    connect(m_scanfieldModule, &ScanfieldModule::cameraSizeChanged, this, &ScanfieldSeamModel::currentCenterChanged);
    connect(m_scanfieldModule, &ScanfieldModule::imageSizeChanged, this, &ScanfieldSeamModel::currentCenterChanged);

    connect(m_hardwareParametersModule, &HardwareParametersModule::attributeModelChanged, this, &ScanfieldSeamModel::currentCenterChanged);

    auto dataChangedHandler = [this] (const QVector<int>& roles)
    {
        dataChanged(index(0), index(rowCount() - 1), roles);
    };

    connect(this, &ScanfieldSeamModel::transformationChanged, this, std::bind(dataChangedHandler, QVector<int>{Qt::UserRole + 3}));
    connect(this, &ScanfieldSeamModel::showAllSeamsChanged, this, std::bind(dataChangedHandler, QVector<int>{Qt::UserRole + 5}));
    connect(this, &ScanfieldSeamModel::currentCenterChanged, this, std::bind(dataChangedHandler, QVector<int>{Qt::UserRole + 5}));

    connect(m_hardwareParametersModule, &HardwareParametersModule::attributeModelChanged, this, std::bind(dataChangedHandler, QVector<int>{Qt::UserRole, Qt::UserRole + 1, Qt::UserRole + 2, Qt::UserRole + 3}));

    connect(m_scanfieldModule, &ScanfieldModule::calibrationCoordinatesRequestProxyChanged, this, std::bind(dataChangedHandler, QVector<int>{Qt::UserRole, Qt::UserRole + 1, Qt::UserRole + 2, Qt::UserRole + 3}));
    connect(m_scanfieldModule, &ScanfieldModule::configurationValidChanged, this, std::bind(dataChangedHandler, QVector<int>{Qt::UserRole, Qt::UserRole + 1, Qt::UserRole + 2, Qt::UserRole + 3}));
    connect(m_scanfieldModule, &ScanfieldModule::cameraSizeChanged, this, std::bind(dataChangedHandler, QVector<int>{Qt::UserRole + 1, Qt::UserRole + 2, Qt::UserRole + 3}));
    connect(m_scanfieldModule, &ScanfieldModule::imageSizeChanged, this, std::bind(dataChangedHandler, QVector<int>{Qt::UserRole + 1, Qt::UserRole + 2, Qt::UserRole + 3}));
}

ScanfieldSeamModel::~ScanfieldSeamModel() = default;

int ScanfieldSeamModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_seams.size();
}

QHash<int, QByteArray> ScanfieldSeamModel::roleNames() const
{
    // cameraCenterValid role enum is used in the ScanfieldSeamFilterModel
    return {
        {Qt::DisplayRole, QByteArrayLiteral("text")},
        {Qt::UserRole, QByteArrayLiteral("cameraCenter")},
        {Qt::UserRole + 1, QByteArrayLiteral("cameraCenterValid")},
        {Qt::UserRole + 2, QByteArrayLiteral("cameraRect")},
        {Qt::UserRole + 3, QByteArrayLiteral("paintedCameraRect")},
        {Qt::UserRole + 4, QByteArrayLiteral("seam")},
        {Qt::UserRole + 5, QByteArrayLiteral("showSeam")}
    };
}

QVariant ScanfieldSeamModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= int (m_seams.size()))
    {
        return {};
    }

    auto seam = m_seams.at(index.row());
    const auto& center = cameraCenter(seam);

    switch (role)
    {
        case Qt::DisplayRole:
            return QStringLiteral("Seam %1").arg(QString::number(seam->visualNumber()));
        case Qt::UserRole:
            return center;
        case Qt::UserRole + 1:
            return m_scanfieldModule->centerValid(center);
        case Qt::UserRole + 2:
            return m_scanfieldModule->cameraRect(center);
        case Qt::UserRole + 3:
            return m_transformation.isIdentity() ? QRectF{} : m_transformation.mapRect(m_scanfieldModule->cameraRect(center));
        case Qt::UserRole + 4:
            return QVariant::fromValue(seam);
        case Qt::UserRole + 5:
            return m_showAllSeams || lastValidCameraCenter(seam) == lastValidCameraCenter(m_seam);
    }

    return {};
}

void ScanfieldSeamModel::setSeamSeries(SeamSeries* seamSeries)
{
    if (m_seamSeries == seamSeries)
    {
        return;
    }

    if (m_seamSeries)
    {
        disconnect(m_seamSeriesDestroyed);
        disconnect(m_seamSeries, &SeamSeries::seamsChanged, this, &ScanfieldSeamModel::updateModel);
    }

    m_seamSeries = seamSeries;

    if (m_seamSeries)
    {
        m_seamSeriesDestroyed = connect(m_seamSeries, &SeamSeries::destroyed, this, std::bind(&ScanfieldSeamModel::setSeamSeries, this, nullptr));
        connect(m_seamSeries, &SeamSeries::seamsChanged, this, &ScanfieldSeamModel::updateModel);
        m_scanfieldModule->setSeries(m_seamSeries->uuid());
    } else
    {
        m_seamSeriesDestroyed = {};
        m_scanfieldModule->setSeries({});
    }

    emit seamSeriesChanged();
}

void ScanfieldSeamModel::updateModel()
{
    beginResetModel();

    m_seams.clear();

    if (m_seamSeries)
    {
        m_seams = m_seamSeries->seams();
    }

    if (m_seam)
    {
        if (std::none_of(m_seams.begin(), m_seams.end(), [this] (auto seam) { return m_seam == seam; }))
        {
            setSeam(nullptr);
        }
    }

    endResetModel();
}

void ScanfieldSeamModel::setSeam(Seam* seam)
{
    if (m_seam == seam)
    {
        return;
    }

    if (m_seam)
    {
        disconnect(m_seamDestroyed);
        disconnect(m_seam, &Seam::hardwareParametersChanged, this, &ScanfieldSeamModel::currentCenterChanged);
    }

    m_seam = seam;

    if (m_seam)
    {
        m_seamDestroyed = connect(m_seam, &Seam::destroyed, this, std::bind(&ScanfieldSeamModel::setSeam, this, nullptr));
        connect(m_seam, &Seam::hardwareParametersChanged, this, &ScanfieldSeamModel::currentCenterChanged);
    } else
    {
        m_seamDestroyed = {};
    }

    emit seamChanged();
}

Seam* ScanfieldSeamModel::lastValidSeam() const
{
    return lastValidCameraCenter(m_seam);
}

void ScanfieldSeamModel::setTransformation(const QMatrix4x4& matrix)
{
    if (m_transformation == matrix)
    {
        return;
    }
    m_transformation = matrix;
    emit transformationChanged();
}

void ScanfieldSeamModel::setShowAllSeams(bool showAllSeams)
{
    if (m_showAllSeams == showAllSeams)
    {
        return;
    }
    m_showAllSeams = showAllSeams;
    emit showAllSeamsChanged();
}

QPointF ScanfieldSeamModel::cameraCenter(Seam* seam) const
{
    if (seam)
    {
        if (auto ps = seam->hardwareParameters())
        {
            auto cameraX = m_hardwareParametersModule->findParameter(ps, HardwareParameters::instance()->properties(HardwareParameters::Key::ScannerNewXPosition).uuid);
            auto cameraY = m_hardwareParametersModule->findParameter(ps, HardwareParameters::instance()->properties(HardwareParameters::Key::ScannerNewYPosition).uuid);

            if (cameraX && cameraY)
            {
                return m_scanfieldModule->scannerToImageCoordinates(cameraX->value().toDouble(), cameraY->value().toDouble());
            }
        }
    }
    return {-1, -1};
}

Seam* ScanfieldSeamModel::lastValidCameraCenter(Seam* seam) const
{
    if (!seam)
    {
        return nullptr;
    }

    auto rit = std::find(m_seams.rbegin(), m_seams.rend(), seam);

    if (rit == m_seams.rend())
    {
        return nullptr;
    }

    auto valid_it = std::find_if(rit, m_seams.rend(), [this] (auto s) { return m_scanfieldModule->centerValid(cameraCenter(s)); });

    if (valid_it == m_seams.rend())
    {
        return nullptr;
    }

    return *valid_it;
}

void ScanfieldSeamModel::selectSeam(const QUuid& id)
{
    if (id.isNull())
    {
        setSeam(nullptr);
        return;
    }

    auto it = std::find_if(m_seams.begin(), m_seams.end(), [&id] (auto seam) { return seam->uuid() == id; });

    setSeam(it != m_seams.end() ? *it : nullptr);
}

QVariantList ScanfieldSeamModel::selectSeams(const QPointF& point)
{
    if (!m_seamSeries || m_transformation.isIdentity())
    {
        return {};
    }

    if (!m_showAllSeams)
    {
        if (lastCenterValid() && m_transformation.mapRect(m_scanfieldModule->cameraRect(lastCenter())).contains(point))
        {
            return {QVariant::fromValue(lastValidSeam())};
        } else
        {
            return {};
        }
    }

    QVariantList selectedSeams;

    transform_if(m_seams.begin(), m_seams.end(), std::back_inserter(selectedSeams),
        [&point, this] (auto seam)
        {
            return pointInSeam(seam, point);
        },
        [] (auto seam)
        {
            return QVariant::fromValue(seam);
        }
    );

    return selectedSeams;
}

bool ScanfieldSeamModel::pointInSeam(Seam* seam, const QPointF& point) const
{
    if (!seam || m_transformation.isIdentity())
    {
        return false;
    }

    const auto& center = cameraCenter(seam);
    return m_scanfieldModule->centerValid(center) && m_transformation.mapRect(m_scanfieldModule->cameraRect(center)).contains(point);
}

bool ScanfieldSeamModel::pointInCurrentSeam(const QPointF& point) const
{
    return pointInSeam(m_seam, point);
}

QPointF ScanfieldSeamModel::currentCenter() const
{
    return cameraCenter(m_seam);
}

QPointF ScanfieldSeamModel::lastCenter() const
{
    return cameraCenter(lastValidSeam());
}

QPointF ScanfieldSeamModel::currentPaintedCenter() const
{
    if (m_transformation.isIdentity())
    {
        return {-1, -1};
    }
    return m_transformation.map(cameraCenter(m_seam));
}

QPointF ScanfieldSeamModel::imageCoordinatesToPaintedPoint(const QPointF& point) const
{

    if (m_transformation.isIdentity())
    {
        return {-1, -1};
    }
    return m_transformation.map(point);
}

QPointF ScanfieldSeamModel::imageToScannerCoordinates(const QPointF& point) const
{
    return m_scanfieldModule->imageToScannerCoordinates(point.x(), point.y());
}

QPointF ScanfieldSeamModel::scannerToImageCoordinates(const QPointF& point) const
{
    return m_scanfieldModule->scannerToImageCoordinates(point.x(), point.y());
}

bool ScanfieldSeamModel::currentCenterValid() const
{
    return m_scanfieldModule->centerValid(currentCenter());
}

bool ScanfieldSeamModel::lastCenterValid() const
{
    return m_scanfieldModule->centerValid(lastCenter());
}

QPointF ScanfieldSeamModel::currentCenterInMM() const
{
    if (!m_seam)
    {
        return {-1, -1};
    }
    const auto& center = currentCenter();

    return m_scanfieldModule->imageToScannerCoordinates(center.x(), center.y());
}

QPointF ScanfieldSeamModel::lastCenterInMM() const
{
    if (!lastValidSeam())
    {
        return {-1, -1};
    }
    const auto& center = lastCenter();

    return m_scanfieldModule->imageToScannerCoordinates(center.x(), center.y());
}

QRectF ScanfieldSeamModel::currentRect() const
{
    if (!currentCenterValid())
    {
        return {};
    }
    return m_scanfieldModule->cameraRect(currentCenter());
}

void ScanfieldSeamModel::setCameraCenter(const QPointF& point)
{
    if (!m_seam)
    {
        return;
    }

    const auto& inverted = m_transformation.inverted();
    const auto& boundPoint = m_scanfieldModule->toValidCameraCenter(inverted.map(point));

    if (currentCenter() == boundPoint)
    {
        return;
    }

    const auto& coords = m_scanfieldModule->imageToScannerCoordinates(boundPoint.x(), boundPoint.y());

    if (!m_seam->hardwareParameters())
    {
        m_seam->createHardwareParameters();
    }
    auto parameterSet = m_seam->hardwareParameters();

    m_hardwareParametersModule->updateParameter(parameterSet, HardwareParameters::instance()->properties(HardwareParameters::Key::ScannerNewXPosition).uuid, coords.x());
    m_hardwareParametersModule->updateParameter(parameterSet, HardwareParameters::instance()->properties(HardwareParameters::Key::ScannerNewYPosition).uuid, coords.y());
    const auto oct{currentDriveWithOCTReference()};
    m_hardwareParametersModule->updateParameter(parameterSet, HardwareParameters::instance()->properties(oct ? HardwareParameters::Key::ScannerDriveWithOCTReference : HardwareParameters::Key::ScannerDriveToPosition).uuid, true);

    auto it = std::find(m_seams.begin(), m_seams.end(), m_seam);

    const auto& idx = index(std::distance(m_seams.begin(), it));

    emit dataChanged(idx, idx, {Qt::UserRole, Qt::UserRole + 1, Qt::UserRole + 2, Qt::UserRole + 3});
    emit currentCenterChanged();
}

void ScanfieldSeamModel::resetCameraCenter()
{
    if (!m_seam)
    {
        return;
    }

    if (auto parameterSet = m_seam->hardwareParameters())
    {
        m_hardwareParametersModule->removeParameter(parameterSet, HardwareParameters::instance()->properties(HardwareParameters::Key::ScannerNewXPosition).uuid);
        m_hardwareParametersModule->removeParameter(parameterSet, HardwareParameters::instance()->properties(HardwareParameters::Key::ScannerNewYPosition).uuid);
        m_hardwareParametersModule->removeParameter(parameterSet, HardwareParameters::instance()->properties(HardwareParameters::Key::ScannerDriveToPosition).uuid);
        m_hardwareParametersModule->removeParameter(parameterSet, HardwareParameters::instance()->properties(HardwareParameters::Key::ScannerDriveWithOCTReference).uuid);

        auto it = std::find(m_seams.begin(), m_seams.end(), m_seam);

        const auto& idx = index(std::distance(m_seams.begin(), it));

        emit dataChanged(idx, idx, {Qt::UserRole, Qt::UserRole + 1, Qt::UserRole + 2, Qt::UserRole + 3});
        emit currentCenterChanged();
    }
}

void ScanfieldSeamModel::setDriveWithOCT(bool set)
{
    if (!m_seam || !currentCenterValid())
    {
        return;
    }
    if (!m_seam->hardwareParameters())
    {
        m_seam->createHardwareParameters();
    }
    auto parameterSet = m_seam->hardwareParameters();
    m_hardwareParametersModule->removeParameter(parameterSet, HardwareParameters::instance()->properties(set ? HardwareParameters::Key::ScannerDriveToPosition : HardwareParameters::Key::ScannerDriveWithOCTReference).uuid);
    m_hardwareParametersModule->updateParameter(parameterSet, HardwareParameters::instance()->properties(set ? HardwareParameters::Key::ScannerDriveWithOCTReference : HardwareParameters::Key::ScannerDriveToPosition).uuid, true);

    emit currentDriveWithOCTReferenceChanged();
}

void ScanfieldSeamModel::setCameraCenterToSeam(int index)
{
    if (!m_seam || index < 0 || index >= int(m_seams.size()))
    {
        return;
    }

    if (auto originSeam = m_seams.at(index))
    {
        setCameraCenter(m_transformation.map(cameraCenter(originSeam)));
    }
}

bool ScanfieldSeamModel::leftEnabled() const
{
    return currentCenterValid() && currentCenter().x() > 0.5 * m_scanfieldModule->cameraSize().width();
}

bool ScanfieldSeamModel::rightEnabled() const
{
    return currentCenterValid() && currentCenter().x() < m_scanfieldModule->imageSize().width() - 0.5 * m_scanfieldModule->cameraSize().width();
}

bool ScanfieldSeamModel::topEnabled() const
{
    return currentCenterValid() && currentCenter().y() > 0.5 * m_scanfieldModule->cameraSize().height();
}

bool ScanfieldSeamModel::bottomEnabled() const
{
    return currentCenterValid() && currentCenter().y() < m_scanfieldModule->imageSize().height() - 0.5 * m_scanfieldModule->cameraSize().height();
}

bool ScanfieldSeamModel::currentDriveWithOCTReference() const
{
    if (!m_seam)
    {
        return false;
    }
    if (const auto ps = m_seam->hardwareParameters())
    {
        const auto hasOct = m_hardwareParametersModule->findParameter(ps, HardwareParameters::instance()->properties(HardwareParameters::Key::ScannerDriveWithOCTReference).uuid) != nullptr;
        const auto hasDrive = m_hardwareParametersModule->findParameter(ps, HardwareParameters::instance()->properties(HardwareParameters::Key::ScannerDriveToPosition).uuid) != nullptr;
        if (!hasOct && !hasDrive)
        {
            return m_octWithReferenceArms;
        }
        return hasOct;
    }

    return m_octWithReferenceArms;
}

void ScanfieldSeamModel::setOctWithReferenceArms(bool set)
{
    if (m_octWithReferenceArms == set)
    {
        return;
    }
    m_octWithReferenceArms = set;
    emit octWithReferenceArmsChanged();
}

}
}
