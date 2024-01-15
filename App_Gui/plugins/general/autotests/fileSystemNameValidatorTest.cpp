#include <QTest>
#include <QSignalSpy>

#include "../fileSystemNameValidator.h"

Q_DECLARE_METATYPE(QValidator::State)

using precitec::gui::FileSystemNameValidator;

class FileSystemNameValidatorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testValidate_data();
    void testValidate();
    void testFixup_data();
    void testFixup();
    void testAllowWhiteSpace();
};

void FileSystemNameValidatorTest::initTestCase()
{
    qRegisterMetaType<QValidator::State>();
}

void FileSystemNameValidatorTest::testValidate_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QValidator::State>("result");

    QTest::newRow("empty") << QString{} << QValidator::Intermediate;
    QTest::newRow("255 characters") <<  QStringLiteral("Lorem_ipsum_dolor_sit_amet__consetetur_sadipscing_elitr__sed_diam_nonumy_eirmod_tempor_invidunt_ut_labore_et_dolore_magna_aliquyam_erat__sed_diam_voluptua__At_vero_eos_et_accusam_et_justo_duo_dolores_et_ea_rebum__Stet_clita_kasd_gubergren__no_sea_takimata") << QValidator::Acceptable;
    QTest::newRow("256 characters") << QStringLiteral("Lorem_ipsum_dolor_sit_amet__consetetur_sadipscing_elitr__sed_diam_nonumy_eirmod_tempor_invidunt_ut_labore_et_dolore_magna_aliquyam_erat__sed_diam_voluptua__At_vero_eos_et_accusam_et_justo_duo_dolores_et_ea_rebum__Stet_clita_kasd_gubergren__no_sea_takimata_") << QValidator::Invalid;
    QTest::newRow("one character") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("whitespace") << QStringLiteral(" ") << QValidator::Invalid;
    QTest::newRow("emoji") << QStringLiteral("😀") << QValidator::Invalid;
    QTest::newRow("alphanumeric") << QStringLiteral("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") << QValidator::Acceptable;
    QTest::newRow("!") << QStringLiteral("!") << QValidator::Invalid;
    QTest::newRow(".") << QStringLiteral(".") << QValidator::Invalid;
    QTest::newRow(",") << QStringLiteral(",") << QValidator::Invalid;
    QTest::newRow("\"") << QStringLiteral("\"") << QValidator::Invalid;
    QTest::newRow("'") << QStringLiteral("'") << QValidator::Invalid;
    QTest::newRow("@") << QStringLiteral("@") << QValidator::Invalid;
    QTest::newRow("#") << QStringLiteral("#") << QValidator::Invalid;
    QTest::newRow("$") << QStringLiteral("$") << QValidator::Invalid;
    QTest::newRow("%") << QStringLiteral("%") << QValidator::Invalid;
    QTest::newRow("^") << QStringLiteral("^") << QValidator::Invalid;
    QTest::newRow("&") << QStringLiteral("&") << QValidator::Invalid;
    QTest::newRow("*") << QStringLiteral("*") << QValidator::Invalid;
    QTest::newRow("(") << QStringLiteral("(") << QValidator::Invalid;
    QTest::newRow(")") << QStringLiteral(")") << QValidator::Invalid;
    QTest::newRow("[") << QStringLiteral("]") << QValidator::Invalid;
    QTest::newRow("]") << QStringLiteral("]") << QValidator::Invalid;
    QTest::newRow("{") << QStringLiteral("}") << QValidator::Invalid;
    QTest::newRow("|") << QStringLiteral("|") << QValidator::Invalid;
    QTest::newRow("\\") << QStringLiteral("\\") << QValidator::Invalid;
    QTest::newRow("/") << QStringLiteral("/") << QValidator::Invalid;
    QTest::newRow("?") << QStringLiteral("?") << QValidator::Invalid;
    QTest::newRow("<") << QStringLiteral("<") << QValidator::Invalid;
    QTest::newRow(">") << QStringLiteral(">") << QValidator::Invalid;
    QTest::newRow("=") << QStringLiteral("=") << QValidator::Invalid;
    QTest::newRow("+") << QStringLiteral("+") << QValidator::Invalid;
    QTest::newRow("~") << QStringLiteral("~") << QValidator::Invalid;
    QTest::newRow("`") << QStringLiteral("`") << QValidator::Invalid;
    QTest::newRow(":") << QStringLiteral(":") << QValidator::Invalid;
    QTest::newRow(";") << QStringLiteral(";") << QValidator::Invalid;
    QTest::newRow("-") << QStringLiteral("-") << QValidator::Acceptable;
    QTest::newRow("_") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("Linebreak") << QStringLiteral("\n") << QValidator::Invalid;
}

void FileSystemNameValidatorTest::testValidate()
{
    FileSystemNameValidator validator{};
    QFETCH(QString, input);
    int pos = 0;
    QTEST(validator.validate(input, pos), "result");
}

