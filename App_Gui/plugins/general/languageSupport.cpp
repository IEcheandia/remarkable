#include "languageSupport.h"
#include "weldmasterPaths.h"
#include "guiConfiguration.h"

#include <QDirIterator>
#include <QFutureWatcher>
#include <QXmlStreamReader>
#include <QtConcurrentRun>

namespace precitec
{
namespace gui
{

LanguageSupport *LanguageSupport::instance()
{
    static LanguageSupport s_instance;
    return &s_instance;
}

LanguageSupport::LanguageSupport(QObject* parent)
    : QObject(parent)
{
    auto watcher = new QFutureWatcher<void>{this};
    connect(watcher, &QFutureWatcher<void>::finished, this,
        [this]
        {
            m_ready = true;
            emit readyChanged();
        }
    );
    watcher->setFuture(QtConcurrent::run(this, &LanguageSupport::init));
}

LanguageSupport::~LanguageSupport() = default;

QString LanguageSupport::getString(const QString& key) const
{
    return getStringWithFallback(key, key);
}

QString LanguageSupport::getStringWithFallback(const QString &key, const QString &fallback) const
{
    if (!m_ready)
    {
        return fallback;
    }
    auto it = m_texts.find(key);
    if (it == m_texts.end())
    {
        return fallback;
    }
    return it->second;
}

void LanguageSupport::init()
{
    static QString s_default = QStringLiteral("en-US");
    static std::map<QString, QString> s_supportedLanguages{
        std::make_pair(QStringLiteral("en"), s_default),
        std::make_pair(QStringLiteral("de"), QStringLiteral("de-DE")),
    };
    QString languageKey;
    auto it = s_supportedLanguages.find(GuiConfiguration::instance()->language());
    if (it != s_supportedLanguages.end())
    {
        languageKey = it->second;
    }
    else
    {
        languageKey = s_default;
    }

    auto parseFiles = [this] (const QString &language)
    {
        QDirIterator dir{WeldmasterPaths::instance()->languageDir(), {QStringLiteral("*.%1.xml").arg(language)}, QDir::Files};
        while (dir.hasNext())
        {
            dir.next();
            parseFile(dir.fileInfo());
        }
    };
    parseFiles(languageKey);
    if (languageKey != s_default)
    {
        parseFiles(s_default);
    }
}

void LanguageSupport::parseFile(const QFileInfo &path)
{
    QFile file(path.absoluteFilePath());
    if (!file.open(QIODevice::ReadOnly))
    {
        return;
    }
    QXmlStreamReader xml{&file};
    xml.readNextStartElement();
    if (xml.name() != QStringLiteral("Sgm2Loc"))
    {
        return;
    }
    const QString text = QStringLiteral("Text");
    while (!xml.atEnd())
    {
        xml.readNextStartElement();
        if (!xml.attributes().hasAttribute(text))
        {
            continue;
        }
        m_texts.emplace(xml.name().toString(), xml.attributes().value(text).toString());
    }
}

}
}
