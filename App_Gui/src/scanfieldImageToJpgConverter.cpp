#include "scanfieldImageToJpgConverter.h"
#include "bmpToJpgConverter.h"
#include <QDebug>

namespace precitec
{
namespace gui
{

ScanfieldImageToJpgConverter::ScanfieldImageToJpgConverter(const QDir& dir)
    : m_dir(dir)
{
}

void ScanfieldImageToJpgConverter::operator()()
{
    const auto& bmps = m_dir.entryInfoList({QStringLiteral("ScanFieldImage*.bmp")}, QDir::Files);
    for (const auto& bmp : bmps)
    {
        BmpToJpgConverter converter{bmp};
        if (converter(90))
        {
            QFile{bmp.absoluteFilePath()}.remove();
        }
    }
}

}
}
