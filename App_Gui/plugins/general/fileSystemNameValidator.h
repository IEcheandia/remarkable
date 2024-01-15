#pragma once

#include <QValidator>

namespace precitec
{
namespace gui
{

/**
 * A QValidator to validate a string which can be used across various file systems.
 *
 * A given input is considered acceptable if it:
 * @li has a minimum length of one character
 * @li a maximum length of 255 characters
 * @li does not allow whitespace characters at the start of the string
 * @li removes trailing whitespaces
 * @li contains characters from the unicode categories Ll, Lu, Pc, Pd, Nd
 * @li if property @link{allowWhiteSpace} is @c true also the white space character is considered acceptable
 *
 * Any other character is considered invalid. The fixup method replaces such characters with _.
 **/
class FileSystemNameValidator : public QValidator
{
    Q_OBJECT
    /**
     * Whether the white space character is an allowed character.
     * If @c false validate will return Invalid for a white space character and fixup will replace it by underscore.
     * The default value is @c false.
     **/
    Q_PROPERTY(bool allowWhiteSpace READ allowWhiteSpace WRITE setAllowWhiteSpace NOTIFY allowWhiteSpaceChanged)
public:
    FileSystemNameValidator(QObject *parent = nullptr);
    ~FileSystemNameValidator() override;

    QValidator::State validate(QString &input, int &pos) const override;
    void fixup(QString &input) const override;

    bool allowWhiteSpace() const
    {
        return m_allowWhiteSpae;
    }
    void setAllowWhiteSpace(bool set);

Q_SIGNALS:
    void allowWhiteSpaceChanged();


private:
    bool m_allowWhiteSpae = false;
};

}
}
