#include "intensityProfileModel.h"

#include <precitec/dataSet.h>

namespace precitec
{
namespace gui
{
namespace components
{
namespace image
{
    IntensityProfileModel::IntensityProfileModel(QObject *parent)
    :QAbstractListModel(parent)
    , m_dataSet(new plotter::DataSet{this})
    {
        m_dataSet->setDrawingMode(plotter::DataSet::DrawingMode::LineWithPoints);
    }

    IntensityProfileModel::~IntensityProfileModel() = default;

    QHash<int, QByteArray> IntensityProfileModel::roleNames() const
    {
        return {
            {Qt::UserRole, QByteArrayLiteral("x")},
            {Qt::UserRole + 1, QByteArrayLiteral("y")},
            {Qt::UserRole + 2, QByteArrayLiteral("intensity")}
        };
    }

    QVariant IntensityProfileModel::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid() || index.row() > int(m_Profile.size()))
        {
            return QVariant{};
        }
        const auto & rSampledPoint = m_Profile[index.row()];
        switch (role)
        {
            case Qt::UserRole:
                return rSampledPoint.x;
            case Qt::UserRole + 1:
                return rSampledPoint.y;
            case Qt::UserRole + 2:
                return rSampledPoint.intensity;
            default:
                return QVariant {};
        }
    }

    int IntensityProfileModel::rowCount(const QModelIndex &parent) const
    {
        return m_Profile.size();
    }

    precitec::gui::components::image::ImageData  IntensityProfileModel::imageData()
    {
        return m_ImageData;
    }
    void IntensityProfileModel::setImageData(const precitec::gui::components::image::ImageData & rImageData)
    {
        m_ImageData = rImageData;
        emit imageDataChanged();
        update();
    }

    QVector2D IntensityProfileModel::startPoint()
    {
        return m_startPoint;
    }

    void IntensityProfileModel::setStartPoint(const QVector2D& point)
    {
        if (!qFuzzyCompare(point, m_startPoint))
        {
            m_startPoint = point;
            emit startPointChanged();
            update();
        }
    }

    QVector2D IntensityProfileModel::endPoint()
    {
        return m_endPoint;
    }

    void IntensityProfileModel::setEndPoint(const QVector2D& point)
    {
        if (!qFuzzyCompare(point, m_endPoint))
        {
            m_endPoint = point;
            emit endPointChanged();
            update();
        }
    }

    void IntensityProfileModel::update()
    {
        beginResetModel();
        computeProfileWithoutInterpolation();
        m_dataSet->clear();

        std::vector<QVector2D> samples;
        samples.reserve(m_Profile.size());
        auto counter = 0;
        std::transform(m_Profile.begin(), m_Profile.end(), std::back_inserter(samples), [&counter] (const auto &value) { return QVector2D{float(counter++), float(value.intensity)}; });
        m_dataSet->addSamples(samples);

        endResetModel();
    }

    IntensityProfileModel::ExtremePoints IntensityProfileModel::computeExtremePoints()
    {
        const precitec::image::BImage & rImage = std::get<precitec::image::BImage>(m_ImageData);
        assert(rImage.isValid() && "computeExtremePoints called outside computeProfile");

        //convert first and last point to image indexes, project external points to border
        int W = rImage.width()-1;  //max x value
        int H = rImage.height()-1; //max y value

        return { std::max(0, std::min(W, int(std::round(m_startPoint.x())))), //iStart
            std::max(0, std::min(H, int(std::round(m_startPoint.y())))), //jStart
            std::max(0, std::min(W, int(std::round(m_endPoint.x())))), //iEnd
            std::min(H, int(std::round(m_endPoint.y()))) //jEnd
        };
    }

    void IntensityProfileModel::computeProfileWithoutInterpolation()
    {
        m_Profile.clear();
        const precitec::image::BImage & rImage = std::get<precitec::image::BImage>(m_ImageData);
        if (!rImage.isValid())
        {
            return;
        }

        ExtremePoints oExtremePoints = computeExtremePoints();

        // Bresenham-Algorithm

        int dx = std::abs(oExtremePoints.iEnd - oExtremePoints.iStart);
        const int sx = oExtremePoints.iStart < oExtremePoints.iEnd ? 1 : -1;

        int dy = - std::abs(oExtremePoints.jEnd - oExtremePoints.jStart);
        const int sy = oExtremePoints.jStart < oExtremePoints.jEnd ? 1 : -1;

        int err = dx + dy;

        int x = oExtremePoints.iStart;
        int y = oExtremePoints.jStart;
        auto * pixel = rImage.rowBegin(y)+x;

        while(x != oExtremePoints.iEnd || y != oExtremePoints.jEnd)
        {
            m_Profile.push_back({static_cast<double>(x), static_cast<double>(y), static_cast<double>(*pixel)});
            int e2 = 2 * err; /* error value e_xy */
            if (e2 > dy)
            {
                err += dy;
                x += sx; /* e_xy+e_x > 0 */
                pixel+=sx;
            }

            if (e2 < dx)
            {
                err += dx;
                y += sy; /* e_xy+e_y < 0 */
                pixel = rImage.rowBegin(y)+x;
            }
        }
        //add last point
        m_Profile.push_back({static_cast<double>(x), static_cast<double>(y), static_cast<double>(*pixel)});
        assert(std::round(m_Profile.front().x) == oExtremePoints.iStart);
        assert(std::round(m_Profile.front().y) == oExtremePoints.jStart);
        assert(std::round(m_Profile.back().x) == oExtremePoints.iEnd);
        assert(std::round(m_Profile.back().y) == oExtremePoints.jEnd);
    }

