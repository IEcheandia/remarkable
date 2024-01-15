#include "scrollBarSynchController.h"

namespace precitec
{
namespace gui
{

ScrollBarSynchController::ScrollBarSynchController(QObject* parent)
    : QObject(parent)
{
}

ScrollBarSynchController::~ScrollBarSynchController() = default;

void ScrollBarSynchController::setPosition(qreal position)
{
    if (qFuzzyCompare(m_position, position))
    {
        return;
    }
    m_position = position;
    emit positionChanged();
}

}
}

