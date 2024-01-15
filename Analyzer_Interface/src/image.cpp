#include "image/image.h"

namespace precitec
{
namespace image
{

void transposeImage(BImage & rOutImage, const BImage & rInImage)
{
    if (rOutImage.overlapsWith(rInImage))
    {
        //can't be done in place, use a temporary image
        BImage inImageCopy(rInImage.size());
        rInImage.copyPixelsTo(inImageCopy);
        assert(!rOutImage.overlapsWith(inImageCopy));
        transposeImage(rOutImage, inImageCopy);
        return;
    }

    auto oImgWidth = rInImage.width();
    auto oImgHeight = rInImage.height();
    rOutImage.resize({oImgHeight, oImgWidth});
    assert(rOutImage.isContiguos());
    auto pPixelTransposed = rOutImage.begin();
    for (int x = 0; x < oImgWidth; x++)
    {
        for (int y = 0; y < oImgHeight; y++)
        {
            *pPixelTransposed = rInImage[y][x];
            ++pPixelTransposed;
        }
    }
    assert(pPixelTransposed == rOutImage.end());
}

BImage transposeImage(const BImage & rInImage)
{
    BImage outImage(Size2d{rInImage.height(), rInImage.width()});
    transposeImage(outImage, rInImage);
    return outImage;
}


}
}
