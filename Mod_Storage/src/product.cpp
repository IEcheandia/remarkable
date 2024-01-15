#include "product.h"
#include "copyMode.h"
#include "jsonSupport.h"
#include "parameterSet.h"
#include "seamInterval.h"
#include "seamSeries.h"
#include "seam.h"
#include "linkedSeam.h"
#include "seamError.h"
#include "intervalError.h"
#include "seamSeriesError.h"
#include "productError.h"
#include "referenceCurve.h"
#include "referenceCurveData.h"
#include "referenceSerializer.h"
#include "precitec/colorMap.h"

#include <QIODevice>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QXmlStreamReader>
#include <QDir>
#include <QThread>

#include <deque>

using precitec::gui::components::plotter::ColorMap;

namespace precitec
{
namespace storage
{

namespace
{

template <class InputIt, class OutputIt, class UnaryOperation1, class UnaryOperation2>
OutputIt transform_if (InputIt first1, InputIt last1, OutputIt d_first, UnaryOperation1 unary_op1, UnaryOperation2 unary_op2)
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

/**
 * @brief Adjusts graph reference of all seams in dest, so all references point into this product.
 *
 * After duplicating seams, the graph references of the duplicates may still point to a link in the
 * source product. These references must be adjusted, so they point to seams of the dest product.
 */
void adjustGraphReferences(const Product* source, const Product* dest)
{
    for (Seam* destSeam : dest->seams())
    {
        if (const auto& newGraphRef = destSeam->graphReference(); hasLinkedGraph(newGraphRef))
        {
            const auto& uuidInSource = std::get<LinkedGraphReference>(newGraphRef).value;
            const auto* const linkedSourceSeam = source->findSeam(uuidInSource);
            const auto* const linkedDestSeam = dest->findSeam(linkedSourceSeam->seamSeries()->number(), linkedSourceSeam->number());
            destSeam->setGraphReference(LinkedGraphReference{linkedDestSeam->uuid()});
        }
    }
}
}

using namespace json;

Product::Product(const QUuid &uuid, QObject *parent)
    : QObject(parent)
    , m_uuid(uuid)
    , m_signalyQualityColorMap(new ColorMap(this))
    , m_errorLevelColorMap(new ColorMap(this))
{
    m_signalyQualityColorMap->setName(QStringLiteral("Signal Quality"));
    m_errorLevelColorMap->setName(QStringLiteral("Error Levels"));

    connect(this, &Product::referenceCurveStorageDirChanged, this, &Product::loadReferenceCurves);
}

Product::~Product() = default;

void Product::setName(const QString &name)
{
    if (m_name == name)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("Name"), m_name, name}));
    }
    m_name = name;
    emit nameChanged();
}

void Product::setType(int type)
{
    if (m_type == type)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("Type"), m_type, type}));
    }
    m_type = type;
    emit typeChanged();
}

void Product::setEndless(bool endless)
{
    if (m_endless == endless)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("Endless"), m_endless, endless}));
    }
    m_endless = endless;
    emit endlessChanged();
}

void Product::setTriggerMode(TriggerMode mode)
{
    if (m_triggerMode == mode)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("Trigger mode"), QVariant::fromValue(m_triggerMode), QVariant::fromValue(mode)}));
    }
    m_triggerMode = mode;
    emit triggerModeChanged();
}

void Product::setTriggerSource(TriggerSource source)
{
    if (m_triggerSource == source)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("Trigger source"), QVariant::fromValue(m_triggerSource), QVariant::fromValue(source)}));
    }
    m_triggerSource = source;
    emit triggerSourceChanged();
}

void Product::setStartPositionYAxis(int pos)
{
    if (m_startPositionYAxis == pos)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("Start Position y axis"), m_startPositionYAxis, pos}));
    }
    m_startPositionYAxis = pos;
    emit startPositionYAxisChanged();
}

void Product::setHardwareParameters(ParameterSet *parameters)
{
    if (m_hardwareParameters == parameters)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(HardwareParameterSetReplacedChange{m_hardwareParameters, parameters}));
    }
    if (parameters)
    {
        parameters->setChangeTrackingEnabled(isChangeTracking());
    }
    delete m_hardwareParameters;
    m_hardwareParameters = parameters;
    emit hardwareParametersChanged();
}

void Product::setDefaultProduct(bool defaultProduct)
{
    if (m_default == defaultProduct)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("Default product"), m_default, defaultProduct}));
    }
    m_default = defaultProduct;
    emit defaultProductChanged();
}

QJsonObject Product::toJson() const
{
    return QJsonObject{
        {
            json::toJson(m_uuid),
            json::toJson(m_lengthUnit),
            json::nameToJson(m_name),
            json::assemblyImageToJson(m_assemblyImage),
            json::typeToJson(m_type),
            json::endlessToJson(m_endless),
            json::defaultToJson(m_default),
            json::hardwareToJson(m_startPositionYAxis, m_hardwareParameters),
            json::triggerToJson(m_triggerSource, m_triggerMode),
            json::toJson(m_filterParameterSets),
            json::toJson(m_seamSeries),
            json::toJson(m_overlyingErrors),
            json::qualityNormToJson(m_qualityNorm),
            json::laserControlToJson(m_laserControlPreset),
            json::lwmTriggerSignalTypeToJson(m_lwmTriggerSignalType),
            json::lwmTriggerSignalThresholdToJson(m_lwmTriggerSignalThreshold),
            qMakePair(QLatin1String("signalQuality"), m_signalyQualityColorMap->toJson()),
            qMakePair(QLatin1String("errorLevels"), m_errorLevelColorMap->toJson())
        }
    };
}