void FileSystemNameValidatorTest::testFixup_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<QValidator::State>("result");

    QTest::newRow("empty") << QString{} << QString{} << QValidator::Intermediate;
    QTest::newRow("256 characters") <<  QStringLiteral("Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata ") << QStringLiteral("Lorem_ipsum_dolor_sit_amet__consetetur_sadipscing_elitr__sed_diam_nonumy_eirmod_tempor_invidunt_ut_labore_et_dolore_magna_aliquyam_erat__sed_diam_voluptua__At_vero_eos_et_accusam_et_justo_duo_dolores_et_ea_rebum__Stet_clita_kasd_gubergren__no_sea_takimata") << QValidator::Acceptable;

    QTest::newRow("whitespace on start") << QStringLiteral(" x") << QStringLiteral("x") << QValidator::Acceptable;
    QTest::newRow("trailing whitespace") << QStringLiteral("x ") << QStringLiteral("x") << QValidator::Acceptable;
    QTest::newRow("whitespace inbetween") << QStringLiteral("x y") << QStringLiteral("x_y") << QValidator::Acceptable;
    QTest::newRow("whitespace") << QStringLiteral(" ") << QStringLiteral("") << QValidator::Intermediate;
    QTest::newRow("emoji") << QStringLiteral("😀") << QStringLiteral("__") << QValidator::Acceptable;
    QTest::newRow("alphanumeric") << QStringLiteral("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") << QStringLiteral("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") << QValidator::Acceptable;
    QTest::newRow("!") << QStringLiteral("!") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow(".") << QStringLiteral(".") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow(",") << QStringLiteral(",") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("\"") << QStringLiteral("\"") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("'") << QStringLiteral("'") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("@") << QStringLiteral("@") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("#") << QStringLiteral("#") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("$") << QStringLiteral("$") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("%") << QStringLiteral("%") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("^") << QStringLiteral("^") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("&") << QStringLiteral("&") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("*") << QStringLiteral("*") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("(") << QStringLiteral("(") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow(")") << QStringLiteral(")") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("[") << QStringLiteral("]") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("]") << QStringLiteral("]") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("{") << QStringLiteral("}") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("|") << QStringLiteral("|") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("\\") << QStringLiteral("\\") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("/") << QStringLiteral("/") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("?") << QStringLiteral("?") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("<") << QStringLiteral("<") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow(">") << QStringLiteral(">") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("=") << QStringLiteral("=") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("+") << QStringLiteral("+") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("~") << QStringLiteral("~") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("`") << QStringLiteral("`") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow(":") << QStringLiteral(":") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow(";") << QStringLiteral(";") << QStringLiteral("_") << QValidator::Acceptable;
    QTest::newRow("-") << QStringLiteral("-") << QStringLiteral("-") << QValidator::Acceptable;
    QTest::newRow("_") << QStringLiteral("_") << QStringLiteral("_") << QValidator::Acceptable;
}

void FileSystemNameValidatorTest::testFixup()
{
    FileSystemNameValidator validator{};
    QFETCH(QString, input);
    validator.fixup(input);
    QTEST(input, "output");
    int pos = 0;
    QTEST(validator.validate(input, pos), "result");
}

void FileSystemNameValidatorTest::testAllowWhiteSpace()
{
    FileSystemNameValidator validator{};
    QCOMPARE(validator.allowWhiteSpace(), false);

    QSignalSpy allowWhiteSpaceChangedSpy{&validator, &FileSystemNameValidator::allowWhiteSpaceChanged};
    QVERIFY(allowWhiteSpaceChangedSpy.isValid());

    QString whitespace = QStringLiteral(" ");
    QString trailingWhitespace = QStringLiteral("x ");
    QString whitespaceAtFront = QStringLiteral(" x");
    QString whitespaceInbetween = QStringLiteral("x y");
    int pos = 0;
    QCOMPARE(validator.validate(whitespace, pos), QValidator::Invalid);
    QString fixup = whitespace;
    validator.fixup(fixup);
    QCOMPARE(fixup, QStringLiteral(""));

    QCOMPARE(validator.validate(trailingWhitespace, pos), QValidator::Intermediate);
    fixup = trailingWhitespace;
    validator.fixup(fixup);
    QCOMPARE(fixup, QStringLiteral("x"));

    QCOMPARE(validator.validate(whitespaceAtFront, pos), QValidator::Invalid);
    fixup = whitespaceAtFront;
    validator.fixup(fixup);
    QCOMPARE(fixup, QStringLiteral("x"));

    QCOMPARE(validator.validate(whitespaceInbetween, pos), QValidator::Invalid);
    fixup = whitespaceInbetween;
    validator.fixup(fixup);
    QCOMPARE(fixup, QStringLiteral("x_y"));

    // now allow the whitespace
    validator.setAllowWhiteSpace(true);
    QCOMPARE(validator.allowWhiteSpace(), true);
    QCOMPARE(allowWhiteSpaceChangedSpy.count(), 1);
    // setting same should not change
    QCOMPARE(validator.allowWhiteSpace(), true);
    QCOMPARE(allowWhiteSpaceChangedSpy.count(), 1);

    QCOMPARE(validator.validate(whitespace, pos), QValidator::Invalid);
    fixup = whitespace;
    validator.fixup(fixup);
    QCOMPARE(fixup, QStringLiteral(""));

    QCOMPARE(validator.validate(trailingWhitespace, pos), QValidator::Intermediate);
    fixup = trailingWhitespace;
    validator.fixup(fixup);
    QCOMPARE(fixup, QStringLiteral("x"));

    QCOMPARE(validator.validate(whitespaceAtFront, pos), QValidator::Invalid);
    fixup = whitespaceAtFront;
    validator.fixup(fixup);
    QCOMPARE(fixup, QStringLiteral("x"));

    QCOMPARE(validator.validate(whitespaceInbetween, pos), QValidator::Acceptable);
    fixup = whitespaceInbetween;
    validator.fixup(fixup);
    QCOMPARE(fixup, QStringLiteral("x y"));

    // and set back to false
    validator.setAllowWhiteSpace(false);
    QCOMPARE(validator.allowWhiteSpace(), false);
    QCOMPARE(allowWhiteSpaceChangedSpy.count(), 2);
}

QTEST_GUILESS_MAIN(FileSystemNameValidatorTest)
#include "fileSystemNameValidatorTest.moc"
