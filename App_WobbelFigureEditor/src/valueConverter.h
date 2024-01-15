#pragma once

#include <QObject>

namespace precitec
{
namespace scanmaster
{
namespace components
{
namespace wobbleFigureEditor
{

/**
* Singleton class providing functions which are used to display value to the frontend and to
* update values correctly to the backend.
**/
class ValueConverter : public QObject
{
    Q_OBJECT

public:
    ~ValueConverter() override;
    static ValueConverter* instance();

    Q_INVOKABLE double convertDoubleToPercent(double value) const;
    Q_INVOKABLE double convertFromPercentDouble(double value) const;

Q_SIGNALS:

private:
    explicit ValueConverter(QObject* parent = nullptr);

};

}
}
}
}

