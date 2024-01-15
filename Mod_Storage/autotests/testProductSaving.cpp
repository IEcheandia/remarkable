#include <QTest>
#include <QSignalSpy>

#include "../src/copyMode.h"
#include "../src/parameter.h"
#include "../src/parameterSet.h"
#include "../src/product.h"
#include "../src/seam.h"
#include "../src/seamInterval.h"
#include "../src/seamSeries.h"

using precitec::storage::CopyMode;
using precitec::storage::Parameter;
using precitec::storage::Product;
using precitec::storage::SeamInterval;
using precitec::storage::ParameterSet;

class TestProductSaving : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testParameter_data();
    void testParameter();
    void testParameterSet();
    void testSeamInterval_data();
    void testSeamInterval();
    void testProductLengthUnit_data();
    void testProductLengthUnit();
};

void TestProductSaving::testParameter_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<QUuid>("typeId");
    QTest::addColumn<QUuid>("filterId");
    QTest::addColumn<QVariant>("value");
    QTest::addColumn<Parameter::DataType>("type");

    QTest::newRow("test") << QStringLiteral("FooBar") << QUuid::createUuid() << QUuid::createUuid() << QVariant{} << Parameter::DataType::Unknown;
    QTest::newRow("empty") << QString() << QUuid() << QUuid() << QVariant{} << Parameter::DataType::Unknown;
    QTest::newRow("int value") << QString() << QUuid() << QUuid() << QVariant{3} << Parameter::DataType::Unknown;
    QTest::newRow("double value") << QString() << QUuid() << QUuid() << QVariant{3.5} << Parameter::DataType::Unknown;
    QTest::newRow("true value") << QString() << QUuid() << QUuid() << QVariant{true} << Parameter::DataType::Unknown;
    QTest::newRow("false value") << QString() << QUuid() << QUuid() << QVariant{false} << Parameter::DataType::Unknown;
    QTest::newRow("boolean") << QString() << QUuid() << QUuid() << QVariant{} << Parameter::DataType::Boolean;
    QTest::newRow("double") << QString() << QUuid() << QUuid() << QVariant{} << Parameter::DataType::Double;
    QTest::newRow("float") << QString() << QUuid() << QUuid() << QVariant{} << Parameter::DataType::Float;
    QTest::newRow("integer") << QString() << QUuid() << QUuid() << QVariant{} << Parameter::DataType::Integer;
    QTest::newRow("unsigned integer") << QString() << QUuid() << QUuid() << QVariant{} << Parameter::DataType::UnsignedInteger;
}

void TestProductSaving::testParameter()
{
    Parameter parameter{QUuid::createUuid(), static_cast<ParameterSet*>(nullptr)};
    QFETCH(QString, name);
    parameter.setName(name);
    QCOMPARE(parameter.name(), name);
    QFETCH(QUuid, typeId);
    parameter.setTypeId(typeId);
    QCOMPARE(parameter.typeId(), typeId);
    QFETCH(QUuid, filterId);
    parameter.setFilterId(filterId);
    QCOMPARE(parameter.filterId(), filterId);
    QFETCH(QVariant, value);
    parameter.setValue(value);
    QCOMPARE(parameter.value(), value);
    QFETCH(Parameter::DataType, type);
    parameter.setType(type);
    QCOMPARE(parameter.type(), type);

    QScopedPointer<Parameter> second{Parameter::fromJson(parameter.toJson(), static_cast<ParameterSet*>(nullptr))};
    QCOMPARE(second->uuid(), parameter.uuid());
    QCOMPARE(second->name(), parameter.name());
    QCOMPARE(second->name(), name);
    QCOMPARE(second->value(), parameter.value());
    QCOMPARE(second->value(), value);
    QCOMPARE(second->type(), parameter.type());
    QCOMPARE(second->type(), type);
    QCOMPARE(second->typeId(), parameter.typeId());
    QCOMPARE(second->typeId(), typeId);
    QCOMPARE(second->filterId(), parameter.filterId());
    QCOMPARE(second->filterId(), filterId);
}

