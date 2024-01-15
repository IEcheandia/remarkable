#include "productError.h"
#include "copyMode.h"
#include "parameter.h"
#include "product.h"
#include "common/graph.h"

#include <Poco/UUIDGenerator.h>

using precitec::interface::FilterParameter;
using precitec::interface::TFilterParameter;

namespace precitec
{
namespace storage
{
namespace
{

Poco::UUID toPoco(const QUuid &uuid)
{
    return Poco::UUID(uuid.toString(QUuid::WithoutBraces).toStdString());
}

}

ProductError::ProductError(QObject *parent)
    : OverlyingError(parent)
{
}

ProductError::ProductError(QUuid uuid, QObject *parent)
    : OverlyingError(std::move(uuid), parent)
{
}

ProductError::~ProductError() = default;

void ProductError::setProduct(Product *product)
{
    if (m_product == product)
    {
        return;
    }
    m_product = product;
    emit productChanged();
}

int ProductError::getIntValue(const QString &name) const
{
    if (name.compare(QLatin1String("SeamInterval")) == 0
        || name.compare(QLatin1String("SeamSeries")) == 0
        || name.compare(QLatin1String("Seam")) == 0)
    {
        return 0;
    }
    return OverlyingError::getIntValue(name);
}

std::string ProductError::getStringValue(const QString &name) const
{
    if (name.compare(QLatin1String("Scope")) == 0)
    {
        return std::string("Product");
    }
    return QUuid{}.toString(QUuid::WithoutBraces).toStdString();
}

std::vector<std::shared_ptr<FilterParameter>> ProductError::toParameterList() const
{
    std::vector<std::shared_ptr<FilterParameter>> list;
    list.reserve(13);

    static const std::map<QString, Parameter::DataType> names{
        {QStringLiteral("Result"), Parameter::DataType::Result},
        {QStringLiteral("Error"), Parameter::DataType::Error},
        {QStringLiteral("Min"), Parameter::DataType::Double},
        {QStringLiteral("Max"), Parameter::DataType::Double},
        {QStringLiteral("Threshold"), Parameter::DataType::Double},
        {QStringLiteral("Scope"), Parameter::DataType::String},
        {QStringLiteral("SeamInterval"), Parameter::DataType::Integer},
        {QStringLiteral("SeamSeries"), Parameter::DataType::Integer},
        {QStringLiteral("Seam"), Parameter::DataType::Integer},
        {QStringLiteral("Reference"), Parameter::DataType::String},
        {QStringLiteral("MiddleReference"), Parameter::DataType::Boolean},
        {QStringLiteral("SecondThreshold"), Parameter::DataType::Double},
        {QStringLiteral("LwmSignalThreshold"), Parameter::DataType::Double}
    };

    const auto typeId = toPoco(variantId());
    const auto filterId = toPoco(uuid());
    for (auto pair : names)
    {
        const auto parameterId = Poco::UUIDGenerator().createRandom();
        const auto parameterName = pair.first.toStdString();
        switch (pair.second)
        {
            case Parameter::DataType::Integer:
            case Parameter::DataType::Enumeration:
            case Parameter::DataType::Error:
            case Parameter::DataType::Result:
                list.emplace_back(std::make_shared<TFilterParameter<int>>(
                                parameterId,
                                parameterName,
                                getIntValue(pair.first),
                                filterId,
                                typeId));
                break;
            case Parameter::DataType::Double:
                list.emplace_back(std::make_shared<TFilterParameter<double>>(
                                parameterId,
                                parameterName,
                                getDoubleValue(pair.first),
                                filterId,
                                typeId));
                break;
            case Parameter::DataType::String:
                list.emplace_back(std::make_shared<TFilterParameter<std::string>>(
                                parameterId,
                                parameterName,
                                getStringValue(pair.first),
                                filterId,
                                typeId));
                break;
            case Parameter::DataType::Boolean:
                list.emplace_back(std::make_shared<TFilterParameter<bool>>(
                                parameterId,
                                parameterName,
                                false,
                                filterId,
                                typeId));
                break;
            default:
                break;
        }
    }

    return list;
}

ProductError *ProductError::fromJson(const QJsonObject &object, Product *parent)
{
    if (object.isEmpty())
    {
        return nullptr;
    }
    ProductError *error = new ProductError(parent);
    error->setProduct(parent);
    error->OverlyingError::fromJson(object);

    return error;
}

bool ProductError::isChangeTracking()
{
    if (m_product)
    {
        return m_product->isChangeTracking();
    }
    return false;
}

ProductError *ProductError::duplicate(CopyMode mode, Product *parent) const
{
    auto newUuid = duplicateUuid(mode, uuid());
    auto se = new ProductError{std::move(newUuid), parent};
    se->setProduct(parent);
    se->setVariantId(variantId());
    se->setName(name());
    se->setResultValue(resultValue());
    se->setErrorType(errorType());
    se->setThreshold(threshold());

    return se;
}

}
}

