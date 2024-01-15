#pragma once

#include <QObject>

namespace precitec
{
namespace gui
{

/**
 * Singleton class providing access to all known paths wrt to removable devices
 **/
class RemovableDevicePaths : public QObject
{
    Q_OBJECT
     /**
     * Relative directory containing separate products (json, ref ... files) with correspondent artifacts
     **/
    Q_PROPERTY(QString separatedProductsDir READ separatedProductsDir CONSTANT)

    Q_PROPERTY(QString separatedProductJsonDir READ separatedProductJsonDir CONSTANT)

    Q_PROPERTY(QString separatedProductReferenceCurveDir READ separatedProductReferenceCurveDir CONSTANT)

    Q_PROPERTY(QString separatedProductScanFieldImagesDir READ separatedProductScanFieldImagesDir CONSTANT)

public:
    ~RemovableDevicePaths() override;

    static RemovableDevicePaths *instance();

    QString separatedProductsDir() const;

    QString separatedProductJsonDir() const;

    QString separatedProductReferenceCurveDir() const;

    QString separatedProductScanFieldImagesDir() const;

private:
    explicit RemovableDevicePaths();
};

}
}
