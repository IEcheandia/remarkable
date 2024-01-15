/**
 * Utility program to convert binary result files (*.result, as produced by Weldmaster Compact)
 * to csv files (compatible with the result files produced by Filtertest)
 * */

#include "resultsShortCSVSerializer.h"
#include "../src/resultsSerializer.h"

using precitec::storage::ResultsSerializer;
using precitec::interface::ResultArgs;

void printUsage()
{
    std::cout << "convertResultsToCSV [-i | -o OutputFolder] filenames\n";
    std::cout << "-i : inplace conversion, the .csv file is saved in the same folder as the corresponding .result file \n" ;
    std::cout << "-o outputFolder : the .csv file are saved in the provided output folder \n";
}

int main(int argc, char * argv[])
{
    
    bool padImgNumber=true;

    const auto extension = QString::fromUtf8(".csv");
    
    if (argc < 2)
    {
        std::cerr << "Filename not provided\n";
        printUsage();
        return EXIT_FAILURE;
    }
    
    std::vector<QString> filenames;
    QString outfolder;
    bool inPlace = false;
    int iCount =1;
    while (iCount < argc)
    {
        if (argv[iCount][0] == '-')
        {
            if (strlen(argv[iCount]) != 2)
            {
                std::cerr << "Missing parameter \n";
                printUsage();
                return EXIT_FAILURE;
            }
            
            switch( argv[iCount][1] )
            {
                case 'o': 
                {
                    iCount ++;
                    if (iCount >= argc)
                    {
                        std::cerr << "Please specify an output folder"<< std::endl;
                        printUsage();
                        return EXIT_FAILURE;
                    }
                    outfolder = QString::fromUtf8(argv[iCount]);
                    std::cout << outfolder.toStdString() << std::endl;
                };
                break;
                case 'i':
                    inPlace = true;
                    break;
                case 'h':
                    printUsage();
                    return EXIT_SUCCESS;
                    break;
            }
        }
        else        
        {
            //command line argument doesn't start with '-' : it's a filename
            filenames.emplace_back(QString::fromUtf8(argv[iCount]));
        }
        iCount ++;
    }
    
    if (filenames.size() == 0)
    {
        return EXIT_SUCCESS;
    }
    
    if (outfolder.isEmpty() && !inPlace)
    {
        std::cout << "Please set the output folder or set the in place option" << std::endl;
        printUsage();
        return EXIT_FAILURE;
    }
        
    if (!outfolder.isEmpty())
    {
        bool outDirCreated = QDir{outfolder}.mkpath(outfolder);
        if (!outDirCreated)
        {
            return EXIT_FAILURE;
        }
        std::cout << "Created output folder " << outfolder.toStdString() << std::endl;
    }

    std::cout << "Requested conversion of " << filenames.size() << " files" << std::endl;
    
    int numProcessed = 0;
    for (const auto & resultFile : filenames)
    {
        QFileInfo resultFileInfo{resultFile};
        bool ok=false;
        int result_enum = resultFileInfo.baseName().toInt(&ok);
        if (!ok)
        {
            std::cout << "Invalid filename """ << resultFile.toStdString() <<  """ " << std::endl;
            continue;
        }
        
        ResultsSerializer serializer;
        serializer.setDirectory(resultFileInfo.path());
        serializer.setFileName(resultFileInfo.fileName());
        
        std::vector<ResultArgs>  resultSets = serializer.deserialize<std::vector>();
        
        std::cout << "Read " << resultSets.size() << " resultsets " << std::endl;
       
        if (resultSets.size() == 0)
        {
            continue;
        }

        QFileInfo outputFile;
        if (!inPlace)
        {
            assert(!outfolder.isEmpty());
            outputFile = QFileInfo{(outfolder)+ QDir::separator() +( resultFileInfo.baseName() + extension)};
            std::cout << "Writing to " <<outputFile.filePath().toStdString()<< std::endl;
            
            if (!QDir(outfolder).mkpath(outputFile.path()))
            {
                std::cerr << "Could not make ouput folder " << std::endl;
                continue;
            }
        }
        else
        {
            outputFile = QFileInfo{(resultFileInfo.absolutePath())+ QDir::separator() +( resultFileInfo.baseName() + extension)};
            std::cout << "Writing to " <<outputFile.filePath().toStdString()<< std::endl;
            
        }
        
        QFile data(outputFile.filePath());
        
        if (!data.open(QFile::WriteOnly | QFile::Append)) 
        {
            std::cerr << "Could not open stream " << std::endl;
            continue;
        }
        
        QTextStream out(&data);
        
        bool validOrder= writeResultSet(out, resultSets, result_enum, padImgNumber);
        
        if (validOrder)
        {
            numProcessed++;
        }
        
        
    } //for filenames
    
   std::cout << "Converted files: " <<  numProcessed << std::endl;
   return EXIT_SUCCESS;
   
}
