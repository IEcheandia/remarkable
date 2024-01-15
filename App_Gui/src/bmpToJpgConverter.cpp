#include "bmpToJpgConverter.h"
#include <QDir>
#include <QImage>

namespace precitec
{
namespace gui
{

BmpToJpgConverter::BmpToJpgConverter(const QFileInfo& bmp)
    : m_bmp(bmp)
{
}

bool BmpToJpgConverter::operator()(int quality)
{
    if (!m_bmp.exists() || !m_bmp.isFile())
    {
        return false;
    }
    QImage bmp{m_bmp.absoluteFilePath()};
    if (bmp.isNull())
    {
        return false;
    }
    return bmp.save(m_bmp.absolutePath() + QDir::separator() + m_bmp.completeBaseName() + QStringLiteral(".jpg"), nullptr, quality);
}


}
}