void Product::toJson(QIODevice *device) const
{
    const QJsonDocument document{toJson()};
    device->write(document.toJson());
}

namespace
{
QJsonDocument fileToJson(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        return {};
    }
    const QByteArray data = file.readAll();
    if (data.isEmpty())
    {
        return {};
    }
    return QJsonDocument::fromJson(data);
}
}

Product *Product::fromJson(const QString &path, QObject *parent)
{
    const auto document = fileToJson(path);
    if (document.isNull())
    {
        return nullptr;
    }
    auto p = fromJson(document.object(), parent);
    if (p)
    {
        p->m_filePath = path;
    }
    return p;
}

Product *Product::fromJson(const QJsonObject &object, QObject *parent)
{
    if (object.isEmpty())
    {
        return nullptr;
    }
    const QUuid uuid = parseUuid(object);
    if (uuid.isNull())
    {
        return nullptr;
    }
    Product *product = new Product(uuid, parent);
    product->setName(parseName(object));
    product->setType(parseType(object));
    product->setEndless(parseEndless(object));
    const auto trigger = parseTrigger(object);
    product->setTriggerSource(std::get<Product::TriggerSource>(trigger));
    product->setTriggerMode(std::get<Product::TriggerMode>(trigger));
    product->setStartPositionYAxis(parseStartPosYAxis(object));
    product->m_seamSeries = parseSeamSeries(object, product);
    std::sort(product->m_seamSeries.begin(), product->m_seamSeries.end(), [] (auto series1, auto series2) {
        return series1->number() < series2->number();
    });
    for (auto s : product->m_seamSeries)
    {
        connect(s, &SeamSeries::seamsChanged, product, &Product::seamsChanged);
    }
    product->setHardwareParameters(parseHardwareParameters(object, product));
    product->setDefaultProduct(parseDefault(object));
    product->m_filterParameterSets = parseFilterParameterSets(object, product);
    product->setLengthUnit(parseLengthUnit(object));
    product->setAssemblyImage(parseAssemblyImage(object));
    product->setLaserControlPreset(json::parseLaserControl(object));
    product->m_overlyingErrors = json::parseProductError(object, product);
    auto qualityIt = object.find(QLatin1String("signalQuality"));
    if (qualityIt != object.end())
    {;
        product->m_signalyQualityColorMap->fromJson(qualityIt.value().toArray());
    }
    auto errorsIt = object.find(QLatin1String("errorLevels"));
    if (errorsIt != object.end())
    {
        product->m_errorLevelColorMap->fromJson(errorsIt.value().toArray());
    }

    product->removeUnusedFilterParameterSets();
    product->setQualityNorm(parseQualityNorm(object));
    product->setLwmTriggerSignalType(parseLwmTriggerSignalType(object));
    product->setLwmTriggerSignalThreshold(parseLwmTriggerSignalThreshold(object));

    return product;
}

