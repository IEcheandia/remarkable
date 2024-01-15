#include <QTest>

#include "../slaveInfoFilterModel.h"

#include "event/ethercatDefines.h"

using precitec::gui::components::ethercat::SlaveInfoFilterModel;

class SlaveInfoFilterModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFilter_data();
    void testFilter();
};

class Model : public QAbstractListModel
{
    Q_OBJECT
public:
    Model(QObject *parent = nullptr) : QAbstractListModel(parent) {}
    ~Model() override {}

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &) const override { return 1; }

    quint32 m_vendor = 0;
    quint32 m_product = 0;
};

QVariant Model::data(const QModelIndex& index, int role) const
{
    Q_UNUSED(index)
    switch (role)
    {
    case Qt::UserRole:
        return m_vendor;
    case Qt::UserRole + 1:
        return m_product;
    }
    return {};
}

void SlaveInfoFilterModelTest::testFilter_data()
{
    QTest::addColumn<int>("vendor");
    QTest::addColumn<int>("product");
    QTest::addColumn<int>("accepted");

    QTest::newRow("EK1100") << VENDORID_BECKHOFF << PRODUCTCODE_EK1100 << 0;
    QTest::newRow("EL1018") << VENDORID_BECKHOFF << PRODUCTCODE_EL1018 << 1;
    QTest::newRow("EL2008") << VENDORID_BECKHOFF << PRODUCTCODE_EL2008 << 1;
    QTest::newRow("EL3102") << VENDORID_BECKHOFF << PRODUCTCODE_EL3102 << 1;
    QTest::newRow("EL3162") << VENDORID_BECKHOFF << PRODUCTCODE_EL3162 << 0;
    QTest::newRow("EL3702") << VENDORID_BECKHOFF << PRODUCTCODE_EL3702 << 1;
    QTest::newRow("EL4102") << VENDORID_BECKHOFF << PRODUCTCODE_EL4102 << 1;
    QTest::newRow("EL4132") << VENDORID_BECKHOFF << PRODUCTCODE_EL4132 << 1;
    QTest::newRow("EL5101") << VENDORID_BECKHOFF << PRODUCTCODE_EL5101 << 0;
    QTest::newRow("kunbus") << VENDORID_KUNBUS << PRODUCTCODE_KUNBUS_GW << 1;
    QTest::newRow("anybus") << VENDORID_HMS << PRODUCTCODE_ANYBUS_GW << 1;
    QTest::newRow("accelnet") << VENDORID_COPLEY << PRODUCTCODE_ACCELNET  << 0;
    QTest::newRow("compax") << VENDORID_PARKER << PRODUCTCODE_COMPAX    << 0;
    QTest::newRow("fieldbus") << VENDORID_HILSCHER << PRODUCTCODE_FIELDBUS << 1;
}

void SlaveInfoFilterModelTest::testFilter()
{
    Model model;
    QFETCH(int, product);
    QFETCH(int, vendor);
    model.m_product = product;
    model.m_vendor = vendor;

    SlaveInfoFilterModel filterModel;
    filterModel.setSourceModel(&model);
    QTEST(filterModel.rowCount(), "accepted");
}

QTEST_GUILESS_MAIN(SlaveInfoFilterModelTest)
#include "slaveInfoFilterModelTest.moc"
