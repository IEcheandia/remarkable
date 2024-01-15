/**
 * Utility program compare binary result files (*.result, as produced by Weldmaster Compact)
 * 
 * */

// stl includes
#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>


#include "common/graph.h"
#include <sstream>

#include "ResultFolderIterator.h"


using precitec::storage::ResultsSerializer;
using precitec::interface::ResultArgs;


enum ComparisonResult
{
    eEqual,
    eDifferentResultEnum,
    eDifferentImageNumber,
    eDifferentResultArgsValidity,
    eDifferentNIOFlag,
    eUnexpectedResultType,
    eDifferentNumberOfElements, //for results of type vector
    eValueDifferenceBeyondTolerance
};


//Compares two ResultArgs, exits as soon as they are different
ComparisonResult compareResults(const ResultArgs & r1, const ResultArgs & r2 )
{
    if (r1.resultType() != r2.resultType())
    {
        std::cout << "Comparing different results " <<r1.resultType() << " "  << r2.resultType() << std::endl;
        return eDifferentResultEnum;
    }
    
    const auto imageNr = r1.context().imageNumber();
    if (imageNr != r2.context().imageNumber())
    {
        std::cout <<  " Different image number " << imageNr << " " << r2.context().imageNumber() << std::endl;
        return eDifferentResultArgsValidity;
    }
    
    if (r1.isValid() != r2.isValid())
    {
        return eDifferentResultArgsValidity;
    }
    if (r1.isNio() != r2.isNio())
    {
        return eDifferentNIOFlag;
    }
    if (r1.isNio() || !r1.isValid())
    {
        return eEqual;
    }
    
    if (r1.type() != precitec::interface::RegTypes::RegDoubleArray  || r1.type() != r2.type())
    {
        std::cout << "Unexpected result type " << std::endl;
        return eUnexpectedResultType;
    }
    
    auto vec1 = r1.value<double>();
    auto vec2 = r2.value<double>();
    auto  num_elements = vec1.size();
    if (num_elements != vec2.size())
    {
        std::cout << "Different number of elements (num lines) " << vec1.size() << " " << vec2.size()  << std::endl;
        return eDifferentNumberOfElements;
    }
    
    for (unsigned int i = 0; i < num_elements; i++)
    {
        double d1 = vec1[i];
        double d2 = vec2[i];
        double abs_diff = std::abs(d1-d2);
        
        //r1 taken as reference
        if ( abs_diff > 1e-08 + 1e-19 * std::abs(d1))
        {
            std::cout << "Different values at image " << std::setw(3) << imageNr << " (" << i << "):" << d1 << " " << d2 << std::endl;
            return eValueDifferenceBeyondTolerance;
        }
    }
    return eEqual;
    
}

//Compares two vector of results (same resultype, all frames)
bool compareResultSet(const std::vector<ResultArgs> & resultVec1, const std::vector<ResultArgs> & resultVec2, bool breakOnError )
{        
    if (resultVec1.size() == 0 && resultVec2.size() == 0)
    {
        std::cout << "Empty results sets " << std::endl;
        return true;
    }
    
    if (resultVec1.size() == 0 || resultVec2.size() == 0)
    {
        std::cout << "One result set is empty " << std::endl;
        return false;
    }
    
    auto it1 = resultVec1.cbegin();
    auto it2 = resultVec2.cbegin();

    int resultEnum =  it1->resultType();   
        
    if (resultVec1.size() != resultVec2.size() )
    {

        std::cout << "Result " << resultEnum << " different number of elements (images)" << resultVec1.size() << " " << resultVec2.size() <<  std::endl;
        return false;
    }

    bool matchingResultSet = true;
    unsigned int cnt = 0;

    for (   ; 
         it1 != resultVec1.cend() && (!breakOnError || matchingResultSet);
        ++it1, ++it2)
    {
        ++cnt;
        
        assert((it2 != resultVec2.cend()) && "Result sets do not have same number of elements");
        
        auto res = compareResults(*it1, *it2);
        assert(res != eUnexpectedResultType);
        matchingResultSet &= (res == eEqual);
    }
    
    assert(!matchingResultSet || cnt == resultVec1.size());
    
    return matchingResultSet;
    
}


void printUsage()
{
    std::cout << "compareResults [-r] [-v] Folder1 Folder2 \n";
    std::cout << "Options: ";
    std::cout << "-r : recursive Folder1 and Folder2 are product results folders, all the results in the corresponding seamseries and seams will be compared \n";
    std::cout << "-v : verbose";
}

