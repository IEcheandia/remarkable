#pragma once

#include <QAbstractListModel>

class PlausibilityCheckerTest;

namespace precitec
{
namespace scantracker
{
namespace components
{
namespace wobbleFigureEditor
{
class FigureAnalyzer;
}
}
}

namespace scanmaster
{
namespace components
{
namespace wobbleFigureEditor
{

/**
* Class that analyzes the properties of the seam, wobble or overlay figure.
**/
class PlausibilityChecker : public QAbstractListModel
{
    Q_OBJECT

    /**
     * Calculated frequency of selected figure which is used to check if the frequency is too high.
     * Check is performed with the frequency warning threshold and is 300 Hz.
     **/
    Q_PROPERTY(double height READ height WRITE setHeight NOTIFY heightChanged)

    /**
     * Calculated frequency of selected figure which is used to check if the frequency is too high.
     * Check is performed with the frequency warning threshold and is 300 Hz.
     **/
    Q_PROPERTY(double width READ width WRITE setWidth NOTIFY widthChanged)

    /**
     * Calculated frequency of selected figure which is used to check if the frequency is too high.
     * Check is performed with the frequency warning threshold and is 300 Hz.
     **/
    Q_PROPERTY(int currentRow READ currentRow NOTIFY currentRowChanged)

public:
    explicit PlausibilityChecker( QObject* parent = nullptr);
    ~PlausibilityChecker() override;

    struct PlausibilityInformation
    {
        int amplitude;
        int frequency;
        double conformity;
    };

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    double height() const
    {
        return m_height;
    }
    void setHeight(double currentFigureHeight);

    double width() const
    {
        return m_width;
    }
    void setWidth(double currentFigureWidth);

    int currentRow() const
    {
        return m_currentRow;
    }

Q_SIGNALS:
    void figureAnalyzerChanged();
    void heightChanged();
    void widthChanged();
    void currentRowChanged();

private:
    void updateCurrentRow();

    double m_height{0.0};
    double m_width{0.0};
    int m_currentRow{-1};

    std::vector<PlausibilityInformation> m_plausibilityInformation =
    {
        PlausibilityInformation{1, 780, 0.70},
        PlausibilityInformation{2, 780, 0.65},
        PlausibilityInformation{6, 312, 0.90},
    };

    friend PlausibilityCheckerTest;
};

}
}
}
}

