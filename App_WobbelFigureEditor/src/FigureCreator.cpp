#include "FigureCreator.h"

#include <QtMath>

namespace
{
struct Spiral
{
    double windings;
    double radius;
    double halfWidth;

    /**
    Returns a point on the spiral.

    @param t curve parameter in [0; 1]
    @return Point for @a t
    */
    QPointF plot(double t) const
    {
        Q_ASSERT(t >= 0 && t <= 1);
        double r = t * radius * windings;
        double u = t * windings * 2 * M_PI;
        return QPointF{std::cos(u), std::sin(u)} * r;
    }

    double computeWindings() const
    {
        return std::floor(halfWidth / radius);
    }

    double computeHalfWidth() const
    {
        return std::floor(windings) * radius;
    }

    double computeRadius() const
    {
        return halfWidth / std::floor(windings);
    }

    /**
    Checks if all three parameters are without conflict.
    */
    bool isConsistent() const
    {
        return qFuzzyCompare(computeHalfWidth(), halfWidth);
    }
};

}

namespace precitec
{
namespace scantracker
{
namespace components
{
namespace wobbleFigureEditor
{

FigureCreator::FigureCreator(QObject* parent) : QObject(parent)
{
}

FigureCreator::~FigureCreator() = default;

FileModel* FigureCreator::fileModel() const
{
    return m_fileModel;
}

void FigureCreator::setFileModel(FileModel* newFileModel)
{
    if (m_fileModel == newFileModel)
    {
        return;
    }
    disconnect(m_fileModelDestroyedConnection);
    m_fileModel = newFileModel;
    if (m_fileModel)
    {
        m_fileModelDestroyedConnection = connect(m_fileModel, &QObject::destroyed, this, std::bind(&FigureCreator::setFileModel, this, nullptr));
    }
    else
    {
        m_fileModelDestroyedConnection = {};
    }
    emit fileModelChanged();
}

FigureCreator::Shape FigureCreator::figureShape() const
{
    return m_figureShape;
}

void FigureCreator::setFigureShape(FigureCreator::Shape shape)
{
    if (m_figureShape != shape)
    {
        m_figureShape = shape;
        emit figureShapeChanged();
        fixConstraints();
    }
}

FileType FigureCreator::fileType() const
{
    return m_fileType;
}

void FigureCreator::setFileType(FileType newFileType)
{
    if (newFileType != m_fileType)
    {
        m_fileType = newFileType;
        emit fileTypeChanged();
    }
}

unsigned int FigureCreator::pointNumber() const
{
    return m_pointNumber;
}

void FigureCreator::setPointNumber(unsigned int number)
{
    if (m_pointNumber != number)
    {
        m_pointNumber = number;
        emit pointNumberChanged();
    }
}

double FigureCreator::alpha() const
{
    return m_alpha;
}

void FigureCreator::setAlpha(const double& newAngle)
{
    if (m_alpha != newAngle)
    {
        m_alpha = newAngle;
        emit alphaChanged();
    }
}

void FigureCreator::recordChange(LastChange change)
{
    m_lastChanges.remove(change);
    m_lastChanges.push_back(change);
}

void FigureCreator::fixConstraints()
{
    /*
    This function fixes contraint parameters (like "the height of an eight-figure must be twice its width") by correcting
    the values of the edited parameters. This is done in the order in which they where changed by the user. We fix the most
    recently edited parameters last. So the effects of the most recenent changes are kept as much as possible.
    */
    switch (m_figureShape)
    {
    case Eight:
        for (auto ch : m_lastChanges)
        {
            switch (ch)
            {
            case Height:
            {
                double const h = 2 * m_figureWidth;
                if (h != m_figureHeight)
                {
                    m_figureHeight = h;
                    emit figureHeightChanged();
                }
                break;
            }
            case Width:
            {
                double const w = m_figureHeight / 2;
                if (m_figureWidth != w)
                {
                    m_figureWidth = w;
                    emit figureWidthChanged();
                }
                break;
            }
            default:
                break;
            }
        }
        break;
    case ArchSpiral:
    {
        Spiral s{m_windings, m_radius, m_figureWidth / 2};

        if (s.isConsistent())
        {
            break;
        }

        for (auto ch : m_lastChanges)
        {
            if (ch == Width)
            {
                s.halfWidth = s.computeHalfWidth();
                break;
            }

            if (ch == Windings)
            {
                s.windings = s.computeWindings();
                break;
            }

            if (ch == Radius)
            {
                s.radius = s.computeRadius();
                break;
            }
        }

        if (s.windings != m_windings)
        {
            m_windings = s.windings;
            emit windingsChanged();
        }

        if (s.radius != m_radius)
        {
            m_radius = s.radius;
            emit radiusChanged();
        }

        if (s.halfWidth * 2 != m_figureWidth)
        {
            m_figureWidth = s.halfWidth * 2;
            emit figureWidthChanged();
        }

        break;
    }
    default:
        break;
    }
}

QPointF FigureCreator::coordinateStart() const
{
    return m_coordinateStart;
}

void FigureCreator::setCoordinateStart(const QPointF& xStart)
{
    if (m_coordinateStart != xStart)
    {
        m_coordinateStart = xStart;
        emit coordinateStartChanged();
        emit lengthChanged();
    }
}

double FigureCreator::amplitude() const
{
    return m_amplitude;
}

void FigureCreator::setAmplitude(const double& newAmplitude)
{
    if (m_amplitude != newAmplitude)
    {
        m_amplitude = newAmplitude;
        emit amplitudeChanged();
    }
}

double FigureCreator::phaseShift() const
{
    return m_phaseShift;
}

void FigureCreator::setPhaseShift(const double& newPhaseShift)
{
    if (m_phaseShift != newPhaseShift)
    {
        m_phaseShift = newPhaseShift;
        emit phaseShiftChanged();
    }
}

double FigureCreator::frequence() const
{
    return m_frequence;
}

void FigureCreator::setFrequence(const double &newFrequence)
{
    if (m_frequence != newFrequence)
    {
        m_frequence = newFrequence;
        emit frequenceChanged();
    }
}

int FigureCreator::frequenceVer() const
{
    return m_frequenceVer;
}

void FigureCreator::setFrequenceVer(int newFrequence)
{
    if (m_frequenceVer != newFrequence)
    {
        m_frequenceVer = newFrequence;
        emit frequenceVerChanged();
    }
}

double FigureCreator::windings() const
{
    return m_windings;
}

void FigureCreator::setWindings(const double& windings)
{
    if (m_windings != windings)
    {
        m_windings = windings;
        emit windingsChanged();

        recordChange(Windings);
        fixConstraints();
    }
}

QPointF FigureCreator::coordinateCenter() const
{
    return m_coordinateCenter;
}

void FigureCreator::setCoordinateCenter(const QPointF& newCenter)
{
    if (m_coordinateCenter != newCenter)
    {
        m_coordinateCenter = newCenter;
        emit coordinateCenterChanged();
    }
}

double FigureCreator::angle() const
{
    return m_angle;
}

void FigureCreator::setAngle(const double& newAngle)
{
    if (m_angle != newAngle)
    {
        m_angle = newAngle;
        emit angleChanged();
    }
}

double FigureCreator::radius() const
{
    return m_radius;
}

void FigureCreator::setRadius(const double& newRadius)
{
    if (m_radius != newRadius)
    {
        m_radius = newRadius;
        emit radiusChanged();

        recordChange(Radius);
        fixConstraints();
    }
}

double FigureCreator::figureWidth() const
{
    return m_figureWidth;
}

void FigureCreator::setFigureWidth(const double &newWidth)
{
    if (m_figureWidth != newWidth)
    {
        m_figureWidth = newWidth;
        emit figureWidthChanged();

        recordChange(Width);
        fixConstraints();
    }
}

double FigureCreator::figureHeight() const
{
    return m_figureHeight;
}

void FigureCreator::setFigureHeight(const double &newHeight)
{
    if (m_figureHeight != newHeight)
    {
        m_figureHeight = newHeight;
        emit figureHeightChanged();

        recordChange(Height);
        fixConstraints();
    }
}

QPointF FigureCreator::coordinateEnd() const
{
    return m_coordinateEnd;
}

void FigureCreator::setCoordinateEnd(const QPointF& newEnd)
{
    if (m_coordinateEnd != newEnd)
    {
        m_coordinateEnd = newEnd;
        emit coordinateEndChanged();
        emit lengthChanged();
    }
}

double FigureCreator::length() const
{
    return QVector2D(m_coordinateStart - m_coordinateEnd).length();
}

void FigureCreator::setLength(double length)
{
    if (qFuzzyCompare(length, this->length()))
    {
        return;
    }

    auto v = QVector2D(m_coordinateEnd - m_coordinateStart);

    v = v.length() ? v.normalized() : QVector2D(1, 0);
    setCoordinateEnd((QVector2D(m_coordinateStart) + v * length).toPointF());
}

int FigureCreator::interference() const
{
    return m_interference;
}

void FigureCreator::setInterference(int newValue)
{
    if (m_interference != newValue)
    {
        m_interference = newValue;
        emit interferenceChanged();
    }
}

int FigureCreator::freeInterference() const
{
    return m_freeInterference;
}

void FigureCreator::setFreeInterference(int newValue)
{
    if (m_freeInterference != newValue)
    {
        m_freeInterference = newValue;
        emit freeInterferenceChanged();
    }
}

void FigureCreator::setOverlayFilename(const QString &newOverlay)
{
    if (m_overlayFilename == newOverlay)
    {
        return;
    }
    m_overlayFilename = newOverlay;
    emit overlayFilenameChanged();
}

void FigureCreator::createFigure()
{
    switch (m_figureShape)
    {
    case Line:
        createLineFigure(m_pointNumber, m_coordinateStart, m_coordinateEnd);
        break;
    case Circle:
        createCircle(m_pointNumber, m_radius, m_coordinateCenter, m_angle);
        break;
    case Eight:
        createEight(m_pointNumber, m_figureWidth, m_coordinateCenter);
        break;
    case ArchSpiral:
        createArchimedicSpiral(m_pointNumber, m_coordinateCenter, m_radius, m_windings);
        break;
    case Ellipse:
        createEllipse(m_pointNumber, m_coordinateCenter, m_figureWidth, m_figureHeight, 0 /*m_angle*/);
        break;
    default:
        Q_UNREACHABLE();
    }

    if (m_interference == 0)
    {
        //Overlap sin to the figure;
        interferenceSignals(0);
    }
    if (m_interference == 1)
    {
        if (m_freeInterference != -1 && m_fileModel)
        {
            interferenceOverlayFunction();
        }
    }
    rotateFigure(m_figure, m_alpha);
}

void FigureCreator::startAtPoint(QPointF startPoint)
{
    createFigure();
    if (m_figure.empty())
    {
        Q_ASSERT(false);
        return;
    }

    QPointF const offset = startPoint - m_figure.front();

    switch (m_figureShape)
    {
    case Line:
        m_coordinateStart = startPoint;
        m_coordinateEnd = calculateLineEndPoint(offset);
        emit coordinateStartChanged();
        emit coordinateEndChanged();
        break;
    default:
        setCoordinateCenter(coordinateCenter() + offset);
    }
}

std::vector<QPointF>* FigureCreator::getFigure()
{
    return &m_figure;
}

void FigureCreator::createLineFigure(unsigned int numberPoints, const QPointF& start, const QPointF& end)
{
    m_figure.clear();

    m_figure = createLine(numberPoints, start, end);
}

void FigureCreator::createCircle(unsigned int numberPoints, double const& radius, const QPointF& center, const double& angle)
{
    m_figure.clear();

    if (numberPoints < 3)
    {
        numberPoints = 3;
    }

    for (unsigned int i = 0; i < numberPoints; i++)
    {
        m_figure.push_back(createCirclePoint(radius, (qDegreesToRadians(angle) / (numberPoints - 1) * i)) + center);
    }
}

void FigureCreator::createEight(unsigned int numberPoints, double width, const QPointF& center)
{
    m_figure.clear();

    if (numberPoints < 8)
    {
        numberPoints = 8;
    }
    m_figure.reserve(numberPoints);

    double const radius = width / 2;
    double const step = 4 * M_PI / (numberPoints - 1);
    for (unsigned int i = 0; i < numberPoints; ++i)
    {
        double const t = i * step - M_PI * .5;
        double const ySign = i < numberPoints / 2 ? 1. : -1.;
        auto p = QPointF{-radius * std::cos(t), ySign * (radius * std::sin(t) + radius)};
        m_figure.push_back(p + center);
    }
}

void FigureCreator::createEllipse(unsigned int numberPoints, const QPointF& center, const double& width, const double& height, const double angle)
{
    m_figure.clear();
    m_figure = createEllipseShape(numberPoints, center, width, height, angle);
}

std::vector<QPointF> FigureCreator::createLine(unsigned int numberPoints, const QPointF& start, const QPointF& end)
{
    std::vector<QPointF> figure;

    if (numberPoints < 2)
    {
        numberPoints = 2;
    }

    QVector2D startEnd{static_cast<float>(end.x() - start.x()), static_cast<float>(end.y() - start.y())};

    for (unsigned int i = 0; i < numberPoints; i++)
    {
        figure.push_back(createLinePoint(start, startEnd, numberPoints, i));
    }

    return figure;
}

void FigureCreator::createArchimedicSpiral(unsigned int numberPoints, const QPointF& center, const double& radius, const double& windings)
{
    if (numberPoints < 10)
    {
        numberPoints = 10;
    }

    m_figure.clear();

    Spiral spiral{windings, radius, 0};

    double const step = 1. / (numberPoints - 1);
    for (unsigned int i = 0; i < numberPoints; i++)
    {
        m_figure.push_back(spiral.plot(i * step) + center);
    }
}

std::vector<QPointF> FigureCreator::createEllipseShape(unsigned int numberPoints, const QPointF& center, const double& width, const double& height, const double angle)
{
    std::vector<QPointF> figure;

    if (numberPoints < 10)
    {
        numberPoints = 10;
    }

    for (unsigned int i = 0; i < numberPoints; i++)
    {
        figure.emplace_back(createEllipsePoint(width, height, i * M_PI * 2 / (numberPoints - 1), angle) + center);
    }

    return figure;
}

void FigureCreator::rotateFigure(std::vector<QPointF>& figure, const double &alpha)
{
    if (alpha == 0.0 || alpha == 360.0)
    {
        return;
    }

    auto alphaRad = alpha*M_PI/180.0;

    if (m_figureShape == Line)
    {
        m_coordinateCenter = figure.front();
    }

    //Without center translation
    /*for (auto &point : figure)
    {
        point = rotatePoint(point, alphaRad);
    }*/

    for (auto &point : figure)
    {
        point = translatePoint(point);
        point = rotatePoint(point, alphaRad);
        point = translatePointBack(point);
    }
}

void FigureCreator::interferenceSignals(int typeOfInterference)
{
    //Get sine with the right number of points;
    std::vector<double> overlapFigure;
    if (typeOfInterference == 0)
    {
        //Interference with sin
        auto stepPerPoint = 2*M_PI/m_figure.size();
        for (unsigned int i = 0; i < m_figure.size(); i++)  //calculate sine
        {
            overlapFigure.push_back(-m_amplitude*std::sin(m_frequence*stepPerPoint*i + m_phaseShift));
        }
    }
    if (typeOfInterference != 0)            //TODO add gleichwertige verteilung!
    {
        //Get figureLength for calculating the size of the overlay figure so that the number of repeats can be right.
        double figureLength = 0.0;
        std::vector<std::pair<QVector2D, double>> figureVectors;
        for (unsigned int i = 1; i < m_figure.size(); i++)
        {
            QVector2D vector{m_figure.at(i)-m_figure.at(i-1)};
            auto length = vector.length();
            figureVectors.push_back({vector, length});
            figureLength += length;
        }
        double XLength = m_figure.at(m_figure.size()-1).x() - m_figure.at(0).x();
        //Calculate the size of the overlay figure, after making a unit vector
        auto overlap = m_fileModel->overlayFigure().functionValues;
        /*m_repeatNumber = 2;
        if (m_repeatNumber == 0)
        {
            return;
        }*/
        std::vector<std::pair<QVector2D, double>> overlayVectors;
        overlapFigure.push_back(overlap.at(0).second);
        for (unsigned int i = 1; i < overlap.size(); i++)
        {
            overlapFigure.push_back(overlap.at(i).second);
            QVector2D overlayVector {static_cast<float>(overlap.at(i).first-overlap.at(i-1).first),static_cast<float>(overlap.at(i).second-overlap.at(i-1).second)};
            auto length = overlayVector.length();
            overlayVectors.push_back({overlayVector, length});
        }
        double XLengthO = overlap.at(overlap.size()-1).first - overlap.at(0).first;
        //Interpolate points between other figure points in the distance which is equal to the overlay distances
        auto oldFigurePoints = m_figure;
        unsigned int oldFigurePointsPointer = 1;
        QPointF figurePoint{oldFigurePoints.at(0)};
        m_figure.clear();
        m_figure.push_back(figurePoint);

        for (auto const &vector : overlayVectors)
        {
            figurePoint.setX(figurePoint.x()+ XLength*vector.first.x()/XLengthO);
            //Set y value
            if (figurePoint.x() > oldFigurePoints.at(oldFigurePointsPointer).x())
            {
                oldFigurePointsPointer++;
            }
            double delta = 1.0;
            if (vector.first.x() != 0.0)
            {
                delta = figureVectors.at(oldFigurePointsPointer-1).first.x()/(XLength*vector.first.x()/XLengthO);
            }
            figurePoint.setY(figurePoint.y() + (figureVectors.at(oldFigurePointsPointer-1).first.y()*delta));
            //Push back the new point
            m_figure.push_back(figurePoint);
        }
    }

    if (m_figure.size() != overlapFigure.size())
    {
        return;
    }

    //Get size and angle of the figure
    auto figureLength = 0.0;            //[mm]
    std::vector<QVector2D> direction;

    //TODO calculate derivate with higher precision (point 2 - point 0) for point 1 --> filtering (smoothing)
    for (unsigned int i = 1; i < m_figure.size(); i++)
    {
        QVector2D vector{static_cast<float>(m_figure.at(i).x() - m_figure.at(i-1).x()),static_cast<float>(m_figure.at(i).y() - m_figure.at(i-1).y())};
        figureLength += vector.length();

        QVector2D perpendicularCounterClockwiseVector{-vector.y(), vector.x()};
        perpendicularCounterClockwiseVector.normalize();
        direction.push_back(perpendicularCounterClockwiseVector);
        if (i == 1)
        {
            direction.push_back(perpendicularCounterClockwiseVector);
        }
        m_figure.at(i-1).setX((direction.at(i-1).x()*overlapFigure.at(i-1) + m_figure.at(i-1).x()));
        m_figure.at(i-1).setY((direction.at(i-1).y()*overlapFigure.at(i-1) + m_figure.at(i-1).y()));   //Minus so the y axis shows to the top
    }

    m_figure.at(m_figure.size()-1).setX((direction.at(m_figure.size()-1).x()*overlapFigure.at(m_figure.size()-1) + m_figure.at(m_figure.size()-1).x()));
    m_figure.at(m_figure.size()-1).setY((direction.at(m_figure.size()-1).y()*overlapFigure.at(m_figure.size()-1) + m_figure.at(m_figure.size()-1).y()));
}

void FigureCreator::interferenceOverlayFunction()
{
    if (!m_fileModel)
    {
        return;
    }
    auto overlap = m_fileModel->loadOverlayFunction(m_overlayFilename).functionValues;

    auto seamPoints = m_pointNumber;
    auto overlapPoints = overlap.size();

    if (overlapPoints == 0)
    {
        return;
    }

    for (unsigned int i = 0; i < seamPoints; i++)
    {
        auto overlapPosition = i % (overlapPoints - 1);
        m_figure.at(i).setX(m_figure.at(i).x() + overlap.at(overlapPosition).first);
        m_figure.at(i).setY(m_figure.at(i).y() + overlap.at(overlapPosition).second);
    }
}

QPointF FigureCreator::createCirclePoint(const double& radius, const double& stepPerPoints)
{
    return {radius * std::cos(stepPerPoints), radius * std::sin(stepPerPoints)};
}

QPointF FigureCreator::createLinePoint(const QPointF& start, const QVector2D& slope, const unsigned int &numberPoints, const unsigned int& step)
{
    return {start.x() + (slope.x()/(numberPoints-1)*step), start.y() + (slope.y()/(numberPoints-1)*step)};
}

QPointF FigureCreator::translatePoint(const QPointF& point)
{
    return {point.x()-m_coordinateCenter.x(), point.y()-m_coordinateCenter.y()};
}

QPointF FigureCreator::translatePointBack(const QPointF& point)
{
    return {point.x()+m_coordinateCenter.x(), point.y()+m_coordinateCenter.y()};
}

QPointF FigureCreator::rotatePoint(const QPointF &point, const double& alpha)
{
    return {point.x()*std::cos(alpha) - point.y()*std::sin(alpha), point.x()*std::sin(alpha) + point.y()*std::cos(alpha)};          //alpha [rad]
}

double FigureCreator::calculateDistance(const QPointF& firstPoint, const QPointF& secondPoint)
{
    QVector2D vector{firstPoint - secondPoint};
    return vector.length();
}

QPointF FigureCreator::calculateCenter(const QPointF& firstPoint, const double& opposite, const double& hypotenuse)
{
    QPointF center {0.0,0.0};
    auto halfOfOpposite = opposite/2.0;
    center.setX(firstPoint.x() + halfOfOpposite);

    auto adjacent = std::sqrt((hypotenuse*hypotenuse) - (halfOfOpposite*halfOfOpposite));
    center.setY(firstPoint.y() + adjacent);

    return center;
}

double FigureCreator::calculateAngleDegree(const double& opposite, const double& hypotenuse)
{
    return 180.0*std::asin(opposite/hypotenuse)/M_PI;
}

QPointF FigureCreator::createEllipsePoint(const double& xLength, const double& yLength, const double& stepPerPoints, double angleOffset)
{
    return {xLength / 2 * std::cos(stepPerPoints + angleOffset), yLength / 2 * std::sin(stepPerPoints + angleOffset)};
}

QPointF FigureCreator::applyTranslation(const QPointF& point, const QPointF& translation)
{
    return point + translation;
}

QPointF FigureCreator::calculateLineEndPoint(const QPointF& offset)
{
    auto lineVector = QVector2D(m_coordinateEnd - (m_coordinateStart - offset));
    auto lineLength = lineVector.length();

    lineVector = lineVector.length() ? lineVector.normalized() : QVector2D(1, 0);
    return (QVector2D(m_coordinateStart) + (lineVector * lineLength)).toPointF();
}

}
}
}
}
