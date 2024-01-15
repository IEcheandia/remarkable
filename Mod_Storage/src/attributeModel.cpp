#include "attributeModel.h"

#include "attribute.h"
#include "attributeField.h"
#include "jsonSupport.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include <QtConcurrentRun>
#include <QFutureWatcher>

#include "module/logType.h"
#include "module/moduleLogger.h"
#include "message/device.h"

namespace precitec
{
namespace storage
{

AttributeModel::AttributeModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

AttributeModel::~AttributeModel() = default;

QVariant AttributeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant{};
    }
    if (role == Qt::DisplayRole)
    {
        return QVariant::fromValue(m_attributes.at(index.row()));
    }
    return QVariant{};
}

int AttributeModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_attributes.size();
}

void AttributeModel::load(const QString &path)
{
    m_loading = true;
    auto watcher = new QFutureWatcher<std::vector<Attribute*>>{this};
    
    connect(watcher, &QFutureWatcher<std::vector<Attribute*>>::finished, this,
        [this, watcher]
        {
            watcher->deleteLater();
            clear();
            auto result = watcher->result();            
            if (result.empty())
            {                
                wmLog(eError, "AttributeModel: error loading attributes file\n");
                m_loading = false;
                return;
            }

            beginResetModel();
            m_attributes = result;
            m_loading = false;
            endResetModel();
        }
    ); 
    watcher->setFuture(QtConcurrent::run([this, path]
    {
            QFile file(path);
            if (!file.open(QIODevice::ReadOnly))
            {
                
                return std::vector<Attribute*>{};
            }         
            const QByteArray data = file.readAll();
            if (data.isEmpty())
            {
                return std::vector<Attribute*>{};
            }           
            QJsonParseError parseError;
            const auto document = QJsonDocument::fromJson(data, &parseError);
            if (document.isNull())
            {
                wmLog(eError, "AttributeModel parse error: " + parseError.errorString().toStdString() +"\n");
                auto errorSnippet = data.mid(std::max(0, parseError.offset-100), 200);
                wmLog(eDebug, "Parse error in file " + path.toStdString() +"\n");
                wmLog(eDebug, errorSnippet.toStdString());
                return std::vector<Attribute*>{};
            }          
            auto attributes = json::parseAttributes(document.object(), nullptr);      
           
            
            for(auto attribute : attributes)
            {
                attribute->moveToThread(thread());
                attribute->setParent(this);
    
                if((attribute->type() == Parameter::DataType::Enumeration) && (attribute->fields().isEmpty()))
                {                                                  
                    attribute->loadFieldsFromJson(document);                    
                }
            } 
            
            return attributes;
        }
    ));
}

void AttributeModel::loadDefault()
{
    load(QString::fromLocal8Bit(qgetenv("WM_BASE_DIR")) + QStringLiteral("/system_graphs/attributes.json"));
}

void AttributeModel::loadDefaultKeyValue()
{
    load(QString::fromLocal8Bit(qgetenv("WM_BASE_DIR")) + QStringLiteral("/system_graphs/keyValueAttributes.json"));
}

void AttributeModel::clear()
{
    if (m_attributes.empty())
    {
        return;
    }
    beginResetModel();
    qDeleteAll(m_attributes);
    m_attributes.clear();
    endResetModel();
}

Attribute *AttributeModel::findAttribute(const QUuid &id) const
{
    auto it = std::find_if(m_attributes.begin(), m_attributes.end(), [id] (const auto a) { return a->uuid() == id; });
    if (it != m_attributes.end())
    {
        return *it;
    }
    return nullptr;
}

Attribute *AttributeModel::findAttributeByName(const QString &name) const
{
    auto it = std::find_if(m_attributes.begin(), m_attributes.end(), [name] (const auto a) { return a->name() == name; });
    if (it != m_attributes.end())
    {
        return *it;
    }
    return nullptr;
}

QJsonObject AttributeModel::toJson() const
{
        return QJsonObject{
            {
                json::toJson(m_attributes)
            }
        };   
}

void AttributeModel::toJsonFile(QIODevice *device) const
{
    const QJsonDocument document{toJson()};
    device->write(document.toJson());
}

std::vector<Attribute*> AttributeModel::findAttributesByVariantId(const QUuid &variantId) const
{
    std::vector<Attribute*> returnVector;
    for (auto attribute : m_attributes)
    {
        if (attribute->variantId() == variantId)
        {
            returnVector.emplace_back(attribute);
        }
    }
    return returnVector;
}

void AttributeModel::createAttribute(const Poco::SharedPtr<interface::KeyValue>& keyValue, const QUuid& hardwareDevice)
{
    Attribute *attribute = new Attribute{QUuid::createUuid(), this};
    attribute->setName(QString::fromStdString(keyValue->key()));
    attribute->setVariantId(hardwareDevice);
    attribute->setFloatingPointPrecision(keyValue->precision());

    switch (keyValue->type())
    {
    case TChar:
        attribute->setMinValue(keyValue->minima<char>());
        attribute->setMaxValue(keyValue->maxima<char>());
        attribute->setDefaultValue(keyValue->defValue<char>());
        attribute->setType(Parameter::DataType::Integer);
        break;
    case TByte:
        attribute->setMinValue(keyValue->minima<byte>());
        attribute->setMaxValue(keyValue->maxima<byte>());
        attribute->setDefaultValue(keyValue->defValue<byte>());
        attribute->setType(Parameter::DataType::Integer);
        break;
    case TInt:
        attribute->setMinValue(keyValue->minima<int>());
        attribute->setMaxValue(keyValue->maxima<int>());
        attribute->setDefaultValue(keyValue->defValue<int>());
        attribute->setType(Parameter::DataType::Integer);
        break;
    case TUInt:
        attribute->setMinValue(keyValue->minima<uint>());
        attribute->setMaxValue(keyValue->maxima<uint>());
        attribute->setDefaultValue(keyValue->defValue<uint>());
        attribute->setType(Parameter::DataType::UnsignedInteger);
        break;
    case TBool:
        attribute->setMinValue(keyValue->minima<bool>());
        attribute->setMaxValue(keyValue->maxima<bool>());
        attribute->setDefaultValue(keyValue->defValue<bool>());
        attribute->setType(Parameter::DataType::Boolean);
        break;
    case TFloat:
        attribute->setMinValue(keyValue->minima<float>());
        attribute->setMaxValue(keyValue->maxima<float>());
        attribute->setDefaultValue(keyValue->defValue<float>());
        attribute->setType(Parameter::DataType::Float);
        break;
    case TDouble:
        attribute->setMinValue(keyValue->minima<double>());
        attribute->setMaxValue(keyValue->maxima<double>());
        attribute->setDefaultValue(keyValue->defValue<double>());
        attribute->setType(Parameter::DataType::Double);
        break;
    case TString:
        attribute->setMinValue(QString::fromStdString(keyValue->minima<std::string>()));
        attribute->setMaxValue(QString::fromStdString(keyValue->maxima<std::string>()));
        attribute->setDefaultValue(QString::fromStdString(keyValue->defValue<std::string>()));
        attribute->setType(Parameter::DataType::String);
        break;
    default:
        break;
    }
    attribute->setReadOnly(keyValue->isReadOnly());

    beginInsertRows({}, rowCount(), rowCount());
    m_attributes.push_back(attribute);
    endInsertRows();
}


}
}
