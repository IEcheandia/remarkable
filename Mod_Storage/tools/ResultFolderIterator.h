#pragma once

#include <QFile>
#include <QDebug>

#include "../src/resultsLoader.h"
#include "../src/resultsSerializer.h"


using precitec::storage::ResultsLoader;
using precitec::storage::ResultsSerializer;

using precitec::interface::ResultArgs;
using precitec::interface::ResultType;

//helper class that provides a const iterator to a QFileInfoList
class FileInfoListIterator
{
    public:
        explicit FileInfoListIterator( QFileInfoList fileinfolist) 
            : m_FileInfoList(fileinfolist),
            it(m_FileInfoList.cbegin())
            {
                
               assert( m_FileInfoList.size() == 0 || !atEnd());
            }
        
        FileInfoListIterator(): 
            FileInfoListIterator(QFileInfoList{})
        {
            assert(atEnd());
        };
        
        
        bool atEnd() const 
        {
            return it >= m_FileInfoList.cend();
        }
        
        void next()
        {
            ++it;
        }
        
        QFileInfo fileInfo() const
        {
            assert(!atEnd());
            return (*it);
        }
        
        int size() const
        {
            return m_FileInfoList.size();
        }
            
    private:
        QFileInfoList m_FileInfoList;
        QFileInfoList::const_iterator it;
};


//Read results with the corresponding seam series and seam number, does not rely on metadata.json
class ResultFolderIterator
{
public:
    static FileInfoListIterator getSeamSeriesInfo(QString prodFolderName)
    {
        QFileInfo prodFolderInfo{prodFolderName};
        QDir prodDir{prodFolderInfo.absoluteFilePath()};
        prodDir.setNameFilters({QStringLiteral("seam_series*")});
        prodDir.setFilter(QDir::Dirs);
        QFileInfoList validFolders;
        for (const auto & info: prodDir.entryInfoList())
        {
            bool validName = false;
            info.fileName().right(4).toInt(&validName);
            if (validName)
            {
                validFolders.push_back(info);
            }
        }
        return FileInfoListIterator{validFolders};
    }
    
    
    static FileInfoListIterator getSeamInfo(QString seamSeriesPath)
    {
         QDir seamSeriesDir{seamSeriesPath};
         seamSeriesDir.setNameFilters({QStringLiteral("seam*")});
         seamSeriesDir.setFilter(QDir::Dirs);
         QFileInfoList validFolders;
         for (const auto & info: seamSeriesDir.entryInfoList())
        {
            bool validName = false;
            info.fileName().right(4).toInt(&validName);
            if (validName)
            {
                validFolders.push_back(info);
            }
            else
                assert(false);
        }
        return FileInfoListIterator{validFolders};
    }
    
    static FileInfoListIterator getResultInfo(QString seamPath)
    {
         QDir seamDir{seamPath};
         seamDir.setNameFilters({QStringLiteral("*.result")});
         seamDir.setFilter(QDir::Files);
         return FileInfoListIterator{seamDir.entryInfoList()};
    }
    
    ResultFolderIterator(QString pInstanceFolder)
    : m_folder(pInstanceFolder)
    {
        m_oSeamSeriesEntryInfoList = getSeamSeriesInfo(m_folder);
        updateSeriesNumber();
        if (m_oSeamSeriesEntryInfoList.atEnd())
        {
            resetSeamEntryInfoList();
            assert(m_oResultEntryInfoList.atEnd());
            m_endReached = true;
            return;
        }
        m_endReached = false;
        resetSeamEntryInfoList(m_oSeamSeriesEntryInfoList.fileInfo().absoluteFilePath());
    }
    
 
    
