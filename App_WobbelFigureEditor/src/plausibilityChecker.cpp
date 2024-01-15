#include "plausibilityChecker.h"

#include "FigureAnalyzer.h"

using precitec::scantracker::components::wobbleFigureEditor::FigureAnalyzer;
using precitec::scantracker::components::wobbleFigureEditor::WobbleFigureEditor;

namespace precitec
{
namespace scanmaster
{
namespace components
{
namespace wobbleFigureEditor
{

PlausibilityChecker::PlausibilityChecker(QObject* parent) : QAbstractListModel(parent)
{
    connect(this, &PlausibilityChecker::heightChanged, this, &PlausibilityChecker::updateCurrentRow);
    connect(this, &PlausibilityChecker::widthChanged, this, &PlausibilityChecker::updateCurrentRow);
}

PlausibilityChecker::~PlausibilityChecker() = default;

QHash<int, QByteArray> PlausibilityChecker::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("amplitude")},
        {Qt::UserRole, QByteArrayLiteral("frequency")},
        {Qt::UserRole + 1, QByteArrayLiteral("conformity")}
    };
}

QVariant PlausibilityChecker::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const auto &plausibility = m_plausibilityInformation.at(index.row());

    switch (role)
    {
        case Qt::DisplayRole:
            return plausibility.amplitude;
        case Qt::UserRole:
            return plausibility.frequency;
        case Qt::UserRole + 1:
            return plausibility.conformity;
    }

    return {};
}

int PlausibilityChecker::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_plausibilityInformation.size();
}

void PlausibilityChecker::setHeight(double currentFigureHeight)
{
    if (m_height == currentFigureHeight)
    {
        return;
    }

    m_height = currentFigureHeight;
    emit heightChanged();
}

void PlausibilityChecker::setWidth(double currentFigureWidth)
{
    if (m_width == currentFigureWidth)
    {
        return;
    }

    m_width = currentFigureWidth;
    emit widthChanged();
}

void PlausibilityChecker::updateCurrentRow()
{
    if ((m_width < 0.00000001 && m_height < 0.00000001) || m_plausibilityInformation.empty())
    {
        m_currentRow = -1;
        emit currentRowChanged();
        return;
    }

    auto amplitude = std::max(m_width, m_height);

    auto it = std::find_if(m_plausibilityInformation.begin(), m_plausibilityInformation.end(),[amplitude] (const auto &amplitudeBorders) {return amplitude <= amplitudeBorders.amplitude;});
    if (it == m_plausibilityInformation.end())
    {
        m_currentRow = rowCount() - 1;
        emit currentRowChanged();
        return;
    }

    m_currentRow = std::distance(m_plausibilityInformation.begin(), it);
    emit currentRowChanged();
}


}
}
}
}
