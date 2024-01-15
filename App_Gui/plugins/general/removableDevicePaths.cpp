#include "removableDevicePaths.h"

namespace precitec
{
namespace gui
{

RemovableDevicePaths::RemovableDevicePaths()
    : QObject()
{
}

RemovableDevicePaths::~RemovableDevicePaths() = default;

RemovableDevicePaths *RemovableDevicePaths::instance()
{
    static RemovableDevicePaths s_instance;
    return &s_instance;
}

QString RemovableDevicePaths::separatedProductsDir() const
{
    return QStringLiteral("/separated_products/");
}

QString RemovableDevicePaths::separatedProductJsonDir() const
{
    return QStringLiteral("product");
}

QString RemovableDevicePaths::separatedProductReferenceCurveDir() const
{
    return QStringLiteral("reference_curve");
}

QString RemovableDevicePaths::separatedProductScanFieldImagesDir() const
{
    return QStringLiteral("scanfieldimages");
}

}
}
