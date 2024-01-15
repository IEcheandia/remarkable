#ifndef FILTERTESTPARAMETERS_H
#define FILTERTESTPARAMETERS_H

// stl includes
#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include "Poco/Environment.h"

struct FilterTestParameters
{
    std::string mXML_Filename;
    std::string mBmpPath;			
    std::string mFilter_Directory;
    std::string mXML_FilterPropFilename ;
    const int mProcessorCount = Poco::Environment::processorCount();
    int	mNbThreads = 1; // oProcessorCount;	
    int	mLoopSeconds = 5;
    std::string mResultFolder = "";
    float mCheckerboardSize = 0;
    std::string mCamGridImageFilename = "";
    bool mHasCanvas = true;
    std::string mCalibrationOverrideWM_BASE_DIR;
    std::string mOutputGprof = "";
    bool mRedirectLogMessages = false;
    bool mPrintAllTimings = false;
    int mNumberOfImages = 0;
    bool mArmAtSequenceRepetition = false;
    bool m_pauseAfterEachImage{false};
    
    void printUsage()
    {
        std::cout << "  command-line usage:" << std::endl;
        std::cout << "  filtertest [options] <xml-file>" << std::endl;
        std::cout << "  xml_file can also be specified by the FILTERTEST_GRAPH env variable" << std::endl;
        std::cout << "    options:" << std::endl;
        std::cout << "    -i <image path> (default: $FILTERTEST_BMPPATH )" << std::endl;
        std::cout << "    -n disable graphical output (default: enabled)" << std::endl;
        std::cout << "    -s <side of equivalent checkerboard to test calibrated coordinates (default: 0)" << std::endl;
        std::cout << "    -r <folder where results are written(default: no results written)" << std::endl;
        std::cout << "    -c <calibration_override_wm_dir  (default: $WM_BASE_DIR)" << std::endl;
        std::cout << "    -o <gprof output>" << std::endl;
        std::cout << "    -q quiet (do not print wmLog messages)" << std::endl;
        std::cout << "    -l log the processing time for all the filters" << std::endl;
        std::cout << "    --numImages <min number of images to play>" << std::endl;
        std::cout << "    -a arm at sequence repetition (sequence repeats if  numImages > images in folder)" << std::endl;
        std::cout << "    -b pause after each image, press space to continue" << std::endl;

    }
    