    bool advanceUntil( std::function<void(ResultFolderIterator*)> onMissingItemCallback, int p_series, int p_seam = -1, int p_result=-1)
    {
        assert(atEnd() || seamSeries() > -1);
        while ( seamSeries() < p_series && !atEnd())
        {
            onMissingItemCallback(this);
            nextSeamSeries();
        }
        
        if (seamSeries() != p_series)
        {
            assert(seamSeries() > p_series || m_oSeamSeriesEntryInfoList.atEnd());
            return false;
        }
        
        if (p_seam == -1)
        {
            return true;
        }
        
        while(seam() < p_seam && nextSeam())
        {
            onMissingItemCallback(this);
            assert(seamSeries() == p_series);
        }
        
        assert(seamSeries() == p_series);
        if (seam() != p_seam)
        {
            assert ( seam() > p_seam || m_oSeamEntryInfoList.atEnd());   //but it's not the global end
            return false;
        }
        
        assert(seamSeries() == p_series &&  seam() == p_seam );
        
        if (p_result == -1)
        {
            return true;
        }
        
        while(result() < p_result && nextResult())
        {
            onMissingItemCallback(this);
            assert(seamSeries() == p_series &&  seam() == p_seam );
        } 
        
        if (result() != p_result)
        {
            assert(m_oResultEntryInfoList.atEnd());
            return false;
        }
        
        assert(seamSeries() == p_series &&  seam() == p_seam  &&  result() == p_result);
        return true;
    }

    
    bool atEnd() const 
    {
        return m_endReached;
    }
    
    QString productFolder() const 
    {
        return m_folder;
    }
    
    QString resultFilename() const {
    if (m_oResultEntryInfoList.size() == 0)
    {
        return  {};
    }
    assert(!m_endReached);
    return m_oResultEntryInfoList.fileInfo().absoluteFilePath();
    }
    
    std::vector<ResultArgs> resultSet() const 
    {
        assert(!m_endReached);
        ResultsSerializer serializer;
        //std::cout << "Read results from " << m_oResultEntryInfoList.fileInfo().fileName().toStdString() 
                // <<  " " << "[" << seamSeries() << "," << seam() << "] R:" << result() << std::endl;
        if (m_oResultEntryInfoList.size() == 0)
        {
            std::cout << "Empty results dir " <<  m_oSeamEntryInfoList.fileInfo().absoluteFilePath().toStdString() << std::endl;
            return {};
        }
        serializer.setDirectory(m_oResultEntryInfoList.fileInfo().absolutePath());
        serializer.setFileName(m_oResultEntryInfoList.fileInfo().fileName());   
        return  serializer.deserialize<std::vector>();
    }
    
    int seamSeries() const 
    {
        return m_series;
    }
    int seam() const 
    {
        return m_seam;
    }
    int result() const 
    {
        return m_result; 
    }
    
    bool next()
    {    
        //before accessing the result, check if we are not at end
        if (m_endReached)
        {
            assert(m_oResultEntryInfoList.atEnd());
            return false;
        }
        
        if (!m_oResultEntryInfoList.atEnd())
        {
            nextResult();
            if (!m_oResultEntryInfoList.atEnd())
            {
                return true;
            }
        }
        
        if (!m_oSeamEntryInfoList.atEnd())
        {
            nextSeam();
            if (!m_oResultEntryInfoList.atEnd() )
            {
                return true;
            }
        }
        
        if (!m_oSeamSeriesEntryInfoList.atEnd())
        {
            nextSeamSeries();
            if (!m_oResultEntryInfoList.atEnd())
            {
                return true;
            }
            else
            {
                m_endReached = true;
                assert(m_oResultEntryInfoList.atEnd());
                return false;
            }
        }
        return false;
    }
    
    void  advanceUntil(int p_series, int p_seam, int p_result)
    {
        assert(atEnd() || seamSeries() > -1);
        while ( seamSeries() != p_series && seam() != p_seam  && result() != p_result &&  !atEnd())
        {
            std::cout << ". " << std::endl;
            next();
        }
    }

private:
    
    void resetResultEntryInfoList()
    {
        m_oResultEntryInfoList = FileInfoListIterator{ {} } ;
    }

    
    void resetResultEntryInfoList(QString path)
    {
        m_oResultEntryInfoList = getResultInfo(path);
        updateResultNumber();
        if ( m_result == -1 && !m_oResultEntryInfoList.atEnd())
        {
            nextResult();
        }
    }