int main(int argc, char * argv[])
{
    
    QString path1;
    QString path2;
    bool productFolderSearch = false;
    bool verbose = false;
    QString options;
    
    
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            const char * opt = argv[i];
            const char * const p_end = opt + strlen(opt);
            ++opt;
            for ( ; opt != p_end; ++opt)
            {
                switch(*opt)
                {
                    case 'r':
                        productFolderSearch = true; 
                        break;
                    case 'v':
                        verbose = true; 
                        break;
                    default:
                        std::cout << "unknown option " << *opt << std::endl; 
                        break;
                }                
            }
            continue;
        }
        
        if (path1.isEmpty())
        {
            path1 = QString::fromUtf8(argv[i]);
            continue;
        }
        
        if (path2.isEmpty())
        {
            path2 = QString::fromUtf8(argv[i]);
            continue;
        }
        
        std::cerr << "Too many input arguments \n";
        printUsage();
        return EXIT_FAILURE;
    }
    
        
    if (path1.isEmpty() || path2.isEmpty())
    {
        std::cerr << "Filenames not provided\n";
        printUsage();
        return EXIT_FAILURE;
    }
    
    
    bool breakOnError = !verbose;
    bool matchingError = false;

    if (productFolderSearch)
    {
        ResultFolderIterator itResultFolder1{ path1};
        ResultFolderIterator itResultFolder2{ path2};
        
        if (verbose)
        {
            std::cout << "Start; Folder 1 " << itResultFolder1.productFolder().toStdString() 
                << " Folder 2 " << itResultFolder2.productFolder().toStdString() << std::endl;
        }


        std::string msg;
        int level = 0;
        
        
        auto fOnMissing = [&msg, &matchingError, &level](ResultFolderIterator* it)
        {
            std::cout <<msg << (*it).seamSeries() ;
            if (level >= 1)
            {
                std::cout << " " << (*it).seam();
            };
            if (level >= 2)
            {
                std::cout << " " << (*it).result();
            };
            std::cout << std::endl;
            matchingError = true;
        };
            
        
        
        bool haveNext = true;
        while (haveNext)
        {
            msg = "missing";
            if (verbose)
            {
                // std::cout << "Iterating " <<  itResultFolder1 << " " << itResultFolder2 << std::endl;
            }

            bool found = itResultFolder1.advanceUntil(fOnMissing, itResultFolder2.seamSeries(), itResultFolder2.seam(), itResultFolder2.result());
            found &= itResultFolder2.advanceUntil(fOnMissing, itResultFolder1.seamSeries(), itResultFolder1.seam(), itResultFolder1.result());

            assert((!itResultFolder1.atEnd() && !itResultFolder2.atEnd()) || !found) ;
            
            if (!found)
            {
                //ok = false;
                std::cout << "Could not found common series - current iterator state: " << itResultFolder1 << " " << itResultFolder2 << std::endl;
                matchingError = true;
                if (itResultFolder1.atEnd() || itResultFolder2.atEnd())
                {
                    haveNext = false;
                    break;
                }
                itResultFolder1.next();
                itResultFolder2.next();
                continue;
            }
            
            
            if (verbose)
            {
                //std::cout << " after advancing " <<  itResultFolder1 << " " << itResultFolder2 << std::endl;
            }
            
            assert (itResultFolder1.seam() == itResultFolder2.seam() 
                && itResultFolder1.seamSeries() == itResultFolder2.seamSeries() 
                && itResultFolder1.result() == itResultFolder2.result()
                && "ResultFolderIterator advanceUntil failed"
                );
            
            if (verbose)
            {
                std::cout << "Found common result subfolder: " << itResultFolder1 << "  " << itResultFolder2 << std::endl;
            }

            //vector of results (same resultype, all frames)
            auto resultSet1 = itResultFolder1.resultSet();
            auto resultSet2 = itResultFolder2.resultSet();

            bool matchingResultSet = compareResultSet(resultSet1, resultSet2, breakOnError);
            
            if (!matchingResultSet)
            {
                matchingError =true;
                std::cout  << "Results " <<  itResultFolder1 << "  " << itResultFolder2 << " do not match " << std::endl;
            }
            if (breakOnError && matchingError)
            {
                break;
            }
            
            haveNext &= itResultFolder1.next();
            haveNext &=itResultFolder2.next();
            
        }
    }
    else
    {
        ResultsSerializer serializer;
        QFileInfo fileInfo1{path1};
        serializer.setDirectory(fileInfo1.absolutePath());
        serializer.setFileName(fileInfo1.fileName());   
        auto resultSet1 = serializer.deserialize<std::vector>();

        ResultsSerializer serializer2;
        QFileInfo fileInfo2{path2};
        serializer2.setDirectory(fileInfo2.absolutePath());
        serializer2.setFileName(fileInfo2.fileName());
        auto resultSet2 = serializer2.deserialize<std::vector>();
        
        
        bool matchingResultSet = compareResultSet(resultSet1, resultSet2, breakOnError);
        if (!matchingResultSet)
        {
            std::cout  << "Results  do not match " << std::endl;
            matchingError =true;
        }
        
    }
    
    
    std::cout << "Matching  error: " << matchingError << std::endl;
    if (matchingError)
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;

}
