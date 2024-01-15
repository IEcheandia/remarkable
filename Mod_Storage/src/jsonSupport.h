#pragma once
#include "parameter.h"
#include "product.h"

#include <QUuid>
#include <QVariant>

#include <vector>

class QObject;
class QJsonObject;

namespace precitec
{
namespace storage
{
class Attribute;
class AttributeField;
class Parameter;
class ParameterSet;
class SeamSeries;
class Seam;
class SeamInterval;
class ResultSetting;
class ReferenceCurve;
class ProductError;
class SeamSeriesError;
class SeamError;
class IntervalError;
class LevelConfig;
class QualityNorm;
class QualityNormResult;
class QualityNormLevel;
class GaugeRange;
class LinkedSeam;

struct SingleGraphReference;
struct SubGraphReference;
struct LinkedGraphReference;
using GraphReference = std::variant<SubGraphReference, SingleGraphReference, LinkedGraphReference>;

namespace json
{

QUuid parseUuid(const QJsonObject &object);
QUuid parseGraph(const QJsonObject &object);
QUuid parseGraphParamSet(const QJsonObject &object);
QUuid parseTypeId(const QJsonObject &object);
QUuid parseFilterId(const QJsonObject &object);
QUuid parseVariantId(const QJsonObject &object);
QUuid parseSegmentId(const QJsonObject &object);
QUuid parseAttributeId(const QJsonObject &object);
QUuid parseQualityNorm(const QJsonObject &object);
QString parseName(const QJsonObject &object);
QString parseUnit(const QJsonObject &object);
QString parseDescription(const QJsonObject &object);
QString parseToolTip(const QJsonObject &object);
QString parseEnumeration(const QJsonObject &object);
QString parseContentName(const QJsonObject &object);
QString parseText(const QJsonObject &object);
QString parseAssemblyImage(const QJsonObject &object);
int parseType(const QJsonObject &object);
int parseMaxLength(const QJsonObject &object);
int parseStep(const QJsonObject &object);
int parseEditListOrder(const QJsonObject &object);
int parseUserLevel(const QJsonObject &object);
bool parseEndless(const QJsonObject &object);
bool parseDefault(const QJsonObject &object);
bool parseActive(const QJsonObject &object);
bool parseMandatory(const QJsonObject &object);
bool parseVisible(const QJsonObject &object);
bool parsePublicity(const QJsonObject &object);
int parseStartPosYAxis(const QJsonObject &object);
int parseNumber(const QJsonObject &object);
int parseTriggerDelta(const QJsonObject &object);
int parseVelocity(const QJsonObject &object);
int parseLocate(const QJsonObject &object);
qint64 parseLength(const QJsonObject &object);
QPointF parsePositionInAssemblyImage(const QJsonObject &object);
QRect parseRoi(const QJsonObject &object);
int parseEnumType(const QJsonObject &object);
int parsePlotterNumber(const QJsonObject &object);
int parsePlottable(const QJsonObject &object);
int parseVisibleItem(const QJsonObject &object);
int parseVisualization(const QJsonObject &object);
int parseFloatingPointPrecision(const QJsonObject &object);
QString parseLineColor(const QJsonObject &object);
std::vector<SeamSeries*> parseSeamSeries(const QJsonObject &object, Product *parent);
std::vector<Seam*> parseSeams(const QJsonObject &object, SeamSeries *parent);
std::vector<SeamInterval*> parseSeamIntervals(const QJsonObject &object, Seam *parent);
std::vector<ParameterSet*> parseFilterParameterSets(const QJsonObject &object, QObject *parent);
std::vector<ParameterFilterGroup*> parseFilterParametersGroups(const QJsonObject &object, QObject *parent);
std::tuple<Product::TriggerSource, Product::TriggerMode> parseTrigger(const QJsonObject &object);
ParameterSet *parseHardwareParameters(const QJsonObject &object, QObject *parent);
std::vector<Parameter*> parseParameters(const QJsonObject &object, ParameterSet *parent);
std::vector<Parameter*> parseParameters(const QJsonObject &object, ParameterFilterGroup *parent);
std::vector<ParameterFilterGroup*> parseParametersGroup(const QJsonObject &object, ParameterSet *parent);
std::vector<Attribute*> parseAttributes(const QJsonObject &object, QObject *parent);
std::vector<AttributeField*> parseAttributeFields(const QJsonObject &object, QObject *parent);
std::vector<ResultSetting*> parseResultItems(const QJsonObject &object, QObject *parent);
std::vector<QualityNorm*> parseQualityNorms(const QJsonObject &object, QObject *parent);
std::vector<QualityNormResult*> parseQualityNormResults(const QJsonObject &object, QualityNorm *parent);
std::vector<QualityNormResult*> parseCommonResults(const QJsonObject &object, QualityNorm *parent);
QualityNormLevel* parseQualityNormLevel(const QJsonObject &object, QualityNormResult *parent, int level);
std::vector<GaugeRange*> parseGaugeRanges(const QJsonObject &object, QualityNormLevel *parent);
std::vector<ProductError*> parseProductError(const QJsonObject &object, Product *parent);
std::vector<SeamSeriesError*> parseSeamSeriesError(const QJsonObject &object, AbstractMeasureTask *parent);
std::vector<SeamError*> parseSeamErrors(const QJsonObject &object, AbstractMeasureTask *parent);
std::vector<IntervalError*> parseIntervalErrors(const QJsonObject &object, AbstractMeasureTask *parent);
std::vector<ReferenceCurve*> parseReferenceCurveSets(const QJsonObject &object, AbstractMeasureTask *parent);
std::vector<ReferenceCurve*> parseReferenceCurves(const QJsonObject& object, AbstractMeasureTask* parent);
std::vector<LinkedSeam*> parseLinkedSeams(const QJsonObject &object, Seam *parent);
std::vector<QUuid> parseSubGraphs(const QJsonObject &object);
QVariant parseValue(const QJsonObject &object);
QVariant parseMinValue(const QJsonObject &object);
QVariant parseMaxValue(const QJsonObject &object);
QVariant parseDefaultValue(const QJsonObject &object);
Parameter::DataType parseDataType(const QJsonObject &object);
Product::LengthUnit parseLengthUnit(const QJsonObject &object);
int parseThicknessLeft(const QJsonObject &object);
int parseThicknessRight(const QJsonObject &object);
int parseTargetDifference(const QJsonObject &object);
int parseMovingDirection(const QJsonObject &object);
int parseLevel(const QJsonObject &object);
QString parseResultName(const QJsonObject &object);
QUuid parseUpper(const QJsonObject &object);
QUuid parseMiddle(const QJsonObject &object);
QUuid parseLower(const QJsonObject &object);
float parseJitter(const QJsonObject &object);
QString parseReferenceType(const QJsonObject &object);
QUuid parseLaserControl(const QJsonObject &object);
std::vector<int> parsePower(const QJsonObject &object);
std::vector<int> parseOffset(const QJsonObject &object);
std::map<QUuid, QUuid> parseIntervalIds(const QJsonObject &object);
std::vector<LevelConfig*> parseLevelConfigs(const QJsonObject &object, IntervalError *parent);
QUuid parseEnvelope(const QJsonObject &object);
bool parseMiddleCurveAsReference(const QJsonObject &object);
qreal parseRange(const QJsonObject &object);
qreal parseMinFactor(const QJsonObject &object);
qreal parseMinOffset(const QJsonObject &object);
qreal parseMaxFactor(const QJsonObject &object);
qreal parseMaxOffset(const QJsonObject &object);
QString parseLinkedSeamLabel(const QJsonObject &object);
int parseLwmTriggerSignalType(const QJsonObject& object);
double parseLwmTriggerSignalThreshold(const QJsonObject& object);
QUuid parseGroupId(const QJsonObject& object);
int parseGroupIndex(const QJsonObject& object);
QStringList parseFileSuffixes(const QJsonObject& object);
QString parseFileLocation(const QJsonObject& object);
QJsonValue filterParamsToJson(ParameterFilterGroup *filterParamGroup);
GraphReference parseGraphReference(const QJsonObject& object);

QPair<QString, QJsonValue> toJson(const QUuid &id);
QPair<QString, QJsonValue> typeIdToJson(const QUuid &id);
QPair<QString, QJsonValue> filterIdToJson(const QUuid &id);
QPair<QString, QJsonValue> graphToJson(const QUuid &id);
QPair<QString, QJsonValue> linkedGraphToJson(const QUuid& id);
QPair<QString, QJsonValue> graphReferenceToJson(const GraphReference& ref);
QPair<QString, QJsonValue> graphParamSetToJson(const QUuid &id);
QPair<QString, QJsonValue> variantIdToJson(const QUuid &id);
QPair<QString, QJsonValue> segmentIdToJson(const QUuid &id);
QPair<QString, QJsonValue> qualityNormToJson(const QUuid &id);
QPair<QString, QJsonValue> nameToJson(const QString &name);
QPair<QString, QJsonValue> assemblyImageToJson(const QString &image);
QPair<QString, QJsonValue> endlessToJson(bool endless);
QPair<QString, QJsonValue> defaultToJson(bool defaultProduct);
QPair<QString, QJsonValue> activeToJson(bool active);
QPair<QString, QJsonValue> typeToJson(int type);
QPair<QString, QJsonValue> lengthToJson(qint64 length);
QPair<QString, QJsonValue> triggerDeltaToJson(int triggerDelta);
QPair<QString, QJsonValue> velocityToJson(int velocity);
QPair<QString, QJsonValue> numberToJson(int number);
QPair<QString, QJsonValue> enumTypeToJson(int type);
QPair<QString, QJsonValue> plotNumToJson(int type);
QPair<QString, QJsonValue> minToJson(double type);
QPair<QString, QJsonValue> maxToJson(double type);
QPair<QString, QJsonValue> colorToJson(const QString &type);
QPair<QString, QJsonValue> plottableToJson(int type);
QPair<QString, QJsonValue> visibleToJson(int type);
QPair<QString, QJsonValue> hardwareToJson(int startPositionYAxis, ParameterSet *hardwareParameters);
QPair<QString, QJsonValue> dataTypeToJson(Parameter::DataType dataType);
QPair<QString, QJsonValue> hardwareParametersToJson(ParameterSet *hardwareParameters);
QPair<QString, QJsonValue> triggerToJson(Product::TriggerSource source, Product::TriggerMode mode);
QPair<QString, QJsonValue> positionInAssemblyImageToJson(const QPointF &point);
QPair<QString, QJsonValue> roiToJson(const QRect &rect);
QPair<QString, QJsonValue> visualizationToJson(int visualization);
QPair<QString, QJsonValue> toJson(const std::vector<Parameter*> &parameters);
QPair<QString, QJsonValue> toJson(const std::vector<Parameter*> &parameters, const QObject* parent);
QPair<QString, QJsonValue> toJson(const std::vector<Attribute*> &attributes);
QPair<QString, QJsonValue> toJson(const std::vector<AttributeField*> &fields);
QPair<QString, QJsonValue> toJson(const QVariant &variant);
QPair<QString, QJsonValue> toJson(Parameter::DataType dataType);
QPair<QString, QJsonValue> toJson(const std::vector<ParameterSet*> &parameterSets);
QPair<QString, QJsonValue> toJson(const std::vector<ParameterFilterGroup*> &parameterGroup);
QPair<QString, QJsonValue> toJson(const std::vector<SeamSeries*> &seamSeries);
QPair<QString, QJsonValue> toJson(const std::vector<Seam*> &seams);
QPair<QString, QJsonValue> toJson(const std::vector<SeamInterval*> &seamIntervals);
QPair<QString, QJsonValue> toJson(const std::vector<ResultSetting*> &resultItems);
QPair<QString, QJsonValue> toJson(const std::vector<ProductError*> &errors);
QPair<QString, QJsonValue> toJson(const std::vector<SeamSeriesError*> &errors);
QPair<QString, QJsonValue> toJson(const std::vector<SeamError*> &errors);
QPair<QString, QJsonValue> toJson(const std::vector<IntervalError*> &errors);
QPair<QString, QJsonValue> toJson(const std::vector<ReferenceCurve*> &referenceCurves);
QPair<QString, QJsonValue> toJson(const std::vector<LinkedSeam*> &linkedSeams);
QPair<QString, QJsonValue> subGraphsToJson(const std::vector<QUuid> &uuids);
QPair<QString, QJsonValue> toJson(Product::LengthUnit unit);
QPair<QString, QJsonValue> thicknessLeftToJson(int thickness);
QPair<QString, QJsonValue> thicknessRightToJson(int thickness);
QPair<QString, QJsonValue> targetDifferenceToJson(int difference);
QPair<QString, QJsonValue> movingDirectionToJson(int direction);
QPair<QString, QJsonValue> levelToJson(int level);
QPair<QString, QJsonValue> resultNameToJson(QString result);
QPair<QString, QJsonValue> upperToJson(const QUuid &id);
QPair<QString, QJsonValue> middleToJson(const QUuid &id);
QPair<QString, QJsonValue> lowerToJson(const QUuid &id);
QPair<QString, QJsonValue> jitterToJson(const float jitter);
QPair<QString, QJsonValue> referenceTypeToJson(const QString &type);
QPair<QString, QJsonValue> laserControlToJson(const QUuid &id);
QPair<QString, QJsonValue> descriptionToJson(QString description);
QPair<QString, QJsonValue> enumerationToJson(QString enumeration);
QPair<QString, QJsonValue> toolTipToJson(QString toolTip);
QPair<QString, QJsonValue> contentNameToJson(QString contentName);
QPair<QString, QJsonValue> unitToJson(QString unit);
QPair<QString, QJsonValue> attributeIdToJson(QUuid attributeId);
QPair<QString, QJsonValue> locateToJson(int locate);
QPair<QString, QJsonValue> textToJson(QString text);
QPair<QString, QJsonValue> powerToJson(const std::vector<int>& power);
QPair<QString, QJsonValue> offsetToJson(const std::vector<int>& offset);
QPair<QString, QJsonValue> levelConfigsToJson(const std::vector<LevelConfig*> &levels);
QPair<QString, QJsonValue> intervalIdsToJson(const std::map<QUuid, QUuid> &errorIds);
QPair<QString, QJsonValue> envelopeToJson(const QUuid &id);
QPair<QString, QJsonValue> middleCurveAsReferenceToJson(bool use);
QPair<QString, QJsonValue> mandatoryToJson(bool mandatory);
QPair<QString, QJsonValue> linkedSeamLabelToJson(const QString &label);
QPair<QString, QJsonValue> lwmTriggerSignalTypeToJson(int type);
QPair<QString, QJsonValue> lwmTriggerSignalThresholdToJson(double threshold);
QPair<QString, QJsonValue> maxValueToJson(const QVariant &maxValue);
QPair<QString, QJsonValue> minValueToJson(const QVariant &minValue);
QPair<QString, QJsonValue> defaultValueToJson(const QVariant &defaultValue);
QPair<QString, QJsonValue> maxLengthToJson(int maxLength);
QPair<QString, QJsonValue> userLevelToJson(int userLevel);
QPair<QString, QJsonValue> visibleToJson(bool visible);
QPair<QString, QJsonValue> stepToJson(int step);
QPair<QString, QJsonValue> editListOrderToJson(int editListOrder);
QPair<QString, QJsonValue> publicityToJson(bool publicity);

}
}

}
