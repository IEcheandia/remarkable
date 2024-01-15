#include "jsonSupport.h"
#include "attribute.h"
#include "attributeField.h"
#include "parameterSet.h"
#include "seam.h"
#include "seamInterval.h"
#include "seamSeries.h"
#include "seamError.h"
#include "resultSetting.h"
#include "referenceCurve.h"
#include "intervalError.h"
#include "seamSeriesError.h"
#include "productError.h"
#include "levelConfig.h"
#include "qualityNorm.h"
#include "qualityNormResult.h"
#include "qualityNormLevel.h"
#include "gaugeRange.h"
#include "linkedSeam.h"
#include "laserControlPreset.h"
#include "parameterFilterGroup.h"

#include <QJsonObject>
#include <QJsonArray>
#include <iostream>
#include <type_traits>

namespace precitec
{
namespace storage
{
namespace json
{

enum class Key
{
    Uuid,
    Name,
    Type,
    Endless,
    Trigger,
    TriggerSource,
    TriggerMode,
    Hardware,
    SeamError,
    StartPosYAxis,
    SeamSeries,
    Seams,
    SeamIntervals,
    Number,
    TriggerDelta,
    Velocity,
    GraphUuid,
    GraphParamSetUuid,
    Length,
    ParameterSet,
    Parameters,
    TypeId,
    Value,
    Default,
    FilterId,
    FilterParameterSets,
    Attributes,
    VariantId,
    SegmentId,
    MinValue,
    MaxValue,
    DefaultValue,
    Unit,
    Description,
    ToolTip,
    Enumeration,
    ContentName,
    Mandatory,
    Visible,
    Publicity,
    MaxLength,
    Step,
    EditListOrder,
    AttributeId,
    AttributeFields,
    Locate,
    Text,
    LengthUnit,
    PlotNumber,
    Plottable,
    LineColor,
    Result,
    AssemblyImage,
    PositionInAssemblyImage,
    X,
    Y,
    UserLevel,
    Visualization,
    Active,
    Precision,
    SubGraphs,
    ThicknessLeft,
    ThicknessRight,
    TargetDifference,
    MovingDirection,
    Level,
    ReferenceCurveSet,
    ReferenceCurves,
    ResultName,
    UpperReference,
    MiddleReference,
    LowerReference,
    Jitter,
    ReferenceType,
    LC_Preset,
    LC_Power,
    LC_Offset,
    IntervalError,
    IntervalIds,
    Envelope,
    UseMiddleCurveAsReference,
    Roi,
    Width,
    Height,
    OverlyingError,
    IntervalLevels,
    QualityNorm,
    QualityNormResults,
    CommonResults,
    GaugeRange,
    LinkedSeams,
    LinkedSeamLabel,
    LwmTriggerSignalType,
    LwmTriggerSignalThreshold,
    GroupId,
    GroupIndex,
    FileSuffixes,
    FileLocation,
    FilterParametersGroup,
    LinkedGraph,
    Fields,
};

static QLatin1String s_keys[] = {
    QLatin1String("uuid"),
    QLatin1String("name"),
    QLatin1String("type"),
    QLatin1String("endless"),
    QLatin1String("trigger"),
    QLatin1String("source"),
    QLatin1String("mode"),
    QLatin1String("hardware"),
    QLatin1String("sumError"),
    QLatin1String("startPosYAxis"),
    QLatin1String("seamSeries"),
    QLatin1String("seams"),
    QLatin1String("seamIntervals"),
    QLatin1String("number"),
    QLatin1String("triggerDelta"),
    QLatin1String("velocity"),
    QLatin1String("graph"),
    QLatin1String("graphParamSet"),
    QLatin1String("length"),
    QLatin1String("parameterSet"),
    QLatin1String("parameters"),
    QLatin1String("typeId"),
    QLatin1String("value"),
    QLatin1String("default"),
    QLatin1String("filterId"),
    QLatin1String("filterParameterSets"),
    QLatin1String("attributes"),
    QLatin1String("variantId"),
    QLatin1String("segmentId"),
    QLatin1String("minValue"),
    QLatin1String("maxValue"),
    QLatin1String("defaultValue"),
    QLatin1String("unit"),
    QLatin1String("description"),
    QLatin1String("toolTip"),
    QLatin1String("enum"),
    QLatin1String("contentName"),
    QLatin1String("mandatory"),
    QLatin1String("visible"),
    QLatin1String("publicity"),
    QLatin1String("maxLength"),
    QLatin1String("step"),
    QLatin1String("editListOrder"),
    QLatin1String("attribute"),
    QLatin1String("fields"),
    QLatin1String("locate"),
    QLatin1String("text"),
    QLatin1String("lengthUnit"),
    QLatin1String("plotterNumber"),
    QLatin1String("plottable"),
    QLatin1String("lineColor"),
    QLatin1String("messwerte"),
    QLatin1String("assemblyImage"),
    QLatin1String("positionInAssemblyImage"),
    QLatin1String("X"),
    QLatin1String("Y"),
    QLatin1String("userLevel"),
    QLatin1String("visualization"),
    QLatin1String("active"),
    QLatin1String("precision"),
    QLatin1String("subgraphs"),
    QLatin1String("thicknessLeft"),
    QLatin1String("thicknessRight"),
    QLatin1String("targetDifference"),
    QLatin1String("movingDirection"),
    QLatin1String("level"),
    QLatin1String("referenceCurveSet"),
    QLatin1String("referenceCurves"),
    QLatin1String("resultName"),
    QLatin1String("upper"),
    QLatin1String("middle"),
    QLatin1String("lower"),
    QLatin1String("jitter"),
    QLatin1String("referenceType"),
    QLatin1String("presetId"),
    QLatin1String("power"),
    QLatin1String("offset"),
    QLatin1String("intervalSumError"),
    QLatin1String("intervalIds"),
    QLatin1String("envelope"),
    QLatin1String("useMiddleCurveAsReference"),
    QLatin1String("roi"),
    QLatin1String("width"),
    QLatin1String("height"),
    QLatin1String("overlyingError"),
    QLatin1String("intervalLevels"),
    QLatin1String("qualityNorm"),
    QLatin1String("qualityNormResults"),
    QLatin1String("commonResults"),
    QLatin1String("gaugeRange"),
    QLatin1String("linkedSeams"),
    QLatin1String("label"),
    QLatin1String("lwmTriggerSignalType"),
    QLatin1String("lwmTriggerSignalThreshold"),
    QLatin1String("groupId"),
    QLatin1String("groupIndex"),
    QLatin1String("suffixes"),
    QLatin1String("location"),
    QLatin1String("parametersGroupedByFilter"),
    QLatin1String("linkedGraph"),
    QLatin1String("fields"),
};

static const std::map<QString, Parameter::DataType> s_map{
        {QStringLiteral("int"), Parameter::DataType::Integer},
        {QStringLiteral("uint"), Parameter::DataType::UnsignedInteger},
        {QStringLiteral("float"), Parameter::DataType::Float},
        {QStringLiteral("double"), Parameter::DataType::Double},
        {QStringLiteral("bool"), Parameter::DataType::Boolean},
        {QStringLiteral("enum"), Parameter::DataType::Enumeration},
        {QStringLiteral("fehler"), Parameter::DataType::Error}, // Legacy support
        {QStringLiteral("error"), Parameter::DataType::Error},
        {QStringLiteral("messwert"), Parameter::DataType::Result}, // Legacy support
        {QStringLiteral("result"), Parameter::DataType::Result},
        {QStringLiteral("sensor"), Parameter::DataType::Sensor},
        {QStringLiteral("string"), Parameter::DataType::String},
        {QStringLiteral("text"), Parameter::DataType::Text},
        {QStringLiteral("seamFigure"), Parameter::DataType::SeamFigure},
        {QStringLiteral("wobbleFigure"), Parameter::DataType::WobbleFigure},
        {QStringLiteral("file"), Parameter::DataType::File},
    };

QUuid parseUuid(const QJsonObject &object, Key key)
{
    auto it = object.find(s_keys[int(key)]);
    if (it == object.end())
    {
        return QUuid();
    }
    return QUuid::fromString(it.value().toString());
}

QUuid parseUuid(const QJsonObject &object)
{
    return parseUuid(object, Key::Uuid);
}

QUuid parseGraph(const QJsonObject &object)
{
    return parseUuid(object, Key::GraphUuid);
}

QUuid parseGraphParamSet(const QJsonObject &object)
{
    return parseUuid(object, Key::GraphParamSetUuid);
}

QUuid parseTypeId(const QJsonObject &object)
{
    return parseUuid(object, Key::TypeId);
}

QUuid parseFilterId(const QJsonObject &object)
{
    return parseUuid(object, Key::FilterId);
}

QUuid parseVariantId(const QJsonObject &object)
{
    return parseUuid(object, Key::VariantId);
}

QUuid parseSegmentId(const QJsonObject &object)
{
    return parseUuid(object, Key::SegmentId);
}

QUuid parseAttributeId(const QJsonObject &object)
{
    return parseUuid(object, Key::AttributeId);
}

QUuid parseQualityNorm(const QJsonObject &object)
{
    return parseUuid(object, Key::QualityNorm);
}

QString parseString(const QJsonObject &object, Key key)
{
    auto it = object.find(s_keys[int(key)]);
    if (it == object.end())
    {
        return QString();
    }
    return it.value().toString();
}

QString parseName(const QJsonObject &object)
{
    return parseString(object, Key::Name);
}

QString parseUnit(const QJsonObject &object)
{
    return parseString(object, Key::Unit);
}

QString parseDescription(const QJsonObject &object)
{
    return parseString(object, Key::Description);
}

QString parseToolTip(const QJsonObject &object)
{
    return parseString(object, Key::ToolTip);
}

QString parseEnumeration(const QJsonObject &object)
{
    return parseString(object, Key::Enumeration);
}

QString parseContentName(const QJsonObject &object)
{
    return parseString(object, Key::ContentName);
}

QString parseText(const QJsonObject &object)
{
    return parseString(object, Key::Text);
}

QString parseAssemblyImage(const QJsonObject &object)
{
    return parseString(object, Key::AssemblyImage);
}

QString parseLinkedSeamLabel(const QJsonObject &object)
{
    return parseString(object, Key::LinkedSeamLabel);
}

int parseInt(const QJsonObject &object, Key key, int defaultValue = 0)
{
    auto it = object.find(s_keys[int(key)]);
    if (it == object.end())
    {
        return defaultValue;
    }
    return it.value().toInt();
}

qreal parseReal(const QJsonObject &object, Key key, qreal defaultValue = 0.0)
{
    auto it = object.find(s_keys[int(key)]);
    if (it == object.end())
    {
        return defaultValue;
    }
    return it.value().toDouble();
}

int parseType(const QJsonObject &object)
{
    return parseInt(object, Key::Type);
}

int parseMaxLength(const QJsonObject &object)
{
    return parseInt(object, Key::MaxLength);
}

int parseStep(const QJsonObject &object)
{
    return parseInt(object, Key::Step);
}

int parseEditListOrder(const QJsonObject &object)
{
    return parseInt(object, Key::EditListOrder);
}

int parseUserLevel(const QJsonObject &object)
{
    return parseInt(object, Key::UserLevel);
}

qint64 parseLength(const QJsonObject &object)
{
    auto it = object.find(s_keys[int(Key::Length)]);
    if (it == object.end())
    {
        return 0;
    }
    auto value = it.value();
    if (value.isDouble())
    {
        return value.toInt();
    }
    bool ok;
    qint64 ret = value.toString().toLongLong(&ok);
    if (ok)
    {
        return ret;
    }
    return 0;
}

QPointF parsePositionInAssemblyImage(const QJsonObject &object)
{
    auto it = object.find(s_keys[int(Key::PositionInAssemblyImage)]);
    if (it == object.end())
    {
        return {-1, -1};
    }
    auto value = it.value().toObject();
    auto xIt = value.find(s_keys[int(Key::X)]);
    auto yIt = value.find(s_keys[int(Key::Y)]);
    if (xIt == value.end() || yIt == value.end())
    {
        return {-1, -1};
    }
    return {xIt.value().toDouble(), yIt.value().toDouble()};
}

QRect parseRoi(const QJsonObject &object)
{
    auto it = object.find(s_keys[int(Key::Roi)]);
    if (it == object.end())
    {
        return {-1, -1, 0, 0};
    }
    auto value = it.value().toObject();
    auto xIt = value.find(s_keys[int(Key::X)]);
    auto yIt = value.find(s_keys[int(Key::Y)]);
    auto widthIt = value.find(s_keys[int(Key::Width)]);
    auto heightIt = value.find(s_keys[int(Key::Height)]);
    if (xIt == value.end() || yIt == value.end() || widthIt == value.end() || heightIt == value.end())
    {
        return {-1, -1, 0, 0};
    }
    return {xIt.value().toInt(), yIt.value().toInt(), widthIt.value().toInt(), heightIt.value().toInt()};
}

bool parseBoolean(const QJsonObject &object, Key key, bool defaultValue = false)
{
    auto it = object.find(s_keys[int(key)]);
    if (it == object.end())
    {
        return defaultValue;
    }
    return it.value().toBool();
}

bool parseEndless(const QJsonObject &object)
{
    return parseBoolean(object, Key::Endless);
}

bool parseDefault(const QJsonObject &object)
{
    return parseBoolean(object, Key::Default);
}

bool parseActive(const QJsonObject &object)
{
    return parseBoolean(object, Key::Active, true);
}

bool parseMandatory(const QJsonObject &object)
{
    return parseBoolean(object, Key::Mandatory);
}

bool parseVisible(const QJsonObject &object)
{
    return parseBoolean(object, Key::Visible);
}

bool parsePublicity(const QJsonObject &object)
{
    return parseBoolean(object, Key::Publicity);
}

int parseEnumType(const QJsonObject &object)
{
    return parseInt(object, Key::Enumeration);
}

int parsePlotterNumber(const QJsonObject &object)
{
    return parseInt(object, Key::PlotNumber);
}

int parsePlottable(const QJsonObject &object)
{
    return parseInt(object, Key::Plottable);
}

int parseVisibleItem(const QJsonObject &object)
{
    return parseInt(object, Key::Visible);
}

int parseVisualization(const QJsonObject &object)
{
    return qBound(0, parseInt(object, Key::Visualization, 1), 2);
}

int parseFloatingPointPrecision(const QJsonObject &object)
{
    return parseInt(object, Key::Precision, 3);
}

QString parseLineColor(const QJsonObject &object)
{
    return parseString(object, Key::LineColor);
}

std::tuple<Product::TriggerSource, Product::TriggerMode> parseTrigger(const QJsonObject &object)
{
    auto it = object.find(s_keys[int(Key::Trigger)]);
    const auto defaultValue = std::make_tuple(Product::TriggerSource::Software, Product::TriggerMode::Burst);
    if (it == object.end())
    {
        return defaultValue;
    }
    const auto triggerObject = it.value().toObject();
    if (triggerObject.isEmpty())
    {
        return defaultValue;
    }
    static const std::map<QString, Product::TriggerSource> s_sourceMap = {
        {QStringLiteral("Software"), Product::TriggerSource::Software},
        {QStringLiteral("External"), Product::TriggerSource::External},
        {QStringLiteral("Grabber"), Product::TriggerSource::GrabberControlled}
    };
    const auto sourceIt = s_sourceMap.find(parseString(triggerObject, Key::TriggerSource));
    const Product::TriggerSource source = (sourceIt != s_sourceMap.end()) ? sourceIt->second : std::get<Product::TriggerSource>(defaultValue);

    static const std::map<QString, Product::TriggerMode> s_modeMap = {
        {QStringLiteral("Single"), Product::TriggerMode::Single},
        {QStringLiteral("Burst"), Product::TriggerMode::Burst},
        {QStringLiteral("Continue"), Product::TriggerMode::Continue},
        {QStringLiteral("None"), Product::TriggerMode::None}
    };
    const auto modeIt = s_modeMap.find(parseString(triggerObject, Key::TriggerMode));
    const Product::TriggerMode mode = (modeIt != s_modeMap.end()) ? modeIt->second : std::get<Product::TriggerMode>(defaultValue);
    return std::make_tuple(source, mode);
}

int parseStartPosYAxis(const QJsonObject &object)
{
    auto it = object.find(s_keys[int(Key::Hardware)]);
    if (it == object.end())
    {
        return 0;
    }
    const auto hardwareObject = it.value().toObject();
    if (hardwareObject.isEmpty())
    {
        return 0;
    }
    return parseInt(hardwareObject, Key::StartPosYAxis);
}

template <typename T, typename R>
std::vector<T*> parseArray(const QJsonObject &object, R *parent, Key key)
{
    std::vector<T*> ret;
    auto it = object.find(s_keys[int(key)]);
    if (it == object.end())
    {
        return ret;
    }
    const auto elements = it.value().toArray();
    ret.reserve(elements.size());

    for (const auto &element : elements)
    {
        auto parsedObject = T::fromJson(element.toObject(), parent);
        if (!parsedObject)
        {
            continue;
        }
        ret.push_back(parsedObject);
    }

    return ret;
}


std::vector<ParameterSet*> parseFilterParamsArray(const QJsonObject &object, QObject* parent, Key key)
{
    std::vector<ParameterSet*> ret;
    auto it = object.find(s_keys[int(key)]);
    if (it == object.end())
    {
        return ret;
    }
    const auto elements = it.value().toArray();
    ret.reserve(elements.size());

    for (const auto &element : elements)
    {
        auto parsedObject = ParameterSet::fromJsonFilterParams(element.toObject(), parent);
        if (!parsedObject)
        {
            continue;
        }
        ret.push_back(parsedObject);
    }

    return ret;
}

std::vector<SeamSeries*> parseSeamSeries(const QJsonObject &object, Product *parent)
{
    return parseArray<SeamSeries>(object, parent, Key::SeamSeries);
}

std::vector<Seam*> parseSeams(const QJsonObject &object, SeamSeries *parent)
{
    return parseArray<Seam>(object, parent, Key::Seams);
}

std::vector<SeamInterval*> parseSeamIntervals(const QJsonObject &object, Seam *parent)
{
    return parseArray<SeamInterval>(object, parent, Key::SeamIntervals);
}

std::vector<ParameterSet*> parseFilterParameterSets(const QJsonObject &object, QObject *parent)
{
    return parseFilterParamsArray(object, parent, Key::FilterParameterSets);
}



int parseNumber(const QJsonObject &object)
{
    return parseInt(object, Key::Number);
}

int parseTriggerDelta(const QJsonObject &object)
{
    return parseInt(object, Key::TriggerDelta);
}

int parseVelocity(const QJsonObject &object)
{
    return parseInt(object, Key::Velocity);
}

int parseLocate(const QJsonObject &object)
{
    return parseInt(object, Key::Locate);
}

ParameterSet *parseHardwareParameters(const QJsonObject &object, QObject *parent)
{
    auto it = object.find(s_keys[int(Key::Hardware)]);
    if (it == object.end())
    {
        return nullptr;
    }
    const auto hardwareObject = it.value().toObject();
    if (hardwareObject.isEmpty())
    {
        return nullptr;
    }
    auto hwIt = hardwareObject.find(s_keys[int(Key::ParameterSet)]);
    if (hwIt == hardwareObject.end())
    {
        return nullptr;
    }

    return ParameterSet::fromJson(hwIt.value().toObject(), parent);
}

std::vector<Parameter*> parseParameters(const QJsonObject &object, ParameterSet *parent)
{
    return parseArray<Parameter>(object, parent, Key::Parameters);
}

std::vector<Parameter*> parseParameters(const QJsonObject &object, ParameterFilterGroup *parent)
{
    return parseArray<Parameter>(object, parent, Key::Parameters);
}

std::vector<ParameterFilterGroup*> parseParametersGroup(const QJsonObject &object, ParameterSet *parent)
{
    return parseArray<ParameterFilterGroup>(object, parent, Key::FilterParametersGroup);
}


std::vector<Attribute*> parseAttributes(const QJsonObject &object, QObject *parent)
{
    return parseArray<Attribute>(object, parent, Key::Attributes);
}

std::vector<AttributeField*> parseAttributeFields(const QJsonObject &object, QObject *parent)
{
    return parseArray<AttributeField>(object, parent, Key::AttributeFields);
}

std::vector<ResultSetting*> parseResultItems(const QJsonObject &object, QObject *parent)
{
    return parseArray<ResultSetting>(object, parent, Key::Result);
}

std::vector<QualityNorm*> parseQualityNorms(const QJsonObject &object, QObject *parent)
{
    return parseArray<QualityNorm>(object, parent, Key::QualityNorm);
}

std::vector<QualityNormResult*> parseQualityNormResults(const QJsonObject &object, QualityNorm *parent)
{
    return parseArray<QualityNormResult>(object, parent, Key::QualityNormResults);
}

std::vector<QualityNormResult*> parseCommonResults(const QJsonObject &object, QualityNorm *parent)
{
    return parseArray<QualityNormResult>(object, parent, Key::CommonResults);
}

QualityNormLevel* parseQualityNormLevel(const QJsonObject &object, QualityNormResult *parent, int level)
{
    auto levelIt = object.find(QStringLiteral("level %1").arg(level));
    if (levelIt != object.end())
    {
        return QualityNormLevel::fromJson(levelIt.value().toObject(), parent, level);
    }
    return new QualityNormLevel(level, parent);
}

std::vector<GaugeRange*> parseGaugeRanges(const QJsonObject &object, QualityNormLevel *parent)
{
    return parseArray<GaugeRange>(object, parent, Key::GaugeRange);
}

std::vector<SeamError*> parseSeamErrors(const QJsonObject &object, AbstractMeasureTask *parent)
{
    return parseArray<SeamError>(object, parent, Key::SeamError);
}

std::vector<SeamSeriesError*> parseSeamSeriesError(const QJsonObject &object, AbstractMeasureTask *parent)
{
    return parseArray<SeamSeriesError>(object, parent, Key::OverlyingError);
}

std::vector<ProductError*> parseProductError(const QJsonObject &object, Product *parent)
{
    return parseArray<ProductError>(object, parent, Key::OverlyingError);
}

// legacy support
std::vector<ReferenceCurve*> parseReferenceCurveSets(const QJsonObject &object, AbstractMeasureTask *parent)
{
    return parseArray<ReferenceCurve>(object, parent, Key::ReferenceCurveSet);
}

std::vector<ReferenceCurve*> parseReferenceCurves(const QJsonObject& object, AbstractMeasureTask* parent)
{
    return parseArray<ReferenceCurve>(object, parent, Key::ReferenceCurves);
}

std::vector<LinkedSeam*> parseLinkedSeams(const QJsonObject &object, Seam *parent)
{
    return parseArray<LinkedSeam>(object, parent, Key::LinkedSeams);
}

std::vector<IntervalError*> parseIntervalErrors(const QJsonObject &object, AbstractMeasureTask *parent)
{
    return parseArray<IntervalError>(object, parent, Key::IntervalError);
}

std::vector<QUuid> parseSubGraphs(const QJsonObject &object)
{
    std::vector<QUuid> ret;
    auto it = object.find(s_keys[int(Key::SubGraphs)]);
    if (it == object.end())
    {
        return ret;
    }
    const auto elements = it.value().toArray();
    ret.reserve(elements.size());

    std::transform(elements.begin(), elements.end(), std::back_inserter(ret), [] (const auto &value) { return QUuid::fromString(value.toString()); });

    return ret;
}

QVariant parseVariant(const QJsonObject &object, Key key)
{
    auto it = object.find(s_keys[int(key)]);
    if (it == object.end())
    {
        return QVariant();
    }
    return it.value().isNull() ? QVariant{} : it.value().toVariant();
}

QVariant parseValue(const QJsonObject &object)
{
    return parseVariant(object, Key::Value);
}

QVariant parseMinValue(const QJsonObject &object)
{
    return parseVariant(object, Key::MinValue);
}

QVariant parseMaxValue(const QJsonObject &object)
{
    return parseVariant(object, Key::MaxValue);
}

QVariant parseDefaultValue(const QJsonObject &object)
{
    return parseVariant(object, Key::DefaultValue);
}

Parameter::DataType parseDataType(const QJsonObject &object)
{
   
    const auto it = s_map.find(parseString(object, Key::Type));
    if (it == s_map.end())
    {
        return Parameter::DataType::Unknown;
    }
    return it->second;
}

Product::LengthUnit parseLengthUnit(const QJsonObject &object)
{
    static const std::map<QString, Product::LengthUnit> s_map{
        {QStringLiteral("millimeter"), Product::LengthUnit::Millimeter},
        {QStringLiteral("degree"), Product::LengthUnit::Degree}
    };
    const auto it = s_map.find(parseString(object, Key::LengthUnit));
    if (it == s_map.end())
    {
        return Product::LengthUnit::Millimeter;
    }
    return it->second;
}

int parseThicknessLeft(const QJsonObject &object)
{
    return parseInt(object, Key::ThicknessLeft);
}

int parseThicknessRight(const QJsonObject &object)
{
    return parseInt(object, Key::ThicknessRight);
}

int parseTargetDifference(const QJsonObject &object)
{
    return parseInt(object, Key::TargetDifference);
}

int parseMovingDirection(const QJsonObject &object)
{
    return parseInt(object, Key::MovingDirection);
}

int parseLevel(const QJsonObject &object)
{
    return parseInt(object, Key::Level);
}

QString parseResultName(const QJsonObject &object)
{
    return parseString(object, Key::ResultName);
}

QUuid parseUpper(const QJsonObject &object)
{
    return parseUuid(object, Key::UpperReference);
}

QUuid parseMiddle(const QJsonObject &object)
{
    return parseUuid(object, Key::MiddleReference);
}

QUuid parseLower(const QJsonObject &object)
{
    return parseUuid(object, Key::LowerReference);
}

float parseJitter(const QJsonObject &object)
{
    return float(parseReal(object, Key::Jitter));
}

QString parseReferenceType(const QJsonObject &object)
{
    return parseString(object, Key::ReferenceType);
}

QUuid parseLaserControl(const QJsonObject &object)
{
    return parseUuid(object, Key::LC_Preset);
}

std::vector<int> parsePower(const QJsonObject &object)
{
    std::vector<int> ret;
    auto it = object.find(s_keys[int(Key::LC_Power)]);
    if (it == object.end())
    {
        // legacy
        auto key = QString(s_keys[int(Key::LC_Power)]);
        for (auto i = 1u; i <= LaserControlPreset::numberOfSamples(); i++)
        {
            auto it = object.find(key + QString::number(i));
            ret.push_back(it == object.end() ? 0 : it.value().toInt());
        }
        return ret;
    }
    const auto elements = it.value().toArray();
    ret.reserve(elements.size());

    for (const auto& element : elements)
    {
        ret.push_back(element.toInt());
    }

    return ret;
}

std::vector<int> parseOffset(const QJsonObject &object)
{
    std::vector<int> ret;
    auto it = object.find(s_keys[int(Key::LC_Offset)]);
    if (it == object.end())
    {
        // legacy
        auto key = QString(s_keys[int(Key::LC_Offset)]);
        for (auto i = 1u; i <= LaserControlPreset::numberOfSamples(); i++)
        {
            auto it = object.find(key + QString::number(i));
            ret.push_back(it == object.end() ? 0 : it.value().toInt());
        }
        return ret;
    }
    const auto elements = it.value().toArray();
    ret.reserve(elements.size());

    for (const auto& element : elements)
    {
        ret.push_back(element.toInt());
    }

    return ret;
}

std::map<QUuid, QUuid> parseIntervalIds(const QJsonObject &object)
{
    std::map<QUuid, QUuid> map;
    auto it = object.find(s_keys[int(Key::IntervalIds)]);
    if (it == object.end())
    {
        return map;
    }
    auto array = it.value().toArray();

    for (const auto &element : array)
    {
        const auto obj = element.toObject();
        const auto itInterval = obj.find(QStringLiteral("interval"));
        const auto itError = obj.find(QStringLiteral("error"));

        if (itInterval == obj.end() || itError == obj.end())
        {
            continue;
        }
        const auto interval = QUuid::fromString(itInterval.value().toString());
        const auto error = QUuid::fromString(itError.value().toString());

        map.emplace(interval , error);
    }
    return map;
}

std::vector<LevelConfig*> parseLevelConfigs(const QJsonObject &object, IntervalError *parent)
{
    std::vector<LevelConfig*> levels;
    auto it = object.find(s_keys[int(Key::IntervalLevels)]);
    if (it == object.end())
    {
        for (auto i = 0u; i < AbstractMeasureTask::maxLevel(); i++)
        {
            levels.push_back(new LevelConfig(parent));
        }
        return levels;
    }
    auto levelsObject = it.value().toObject();
    for (auto i = 0u; i < AbstractMeasureTask::maxLevel(); i++)
    {
        auto levelIt = levelsObject.find(QStringLiteral("level %1").arg(i + 1));
        if (levelIt == levelsObject.end())
        {
            levels.push_back(new LevelConfig(parent));
            continue;
        }
        levels.push_back(LevelConfig::fromJson(levelIt.value().toObject(), parent));
    }
    return levels;
}

QUuid parseEnvelope(const QJsonObject &object)
{
    return parseUuid(object, Key::Envelope);
}

bool parseMiddleCurveAsReference(const QJsonObject &object)
{
    return parseBoolean(object, Key::UseMiddleCurveAsReference);
}

int parseLwmTriggerSignalType(const QJsonObject& object)
{
    return parseInt(object, Key::LwmTriggerSignalType, -1);
}

double parseLwmTriggerSignalThreshold(const QJsonObject& object)
{
    return parseReal(object, Key::LwmTriggerSignalThreshold);
}

QUuid parseGroupId(const QJsonObject &object)
{
    return parseUuid(object, Key::GroupId);
}

int parseGroupIndex(const QJsonObject& object)
{
    return parseInt(object, Key::GroupIndex);
}

QStringList parseFileSuffixes(const QJsonObject& object)
{
    auto it = object.find(s_keys[int(Key::FileSuffixes)]);
    if (it == object.end())
    {
        return {};
    }
    const auto elements = it.value().toArray();
    QStringList ret{};
    ret.reserve(elements.size());

    for (const auto &element : elements)
    {
        ret.append(element.toString());
    }
    return ret;
}

QString parseFileLocation(const QJsonObject& object)
{
    return parseString(object, Key::FileLocation);
}

GraphReference parseGraphReference(const QJsonObject& object)
{
    auto subGraph = parseSubGraphs(object);
    auto graph = parseGraph(object);
    auto linkedGraph = parseUuid(object, Key::LinkedGraph);
    if (!subGraph.empty())
    {
        return SubGraphReference{std::move(subGraph)};
    }
    else if (!graph.isNull())
    {
        return SingleGraphReference{std::move(graph)};
    }
    else if (!linkedGraph.isNull())
    {
        return LinkedGraphReference{std::move(linkedGraph)};
    }
    return {};
}

QString uuidToString(const QUuid &id)
{
    return id.toString(QUuid::WithoutBraces);
}

QPair<QString, QJsonValue> toJson(const QUuid &id)
{  
    return qMakePair(s_keys[int(Key::Uuid)], uuidToString(id));
}

QPair<QString, QJsonValue> typeIdToJson(const QUuid &id)
{
    return qMakePair(s_keys[int(Key::TypeId)], uuidToString(id));
}

QPair<QString, QJsonValue> filterIdToJson(const QUuid &id)
{
    return qMakePair(s_keys[int(Key::FilterId)], uuidToString(id));
}

QPair<QString, QJsonValue> graphToJson(const QUuid &id)
{
    return qMakePair(s_keys[int(Key::GraphUuid)], uuidToString(id));
}

QPair<QString, QJsonValue> linkedGraphToJson(const QUuid& id)
{
    return qMakePair(s_keys[int(Key::LinkedGraph)], uuidToString(id));
}

QPair<QString, QJsonValue> graphReferenceToJson(const GraphReference& ref)
{
    return std::visit(precitec::storage::overloaded{[](const SingleGraphReference& singleRef)
                                                    {
                                                        return graphToJson(singleRef.value);
                                                    },
                                                    [](const SubGraphReference& subRef)
                                                    {
                                                        return subGraphsToJson(subRef.value);
                                                    },
                                                    [](const LinkedGraphReference& linkedRef)
                                                    {
                                                        return linkedGraphToJson(linkedRef.value);
                                                    }},
                      ref);
}

QPair<QString, QJsonValue> graphParamSetToJson(const QUuid &id)
{
    return qMakePair(s_keys[int(Key::GraphParamSetUuid)], uuidToString(id));
}

QPair<QString, QJsonValue> variantIdToJson(const QUuid &id)
{
    return qMakePair(s_keys[int(Key::VariantId)], uuidToString(id));
}

QPair<QString, QJsonValue> segmentIdToJson(const QUuid &id)
{
    return qMakePair(s_keys[int(Key::SegmentId)], uuidToString(id));
}

QPair<QString, QJsonValue> qualityNormToJson(const QUuid &id)
{
    return qMakePair(s_keys[int(Key::QualityNorm)], uuidToString(id));
}

QPair<QString, QJsonValue> nameToJson(const QString &name)
{   
    return qMakePair(s_keys[int(Key::Name)], name);
}

QPair<QString, QJsonValue> assemblyImageToJson(const QString &image)
{
    return qMakePair(s_keys[int(Key::AssemblyImage)], image);
}

QPair<QString, QJsonValue> boolToJson(bool value, Key key)
{
    return qMakePair(s_keys[int(key)], value);
}

QPair<QString, QJsonValue> endlessToJson(bool endless)
{
    return boolToJson(endless, Key::Endless);
}

QPair<QString, QJsonValue> defaultToJson(bool defaultProduct)
{
    return boolToJson(defaultProduct, Key::Default);
}

QPair<QString, QJsonValue> activeToJson(bool active)
{
    return boolToJson(active, Key::Active);
}

QPair<QString, QJsonValue> intToJson(int value, Key key)
{
    return qMakePair(s_keys[int(key)], value);
}

QPair<QString, QJsonValue> realToJson(qreal value, Key key)
{
    return qMakePair(s_keys[int(key)], value);
}

QPair<QString, QJsonValue> typeToJson(int type)
{
    return intToJson(type, Key::Type);
}

QPair<QString, QJsonValue> lengthToJson(qint64 length)
{
    return qMakePair(s_keys[int(Key::Length)], QString::number(length));
}

QPair<QString, QJsonValue> triggerDeltaToJson(int triggerDelta)
{
    return intToJson(triggerDelta, Key::TriggerDelta);
}

QPair<QString, QJsonValue> velocityToJson(int velocity)
{
    return intToJson(velocity, Key::Velocity);
}

QPair<QString, QJsonValue> numberToJson(int number)
{
    return intToJson(number, Key::Number);
}

QPair<QString, QJsonValue> positionInAssemblyImageToJson(const QPointF &point)
{
    return qMakePair(s_keys[int(Key::PositionInAssemblyImage)], QJsonObject{
        realToJson(point.x(), Key::X),
        realToJson(point.y(), Key::Y)
    });
}

QPair<QString, QJsonValue> roiToJson(const QRect &rect)
{
    return qMakePair(s_keys[int(Key::Roi)], QJsonObject{
        intToJson(rect.x(), Key::X),
        intToJson(rect.y(), Key::Y),
        intToJson(rect.width(), Key::Width),
        intToJson(rect.height(), Key::Height)
    });

}

QPair<QString, QJsonValue> visualizationToJson(int visualization)
{
    return intToJson(visualization, Key::Visualization);
}

QPair<QString, QJsonValue> enumTypeToJson(int type)
{
    return intToJson(type, Key::Enumeration);
}

QPair<QString, QJsonValue> plotNumToJson(int type)
{
    return intToJson(type, Key::PlotNumber);
}

QPair<QString, QJsonValue> minToJson(double type)
{
    return realToJson(type, Key::MinValue);
}

QPair<QString, QJsonValue> maxToJson(double type)
{
    return realToJson(type, Key::MaxValue);
}

QPair<QString, QJsonValue> colorToJson(const QString &color)
{
    return qMakePair(s_keys[int(Key::LineColor)], color);
}

QPair<QString, QJsonValue> plottableToJson(int type)
{
    return intToJson(type, Key::Plottable);
}

QPair<QString, QJsonValue> visibleToJson(int type)
{
    return intToJson(type, Key::Visible);
}

QPair<QString, QJsonValue> startPositionYAxisToJson(int startPosition)
{
    return intToJson(startPosition, Key::StartPosYAxis);
}


QString getKeyFromValue(std::map<QString, Parameter::DataType> map, Parameter::DataType value)
{
    for(auto kv : map)
    {
        if(kv.second == value)
        {         
            return kv.first;
        }
    }    
    return QString::number(static_cast<int>(value));
}
        
QPair<QString, QJsonValue> dataTypeToJson(Parameter::DataType dataType)
{    
    const auto it = s_map.find(getKeyFromValue(s_map, dataType));
    if (it == s_map.end())
    {
     
        return qMakePair(s_keys[int(Key::Type)], QString::fromLatin1("int"));
    }    
  
    return qMakePair(s_keys[int(Key::Type)], it->first);
}

QJsonValue parameterSetToJson(ParameterSet *ps)
{
    if (!ps)
    {
        return QJsonObject{};
    }
    return ps->toJson();
}

QJsonValue filterParamsToJson(ParameterFilterGroup *filterParamGroup)
{
    if (!filterParamGroup)
    {
        return QJsonObject{};
    }
    return filterParamGroup->toJson();
}

QPair<QString, QJsonValue> hardwareToJson(int startPositionYAxis, ParameterSet *hardwareParameters)
{
    return qMakePair(s_keys[int(Key::Hardware)], QJsonObject{
        {
            startPositionYAxisToJson(startPositionYAxis),
            {s_keys[int(Key::ParameterSet)], parameterSetToJson(hardwareParameters)}
        }});
}

QPair<QString, QJsonValue> hardwareParametersToJson(ParameterSet *hardwareParameters)
{
    return qMakePair(s_keys[int(Key::Hardware)], QJsonObject{
        {
            {s_keys[int(Key::ParameterSet)], parameterSetToJson(hardwareParameters)}
        }});
}



QPair<QString, QJsonValue> triggerSourceToJson(Product::TriggerSource source)
{
    QString sourceName;
    switch (source)
    {
    case Product::TriggerSource::External:
        sourceName = QStringLiteral("External");
        break;
    case Product::TriggerSource::GrabberControlled:
        sourceName = QStringLiteral("Grabber");
        break;
    case Product::TriggerSource::Software:
        sourceName = QStringLiteral("Software");
        break;
    };
    return qMakePair(s_keys[int(Key::TriggerSource)], sourceName);
}

QPair<QString, QJsonValue> triggerModeToJson(Product::TriggerMode mode)
{
    QString modeName;
    switch (mode)
    {
    case Product::TriggerMode::Burst:
        modeName = QStringLiteral("Burst");
        break;
    case Product::TriggerMode::Continue:
        modeName = QStringLiteral("Continue");
        break;
    case Product::TriggerMode::None:
        modeName = QStringLiteral("None");
        break;
    case Product::TriggerMode::Single:
        modeName = QStringLiteral("Single");
        break;
    }
    return qMakePair(s_keys[int(Key::TriggerMode)], modeName);
}

QPair<QString, QJsonValue> triggerToJson(Product::TriggerSource source, Product::TriggerMode mode)
{
    return qMakePair(s_keys[int(Key::Trigger)], QJsonObject{
        {
            triggerSourceToJson(source),
            triggerModeToJson(mode)
        }});
}

template <typename T>
QJsonArray toJsonArray(const std::vector<T*> elements)
{  
    QJsonArray array;
    for (auto element : elements)
    {         
       array << element->toJson();
    }
    return array;
}

template <typename T>
QJsonArray toJsonArray(const std::vector<T*> elements, const QObject* parent)
{  
    QJsonArray array;
    for (auto element : elements)
    {       
        array << element->toJson(parent);    
    }
    return array;
}

QPair<QString, QJsonValue> toJson(const std::vector<Parameter*> &parameters)
{
    return qMakePair(s_keys[int(Key::Parameters)], toJsonArray(parameters));
}



QPair<QString, QJsonValue> toJson(const std::vector<Parameter*> &parameters, const QObject* parent)
{
    return qMakePair(s_keys[int(Key::Parameters)], toJsonArray(parameters, parent));
}

QPair<QString, QJsonValue> toJson(const std::vector<Attribute*> &attributes)
{
    
    return qMakePair(s_keys[int(Key::Attributes)], toJsonArray(attributes));
}

QPair<QString, QJsonValue> toJson(const std::vector<AttributeField*> &fields)
{
    return qMakePair(s_keys[int(Key::Fields)], toJsonArray(fields));
}

QPair<QString, QJsonValue> toJson(const QVariant &variant)
{
    return qMakePair(s_keys[int(Key::Value)], QJsonValue::fromVariant(variant));
}



QPair<QString, QJsonValue> toJson(Parameter::DataType dataType)
{
    QString dataTypeString;
    switch (dataType)
    {
    case Parameter::DataType::Integer:
        dataTypeString = QStringLiteral("int");
        break;
    case Parameter::DataType::UnsignedInteger:
        dataTypeString = QStringLiteral("uint");
        break;
    case Parameter::DataType::Float:
        dataTypeString = QStringLiteral("float");
        break;
    case Parameter::DataType::Double:
        dataTypeString = QStringLiteral("double");
        break;
    case Parameter::DataType::Boolean:
        dataTypeString = QStringLiteral("bool");
        break;
    case Parameter::DataType::Enumeration:
        dataTypeString = QStringLiteral("enum");
        break;
    case Parameter::DataType::Error:
        dataTypeString = QStringLiteral("error");
        break;
    case Parameter::DataType::Result:
        dataTypeString = QStringLiteral("result");
        break;
    case Parameter::DataType::String:
        dataTypeString = QStringLiteral("string");
        break;
    case Parameter::DataType::Text:
        dataTypeString = QStringLiteral("text");
        break;
    case Parameter::DataType::Sensor:
        dataTypeString = QStringLiteral("sensor");
        break;
    case Parameter::DataType::SeamFigure:
        dataTypeString = QStringLiteral("seamFigure");
        break;
    case Parameter::DataType::WobbleFigure:
        dataTypeString = QStringLiteral("wobbleFigure");
        break;
    case Parameter::DataType::File:
        dataTypeString = QStringLiteral("file");
        break;
    case Parameter::DataType::Unknown:
        // nothing
        break;
    }

    return qMakePair(s_keys[int(Key::Type)], dataTypeString);
}

QPair<QString, QJsonValue> toJson(const std::vector<ParameterSet*> &parameterSets)
{
    return qMakePair(s_keys[int(Key::FilterParameterSets)], toJsonArray(parameterSets));
}

QPair<QString, QJsonValue> toJson(const std::vector<ParameterFilterGroup*> &parameterGroup)
{
    return qMakePair(s_keys[int(Key::FilterParametersGroup)], toJsonArray(parameterGroup));
}


QPair<QString, QJsonValue> toJson(const std::vector<SeamSeries*> &seamSeries)
{
    return qMakePair(s_keys[int(Key::SeamSeries)], toJsonArray(seamSeries));
}

QPair<QString, QJsonValue> toJson(const std::vector<Seam*> &seams)
{
    return qMakePair(s_keys[int(Key::Seams)], toJsonArray(seams));
}

QPair<QString, QJsonValue> toJson(const std::vector<SeamInterval*> &seamIntervals)
{
    return qMakePair(s_keys[int(Key::SeamIntervals)], toJsonArray(seamIntervals));
}

QPair<QString, QJsonValue> toJson(const std::vector<ResultSetting *> &resultItems)
{
    return qMakePair(s_keys[int(Key::Result)], toJsonArray(resultItems));
}

QPair<QString, QJsonValue> toJson(const std::vector<SeamError*> &errors)
{
    return qMakePair(s_keys[int(Key::SeamError)], toJsonArray(errors));
}

QPair<QString, QJsonValue> toJson(const std::vector<ProductError*> &errors)
{
    return qMakePair(s_keys[int(Key::OverlyingError)], toJsonArray(errors));
}

QPair<QString, QJsonValue> toJson(const std::vector<SeamSeriesError*> &errors)
{
    return qMakePair(s_keys[int(Key::OverlyingError)], toJsonArray(errors));
}

QPair<QString, QJsonValue> toJson(const std::vector<ReferenceCurve*> &referenceCurves)
{
    return qMakePair(s_keys[int(Key::ReferenceCurves)], toJsonArray(referenceCurves));
}

QPair<QString, QJsonValue> toJson(const std::vector<LinkedSeam*> &linkedSeams)
{
    return qMakePair(s_keys[int(Key::LinkedSeams)], toJsonArray(linkedSeams));
}

QPair<QString, QJsonValue> toJson(const std::vector<IntervalError*> &errors)
{
    return qMakePair(s_keys[int(Key::IntervalError)], toJsonArray(errors));
}

QPair<QString, QJsonValue> subGraphsToJson(const std::vector<QUuid> &uuids)
{
    QJsonArray array;
    std::transform(uuids.begin(), uuids.end(), std::back_inserter(array), [] (auto uuid) { return uuidToString(uuid); });
    return qMakePair(s_keys[int(Key::SubGraphs)], array);
}

QPair<QString, QJsonValue> toJson(Product::LengthUnit unit)
{
    QString lengthUnitString;
    switch (unit)
    {
    case Product::LengthUnit::Millimeter:
        lengthUnitString = QStringLiteral("millimeter");
        break;
    case Product::LengthUnit::Degree:
        lengthUnitString = QStringLiteral("degree");
        break;
    }
    return qMakePair(s_keys[int(Key::LengthUnit)], lengthUnitString);
}

QPair<QString, QJsonValue> thicknessLeftToJson(int thickness)
{
    return intToJson(thickness, Key::ThicknessLeft);
}

QPair<QString, QJsonValue> thicknessRightToJson(int thickness)
{
    return intToJson(thickness, Key::ThicknessRight);
}

QPair<QString, QJsonValue> targetDifferenceToJson(int difference)
{
    return intToJson(difference, Key::TargetDifference);
}

QPair<QString, QJsonValue> movingDirectionToJson(int direction)
{
    return intToJson(direction, Key::MovingDirection);
}

QPair<QString, QJsonValue> levelToJson(int level)
{
    return intToJson(level, Key::Level);
}

QPair<QString, QJsonValue> resultNameToJson(QString result)
{
    return qMakePair(s_keys[int(Key::ResultName)], result);
}

QPair<QString, QJsonValue> upperToJson(const QUuid &id)
{
    return qMakePair(s_keys[int(Key::UpperReference)], uuidToString(id));
}

QPair<QString, QJsonValue> middleToJson(const QUuid &id)
{
    return qMakePair(s_keys[int(Key::MiddleReference)], uuidToString(id));
}

QPair<QString, QJsonValue> lowerToJson(const QUuid &id)
{
    return qMakePair(s_keys[int(Key::LowerReference)], uuidToString(id));
}

QPair<QString, QJsonValue> jitterToJson(const float jitter)
{
    return realToJson(jitter, Key::Jitter);
}

QPair<QString, QJsonValue> referenceTypeToJson(const QString &type)
{
    return qMakePair(s_keys[int(Key::ReferenceType)], type);
}

QPair<QString, QJsonValue> laserControlToJson(const QUuid &id)
{
    return qMakePair(s_keys[int(Key::LC_Preset)], uuidToString(id));
}

QPair<QString, QJsonValue> descriptionToJson(QString description)
{
    return qMakePair(s_keys[int(Key::Description)], description);
}

QPair<QString, QJsonValue> toolTipToJson(QString toolTip)
{
    return qMakePair(s_keys[int(Key::ToolTip)], toolTip);
}

QPair<QString, QJsonValue> enumerationToJson(QString enumeration)
{
    return qMakePair(s_keys[int(Key::Enumeration)], enumeration);
}

QPair<QString, QJsonValue> contentNameToJson(QString contentName)
{
    return qMakePair(s_keys[int(Key::ContentName)], contentName);
}

QPair<QString, QJsonValue> unitToJson(QString unit)
{
    return qMakePair(s_keys[int(Key::Unit)], unit);
}

QPair<QString, QJsonValue> attributeIdToJson(QUuid attributeId)
{
    return qMakePair(s_keys[int(Key::AttributeId)], uuidToString(attributeId));
}

QPair<QString, QJsonValue> locateToJson(int locate)
{
    return intToJson(locate, Key::Locate);
}

QPair<QString, QJsonValue> textToJson(QString text)
{
    return qMakePair(s_keys[int(Key::Text)], text);
}

QPair<QString, QJsonValue> powerToJson(const std::vector<int>& power)
{
    QJsonArray array;
    for (auto pow : power)
    {
        array << pow;
    }
    return qMakePair(s_keys[int(Key::LC_Power)], array);
}

QPair<QString, QJsonValue> offsetToJson(const std::vector<int>& offset)
{
    QJsonArray array;
    for (auto off : offset)
    {
        array << off;
    }
    return qMakePair(s_keys[int(Key::LC_Offset)], array);
}

QPair<QString, QJsonValue> levelConfigsToJson(const std::vector<LevelConfig*> &levels)
{
    QJsonObject obj;
    for (auto i = 0u; i < levels.size(); i++)
    {
        obj.insert(QStringLiteral("level %1").arg(i + 1), levels.at(i)->toJson());
    }
    return qMakePair(s_keys[int(Key::IntervalLevels)], obj);
}

QPair<QString, QJsonValue> intervalIdsToJson(const std::map<QUuid, QUuid> &errorIds)
{
    QJsonArray array;
    for (auto it : errorIds)
    {
        QJsonObject obj = {
            qMakePair(QStringLiteral("interval"), uuidToString(it.first)),
            qMakePair(QStringLiteral("error"), uuidToString(it.second))
        };
        array << obj;
    }
    return qMakePair(s_keys[int(Key::IntervalIds)], array);
}

QPair<QString, QJsonValue> envelopeToJson(const QUuid &id)
{
    return qMakePair(s_keys[int(Key::Envelope)], uuidToString(id));
}

QPair<QString, QJsonValue> middleCurveAsReferenceToJson(bool use)
{
    return boolToJson(use, Key::UseMiddleCurveAsReference);
}

QPair<QString, QJsonValue> mandatoryToJson(bool mandatory)
{    
    return boolToJson(mandatory, Key::Mandatory);
}


QPair<QString, QJsonValue> linkedSeamLabelToJson(const QString &label)
{
    return qMakePair(s_keys[int(Key::LinkedSeamLabel)], label);
}

QPair<QString, QJsonValue> lwmTriggerSignalTypeToJson(int type)
{
    return intToJson(type, Key::LwmTriggerSignalType);
}

QPair<QString, QJsonValue> lwmTriggerSignalThresholdToJson(double threshold)
{
    return realToJson(threshold, Key::LwmTriggerSignalThreshold);
}

QPair<QString, QJsonValue> maxValueToJson(const QVariant &maxValue)
{
    return qMakePair(s_keys[int(Key::MaxValue)], QJsonValue::fromVariant(maxValue));
}

QPair<QString, QJsonValue> minValueToJson(const QVariant &minValue)
{
    return qMakePair(s_keys[int(Key::MinValue)], QJsonValue::fromVariant(minValue));
}

QPair<QString, QJsonValue> defaultValueToJson(const QVariant &defaultValue)
{
    return qMakePair(s_keys[int(Key::DefaultValue)], QJsonValue::fromVariant(defaultValue));
}

QPair<QString, QJsonValue> maxLengthToJson(int maxLength)
{   
    return intToJson(maxLength, Key::MaxLength);
}

QPair<QString, QJsonValue> userLevelToJson(int userLevel)
{
    return intToJson(userLevel, Key::UserLevel);
}

QPair<QString, QJsonValue> visibleToJson(bool visible)
{
    return boolToJson(visible, Key::Visible);
}

QPair<QString, QJsonValue> stepToJson(int step)
{
    return intToJson(step, Key::Step);
}

QPair<QString, QJsonValue> editListOrderToJson(int editListOrder)
{
    return intToJson(editListOrder, Key::EditListOrder);
}

QPair<QString, QJsonValue> publicityToJson(bool publicity)
{    
    return boolToJson(publicity, Key::Publicity);
}

}
}
}