Product *Product::fromXml(const QString &path, QObject *parent)
{
    QFile xmlFile{path};
    if (!xmlFile.open(QIODevice::ReadOnly))
    {
        return nullptr;
    }
    QXmlStreamReader xml{&xmlFile};
    xml.readNextStartElement();
    if (xml.qualifiedName().compare(QLatin1String("Produkt")) != 0)
    {
        return nullptr;
    }
    Product *product = nullptr;
    while (!xml.atEnd())
    {
        xml.readNextStartElement();
        if (xml.qualifiedName().compare(QLatin1String("ProduktData")) == 0)
        {
            while (!xml.atEnd())
            {
                if (!xml.readNextStartElement())
                {
                    if (xml.qualifiedName().compare(QLatin1String("ProduktData")) == 0)
                    {
                        break;
                    }
                    continue;
                }
                if (xml.qualifiedName().compare(QLatin1String("ProduktID")) == 0)
                {
                    const QUuid productId = QUuid::fromString(xml.readElementText());
                    if (!productId.isNull() && !product)
                    {
                        product = new Product(productId, parent);
                    }
                    continue;
                }
                if (!product)
                {
                    continue;
                }
                if (xml.qualifiedName().compare(QLatin1String("ProduktName")) == 0)
                {
                    product->setName(xml.readElementText());
                    continue;
                }
                if (xml.qualifiedName().compare(QLatin1String("Code")) == 0)
                {
                    product->setType(xml.readElementText().toInt());
                    continue;
                }
                if (xml.qualifiedName().compare(QLatin1String("Messung")) == 0)
                {
                    const auto attributes = xml.attributes();
                    product->setEndless(attributes.value(QLatin1String("Endlosmessung")).compare(QLatin1String("true"), Qt::CaseInsensitive) == 0);
                    product->setTriggerSource(Product::TriggerSource(attributes.value(QLatin1String("TriggerQuelle")).toInt()));
                    product->setTriggerMode(Product::TriggerMode(attributes.value(QLatin1String("TriggerSchwelle")).toInt()));
                    continue;
                }
                if (xml.qualifiedName().compare(QLatin1String("StartPosYAchse")) == 0)
                {
                    product->setStartPositionYAxis(xml.readElementText().toInt());
                    continue;
                }
                if (xml.qualifiedName().compare(QLatin1String("UnitType")) == 0)
                {
                    const int unitType = xml.readElementText().toInt();
                    if (unitType == 0)
                    {
                        product->setLengthUnit(Product::LengthUnit::Degree);
                    }
                    else if (unitType == 1)
                    {
                        product->setLengthUnit(Product::LengthUnit::Millimeter);
                    }
                    continue;
                }
                if (xml.qualifiedName().compare(QLatin1String("SumErrorsParameters")) == 0)
                {
                    // TODO: what is it?
                    // skip for the moment
                    while (!xml.atEnd())
                    {
                        if (!xml.readNextStartElement())
                        {
                            if (xml.qualifiedName().compare(QLatin1String("SumErrorsParameters")) == 0)
                            {
                                break;
                            }
                            continue;
                        }
                    }
                }
                if (xml.qualifiedName().compare(QLatin1String("HardwareParameters")) == 0)
                {
                    // hardware parameters are skipped as the uuids do not match
                    while (!xml.atEnd())
                    {
                        if (!xml.readNextStartElement())
                        {
                            if (xml.qualifiedName().compare(QLatin1String("HardwareParameters")) == 0)
                            {
                                break;
                            }
                            continue;
                        }
                    }
                }
                if (xml.qualifiedName().compare(QLatin1String("Messaufgabe")) == 0)
                {
                    if (auto seamSeries = SeamSeries::fromXml(xml, product))
                    {
                        product->m_seamSeries.push_back(seamSeries);
                    }
                    continue;
                }
            }
        }
    }
    for (auto s : product->m_seamSeries)
    {
        connect(s, &SeamSeries::seamsChanged, product, &Product::seamsChanged);
    }

    return product;
}

Product *Product::duplicate(CopyMode mode, QObject *parent) const
{
    auto newProductUuid = duplicateUuid(mode, uuid());
    auto product = new Product{newProductUuid, parent};
    product->setChangeTrackingEnabled(isChangeTracking());
    product->setName(name());
    product->setType(type());
    product->setEndless(isEndless());
    product->setTriggerSource(triggerSource());
    product->setTriggerMode(triggerMode());
    product->setStartPositionYAxis(startPositionYAxis());
    product->setDefaultProduct(isDefaultProduct());
    product->setLengthUnit(lengthUnit());
    product->setAssemblyImage(assemblyImage());
    product->setQualityNorm(qualityNorm());
    product->setLwmTriggerSignalType(lwmTriggerSignalType());
    product->setLwmTriggerSignalThreshold(lwmTriggerSignalThreshold());

    if (m_hardwareParameters)
    {
        product->m_hardwareParameters = m_hardwareParameters->duplicate(mode, product);
    }
    product->m_seamSeries.reserve(m_seamSeries.size());
    for (auto seamSeries : m_seamSeries)
    {
        product->m_seamSeries.push_back(seamSeries->duplicate(mode, product));
    }
    for (auto s : product->m_seamSeries)
    {
        connect(s, &SeamSeries::seamsChanged, product, &Product::seamsChanged);
    }
    if (mode == CopyMode::WithDifferentIds)
    {
        adjustGraphReferences(this, product);
    }

    // duplication of the filter parameters is handled by the measure tasks
    for (auto error : m_overlyingErrors)
    {
        product->m_overlyingErrors.push_back(error->duplicate(mode, product));
    }

    product->signalyQualityColorMap()->copy(m_signalyQualityColorMap);
    product->errorLevelColorMap()->copy(m_errorLevelColorMap);
    product->setLaserControlPreset(m_laserControlPreset);

    for (auto* ps : product->m_filterParameterSets)
    {
        ps->updateGrouping();
    }

    return product;
}

ParameterSet *Product::findHardwareParameterSet(const QUuid &id) const
{
    if (hardwareParameters() && hardwareParameters()->uuid() == id)
    {
        return hardwareParameters();
    }
    for (auto series : m_seamSeries)
    {
        if (auto hw = series->findHardwareParameterSet(id))
        {
            return hw;
        }
    }
    return nullptr;
}

Seam *Product::findSeam(const QUuid &id) const
{
    for (auto seamSeries : m_seamSeries)
    {
        if (auto s = seamSeries->findSeam(id))
        {
            return s;
        }
    }
    return nullptr;
}


Seam *Product::findSeam(quint32 seamSeries, quint32 seam) const
{
    for (auto ss : m_seamSeries)
    {
        if (ss->number() != seamSeries)
        {
            continue;
        }
        return ss->findSeam(seam);
    }
    return nullptr;
}

