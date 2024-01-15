#pragma once

#include <QAbstractListModel>

class WobbleFigureModelTest;

namespace precitec
{
namespace scanmaster
{
namespace components
{
namespace wobbleFigureEditor
{

class WobbleFigureModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(unsigned int pointCount READ pointCount WRITE setPointCount NOTIFY pointCountChanged)
    Q_PROPERTY(bool isDualChannelModulation READ isDualChannelModulation WRITE setIsDualChannelModulation NOTIFY isDualChannelModulationChanged)

    Q_PROPERTY(unsigned int microVectorFactor READ microVectorFactor WRITE setMicroVectorFactor NOTIFY microVectorFactorChanged);
    Q_PROPERTY(QModelIndex frequencyIndex READ frequencyIndex WRITE setFrequencyIndex NOTIFY frequencyIndexChanged);

    Q_PROPERTY(int frequency READ frequency NOTIFY frequencyChanged);
    Q_PROPERTY(int lowestFrequency READ lowestFrequency WRITE setLowestFrequency NOTIFY lowestFrequencyChanged)
    Q_PROPERTY(int highestFrequency READ highestFrequency WRITE setHighestFrequency NOTIFY highestFrequencyChanged)

public:
    explicit WobbleFigureModel( QObject* parent = nullptr);
    ~WobbleFigureModel() override;

    struct WobbleFrequencyInformation
    {
        int frequency;
        unsigned int microVectorFactor;
    };

    enum class WobbleFrequency
    {
        LowestFrequency = 1,
        HighestFrequency = 1000
    };
    Q_ENUM(WobbleFrequency)

    unsigned int pointCount() const
    {
        return m_pointCount;
    }
    void setPointCount(unsigned int newNumberOfPoints);

    bool isDualChannelModulation() const
    {
        return m_isDualChannelModulation;
    };
    void setIsDualChannelModulation(bool isDualChannel);

    unsigned int microVectorFactor() const
    {
        return m_microVectorFactor;
    }
    void setMicroVectorFactor(unsigned int newMicroVectorFactor);

    QModelIndex frequencyIndex() const
    {
        return m_frequencyIndex;
    }
    void setFrequencyIndex(const QModelIndex &newModelIndex);

    int frequency() const
    {
        return data(m_frequencyIndex).toInt();
    }

    int lowestFrequency() const
    {
        return m_lowestFrequency;
    }
    void setLowestFrequency(int newFrequency);

    int highestFrequency() const
    {
        return m_highestFrequency;
    }
    void setHighestFrequency(int newFrequency);

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

Q_SIGNALS:
    void pointCountChanged();
    void isDualChannelModulationChanged();

    void microVectorFactorChanged();
    void frequencyIndexChanged();

    void frequencyChanged();
    void lowestFrequencyChanged();
    void highestFrequencyChanged();

    void microVectorFactorWillBeChanged();

    void reset();

private:
    bool checkIfFrequencyIsNotAlreadyPresent(int frequency);
    void calculatePossibleFrequencies();
    void indexByMicroVectorFactor();
    void updateMicroVectorFactorByIsDualModulation();
    void updateMicroVectorFactorByLowestFrequency();
    void updateMicroVectorFactorByHighestFrequency();
    bool checkIfMicroVectorFactorIsEven(int microVectorFactor);
    int calculateFrequency(unsigned int microVectorFactor);
    void resetModel();

    unsigned int m_pointCount{0};
    bool m_isDualChannelModulation{false};

    unsigned int m_microVectorFactor{0};
    QModelIndex m_frequencyIndex;

    int m_lowestFrequency{static_cast<int> (WobbleFrequency::LowestFrequency)};
    int m_highestFrequency{static_cast<int> (WobbleFrequency::HighestFrequency)};

    unsigned int m_counter{25000};                              //A wobble figure has at least two points (p). To get the full range of frequencies (f) [25kHz - 1 Hz] the max micro vector factor (uVF) has to be 25000. f = 1 / (10us * uVF * p) --> uVF = 1 / (10us * f * p)

    std::vector<WobbleFrequencyInformation> m_possibleFrequencies;

    friend WobbleFigureModelTest;
};

}
}
}
}
