#pragma once

#include "abstractPlotterDataModel.h"

namespace precitec
{
namespace gui
{

class AbstractSingleSeamDataModel : public AbstractPlotterDataModel
{
    Q_OBJECT

    /**
     * The currently selected/highlighted image number
     **/
    Q_PROPERTY(quint32 selectedImageNumber READ selectedImageNumber NOTIFY selectedImageNumberChanged)

    /**
     * The first image number of the loaded results
     **/
    Q_PROPERTY(quint32 firstImageNumber READ firstImageNumber NOTIFY lastPositionChanged)

    /**
     * The last image number of the loaded results
     **/
    Q_PROPERTY(quint32 lastImageNumber READ lastImageNumber NOTIFY lastPositionChanged)

    /**
     * The position to the currently selected/highlighted image number
     **/
    Q_PROPERTY(qreal selectedImagePosition READ selectedImagePosition NOTIFY selectedImagePositionChanged)

public:
    ~AbstractSingleSeamDataModel();

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    int maxIndex() const override
    {
        return 0;
    }

    QPointer<precitec::storage::Seam> findSeam(uint index) const override
    {
        Q_UNUSED(index)
        return m_currentSeam;
    }

    quint32 selectedImageNumber() const;
    quint32 firstImageNumber() const;
    quint32 lastImageNumber() const;
    qreal selectedImagePosition() const;

    Q_INVOKABLE void selectImageNumber(quint32 hint);
    Q_INVOKABLE void clear() override;
    Q_INVOKABLE void selectImagePosition(qreal hint);

Q_SIGNALS:
    void lastPositionChanged();
    void selectedImageNumberChanged();
    void selectedImagePositionChanged();
    void currentSeamChanged();

protected:
    explicit AbstractSingleSeamDataModel(QObject* parent = nullptr);

    void insertNewResult(const precitec::interface::ResultArgs& result, precitec::storage::ResultSetting* resultConfig, const std::list<std::pair<QVector2D, float>>& signalSamples, const std::list<QVector2D>& upperReferenceSamples, const std::list<QVector2D>& lowerReferenceSamples);
    void insertNewSensor(int sensorId, precitec::storage::ResultSetting* resultConfig, const std::list<std::pair<QVector2D, float>>& signalSamples);
    void addResults(int resultIndex, const precitec::interface::ResultArgs& result, precitec::storage::ResultSetting* resultConfig, const std::list<std::pair<QVector2D, float>>& signalSamples, const std::list<QVector2D>& upperReferenceSamples, const std::list<QVector2D>& lowerReferenceSamples);
    void addSignalSamples(int resultIndex, precitec::storage::ResultSetting* resultConfig, int sensorId, const std::list<std::pair<QVector2D, float>>& signalSamples);

    void setSeam(precitec::storage::Seam* seam);

    void insertImagePosition(std::pair<qint32, qreal> position);
    void resetImageNumber();

private:
    typedef std::map<quint32, qreal> ImageNumberToPosition;
    static bool foundPosition(qreal position, const ImageNumberToPosition::const_iterator& it, const ImageNumberToPosition::const_iterator& last);

    QPointer<precitec::storage::Seam> m_currentSeam;

    ImageNumberToPosition m_imageNumberToPosition;
    ImageNumberToPosition::const_iterator m_selectedImageNumber = m_imageNumberToPosition.end();
    qreal m_selectedImagePosition = 0.0;

};

}
}

