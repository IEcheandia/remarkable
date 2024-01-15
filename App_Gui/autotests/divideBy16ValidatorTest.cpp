#include "../src/divideBy16Validator.h"

#include <QTest>

Q_DECLARE_METATYPE(QValidator::State)

using precitec::gui::DivideBy16Validator;

class DivideBy16ValidatorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testValidate_data();
    void testValidate();
    void testFixup_data();
    void testFixup();
};

void DivideBy16ValidatorTest::initTestCase()
{
    qRegisterMetaType<QValidator::State>();
}

void DivideBy16ValidatorTest::testValidate_data()
{
    QTest::addColumn<bool>("zeroBased");
    QTest::addColumn<QString>("input");
    QTest::addColumn<QValidator::State>("result");

    QTest::newRow("empty") << false << QString{} << QValidator::Intermediate;
    QTest::newRow("0/1 based") << false << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("1/1 based") << false << QStringLiteral("1") << QValidator::Intermediate;
    QTest::newRow("2/1 based") << false << QStringLiteral("2") << QValidator::Intermediate;
    QTest::newRow("3/1 based") << false << QStringLiteral("3") << QValidator::Intermediate;
    QTest::newRow("4/1 based") << false << QStringLiteral("4") << QValidator::Intermediate;
    QTest::newRow("5/1 based") << false << QStringLiteral("5") << QValidator::Intermediate;
    QTest::newRow("6/1 based") << false << QStringLiteral("6") << QValidator::Intermediate;
    QTest::newRow("7/1 based") << false << QStringLiteral("7") << QValidator::Intermediate;
    QTest::newRow("8/1 based") << false << QStringLiteral("8") << QValidator::Intermediate;
    QTest::newRow("9/1 based") << false << QStringLiteral("9") << QValidator::Intermediate;
    QTest::newRow("10/1 based") << false << QStringLiteral("10") << QValidator::Intermediate;
    QTest::newRow("11/1 based") << false << QStringLiteral("11") << QValidator::Intermediate;
    QTest::newRow("12/1 based") << false << QStringLiteral("12") << QValidator::Intermediate;
    QTest::newRow("13/1 based") << false << QStringLiteral("13") << QValidator::Intermediate;
    QTest::newRow("14/1 based") << false << QStringLiteral("14") << QValidator::Intermediate;
    QTest::newRow("15/1 based") << false << QStringLiteral("15") << QValidator::Intermediate;
    QTest::newRow("16/1 based") << false << QStringLiteral("16") << QValidator::Acceptable;
    QTest::newRow("17/1 based") << false << QStringLiteral("17") << QValidator::Intermediate;

    QTest::newRow("0/0 based") << true << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("1/0 based") << true << QStringLiteral("1") << QValidator::Intermediate;
    QTest::newRow("2/0 based") << true << QStringLiteral("2") << QValidator::Intermediate;
    QTest::newRow("3/0 based") << true << QStringLiteral("3") << QValidator::Intermediate;
    QTest::newRow("4/0 based") << true << QStringLiteral("4") << QValidator::Intermediate;
    QTest::newRow("5/0 based") << true << QStringLiteral("5") << QValidator::Intermediate;
    QTest::newRow("6/0 based") << true << QStringLiteral("6") << QValidator::Intermediate;
    QTest::newRow("7/0 based") << true << QStringLiteral("7") << QValidator::Intermediate;
    QTest::newRow("8/0 based") << true << QStringLiteral("8") << QValidator::Intermediate;
    QTest::newRow("9/0 based") << true << QStringLiteral("9") << QValidator::Intermediate;
    QTest::newRow("10/0 based") << true << QStringLiteral("10") << QValidator::Intermediate;
    QTest::newRow("11/0 based") << true << QStringLiteral("11") << QValidator::Intermediate;
    QTest::newRow("12/0 based") << true << QStringLiteral("12") << QValidator::Intermediate;
    QTest::newRow("13/0 based") << true << QStringLiteral("13") << QValidator::Intermediate;
    QTest::newRow("14/0 based") << true << QStringLiteral("14") << QValidator::Intermediate;
    QTest::newRow("15/0 based") << true << QStringLiteral("15") << QValidator::Acceptable;
    QTest::newRow("16/0 based") << true << QStringLiteral("16") << QValidator::Intermediate;
    QTest::newRow("17/0 based") << true << QStringLiteral("17") << QValidator::Intermediate;
}

