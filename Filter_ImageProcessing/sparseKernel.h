#ifndef SPARSEKERNEL_H
#define SPARSEKERNEL_H

#include "geo/geo.h"
#include "image/image.h"

namespace precitec
{
namespace filter
{
    class SparseKernel
    {
        public:
            SparseKernel();
            SparseKernel (const image::BImage & rBmpImage);
            bool empty() const;
            SparseKernel getNormalizedKernel() const;
            image::DImage crossCorrelation(const image::BImage & rImage, bool normalizeKernel) const;
            image::DImage normalizedCrossCorrelation (const image::BImage & rImage, bool normalizeKernel) const;
            image::DImage sumOfSquaredDifferences (const image::BImage & rImage) const;
        private:
            struct KernelElement
            {
                double x;
                double y;
                double v;
            };
            template<bool t_normalizedCrossCorrelation>
            image::DImage computeCrossCorrelation(const image::BImage & rImage, bool normalizeKernel) const;

            std::vector<KernelElement> m_pointlist;
            geo2d::DPoint m_minCoord;
            geo2d::DPoint m_maxCoord;
            geo2d::Point m_kernelSize;
            double m_sum;
            double m_squareSum;
    };
}
}
#endif // SPARSEKERNEL_H