SeamSeries *Product::findSeamSeries(quint32 seamSeries) const
{
    for (auto ss : m_seamSeries)
    {
        if (ss->number() == seamSeries)
        {
             return ss;
        }
    }
    return nullptr;
}

SeamSeries *Product::findSeamSeries(const QUuid &id) const
{
    for (auto ss : m_seamSeries)
    {
        if (ss->uuid() == id)
        {
             return ss;
        }
    }
    return nullptr;
}


AbstractMeasureTask *Product::findMeasureTask(const QUuid &id) const
{
    for (auto seamSeries : m_seamSeries)
    {
        if (seamSeries->uuid() == id)
        {
            return seamSeries;
        }
        for (auto s : seamSeries->seams())
        {
            if (s->uuid() == id)
            {
                return s;
            }
            for (auto i : s->seamIntervals())
            {
                if (i->uuid() == id)
                {
                    return i;
                }
            }
        }
    }
    return nullptr;
}

QVariantList Product::allSeams() const
{
    QVariantList seams;
    for (auto seamSeries : m_seamSeries)
    {
        const auto& s = seamSeries->seams();
        std::transform(s.begin(), s.end(), std::back_inserter(seams), [] (auto seam) { return QVariant::fromValue(seam); });
    }
    return seams;
}

QVariantList Product::allRealSeams() const
{
    QVariantList realSeams;
    for (auto seamSeries : m_seamSeries)
    {
        const auto& seams = seamSeries->seams();

        transform_if(seams.begin(), seams.end(), std::back_inserter(realSeams),
            [] (auto seam)
            {
                return !seam->metaObject()->inherits(&LinkedSeam::staticMetaObject);
            },
            [] (auto seam)
            {
                return QVariant::fromValue(seam);
            }
        );
    }
    return realSeams;
}

std::vector<Seam*> Product::seams() const
{
    std::vector<Seam*> seams;
    for (auto seamSeries : m_seamSeries)
    {
        const auto& s = seamSeries->seams();
        std::copy(s.begin(), s.end(), std::back_inserter(seams));
    }
    return seams;
}

QVariantList Product::allSeamSeries() const
{
    QVariantList seamSeries;
    std::transform(m_seamSeries.begin(), m_seamSeries.end(), std::back_inserter(seamSeries), [] (auto s) { return QVariant::fromValue(s); });
    return seamSeries;
}

ParameterSet *Product::filterParameterSet(const QUuid &id) const
{
    auto it = std::find_if(m_filterParameterSets.begin(), m_filterParameterSets.end(), [id] (auto ps) { return ps->uuid() == id; });
    if (it != m_filterParameterSets.end())
    {
        return *it;
    }
    return nullptr;
}

void Product::discardFilterParameterSet(ParameterSet* parameterSet)
{
    if (isChangeTracking())
    {
        // do not discard while the product is modified, it would result in incorrect storage
        return;
    }
    if (auto it = std::find(m_filterParameterSets.begin(), m_filterParameterSets.end(), parameterSet); it != m_filterParameterSets.end())
    {
        m_filterParameterSets.erase(it);
        parameterSet->deleteLater();
        m_discardedFilterParameterSets.push_back(parameterSet->uuid());
    }
}

bool Product::containsFilterParameterSet(const QUuid& id) const
{
    if (filterParameterSet(id))
    {
        return true;
    }
    auto it = std::find(m_discardedFilterParameterSets.begin(), m_discardedFilterParameterSets.end(), id);
    return it != m_discardedFilterParameterSets.end();
}

namespace
{
QJsonArray findFilterParameterSetArray(const QString& path)
{
    const auto jsonDocument = fileToJson(path).object();
    auto it = jsonDocument.find(QLatin1String{"filterParameterSets"});
    if (it != jsonDocument.end())
    {
        return it->toArray();
    }
    return {};
}

QJsonObject findFilterParameterSet(const QJsonArray& array, const QUuid& id)
{
    const auto it = std::find_if(array.begin(), array.end(), [id] (const auto& value) { return parseUuid(value.toObject()) == id; });
    if (it != array.end())
    {
        return it->toObject();
    }
    return {};
}

}

void Product::ensureFilterParameterSetLoaded(const QUuid& id)
{
    auto it = std::find(m_discardedFilterParameterSets.begin(), m_discardedFilterParameterSets.end(), id);
    if (it == m_discardedFilterParameterSets.end())
    {
        return;
    }

    const QJsonArray array = findFilterParameterSetArray(m_filePath);
    if (ensureFilterParameterSetLoaded(findFilterParameterSet(array, id)))
    {
        m_discardedFilterParameterSets.erase(it);
    }

}

bool Product::ensureFilterParameterSetLoaded(const QJsonObject& object)
{
    QObject* parent = this;
    const bool sameThread = (QThread::currentThread() == this->thread());
    if (!sameThread)
    {
        parent = nullptr;
    }
    auto* parameterSet{ParameterSet::fromJson(object, parent)};
    if (!parameterSet)
    {
        qDebug() << "Failed to load FilterParameterSet from Json";
        return false;
    }
    if (!sameThread)
    {
        parameterSet->moveToThread(this->thread());
        parameterSet->setParent(this);
    }
    m_filterParameterSets.push_back(parameterSet);
    return true;
}