    void IntensityProfileModel::computeProfileWithMainDirectionInterpolation()
    {
        m_Profile.clear();
        const precitec::image::BImage & rImage = std::get<precitec::image::BImage>(m_ImageData);
        if (!rImage.isValid())
        {
            return;
        }

        ExtremePoints oExtremePoints = computeExtremePoints();

        const unsigned int xDistance = std::abs(oExtremePoints.iEnd-oExtremePoints.iStart);
        const unsigned int yDistance = std::abs(oExtremePoints.jEnd-oExtremePoints.jStart);

        //special case: coincident points
        if (xDistance == 0 && yDistance == 0)
        {
            m_Profile.push_back({ static_cast<double>(oExtremePoints.iStart),  static_cast<double>(oExtremePoints.jStart), static_cast<double>(rImage.getValue( oExtremePoints.iStart,  oExtremePoints.jStart))});
            return;
        }

        const SamplingDirection oDirection = xDistance > yDistance?
            SamplingDirection::Horizontal : SamplingDirection::Vertical;

            const unsigned int numPoints = (oDirection == SamplingDirection::Horizontal? xDistance : yDistance) + 1; //inclusive of end point
        m_Profile.reserve(numPoints);

        if (oDirection == SamplingDirection::Horizontal)
        {

            assert(xDistance > 0);
            const int dx=  oExtremePoints.iEnd >  oExtremePoints.iStart ? 1: -1;
            const double a = double( oExtremePoints.jEnd -  oExtremePoints.jStart)/double( oExtremePoints.iEnd -  oExtremePoints.iStart);
            const double b =  oExtremePoints.jStart - a * oExtremePoints.iStart;
            //distance between sampled points: sqrt(xDistance^2+ yDistance^2 )/xDistance

            double y(oExtremePoints.jStart);
            for (int i = oExtremePoints.iStart, iMax = oExtremePoints.iEnd + dx;  //inclusive of end point
                i != iMax; //test for inequality, because we don't know if we are incrementing or decrementing
                i+=dx, y=(a*i+b))  // y+= a would be too sensible to floating point errors
            {
                //linear interpolation between 2 neighboring pixel
                unsigned int j = std::floor(y);
                double k = y - j; //e.g k = 0.5 means that the sampling point is between 2 pixels

                double intensity = k > 0.1 ? (1-k)*rImage.getValue(i,j) + k * rImage.getValue(i, j+1): rImage.getValue(i,j);
                m_Profile.push_back({static_cast<double>(i), y, intensity});
            }
        }
        else
        {
            assert(yDistance > 0);
            const int dy =  oExtremePoints.jEnd >  oExtremePoints.jStart ? 1:-1;
            // x = ay+b
            const double a = double( oExtremePoints.iEnd -  oExtremePoints.iStart)/double(oExtremePoints.jEnd - oExtremePoints.jStart);
            const double b =  oExtremePoints.iStart - a * oExtremePoints.jStart;
            //distance between sampled points: sqrt(xDistance^2+ yDistance^2 )/yDistance

            double x(oExtremePoints.iStart);
            for (int j =  oExtremePoints.jStart, jMax =  oExtremePoints.jEnd + dy; //inclusive of end point
                j != jMax; //test for inequality, because we don't know if we are incrementing or decrementing
                j+=dy, x=(a*j+b)) //x+=a would be too sensible to floating point errors
            {
                //linear interpolation between 2 neighboring pixel
                unsigned int i = std::floor(x);
                double k = x - i;  //e.g k = 0.5 means that the sampling point is between 2 pixels

                auto * pixel = rImage.rowBegin(j) + i;
                const auto &currentPixel = *pixel;
                double intensity = currentPixel;
                if (k > 0.1)
                {
                    const auto &nextPixel = (*(++pixel));
                    intensity = (1-k) * currentPixel + k * nextPixel;
                }
                m_Profile.push_back({x, static_cast<double>(j), intensity});
            }
        }

        assert(m_Profile.size() == numPoints);
        assert(std::round(m_Profile.front().x) == oExtremePoints.iStart);
        assert(std::round(m_Profile.front().y) == oExtremePoints.jStart);
        assert(std::round(m_Profile.back().x) == oExtremePoints.iEnd);
        assert(std::round(m_Profile.back().y) == oExtremePoints.jEnd);
    }
}
}
}
}

