#pragma once

#include <QObject>

class QFileInfo;

namespace precitec
{
namespace gui
{

/**
 * This singleton class provides access to all Weldmaster translated strings.
 **/
class LanguageSupport : public QObject
{
    Q_OBJECT
    /**
     * Property indicating whether all languages are loaded.
     **/
    Q_PROPERTY(bool ready READ isReady NOTIFY readyChanged)
public:
    ~LanguageSupport() override;

    bool isReady() const
    {
        return m_ready;
    }

    /**
     * @returns the translated string for the given @p key, if not present the @p key is returned.
     **/
    Q_INVOKABLE QString getString(const QString &key) const;

    /**
     * @returns the translated string for the given @p key, if not present the @p fallback is returned.
     **/
    Q_INVOKABLE QString getStringWithFallback(const QString &key, const QString &fallback = {}) const;

    /**
     * @returns Singleton instance
     **/
    static LanguageSupport *instance();

Q_SIGNALS:
    void readyChanged();

private:
    explicit LanguageSupport(QObject *parent = nullptr);
    void init();
    void parseFile(const QFileInfo &path);
    bool m_ready = false;
    std::map<QString, QString> m_texts;
};


}
}

Q_DECLARE_METATYPE(precitec::gui::LanguageSupport*)