void Product::ensureAllFilterParameterSetsLoaded()
{
    if (m_discardedFilterParameterSets.empty())
    {
        return;
    }
    const auto array = findFilterParameterSetArray(m_filePath);
    for (const auto& id : m_discardedFilterParameterSets)
    {
        ensureFilterParameterSetLoaded(findFilterParameterSet(array, id));
    }
    m_discardedFilterParameterSets.clear();
}

void Product::addFilterParameterSet(ParameterSet *set)
{
    set->setChangeTrackingEnabled(isChangeTracking());

    ensureFilterParameterSetLoaded(set->uuid());
    auto it = std::find_if(m_filterParameterSets.begin(), m_filterParameterSets.end(), [set] (auto ps) { return ps->uuid() == set->uuid(); });
    if (it == m_filterParameterSets.end())
    {
        if (isChangeTracking())
        {
            addChange(std::move(FilterParameterSetAddedChange{set}));
        }
        m_filterParameterSets.push_back(set);
    } else if (*it != set)
    {
        if (isChangeTracking())
        {
            addChange(std::move(FilterParameterSetReplacedChange{*it, set}));
        }
        (*it)->deleteLater();
        *it = set;
    }
}

void Product::removeFilterParameterSet(const QUuid& id)
{
    ensureFilterParameterSetLoaded(id);
    auto it = std::find_if(m_filterParameterSets.begin(), m_filterParameterSets.end(), [id] (auto ps) { return ps->uuid() == id; });
    if (it != m_filterParameterSets.end())
    {
        if (isChangeTracking())
        {
            addChange(std::move(FilterParameterSetRemovedChange{*it}));
        }
        (*it)->deleteLater();
        m_filterParameterSets.erase(it);
    }
}

void Product::setLengthUnit(LengthUnit unit)
{
    if (m_lengthUnit == unit)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("Length unit"), QVariant::fromValue(m_lengthUnit), QVariant::fromValue(unit)}));
    }
    m_lengthUnit = unit;
    emit lengthUnitChanged();
}

Seam *Product::createSeam()
{
    if (m_seamSeries.empty())
    {
        return nullptr;
    }
    return m_seamSeries.front()->createSeam();
}

void Product::createFirstSeamSeries()
{
    if (!m_seamSeries.empty())
    {
        return;
    }
    createSeamSeries();
}

SeamSeries *Product::createSeamSeries()
{
    auto seamSeries = new SeamSeries{QUuid::createUuid(), this};
    addSeamSeries(seamSeries);
    return seamSeries;
}

SeamSeries *Product::createSeamSeriesCopy(CopyMode mode, SeamSeries *seriesToCopy)
{
    if (!seriesToCopy)
    {
        return nullptr;
    }
    
    auto newSeamSeries = seriesToCopy->duplicate(mode, this);
    addSeamSeries(newSeamSeries);
    if (!newSeamSeries->seams().empty())
    {
        emit seamsChanged();
    }
    return newSeamSeries;
}

void Product::addSeamSeries(SeamSeries *seamSeries)
{
    auto it = std::max_element(m_seamSeries.begin(), m_seamSeries.end(), [] (auto a, auto b) { return a->number() < b->number(); });
    if (it != m_seamSeries.end())
    {
        seamSeries->setNumber((*it)->number() + 1);
    }
    connect(seamSeries, &SeamSeries::seamsChanged, this, &Product::seamsChanged);
    if (isChangeTracking())
    {
        addChange(std::move(SeamSeriesCreatedChange{seamSeries}));
    }
    m_seamSeries.emplace_back(seamSeries);
    emit seamSeriesChanged();
}

void Product::destroySeamSeries(SeamSeries *seam)
{
    auto it = std::find_if(m_seamSeries.begin(), m_seamSeries.end(), [seam] (auto s) { return s == seam; });
    if (it == m_seamSeries.end())
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(SeamSeriesRemovedChange{*it}));
    }
    (*it)->deleteLater();
    m_seamSeries.erase(it);
    emit seamSeriesChanged();
    emit seamsChanged();
}

void Product::setFilePath(const QString &filePath)
{
    if (filePath == m_filePath)
    {
        return;
    }
    m_filePath = filePath;
    emit filePathChanged();
}

void Product::setReferenceCurveStorageDir(const QString &dir)
{
    if (m_referenceCurveStorageDir.compare(dir, Qt::CaseInsensitive) == 0)
    {
        return;
    }
    m_referenceCurveStorageDir = dir;
    emit referenceCurveStorageDirChanged();
}

bool Product::save(const QString &path) const
{
    QSaveFile file = (path.isEmpty()) ? QSaveFile(filePath()) : QSaveFile(path);
    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }
    toJson(&file);
    return file.commit();
}

void Product::setAssemblyImage(const QString &image)
{
    if (m_assemblyImage == image)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("Assembly image"), m_assemblyImage, image}));
    }
    m_assemblyImage = image;
    emit assemblyImageChanged();
}

void Product::addChange(ChangeTracker &&change)
{
    m_changeTracker.emplace_back(std::move(change));
}

