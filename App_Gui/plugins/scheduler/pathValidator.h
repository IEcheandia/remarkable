#pragma once
#include <QValidator>


namespace precitec
{
namespace gui
{
namespace components
{
namespace scheduler
{

/**
 * Validator which checks whether a given input is a valid path.
 **/
class PathValidator : public QValidator
{
    Q_OBJECT
    /**
     * Whether the path is for a local file system and should be validated also on existance.
     * If @c false only a subset of validation can be performed as it is not possible to evaluate whether a path exists.
     **/
    Q_PROPERTY(bool localFileSystem READ isLocalFileSystem WRITE setLocalFileSystem NOTIFY localFileSystemChanged)
public:
    PathValidator(QObject *parent = nullptr);
    ~PathValidator() override;

    QValidator::State validate(QString &input, int &pos) const override;

    bool isLocalFileSystem() const
    {
        return m_localFileSystem;
    }
    void setLocalFileSystem(bool value);

Q_SIGNALS:
    void localFileSystemChanged();

private:
    bool m_localFileSystem{true};
};

}
}
}
}
