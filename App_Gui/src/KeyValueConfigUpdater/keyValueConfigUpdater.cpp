#include "keyValueConfigUpdater.h"

#include <QFile>
#include <QSaveFile>

namespace precitec
{

void KeyValueConfigUpdater::setFilePath(const QString &filePath)
{
    m_filePath = filePath;
}

void KeyValueConfigUpdater::setFallbackFilePath(const QString &filePath)
{
    m_fallbackFilePath = filePath;
}

void KeyValueConfigUpdater::addKeyValue(const QByteArray &key, const QByteArray &value)
{
    m_configKeys.emplace(key, value);
}

std::vector<QByteArray> KeyValueConfigUpdater::readLines() const
{
    QFile file;
    file.setFileName(m_filePath);
    if (!file.exists())
    {
        file.setFileName(m_fallbackFilePath);
    }
    if (!file.exists())
    {
        return {};
    }
    if (!file.open(QIODevice::ReadOnly))
    {
        return {};
    }
    std::vector<QByteArray> lines;
    QByteArray line;
    do
    {
        line = file.readLine();
        lines.push_back(line);
    } while (!line.isEmpty());
    return lines;
}

void KeyValueConfigUpdater::writeLines(const std::vector<QByteArray> &lines)
{
    if (lines.empty())
    {
        return;
    }
    QFile file{m_filePath};
    if (!file.open(QIODevice::WriteOnly))
    {
        return;
    }

    auto updateLine = [&file, this] (const auto &kv)
    {
        if (kv.second.compare("!DELETE!") == 0)
        {
            return;
        }
        file.write(kv.first);
        if (m_useSpaces)
        {
            file.write(QByteArrayLiteral(" "));
        }
        file.write(QByteArrayLiteral("="));
        if (m_useSpaces)
        {
            file.write(QByteArrayLiteral(" "));
        }
        file.write(kv.second);
        file.write(QByteArrayLiteral("\n"));
    };

    for (const auto &line : lines)
    {
        bool found = false;
        auto it = m_configKeys.begin();
        while (it != m_configKeys.end())
        {
            if (line.startsWith(it->first))
            {
                found = true;
                updateLine(*it);
                m_configKeys.erase(it);
                break;
            }
            it++;
        }
        if (!found)
        {
            file.write(line);
        }
    }

    // add all remaining lines
    std::for_each(m_configKeys.begin(), m_configKeys.end(), updateLine);
    m_configKeys.clear();

    file.flush();
    file.close();
}

void KeyValueConfigUpdater::execute()
{
    writeLines(readLines());
}

}
