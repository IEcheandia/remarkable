#pragma once

#include <QString>
#include <map>
#include <vector>

namespace precitec
{

class KeyValueConfigUpdater
{
public:
    void setFilePath(const QString &filePath);
    void setFallbackFilePath(const QString &filePath);

    QString filePath() const
    {
        return m_filePath;
    }
    QString fallbackFilePath() const
    {
        return m_fallbackFilePath;
    }

    void setUseSpaces(bool useSpaces)
    {
        m_useSpaces = useSpaces;
    }

    void addKeyValue(const QByteArray &key, const QByteArray &value);

    void execute();

private:
    std::vector<QByteArray> readLines() const;
    void writeLines(const std::vector<QByteArray> &lines);
    QString m_filePath;
    QString m_fallbackFilePath;
    std::map<QByteArray, QByteArray> m_configKeys;
    bool m_useSpaces = true;
};

}
