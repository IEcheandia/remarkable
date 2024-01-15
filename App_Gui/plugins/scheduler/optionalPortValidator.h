#pragma once

#include <QIntValidator>

namespace precitec
{
namespace gui
{
namespace components
{
namespace scheduler
{

/**
 * Specialization of QIntValidator to validate a port. An empty input string is acceptable, otherwise it follows the QIntValidator.
 **/
class OptionalPortValidator : public QIntValidator
{
    Q_OBJECT
public:
    OptionalPortValidator(QObject* parent = nullptr);
    ~OptionalPortValidator() override;

    QValidator::State validate(QString& input, int& pos) const override;
};

}
}
}
}