QJsonArray Product::changes() const
{
    QJsonArray changes;
    std::transform(m_changeTracker.begin(), m_changeTracker.end(), std::back_inserter(changes), [] (const ChangeTracker &change) { return change.json(); });
    QJsonArray seamSeriesChanges;
    for (auto seamSeries : m_seamSeries)
    {
        auto changesOnSeries = seamSeries->changes();
        if (changesOnSeries.empty())
        {
            continue;
        }
        seamSeriesChanges.push_back(QJsonObject{qMakePair(QString::number(seamSeries->number()), changesOnSeries)});
    }
    if (!seamSeriesChanges.empty())
    {
        changes.push_back(QJsonObject{qMakePair(QStringLiteral("seamSeries"), seamSeriesChanges)});
    }
    if (m_hardwareParameters)
    {
        const auto hardwareParameterChanges = m_hardwareParameters->changes();
        if (!hardwareParameterChanges.empty())
        {
            changes.push_back(QJsonObject{qMakePair(QStringLiteral("hardwareParameters"), hardwareParameterChanges)});
        }
    }
    QJsonArray filterParameterChanges;
    for (auto ps : m_filterParameterSets)
    {
        auto changesOnParameterSet = ps->changes();
        if (changesOnParameterSet.empty())
        {
            continue;
        }
        filterParameterChanges.push_back(QJsonObject{qMakePair(ps->uuid().toString(), changesOnParameterSet)});
    }
    if (!filterParameterChanges.empty())
    {
        changes.push_back(QJsonObject{qMakePair(QStringLiteral("parameterSets"), filterParameterChanges)});
    }
    changes.push_back(QJsonObject{qMakePair(QStringLiteral("product"),
        QJsonObject{
            qMakePair(QStringLiteral("uuid"), m_uuid.toString()),
            qMakePair(QStringLiteral("name"), m_name),
            qMakePair(QStringLiteral("type"), m_type)
        }
    )});
    return changes;
}

void Product::setChangeTrackingEnabled(bool set)
{
    m_changeTracking = set;
    if (m_hardwareParameters)
    {
        m_hardwareParameters->setChangeTrackingEnabled(m_changeTracking);
    }
    for (auto ps : m_filterParameterSets)
    {
        ps->setChangeTrackingEnabled(m_changeTracking);
        for(auto pg : ps->parameterGroups())
        {
            pg->setChangeTrackingEnabled(m_changeTracking);            
        }
    }
    for (auto seamSeries : m_seamSeries)
    {
        if (seamSeries->hardwareParameters())
        {
            seamSeries->hardwareParameters()->setChangeTrackingEnabled(set);
        }
        const auto s = seamSeries->seams();
        for (auto seam : s)
        {
            if (seam->hardwareParameters())
            {
                seam->hardwareParameters()->setChangeTrackingEnabled(set);
            }
            const auto intervals = seam->seamIntervals();
            for (auto i : intervals)
            {
                if (i->hardwareParameters())
                {
                    i->hardwareParameters()->setChangeTrackingEnabled(set);
                }
            }
        }
    }
}

std::vector<SeamError*> Product::allSeamErrors() const
{
    std::vector<SeamError*> result;
    for (auto seamSeries : m_seamSeries)
    {
        const auto &errors = seamSeries->allSeamErrors();
        result.reserve(result.size() + errors.size());
        for (auto error : errors)
        {
            result.push_back(error);
        }
    }
    return result;
}

std::vector<SeamSeriesError*> Product::allSeamSeriesErrors() const
{
    std::vector<SeamSeriesError*> result;
    for (auto seamSeries : m_seamSeries)
    {
        const auto &errors = seamSeries->overlyingErrors();
        result.reserve(result.size() + errors.size());
        for (auto error : errors)
        {
            result.push_back(error);
        }
    }
    return result;
}

ProductError* Product::addOverlyingError(const QUuid &variantId)
{
    ProductError *error = new ProductError(this);
    error->setProduct(this);
    error->setVariantId(variantId);
    m_overlyingErrors.push_back(error);
    return error;
}

void Product::removeOverlyingError(int index)
{
    if (index < 0 || index >= int(m_overlyingErrors.size()))
    {
        return;
    }

    auto it = m_overlyingErrors.begin();
    std::advance(it, index);

    if (it != m_overlyingErrors.end())
    {
        (*it)->deleteLater();
        m_overlyingErrors.erase(it);
    }
}

std::vector<IntervalError*> Product::allIntervalErrors() const
{
    std::vector<IntervalError*> result;

    for (auto seamSerie : m_seamSeries)
    {
        const auto &intervalErrors = seamSerie->allIntervalErrors();
        result.reserve(result.size() + intervalErrors.size());
        for (auto error : intervalErrors)
        {
            result.push_back(error);
        }
    }

    return result;
}

int Product::intervalErrorCount()
{
    int summ = 0;
    for (auto series : m_seamSeries)
    {
        summ += series->intervalErrorCount();
    }
    return summ;
}

