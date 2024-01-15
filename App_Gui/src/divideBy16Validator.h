#pragma once

#include <QIntValidator>

namespace precitec
{
namespace gui
{

class DivideBy16Validator : public QIntValidator
{
    Q_OBJECT
    Q_PROPERTY(bool zeroBased READ isZeroBased WRITE setZeroBased NOTIFY zeroBasedChanged)
public:
    DivideBy16Validator(QObject *parent = nullptr);
    ~DivideBy16Validator() override;

    bool isZeroBased() const
    {
        return m_zeroBased;
    }
    void setZeroBased(bool set);
    QValidator::State validate(QString &input, int &pos) const override;
    void fixup(QString &input) const override;

Q_SIGNALS:
    void zeroBasedChanged();

private:
    bool m_zeroBased = false;
};

}
}