    void resetSeamEntryInfoList()
    {
        m_oSeamEntryInfoList = FileInfoListIterator{ {}};
        resetResultEntryInfoList();
    }
    
    void resetSeamEntryInfoList(QString path)
    {
        m_oSeamEntryInfoList = getSeamInfo(path);
        if (m_oSeamEntryInfoList.atEnd())
        {
            resetResultEntryInfoList();
        }
        updateSeamNumber();
        resetResultEntryInfoList(m_oSeamEntryInfoList.fileInfo().absoluteFilePath());
    }
    
    bool nextSeamSeries()
    {
        m_oSeamSeriesEntryInfoList.next();
        
        updateSeriesNumber();
        
        //reset following  iterators
        if (m_oSeamSeriesEntryInfoList.atEnd())
        {
            resetSeamEntryInfoList();
            return false;
        }
        
        assert(m_series != -1 && "folder names not filtered");
    
        resetSeamEntryInfoList(m_oSeamSeriesEntryInfoList.fileInfo().absoluteFilePath());
        return true;
        
    }
    
    bool nextSeam()
    {
        m_oSeamEntryInfoList.next();
        
        if (m_oSeamEntryInfoList.atEnd())
        {
            resetResultEntryInfoList();
            return false;
        }
        updateSeamNumber();
        resetResultEntryInfoList(m_oSeamEntryInfoList.fileInfo().absoluteFilePath());
        return true;
        
    }

    
        
    void updateSeriesNumber()
    {
        bool validName = false;
        if (!m_oSeamSeriesEntryInfoList.atEnd())
        {
            m_series = m_oSeamSeriesEntryInfoList .fileInfo().fileName().right(4).toInt(&validName);
            assert(validName && "getInfoList returned an invalid folder"); 
        }
        if (!validName)
        {
            m_series = -1;
        }
    }
    
    
    
    void updateSeamNumber()
    {
        bool validName = false;
        if (!m_oSeamEntryInfoList.atEnd())
        {
            m_seam = m_oSeamEntryInfoList.fileInfo().fileName().right(4).toInt(&validName);
            assert(validName && "getInfoList returned an invalid folder"); 
        }
        if (!validName)
        {
            m_seam = -1;
        }

    }
    
    void updateResultNumber() 
    {
        bool ok = false;
         if (!m_oResultEntryInfoList.atEnd())
        {
            m_result = m_oResultEntryInfoList.fileInfo().baseName().toInt(&ok);
            if (!ok)
            {
                std::cerr << "Invalid result filename " << m_oResultEntryInfoList.fileInfo().absoluteFilePath().toStdString() << std::endl;
            }
        }
        if (!ok)
        {
            m_result = -1;
        }
    }
    
    bool nextResult()
    {
        m_oResultEntryInfoList.next();
        updateResultNumber();
        if ( m_result == -1 && !m_oResultEntryInfoList.atEnd())
        {
            return nextResult();
        }
        return m_result != -1;
    }
   
    FileInfoListIterator m_oSeamSeriesEntryInfoList;
    FileInfoListIterator m_oSeamEntryInfoList;
    FileInfoListIterator m_oResultEntryInfoList;
    
    bool m_endReached = false;
    
    int m_series = -1; 
    int m_seam = -1; 
    int m_result = -1;
    QFileInfo lastResultFileInfo;
    QString m_folder;
};

std::ostream& operator<< (std::ostream& stream, const ResultFolderIterator& r)  
{
    stream << "[" << std::setw(2) << r.seamSeries() << "," << std::setw(2) << r.seam() << "] R:" << std::setw(6) <<  r.result() << " " ;
    return stream;
};