std::vector<ReferenceCurve*> Product::allReferenceCurves() const
{
    std::vector<ReferenceCurve*> result;

    for (auto series : m_seamSeries)
    {
        const auto &refCurves = series->allReferenceCurves();
        result.reserve(result.size() + refCurves.size());
        for (auto referenceCurve : refCurves)
        {
            result.push_back(referenceCurve);
        }
    }

    return result;
}

void Product::loadReferenceCurves()
{
    auto dir = QDir{m_referenceCurveStorageDir};

    if (!dir.exists())
    {
        return;
    }

    const auto& referenceCurves = allReferenceCurves();

    ReferenceSerializer serializer;
    serializer.setDirectory(dir);

    const auto files = dir.entryInfoList(QStringList{QStringLiteral("*.ref")}, QDir::Files | QDir::Readable);
    const auto product_it = std::find_if(files.begin(), files.end(), [this] (auto file) {
            const auto uuid = QUuid::fromString(file.baseName());
            return m_uuid == uuid;
        });

    if (product_it != files.end())
    {
        const auto& file = (*product_it);
        serializer.setFileName(file.fileName());
        serializer.deserialize(this);
    }

    // read and remove legacy files, update product file if needed
    std::vector<QString> filesToRemove;
    for (const auto& file : files)
    {
        const auto uuid = QUuid::fromString(file.baseName());

        const auto it = std::find_if(referenceCurves.begin(), referenceCurves.end(), [&uuid] (auto curve) {
            return curve->uuid() == uuid;
        });

        if (it != referenceCurves.end())
        {
            filesToRemove.emplace_back(file.fileName());
            serializer.setFileName(file.fileName());

            LegacyReferenceCurve curve;
            if (serializer.deserialize(curve))
            {
                setReferenceCurveData(curve.m_upper, curve.m_upperData);
                setReferenceCurveData(curve.m_middle, curve.m_middleData);
                setReferenceCurveData(curve.m_lower, curve.m_lowerData);
            }
        }
    }

    if (!filesToRemove.empty())
    {
        saveReferenceCurves();
    }

    for (const auto& file : filesToRemove)
    {
        dir.remove(file);
    }
}

bool Product::saveReferenceCurves(const QString &pathDir)
{
    const auto dir = (pathDir.isEmpty()) ? QDir{m_referenceCurveStorageDir} : QDir{pathDir};

    if (!dir.exists())
    {
        dir.mkpath(dir.absolutePath());
    }

    const QString fileName{m_uuid.toString(QUuid::WithoutBraces) + QStringLiteral(".ref")};

    if (dir.exists(fileName))
    {
        QFile file(dir.absoluteFilePath(fileName));
        file.remove();
    }

    ReferenceSerializer serializer;
    serializer.setDirectory(dir.absolutePath());
    serializer.setFileName(fileName);

    const auto& referenceCurves = allReferenceCurves();

    // clean reference data, which does not belong to any existing reference curve
    m_referenceCurveData.erase(std::remove_if(m_referenceCurveData.begin(), m_referenceCurveData.end(), [&referenceCurves] (auto curveData) {
        return std::none_of(referenceCurves.begin(), referenceCurves.end(), [curveData] (auto curve) { return curve->hasId(curveData->uuid()); });
    }), m_referenceCurveData.end());

    if (!serializer.serialize(this))
    {
        return false;
    }

    return true;
}

Seam *Product::previousSeam(Seam *seam) const
{
    if (!seam)
    {
        return nullptr;
    }
    if (auto previous = seam->seamSeries()->previousSeam(seam))
    {
        return previous;
    }
    if (auto seamSeries = previousSeamSeries(seam->seamSeries()))
    {
        if (!seamSeries->seams().empty())
        {
            return seamSeries->seams().back();
        }
    }

    return nullptr;
}

Seam *Product::nextSeam(Seam *seam) const
{
    if (!seam)
    {
        return nullptr;
    }
    if (auto next = seam->seamSeries()->nextSeam(seam))
    {
        return next;
    }
    if (auto seamSeries = nextSeamSeries(seam->seamSeries()))
    {
        if (!seamSeries->seams().empty())
        {
            return seamSeries->seams().front();
        }
    }
    return nullptr;
}

SeamSeries *Product::previousSeamSeries(SeamSeries *seamSeries) const
{
    auto it = std::find(m_seamSeries.begin(), m_seamSeries.end(), seamSeries);
    if (it == m_seamSeries.end())
    {
        // not found
        return nullptr;
    }
    if (it == m_seamSeries.begin())
    {
        // already at beginning
        return nullptr;
    }
    it--;
    return *it;
}

SeamSeries *Product::nextSeamSeries(SeamSeries *seam) const
{
    auto it = std::find(m_seamSeries.begin(), m_seamSeries.end(), seam);
    if (it == m_seamSeries.end())
    {
        // not found
        return nullptr;
    }
    it++;
    if (it == m_seamSeries.end())
    {
        return nullptr;
    }
    return *it;
}

