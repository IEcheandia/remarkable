#include "optionalPortValidator.h"

namespace precitec
{
namespace gui
{
namespace components
{
namespace scheduler
{

OptionalPortValidator::OptionalPortValidator(QObject* parent)
    : QIntValidator(parent)
{
    setBottom(0);
    setTop(65535);
}

OptionalPortValidator::~OptionalPortValidator() = default;

QValidator::State OptionalPortValidator::validate(QString& input, int& pos) const
{
    if (input.isEmpty())
    {
        return QValidator::Acceptable;
    }
    return QIntValidator::validate(input, pos);
}


}
}
}
}
