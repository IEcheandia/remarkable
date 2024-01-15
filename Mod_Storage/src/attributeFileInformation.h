#pragma once

#include <QMetaObject>
#include <QStringList>

#include <optional>

class QJsonObject;

namespace precitec
{
namespace storage
{

/**
 * Describes information about an Attribute of type @link{Parameter::File}.
 **/
class AttributeFileInformation
{
    Q_GADGET
    /**
     * A list of suffixes this file Attribute can handle. E.g. for images might be ["bmp", "png", "jpeg"].
     * For no restrictions it is an empty list.
     **/
    Q_PROPERTY(QStringList suffixes READ suffixes CONSTANT)
    /**
     * The location where the file should be stored relative to WM_BASE_DIRECTORY.
     **/
    Q_PROPERTY(QString location READ location CONSTANT)
public:
    AttributeFileInformation();
    ~AttributeFileInformation();

    void setSuffixes(const QStringList& suffixes)
    {
        m_suffixes = suffixes;
    }
    const QStringList& suffixes() const
    {
        return m_suffixes;
    }

    void setLocation(const QString& location)
    {
        m_location = location;
    }
    const QString& location() const
    {
        return m_location;
    }

    static std::optional<AttributeFileInformation> fromJson(const QJsonObject& object);

private:
    QStringList m_suffixes;
    QString m_location;
};

}
}