void Product::removeUnusedFilterParameterSets()
{
    std::deque<QUuid> usedFilterParameterSets;
    auto addGraph = [&usedFilterParameterSets] (const QUuid &uuid)
    {
        if (!uuid.isNull())
        {
            usedFilterParameterSets.push_back(uuid);
        }
    };
    for (auto series : m_seamSeries)
    {
        addGraph(series->graphParamSet());
        for (auto seam : series->seams())
        {
            addGraph(seam->graphParamSet());
            for (auto interval : seam->seamIntervals())
            {
                addGraph(interval->graphParamSet());
            }
        }
    }
    auto it = m_filterParameterSets.begin();
    while (it != m_filterParameterSets.end())
    {
        if (std::find(usedFilterParameterSets.begin(), usedFilterParameterSets.end(), (*it)->uuid()) == usedFilterParameterSets.end())
        {
            (*it)->deleteLater();
            it = m_filterParameterSets.erase(it);
        } else
        {
            it++;
        }
    }
}

void Product::setQualityNorm(const QUuid& qualityNorm)
{
    if (m_qualityNorm == qualityNorm)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("Quality Norm"), m_qualityNorm, qualityNorm}));
    }
    m_qualityNorm = qualityNorm;
    emit qualityNormChanged();
}

void Product::setLaserControlPreset(const QUuid &preset)
{
    if (m_laserControlPreset == preset)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{QStringLiteral("laserControlPreset"), m_laserControlPreset, preset}));
    }
    m_laserControlPreset = preset;
    emit laserControlPresetChanged();
}

void Product::setLwmTriggerSignalType(int enumType)
{
    if (m_lwmTriggerSignalType == enumType)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{QStringLiteral("lwmTriggerSignalType"), m_lwmTriggerSignalType, enumType}));
    }
    m_lwmTriggerSignalType = enumType;
    emit lwmTriggerSignalTypeChanged();
}

void Product::setLwmTriggerSignalThreshold(double threshold)
{
    if (qFuzzyCompare(m_lwmTriggerSignalThreshold, threshold))
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{QStringLiteral("lwmTriggerSignalThreshold"), m_lwmTriggerSignalThreshold, threshold}));
    }
    m_lwmTriggerSignalThreshold = threshold;
    emit lwmTriggerSignalThresholdChanged();
}

void Product::createHardwareParameters()
{
    if (m_hardwareParameters)
    {
        return;
    }
    m_hardwareParameters = new ParameterSet(QUuid::createUuid(), this);
    if (isChangeTracking())
    {
        addChange(std::move(HardwareParametersCreatedChange(m_hardwareParameters)));
    }
    m_hardwareParameters->setChangeTrackingEnabled(isChangeTracking());
    emit hardwareParametersChanged();
}

ReferenceCurveData* Product::referenceCurveData(const QUuid& id)
{
    auto it = std::find_if(m_referenceCurveData.begin(), m_referenceCurveData.end(), [&id] (auto curve) { return curve->uuid() == id; });

    if (it != m_referenceCurveData.end())
    {
        return *it;
    }

    return nullptr;
}

ReferenceCurveData* Product::findOrCreateReferenceCurveData(const QUuid& id)
{
    if (auto curve = referenceCurveData(id))
    {
        return curve;
    }

    auto newCurve = new ReferenceCurveData{id, this};
    m_referenceCurveData.emplace_back(newCurve);

    return newCurve;
}

void Product::setReferenceCurveData(const QUuid& id, const std::forward_list<QVector2D>& samples, std::size_t count)
{
    auto data = findOrCreateReferenceCurveData(id);
    data->clear();
    data->setSamples(samples, count);

    emit referenceCurveDataChanged();
}

void Product::setReferenceCurveData(const QUuid& id, const std::vector<QVector2D>& samples)
{
    auto data = findOrCreateReferenceCurveData(id);
    data->clear();
    data->setSamples(samples);

    emit referenceCurveDataChanged();
}

void Product::copyReferenceCurveData(Product* sourceProduct, ReferenceCurve* sourceCurve, ReferenceCurve* newCurve)
{
    if (!sourceProduct || !sourceCurve || !newCurve)
    {
        return;
    }

    auto sourceUpper = sourceProduct->referenceCurveData(sourceCurve->upper());
    if (sourceUpper && !sourceUpper->samples().empty())
    {
        setReferenceCurveData(newCurve->upper(), sourceUpper->samples());
    }

    auto sourceMiddle = sourceProduct->referenceCurveData(sourceCurve->middle());
    if (sourceMiddle && !sourceMiddle->samples().empty())
    {
        setReferenceCurveData(newCurve->middle(), sourceMiddle->samples());
    }

    auto sourceLower = sourceProduct->referenceCurveData(sourceCurve->lower());
    if (sourceLower && !sourceLower->samples().empty())
    {
        setReferenceCurveData(newCurve->lower(), sourceLower->samples());
    }
}

QDataStream &operator<<(QDataStream &out, Product* product)
{
    out << quint32(product->m_referenceCurveData.size());
    for (auto data : product->m_referenceCurveData)
    {
        out << data;
    }
    return out;
}

QDataStream &operator>>(QDataStream &in, Product* product)
{
    quint32 count;
    in >> count;
    for (auto i = 0u; i < count; i++)
    {
        auto data = new ReferenceCurveData{product};
        in >> data;
        product->m_referenceCurveData.emplace_back(data);
    }
    return in;
}

}
}
