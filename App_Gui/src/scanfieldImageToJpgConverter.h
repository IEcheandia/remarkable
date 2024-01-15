#pragma once
#include <QDir>

namespace precitec
{
namespace gui
{

/**
 * Functor class to convert all ScanFieldImage bmps (and thumbnails) in a given directory to jpg
 * and removes the original bmp.
 **/
class ScanfieldImageToJpgConverter
{
public:
    /**
     * @param dir The directory containing ScanFieldImages
     **/
    ScanfieldImageToJpgConverter(const QDir& dir);

    /**
     * Perform the conversion of all bmp to jpg in the directory.
     **/
    void operator()();

private:
    QDir m_dir;
};

}
}
