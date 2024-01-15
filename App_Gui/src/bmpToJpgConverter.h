#pragma once
#include <QFileInfo>

namespace precitec
{
namespace gui
{

/**
 * Functor class to convert a bmp file to jpg using QImage.
 *
 * Loads the provided bmp file in QImage and uses save to store as .jpg.
 **/
class BmpToJpgConverter
{
public:
    /**
     * @param bmp The file info to the bmp to convert
     **/
    BmpToJpgConverter(const QFileInfo& bmp);

    /**
     * Converts the bmp to jpg using @p quality (range 0 to 100).
     * @returns @c true on success, @c false otherwise
     **/
    bool operator()(int quality);

private:
    QFileInfo m_bmp;
};

}
}