void DivideBy16ValidatorTest::testValidate()
{
    DivideBy16Validator validator{};
    QFETCH(bool, zeroBased);
    validator.setZeroBased(zeroBased);
    QFETCH(QString, input);
    int pos = 0;
    QTEST(validator.validate(input, pos), "result");
}

void DivideBy16ValidatorTest::testFixup_data()
{
    QTest::addColumn<bool>("zeroBased");
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<QValidator::State>("result");

    QTest::newRow("empty") << false << QString{} << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("0/1 based") << false << QStringLiteral("0") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("1/1 based") << false << QStringLiteral("1") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("2/1 based") << false << QStringLiteral("2") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("3/1 based") << false << QStringLiteral("3") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("4/1 based") << false << QStringLiteral("4") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("5/1 based") << false << QStringLiteral("5") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("6/1 based") << false << QStringLiteral("6") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("7/1 based") << false << QStringLiteral("7") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("8/1 based") << false << QStringLiteral("8") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("9/1 based") << false << QStringLiteral("9") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("10/1 based") << false << QStringLiteral("10") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("11/1 based") << false << QStringLiteral("11") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("12/1 based") << false << QStringLiteral("12") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("13/1 based") << false << QStringLiteral("13") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("14/1 based") << false << QStringLiteral("14") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("15/1 based") << false << QStringLiteral("15") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("16/1 based") << false << QStringLiteral("16") << QStringLiteral("16") << QValidator::Acceptable;
    QTest::newRow("17/1 based") << false << QStringLiteral("17") << QStringLiteral("16") << QValidator::Acceptable;

    QTest::newRow("0/0 based") << true << QStringLiteral("0") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("1/0 based") << true << QStringLiteral("1") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("2/0 based") << true << QStringLiteral("2") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("3/0 based") << true << QStringLiteral("3") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("4/0 based") << true << QStringLiteral("4") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("5/0 based") << true << QStringLiteral("5") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("6/0 based") << true << QStringLiteral("6") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("7/0 based") << true << QStringLiteral("7") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("8/0 based") << true << QStringLiteral("8") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("9/0 based") << true << QStringLiteral("9") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("10/0 based") << true << QStringLiteral("10") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("11/0 based") << true << QStringLiteral("11") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("12/0 based") << true << QStringLiteral("12") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("13/0 based") << true << QStringLiteral("13") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("14/0 based") << true << QStringLiteral("14") << QStringLiteral("0") << QValidator::Acceptable;
    QTest::newRow("15/0 based") << true << QStringLiteral("15") << QStringLiteral("15") << QValidator::Acceptable;
    QTest::newRow("16/0 based") << true << QStringLiteral("16") << QStringLiteral("15") << QValidator::Acceptable;
    QTest::newRow("17/0 based") << true << QStringLiteral("17") << QStringLiteral("15") << QValidator::Acceptable;
}

void DivideBy16ValidatorTest::testFixup()
{
    DivideBy16Validator validator{};
    QFETCH(bool, zeroBased);
    validator.setZeroBased(zeroBased);
    QFETCH(QString, input);
    validator.fixup(input);
    QTEST(input, "output");
    int pos = 0;
    QTEST(validator.validate(input, pos), "result");
}

QTEST_GUILESS_MAIN(DivideBy16ValidatorTest)
#include "divideBy16ValidatorTest.moc"
