#include "fileSystemNameValidator.h"

namespace precitec
{
namespace gui
{

FileSystemNameValidator::FileSystemNameValidator(QObject* parent)
    : QValidator(parent)
{
}

FileSystemNameValidator::~FileSystemNameValidator() = default;

void FileSystemNameValidator::fixup(QString& input) const
{
    if (input.isEmpty())
    {
        return;
    }
    input = input.trimmed();
    if (input.length() > 255)
    {
        input = input.left(255);
    }
    for (auto it = input.begin(); it != input.end(); it++)
    {
        if (it->isDigit())
        {
            continue;
        }
        if (it->category() == QChar::Letter_Lowercase)
        {
            continue;
        }
        if (it->category() == QChar::Letter_Uppercase)
        {
            continue;
        }
        if (it->category() == QChar::Punctuation_Connector)
        {
            continue;
        }
        if (it->category() == QChar::Punctuation_Dash)
        {
            continue;
        }
        if (m_allowWhiteSpae && *it == QChar(QLatin1Char(' ')))
        {
            continue;
        }
        *it = QLatin1Char('_');
    }
}

QValidator::State FileSystemNameValidator::validate(QString& input, int& pos) const
{
    if (input.isEmpty())
    {
        return QValidator::Intermediate;
    }
    if (input.length() > 255)
    {
        return QValidator::Invalid;
    }
    if (input.front().isSpace())
    {
        return QValidator::Invalid;
    }
    if (input.back().isSpace())
    {
        return QValidator::Intermediate;
    }
    for (auto it = input.cbegin(); it != input.cend(); it++)
    {
        if (it->isDigit())
        {
            continue;
        }
        if (it->category() == QChar::Letter_Lowercase)
        {
            continue;
        }
        if (it->category() == QChar::Letter_Uppercase)
        {
            continue;
        }
        if (it->category() == QChar::Punctuation_Connector)
        {
            continue;
        }
        if (it->category() == QChar::Punctuation_Dash)
        {
            continue;
        }
        if (m_allowWhiteSpae && *it == QChar(QLatin1Char(' ')))
        {
            continue;
        }
        return QValidator::Invalid;
    }
    return QValidator::Acceptable;
}

void FileSystemNameValidator::setAllowWhiteSpace(bool set)
{
    if (m_allowWhiteSpae == set)
    {
        return;
    }
    m_allowWhiteSpae = set;
    emit allowWhiteSpaceChanged();
}

}
}
