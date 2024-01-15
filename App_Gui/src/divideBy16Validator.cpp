#include "divideBy16Validator.h"

namespace precitec
{
namespace gui
{

DivideBy16Validator::DivideBy16Validator(QObject *parent)
    : QIntValidator(parent)
{
}

DivideBy16Validator::~DivideBy16Validator() = default;

QValidator::State DivideBy16Validator::validate(QString& input, int& pos) const
{
    const auto state = QIntValidator::validate(input, pos);
    if (state == QValidator::Invalid || state == QValidator::Intermediate)
    {
        return state;
    }
    bool ok = false;
    auto value = input.toInt(&ok);
    if (!ok)
    {
        return QValidator::Intermediate;
    }
    if (m_zeroBased && value != 0)
    {
        value++;
    }
    if (value % 16 == 0)
    {
        return QValidator::Acceptable;
    }
    return QValidator::Intermediate;
}

void DivideBy16Validator::setZeroBased(bool set)
{
    if (m_zeroBased == set)
    {
        return;
    }
    m_zeroBased = set;
    emit zeroBasedChanged();
}

void DivideBy16Validator::fixup(QString &input) const
{
    bool ok = false;
    auto value = input.toInt(&ok);
    if (!ok)
    {
        input = QString::number(0);
        return;
    }
    if (m_zeroBased)
    {
        value++;
    }
    value = (value / 16) * 16;
    if (m_zeroBased && value != 0)
    {
        value--;
    }
    input = QString::number(value);
}

}
}