void  testResultIterator(QString prodFolderName)
{    
        std::cout << "Read Product Instance folder " << prodFolderName.toStdString() << std::endl;

        QFileInfo prodFolderInfo{prodFolderName};

        ResultFolderIterator testFileIterator{prodFolderName};
        QDir prodDir{prodFolderInfo.absoluteFilePath()};
        prodDir.setNameFilters({QStringLiteral("seam_series*")});
        prodDir.setFilter(QDir::Dirs);
        const auto seamSeriesInfos = prodDir.entryInfoList();
        for (const auto &seamSeriesInfo : seamSeriesInfos)
        {
            
            bool ok = false;
            const auto seamSeriesNumber = seamSeriesInfo.fileName().right(4).toInt(&ok);
            std::cout << " Seam Series " << seamSeriesNumber << std::endl;
            if (!ok)
            {
                continue;
            }
            QDir seamSeriesDir{seamSeriesInfo.absoluteFilePath()};
            seamSeriesDir.setNameFilters({QStringLiteral("seam*")});
            seamSeriesDir.setFilter(QDir::Dirs);
            const auto seamInfos = seamSeriesDir.entryInfoList();
            for (const auto &seamInfo : seamInfos)
            {
                bool ok = false;
                const auto seamNumber = seamInfo.fileName().right(4).toInt(&ok);
                if (!ok)
                {
                    continue;
                }
                std::cout << "\t" << " Seam  " << seamNumber << std::endl;
                
                QDir seamDir{seamInfo.absoluteFilePath()};
                seamDir.setNameFilters({QStringLiteral("*.result")});
                seamDir.setFilter(QDir::Files);
                //std::cout << seamSeriesNumber << " " << seamNumber 
                std::cout << "\t\t";
                int seamFirstImage = -1;
                int seamLastImage = -1;
                for (const auto & resultInfo : seamDir.entryInfoList())
                {
                    bool ok = false;
                    const auto resultNumber = resultInfo.baseName().toInt(&ok);

                    std::cout << resultNumber;
                    ResultsSerializer serializer;
                    serializer.setDirectory(resultInfo.path());
                    serializer.setFileName(resultInfo.fileName());
                    std::vector<ResultArgs>  results = serializer.deserialize<std::vector>();
                    int num_nio= 0;
                    int num_valid = 0;
                    
                    int firstImageNumber=-1;
                    int lastImageNumber=-1;
                    for (const auto & resultSet: results)
                    {
                        if (resultSet.isValid())
                        {
                            num_valid++;
                        }
                        if (resultSet.isNio())
                        {
                            num_nio++;
                        }
                        
                        assert(resultSet.resultType() == resultNumber);
                        if (firstImageNumber == -1)
                        {
                            firstImageNumber = resultSet.context().imageNumber();
                            if (seamFirstImage == -1)
                            {
                                seamFirstImage = firstImageNumber;
                            }
                        }
                        lastImageNumber = resultSet.context().imageNumber();
                    }
                    
                    if (firstImageNumber < seamFirstImage)
                    {
                        seamFirstImage = firstImageNumber;
                    }
                    if (lastImageNumber > seamLastImage)
                    {
                        seamLastImage = lastImageNumber;
                    }

                    //int cntNio = std::count_if(results.begin(), results.end(), isNio);
                    std::cout  << "["<< num_nio<<"/"<< num_valid << "/"<< results.size()<< "@"<<firstImageNumber <<"-" << lastImageNumber<< "]; ";
                    
                            
                    std::cout << "\t " << testFileIterator.resultFilename().toStdString() << std::endl;
                    std::cout << resultInfo.absoluteFilePath().toStdString()  << " " << testFileIterator.resultFilename().toStdString() << std::endl;
                    assert(resultInfo.absoluteFilePath() == testFileIterator.resultFilename());
                    std::cout << "File iterator " << testFileIterator.seamSeries() << " " << testFileIterator.seam()  << " " << testFileIterator.result() << std::endl;
                    assert(seamNumber == testFileIterator.seam());
                    assert(seamSeriesNumber == testFileIterator.seamSeries());
                    assert(resultNumber == testFileIterator.result());
                    testFileIterator.next();
                }
                std::cout << std::endl;
                

            }
        }
    
        assert(testFileIterator.atEnd());

}



