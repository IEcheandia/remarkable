// stl includes
#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include <QIODevice>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "../src/product.h"

#include "common/graph.h"

using precitec::storage::Product;

int main(int argc, char * argv[])
{
    if (argc < 2)
    {
        std::cerr << "Filename not provided\n";
        return EXIT_FAILURE;
    }
    QString fileName = QString::fromUtf8(argv[1]);
    std::unique_ptr<Product> product{Product::fromJson(fileName)};
    if (!product)
    {
        std::cerr << "Error when parsing product " << fileName.toStdString() << "\n";
        //try to get the parsing exception - reimplement Product::fromJson(const QString &path, QObject *parent)
        QFile file(fileName);
    
        if (!file.open(QIODevice::ReadOnly))
        {
            std::cerr << "Could not open file\n";
            return EXIT_FAILURE;
        }
        const QByteArray data = file.readAll();
        if (data.isEmpty())
        {
            std::cerr << "Empty file\n";
            return EXIT_FAILURE;
        }
        QJsonParseError error;
        
        const auto document = QJsonDocument::fromJson(data, &error);
        if (document.isNull())
        {
            
            std::cerr << error.errorString().toStdString() << "[ offset " << error.offset << "]\n";
            int start = error.offset - 100;
            start = start < 0 ? 0: start;
            std::cout << start << ": " << data.mid(start, error.offset - start).toStdString() << "\n";
            std::cout << error.offset << ": " << data.mid(error.offset,100).toStdString() << "\n";
            return EXIT_FAILURE;
        }
        //auto p =Product::fromJson(document.object(), parent);
        return EXIT_FAILURE;
    }
    std::cout << "Product " << product->name().toStdString()  << " succesfully read from " << fileName.toStdString()<< std::endl;
    return EXIT_SUCCESS;
    
}
