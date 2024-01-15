/**
 * Serialize some fields of ResultArgs (only resultType and resultValue) to CSV file
 */

#pragma once

// stl includes
#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include <QTextStream>
#include "event/results.h"



void writeHeader(QTextStream &out) 
{
    out << "imageNumber;position;resultType;resultSize;resultValue" << "\n";
}

void writeEmptyResult(QTextStream &out, int imgNumber) 
{
    out << imgNumber<< "; ; ; ;" << "\n";
}

QTextStream &operator<<(QTextStream &out, const precitec::interface::ResultArgs &result)
{
    out << result.context().imageNumber() << ";"
        << result.context().position() << ";"
        << result.resultType() << ";"
        << result.value<double>().size() << ";";;
    
    for (const auto & v : result.value<double>())
    {
        out << v << ";";
    }
    out << "\n";   
    
    return out;
}

bool writeResultSet(QTextStream &out, const std::vector<precitec::interface::ResultArgs> &resultSets,
    int expectedResultEnum, bool padImgNumber)
{
        writeHeader(out);
        
        bool validOrder = true;
        int imgNumber = -1;
        
        for (const auto & result : resultSets)
        {
            assert(result.resultType() == expectedResultEnum);
            if (result.context().imageNumber() <= imgNumber)
            {
                validOrder = false;
                break;
            }
            if (padImgNumber)
            {
                ++imgNumber;
                for( ; imgNumber < result.context().imageNumber(); ++imgNumber)
                {
                    writeEmptyResult(out, imgNumber);
                }
            }
            else
            {
                imgNumber = result.context().imageNumber();
            }
            
            out << result;
            
        }// for each resultset  
    
    return validOrder;
}