void TestProductSaving::testParameterSet()
{
    ParameterSet set{QUuid::createUuid()};

    QScopedPointer<ParameterSet> set2{ParameterSet::fromJson(set.toJson())};
    QCOMPARE(set2->uuid(), set.uuid());
    QCOMPARE(set2->parameters().empty(), true);
}

void TestProductSaving::testSeamInterval_data()
{
    QTest::addColumn<int>("length");
    QTest::addColumn<int>("number");
    QTest::addColumn<QUuid>("graph");
    QTest::addColumn<QUuid>("graphParamSet");
    QTest::addColumn<bool>("changeIntervalUuid");

    QTest::newRow("reasonable data, duplicatee and change UUID") 
        << 300 << 2 <<  QUuid::createUuid() << QUuid::createUuid() << true;
    QTest::newRow("reasonable data, duplicate and keep UUID") 
        << 300 << 2 <<  QUuid::createUuid() << QUuid::createUuid() << false;
    QTest::newRow("default") << 0 << 0 << QUuid() << QUuid() << true;
}

void TestProductSaving::testSeamInterval()
{
    SeamInterval si{QUuid::createUuid(), nullptr};
    QVERIFY(!si.hardwareParameters());
    QFETCH(int, length);
    si.setLength(length);
    QCOMPARE(si.length(), length);
    QFETCH(int, number);
    si.setNumber(number);
    QCOMPARE(si.number(), number);
    QCOMPARE(si.visualNumber(), number + 1);
    QFETCH(QUuid, graph);
    si.setGraph(graph);
    QCOMPARE(si.graph(), graph);
    QFETCH(QUuid, graphParamSet);
    si.setGraphParamSet(graphParamSet);
    QCOMPARE(si.graphParamSet(), graphParamSet);

    QScopedPointer<SeamInterval> si2(SeamInterval::fromJson(si.toJson(), nullptr));
    QCOMPARE(si2->uuid(), si.uuid());
    QCOMPARE(si2->length(), si.length());
    QCOMPARE(si2->length(), length);
    QCOMPARE(si2->number(), si.number());
    QCOMPARE(si2->number(), number);

    QFETCH(bool, changeIntervalUuid);
    const auto copyMode = changeIntervalUuid ? CopyMode::WithDifferentIds : CopyMode::Identical;
    QScopedPointer<SeamInterval> duplicated(si.duplicate(copyMode, nullptr));
    QCOMPARE(duplicated->uuid() != si.uuid(), changeIntervalUuid);
    QCOMPARE(duplicated->length(), si.length());
    QCOMPARE(duplicated->length(), length);
    QCOMPARE(duplicated->number(), si.number());
    QCOMPARE(duplicated->number(), number);
}

void TestProductSaving::testProductLengthUnit_data()
{
    QTest::addColumn<Product::LengthUnit>("unit");
    QTest::addColumn<bool>("changed");

    QTest::newRow("millimeter") << Product::LengthUnit::Millimeter << false;
    QTest::newRow("degree") << Product::LengthUnit::Degree << true;
}

void TestProductSaving::testProductLengthUnit()
{
    Product p{QUuid::createUuid()};
    QFETCH(Product::LengthUnit, unit);
    QSignalSpy lengthUnitChangedSpy(&p, &Product::lengthUnitChanged);
    QVERIFY(lengthUnitChangedSpy.isValid());
    p.setLengthUnit(unit);
    QTEST(!lengthUnitChangedSpy.isEmpty(), "changed");

    QScopedPointer<Product> p2{Product::fromJson(p.toJson())};
    QVERIFY(!p2.isNull());
    QCOMPARE(p2->lengthUnit(), unit);
}

QTEST_GUILESS_MAIN(TestProductSaving)
#include "testProductSaving.moc"
