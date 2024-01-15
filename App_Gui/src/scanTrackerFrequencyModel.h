#pragma once

#include <QAbstractListModel>

#include "../Mod_WeldHeadControl/include/viWeldHead/serialToTracker.h"

namespace precitec
{

namespace gui
{

class ScanTrackerFrequencyModel : public QAbstractListModel
{
    Q_OBJECT
public:
    ScanTrackerFrequencyModel(QObject *parent = nullptr);
    ~ScanTrackerFrequencyModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = {}) const override;

private:
    std::map<int, int> m_frequencies = {
        {static_cast<int>(hardware::eFreq30),  30},
        {static_cast<int>(hardware::eFreq40),  40},
        {static_cast<int>(hardware::eFreq50),  50},
        {static_cast<int>(hardware::eFreq100), 100},
        {static_cast<int>(hardware::eFreq150), 150},
        {static_cast<int>(hardware::eFreq200), 200},
        {static_cast<int>(hardware::eFreq250), 250},
        {static_cast<int>(hardware::eFreq300), 300},
        {static_cast<int>(hardware::eFreq350), 350},
        {static_cast<int>(hardware::eFreq400), 400},
        {static_cast<int>(hardware::eFreq450), 450},
        {static_cast<int>(hardware::eFreq500), 500},
        {static_cast<int>(hardware::eFreq550), 550},
        {static_cast<int>(hardware::eFreq600), 600},
        {static_cast<int>(hardware::eFreq650), 650},
        {static_cast<int>(hardware::eFreq700), 700},
        {static_cast<int>(hardware::eFreq750), 750}
    };
};

}
}