    bool parse(int argc, char * argv[])
    {
        if (argc < 2)
        {
            std::cout << "Filtertest: No command line arguments given, loading filtergraph from: " << mXML_Filename << std::endl;
            printUsage();
            return false;
        }
            
        // read parameters from environment variables, to use as default if no command line argument is present
        mBmpPath="";
        mXML_Filename = "";
        if (char * s = getenv("FILTERTEST_BMPPATH"))
        {
            mBmpPath = s;
            if ( mBmpPath.empty() == false && ( mBmpPath.back() == '\"')) // handle win case with path like "D:\VideoRepository\Pore Analysis\Audi\sub\"
            {
                mBmpPath =  std::string{ mBmpPath.begin(), mBmpPath.end() - 1 };
            }
        }
        if (char * s = getenv("FILTERTEST_GRAPH")) 
        {
            mXML_Filename = s;
        }
            
            
        if (char * s = getenv( "WM_BASE_DIR" ))  // if defined, it's used also as component path in XmlGraphBuilder
        {
            mCalibrationOverrideWM_BASE_DIR = s;
        }
        if ( mCalibrationOverrideWM_BASE_DIR.empty())
        {
            mCalibrationOverrideWM_BASE_DIR = ".";
        }
        
        int iCount = 1;
        bool valid = true;

        // loop through the command line options
        while( iCount < argc )
        {
            if (argv[iCount][0] == '-')
            {
                switch( argv[iCount][1] )
                {
                case 'i':
                    if (iCount+1 < argc)
                    {
                        mBmpPath = argv[iCount+1];
                        if ( mBmpPath.empty() == false && ( mBmpPath.back() == '\"')) // handle win case with path like "D:\VideoRepository\Pore Analysis\Audi\sub\"
                        {
                            mBmpPath    =   std::string{ mBmpPath.begin(), mBmpPath.end() - 1 };
                        }
                        iCount++;
                        std::cout << "Filtertest image directory: " << mBmpPath << std::endl;
                    } 
                    else 
                    {
                        std::cout << "Please specify a directory!" << std::endl;
                        valid = false;
                    }
                    break;
                case 'g':
                    if ( iCount + 1 < argc )
                    {
                        mCamGridImageFilename = argv[iCount + 1];

                        iCount++;
                        std::cout << "CamGridImageFilename: " << mCamGridImageFilename << std::endl;
                        std::ifstream f( mCamGridImageFilename.c_str());
                        if ( !(f.good()) )
                        {
                            std::cout << mCamGridImageFilename << " not accessible " << std::endl;
                            valid = false;
                        }
                    }
                    
                    break;
                case 'n':
                    mHasCanvas = false;
                    std::cout << "Graphical output disabled." << std::endl;
                    break;

                case 'p':
                    if (iCount+1 < argc)
                    {
                        mNbThreads = std::atoi(argv[iCount+1]);
                        iCount++;
                        std::cout << "Number of parallel threads not supported" << std::endl;
                    } 
                    else 
                    {
                        std::cout << "Please specify a number!" << std::endl;
                        valid = false;
                    }
                    break;

                case 't':
                    if (iCount + 1 < argc)
                    {
                        mLoopSeconds = std::atoi(argv[iCount + 1]) ;
                        iCount++;
                        std::cout << "Loop time not supported" << std::endl;
                    }
                    else
                    {
                        std::cout << "Please specify a number!" << std::endl;
                        valid = false;
                    }
                    break;

                case 's':
                    if ( iCount + 1 < argc )
                    {
                        mCheckerboardSize = std::atof(argv[iCount + 1]);
                        iCount++;
                        std::cout << "Checkerboard size : " << mCheckerboardSize << " mm" << std::endl;
                    }
                    else
                    {
                        std::cout << "Please specify a number!" << std::endl;
                        valid = false;
                    }
                    break;
                case 'r':
                    if (iCount + 1 < argc)
                    {
                        mResultFolder = argv[iCount+1];
                        std::cout << "Result folder " << mResultFolder << std::endl;
                    }
                    else
                    {
                        std::cout << "Please specify a directory!" << std::endl;
                        valid = false;
                    }
                    iCount++;
                    break;
                case 'c':
                    if (iCount+1 < argc)
                    {
                        mCalibrationOverrideWM_BASE_DIR = argv[iCount+1];
                        if ( mCalibrationOverrideWM_BASE_DIR.empty() == false && ( mCalibrationOverrideWM_BASE_DIR.back() == '\"')) // handle win case with path like "D:\VideoRepository\Pore Analysis\Audi\sub\"
                        {
                            mCalibrationOverrideWM_BASE_DIR    =   std::string{ mCalibrationOverrideWM_BASE_DIR.begin(), mCalibrationOverrideWM_BASE_DIR.end() - 1 };
                        }
                        iCount++;
                        std::cout << "calibration_override_wm_dir: " << mCalibrationOverrideWM_BASE_DIR << std::endl;
                        
                    } 
                    else 
                    {
                        std::cout << "Please specify a directory for calibration configuration!" << std::endl;
                        valid = false;
                    }
                    break;   
                case 'o':
                    if (iCount + 1 < argc)
                    {
                        mOutputGprof = argv[iCount+1];
                        std::cout << "Profiler output " << mOutputGprof << std::endl;
                    }
                    else
                    {
                        std::cout << "Please specify a filtename for the profiler output!" << std::endl;
                        valid = false;
                    }
                    iCount++;
                    break;
                case 'q':
                    mRedirectLogMessages = true;
                    std::cout << "Redirect log messages " << std::endl;
                    break;
                case 'l':
                    mPrintAllTimings = true;
                    std::cout << "Log the processing time for all the filters" << std::endl;
                    break;
                case 'a':
                    mArmAtSequenceRepetition = true;
                    std::cout << " arm at sequence repetion " << std::endl;
                    break;
                case 'b':
                    m_pauseAfterEachImage = true;
                    std::cout << "Pause after each image" << std::endl;
                    break;
                case '-':
                    if (strcmp(argv[iCount], "--numImages") == 0)
                    {
                        if (iCount + 1 < argc)
                        {
                            mNumberOfImages = atoi(argv[iCount+1]);
                            if (mNumberOfImages > 0)
                            {
                                std::cout << "NumberOfImages " << mNumberOfImages << std::endl;
                            }
                        }
                        else
                        {
                            std::cout << "Please specify a minimum number of image!" << std::endl;
                            valid = false;
                        }
                        iCount++;
                        break;
                    }
                } // end switch
                // default: omitted
            } 
            else 
            {

                mXML_Filename = argv[iCount];
            }

            iCount++;
        }
        
        if ( mXML_Filename.empty())
        {
            if (mCheckerboardSize == 0)
            {
                std::cout << "No graph provided" << std::endl;
                valid = false;
            }
            else
            {
                std::cout << "No graph specified, processing only calibration file" << std::endl;
                valid = true;
            }
        }
        else
        {
            std::cout << "Filtertest xml file: " << mXML_Filename << std::endl;
        }
        
        return valid;

    }
    
};


#endif
