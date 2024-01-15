#include "xlsxAbstractWorksheet.h"

#include <QObject>

#include <set>
#include <vector>

namespace precitec
{

namespace storage
{
class Product;
class SeamSeries;
class Seam;
class ParameterSet;
class AbstractMeasureTask;
}

namespace gui
{

class HardwareParametersWorksheet : public AbstractWorksheet
{
    Q_OBJECT
public:
    HardwareParametersWorksheet(QObject *parent = nullptr);
    virtual ~HardwareParametersWorksheet() = default;
    void setProduct(storage::Product *product);

private:
    void formWorksheet() override;
    void initSeamSeries(storage::SeamSeries *seamSeries);
    void initParameterSet(storage::ParameterSet *set);
    void initSeam(storage::Seam *seam);

    std::set<QString> m_hardwareParameterKeys;
    std::vector<storage::AbstractMeasureTask *> m_measureTasks;

    storage::Product *m_product = nullptr;
    QMetaObject::Connection m_productDestroyed;
};

} // namespace gui
} // namespace precitec