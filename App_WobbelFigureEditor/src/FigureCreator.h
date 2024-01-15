#pragma once

#include <QObject>
#include <QPointF>
#include <QVector2D>
#include <list>

#include "FileModel.h"
#include "fileType.h"

using precitec::scantracker::components::wobbleFigureEditor::FileType;

class FigureCreatorTest;

namespace precitec
{

namespace scantracker
{

namespace components
{

namespace wobbleFigureEditor
{

class FigureCreator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::FileModel* fileModel READ fileModel WRITE setFileModel NOTIFY fileModelChanged)

    Q_PROPERTY(Shape figureShape READ figureShape WRITE setFigureShape NOTIFY figureShapeChanged)
    Q_PROPERTY(precitec::scantracker::components::wobbleFigureEditor::FileType fileType READ fileType WRITE setFileType NOTIFY fileTypeChanged)

    Q_PROPERTY(unsigned int pointNumber READ pointNumber WRITE setPointNumber NOTIFY pointNumberChanged)
    Q_PROPERTY(double alpha READ alpha WRITE setAlpha NOTIFY alphaChanged)
    Q_PROPERTY(QPointF coordinateStart READ coordinateStart WRITE setCoordinateStart NOTIFY coordinateStartChanged)
    Q_PROPERTY(double amplitude READ amplitude WRITE setAmplitude NOTIFY amplitudeChanged)
    Q_PROPERTY(double phaseShift READ phaseShift WRITE setPhaseShift NOTIFY phaseShiftChanged)
    Q_PROPERTY(double frequence READ frequence WRITE setFrequence NOTIFY frequenceChanged)
    Q_PROPERTY(int frequenceVer READ frequenceVer WRITE setFrequenceVer NOTIFY frequenceVerChanged)
    Q_PROPERTY(double windings READ windings WRITE setWindings NOTIFY windingsChanged)
    Q_PROPERTY(QPointF coordinateCenter READ coordinateCenter WRITE setCoordinateCenter NOTIFY coordinateCenterChanged)
    Q_PROPERTY(double angle READ angle WRITE setAngle NOTIFY angleChanged)
    Q_PROPERTY(double radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(double figureWidth READ figureWidth WRITE setFigureWidth NOTIFY figureWidthChanged)
    Q_PROPERTY(double figureHeight READ figureHeight WRITE setFigureHeight NOTIFY figureHeightChanged)

    Q_PROPERTY(QPointF coordinateEnd READ coordinateEnd WRITE setCoordinateEnd NOTIFY coordinateEndChanged)
    Q_PROPERTY(double length READ length WRITE setLength NOTIFY lengthChanged)

    Q_PROPERTY(int interference READ interference WRITE setInterference NOTIFY interferenceChanged)
    Q_PROPERTY(int freeInterference READ freeInterference WRITE setFreeInterference NOTIFY freeInterferenceChanged)

    Q_PROPERTY(QString overlayFilename READ overlayFilename WRITE setOverlayFilename NOTIFY overlayFilenameChanged)

    enum LastChange
    {
        Width,
        Height,
        Windings,
        Radius,
    };

    std::list<LastChange> m_lastChanges = {Width, Height, Windings, Radius}; // remembers the order of the most recently changed constraining parameters, so we can update dependent parameters on demand

    void recordChange(LastChange change);
    void fixConstraints();

public:
    enum Shape
    {
        Line = 0,
        Circle = 1,
        Eight = 2,
        ArchSpiral = 7,
        Ellipse = 10,
    };

    Q_ENUMS(Shape)

    explicit FigureCreator(QObject* parent = nullptr);
    ~FigureCreator() override;

    FileModel* fileModel() const;
    void setFileModel(FileModel* newFileModel);
    Shape figureShape() const;
    void setFigureShape(Shape shape);
    FileType fileType() const;
    void setFileType(FileType newFileType);
    unsigned int pointNumber() const;
    void setPointNumber(unsigned int number);
    double alpha() const;
    void setAlpha(const double &newAngle);
    QPointF coordinateStart() const;
    void setCoordinateStart(const QPointF &xStart);
    double amplitude() const;
    void setAmplitude(const double &newAmplitude);
    double phaseShift() const;
    void setPhaseShift(const double &newPhaseShift);
    double frequence() const;
    void setFrequence(const double &newFrequence);
    int frequenceVer() const;
    void setFrequenceVer(int newFrequence);
    double windings() const;
    void setWindings(const double &windings);
    QPointF coordinateCenter() const;
    void setCoordinateCenter(const QPointF& newCenter);
    double angle() const;
    void setAngle(const double& newAngle);
    double radius() const;
    void setRadius(const double& newRadius);
    double figureWidth() const;
    void setFigureWidth(const double& newWidth);
    double figureHeight() const;
    void setFigureHeight(const double& newHeight);
    QPointF coordinateEnd() const;
    void setCoordinateEnd(const QPointF& newEnd);
    double length() const;
    void setLength(double length);
    int interference() const;
    void setInterference(int newValue);
    int freeInterference() const;
    void setFreeInterference(int newValue);

    QString overlayFilename() const
    {
        return m_overlayFilename;
    }
    void setOverlayFilename(const QString &newOverlay);

    Q_INVOKABLE void createFigure();
    Q_INVOKABLE void startAtPoint(QPointF startPoint);
    std::vector<QPointF>* getFigure();

    void createLineFigure(unsigned int numberPoints, const QPointF& start, const QPointF& end);
    void createCircle(unsigned int numberPoints, const double& radius, const QPointF& center, const double& angle);
    void createEight(unsigned int numberPoints, double width, const QPointF& center);
    void createEllipse(unsigned int numberPoints, const QPointF& center, const double& width, const double& height, const double angle);
    void createArchimedicSpiral(unsigned int numberPoints, const QPointF& center, const double& radius, const double& windings);

    std::vector<QPointF> createLine(unsigned int numberPoints, const QPointF& start, const QPointF& end);
    void rotateFigure(std::vector<QPointF>& figure, const double& alpha); //TODO add center to the calculation
    std::vector<QPointF> createEllipseShape(unsigned int numberPoints, const QPointF& center, const double& width, const double& height, const double angle);

Q_SIGNALS:
    void fileModelChanged();
    void figureShapeChanged();
    void fileTypeChanged();
    void pointNumberChanged();
    void alphaChanged();
    void coordinateStartChanged();
    void amplitudeChanged();
    void phaseShiftChanged();
    void frequenceChanged();
    void frequenceVerChanged();
    void windingsChanged();
    void coordinateCenterChanged();
    void angleChanged();
    void radiusChanged();
    void figureWidthChanged();
    void figureHeightChanged();
    void coordinateOffsetChanged();
    void coordinateEndChanged();
    void lengthChanged();
    void interferenceChanged();
    void freeInterferenceChanged();

    void overlayFilenameChanged();
private:
    void interferenceSignals(int typeOfInterference);
    void interferenceOverlayFunction();
    //void interpolateBetweenTwoPoints(const QPointF &start, const QPointF &end, );
    QPointF createCirclePoint(const double& radius, const double& stepPerPoints);
    QPointF createLinePoint(const QPointF& start, const QVector2D& slope, const unsigned int& numberPoints, const unsigned int& step);
    QPointF translatePoint(const QPointF& point);
    QPointF translatePointBack(const QPointF& point); //Kann weg!
    QPointF rotatePoint(const QPointF& point, const double& alpha);
    double calculateDistance(const QPointF& firstPoint, const QPointF& secondPoint);
    QPointF calculateCenter(const QPointF &firstPoint, const double &opposite, const double &hypotenuse);
    double calculateAngleDegree(const double& opposite, const double& hypotenuse);
    QPointF createEllipsePoint(const double& xLength, const double& yLength, const double& stepPerPoints, double angleOffset);
    QPointF applyTranslation(const QPointF& point, const QPointF& translation);
    QPointF calculateLineEndPoint(const QPointF& offset);

    FileModel* m_fileModel = nullptr;
    QMetaObject::Connection m_fileModelDestroyedConnection;
    //Properties for figure creation
    FileType m_fileType{FileType::None};
    Shape m_figureShape = Line;
    unsigned int m_pointNumber = 100;
    double m_alpha = 0.0;
    QPointF m_coordinateStart = {0, 0};
    //Frequence
    double m_amplitude = 0;
    double m_phaseShift = 0;
    double m_frequence = 0;
    int m_frequenceVer = 0;
    double m_windings = 3;
    //Circle
    QPointF m_coordinateCenter = {0, 0};
    double m_angle = 360;
    double m_radius = 1;
    //Rectangle
    double m_figureWidth = 3;
    double m_figureHeight = 6;
    //Free
    int m_segmentType = -1;
    QPointF m_coordinateEnd = {3, 0};
    //Interference
    int m_interference = 2;
    int m_freeInterference = -1;

    QString m_overlayFilename;

    std::vector<QPointF> m_figure;

    friend FigureCreatorTest;
};

}
}
}
}

