/** @defgroup VI_InspectionControl VI_InspectionControl
 */

/*
 * ProxyInspectionCmd.cpp
 *
 *  Created on: 18.05.2010
 *      Author: f.agrawal
 */

#include "AppMain.h"
#include "AutoRunner.h"

#include "Poco/SortedDirectoryIterator.h"
#include "Poco/RegularExpression.h"

#include "common/systemConfiguration.h"
#include "event/inspectionToS6k.proxy.h"

namespace precitec
{

namespace viInspectionControl
{

AppMain::AppMain():
	BaseModule(system::module::VIInspectionControl),		// wir brauchen immer die moduleId!!
	inspectProxy_(),
	inspectCmdProxy_(),
	m_dbNotificationServer(m_oDbProxy),
	m_dbNotificationHandler(&m_dbNotificationServer),
	m_control(inspectProxy_, inspectCmdProxy_, m_oSensorProxy, m_oEthercatOutputsProxy, m_oS6K_InfoToProcessesProxy, m_oDbProxy, &m_dbNotificationServer),
	m_oInspectionOutHandler(&m_control),
    m_oS6K_InfoFromProcessesServer( m_control ),
    m_oS6K_InfoFromProcessesHandler( &m_oS6K_InfoFromProcessesServer ),
	m_oTriggerCmdServer( m_control ),
	m_oTriggerCmdHandler( &m_oTriggerCmdServer ),
	m_oEthercatInputsServer( m_control ),
	m_oEthercatInputsHandler( &m_oEthercatInputsServer ),
    m_oControlSimulationServer( m_control ),
    m_oControlSimulationHandler( &m_oControlSimulationServer ),
	m_oResultsServer( m_control),
	m_oResultsHandler(&m_oResultsServer),
    m_resultsProxy(new TResults<EventProxy>{}),
	m_oDeviceServer( m_control ),
	m_oDeviceHandler( &m_oDeviceServer )	
    
{
    system::raiseRtPrioLimit();

    if ((SystemConfiguration::instance().getBool("SOUVIS6000_Application", false)) ||
        (SystemConfiguration::instance().getBool("SCANMASTER_Application", false)) ||
        (SystemConfiguration::instance().getBool("Communication_To_LWM_Device_Enable", false)))
    {
        m_control.StartCyclicTaskThread();
    }

	registerPublication(&inspectProxy_);
	registerPublication(&inspectCmdProxy_);
	registerPublication( &m_oSensorProxy);
	registerPublication(&m_oDbProxy);
    registerPublication(m_resultsProxy.get());

    if (m_control.isSOUVIS6000_Application())
    {
        m_inspectionToS6k.reset(new TInspectionToS6k<EventProxy>{});
        m_control.setInspectionToS6k(m_inspectionToS6k);
        registerPublication(m_inspectionToS6k.get());
    }

    m_oInspectionOutHandler.setRealTimePriority(system::Priority::InspectionControl);
    m_oTriggerCmdHandler.setRealTimePriority(system::Priority::InspectionControl);
    m_oEthercatInputsHandler.setRealTimePriority(system::Priority::EtherCATDependencies);
    m_oResultsHandler.setRealTimePriority(system::Priority::Results);
    m_oS6K_InfoFromProcessesHandler.setRealTimePriority(system::Priority::InspectionControl);

	registerSubscription(&m_oInspectionOutHandler);
	registerSubscription(&m_oTriggerCmdHandler);
	registerSubscription(&m_oEthercatInputsHandler);
    registerSubscription(&m_oControlSimulationHandler);
	registerSubscription(&m_oResultsHandler);

	registerPublication(&m_oEthercatOutputsProxy);
    registerPublication(&m_oS6K_InfoToProcessesProxy);
    registerSubscription(&m_oS6K_InfoFromProcessesHandler);

    registerSubscription(&m_oDeviceHandler, system::module::VIInspectionControl);
    
    registerSubscription(&m_dbNotificationHandler);

	initialize(this);

    m_control.setResultsProxy(m_resultsProxy);

	ConnectionConfiguration::instance().setInt( pidKeys[INSPECTIONCONTROL_KEY_INDEX], getpid() ); // let ConnectServer know our pid
}

AppMain::~AppMain()
{
}

int AppMain::init(int argc, char * argv[])
{
	processCommandLineArguments(argc, argv);
	m_simulateHW = false;

	for (int i = 0; i < argc; ++i) {
		//std::printf("[%d]: %s\n",i,argv[i]);
		if( strstr(argv[i],"nohw") != NULL ){

			m_simulateHW = true;
			break;
		}

	}
	if(argc > 3 && !(m_simulateHW)){
		ShowHelpInfo();
	}
	m_control.Init(m_simulateHW);

    FindEnvironmentPath();

	return 0;
}

void AppMain::ShowHelpInfo()
{
	std::printf("Invalid parameter!\n");
	std::printf("You may use '-nohw' to enable no hardware modus!\n");
	throw Poco::Exception("Invalid parameter!");
}

void AppMain::FindEnvironmentPath()
{
   	if(m_strBasepath.empty() )
	{
		Poco::Path pathBase;
		const std::string wmBase = "WM_BASE_DIR";
		Poco::Path pathSub("video");
		pathSub.append("testsequences");
		if(Poco::Environment::has(wmBase))
		{
			std::string strWmDir = Poco::Environment::get(wmBase);
			pathBase = Poco::Path(strWmDir);
			pathBase.append(pathSub);
		}
		else
		{
			std::string strFallback = Poco::Environment::get("HOME");
			Poco::Path pathFallback(strFallback);
			pathFallback.append(pathSub);
			pathBase = Poco::Path(pathFallback);
		}

		Poco::File fileCheck(pathBase.toString());
		if( ! fileCheck.exists() )
		{
			try
			{
				fileCheck.createDirectories();
			}
			catch(Poco::Exception const & _ex)
			{
				wmLog( eDebug, " %s Can't create folder %s %s\n", __FUNCTION__, pathBase.toString().c_str(), _ex.name() );
				return;
			}
		}

		m_strBasepath = pathBase.toString();
        //std::cout << "AppMain::FindEnvironmentPath " << m_strBasepath << std::endl; 
	}
}

bool AppMain::CheckSequencesPath(std::string strSequencesPath)
{
	Poco::File fileCheck(strSequencesPath);
	if( ! fileCheck.exists() )
	    return false;	
    else
        return true;
}

void AppMain::ScanSequencesFolder()
{
		try
		{
			std::stringstream strstr;
			strstr << __FUNCTION__ << " start " << m_strBasepath << std::endl;
			wmLog(eDebug, strstr.str());
			const Poco::SortedDirectoryIterator	oEndIt;
			Poco::SortedDirectoryIterator subDirectoryIt(m_strBasepath);
            m_NbImages = 0;
            m_NbSamples = 0;
            m_Sequences.clear();
            for (; subDirectoryIt != oEndIt; ++subDirectoryIt)
			{ // all product folders in base folder
				SequenceType oSequence;
                std::string folderName = subDirectoryIt->path();
				//std::cout << "cccccccccccccc SubDir" << folderName << endl;
                ProductFolderMap_t productFolderMap;
				if (!FindProductFolders(folderName, productFolderMap))
				{
					continue;
				}
                ProductFolderMap_t::Iterator productIt = productFolderMap.begin();
				for (; productIt != productFolderMap.end(); ++productIt)
				{
					ProductType oProduct;
                    uint32_t  productNumber = productIt->second.second;
					std::string productFolderName = productIt->first;
                    ProductFolderMap_t productFolderMap;
                    oProduct.productNumber = productNumber;
					Poco::SortedDirectoryIterator seamseriesIt(productFolderName);
					for (; seamseriesIt != oEndIt; ++seamseriesIt)
					{ // all seamseries folders in product folder
						SeamSerieType oSeamSerie;
                        std::string folderName = seamseriesIt->path();
                        //std::cout << "cccccccccccccc SeamSeriesDir" << folderName << endl;
						uint32_t  seamseriesNumber;
						if (!ParseFolderName(folderName, "seam_series([0-9]+)$", seamseriesNumber))
						{
                            continue;
						}
                        oSeamSerie.seamSerieNumber = seamseriesNumber;
						Poco::SortedDirectoryIterator seamIt(seamseriesIt->path());
						for (; seamIt != oEndIt; ++seamIt)
						{ // all seam folders in seam series folder
							SeamType oSeam;
                            m_NbImages = 0;
                            m_NbSamples = 0;
                            std::string folderName = seamIt->path();
                            //std::cout << "cccccccccccccc SeamDir" << folderName << endl;
							uint32_t  seamNumber;
							if (!ParseFolderName(folderName, "seam([0-9]+)$", seamNumber))
							{
								continue;
							}
                            oSeam.seamNumber = seamNumber;
							Poco::SortedDirectoryIterator fileIt(seamIt->path());
							for (; fileIt != oEndIt; ++fileIt)
							{ // all files in seam folder
								std::string fileName = fileIt->path();
                                VdrFileType vdrFileType;
                                uint32_t  vdrNumber;
                                vdrFileType = parseVdrFileName(fileName, vdrNumber);
								if (vdrFileType == UnknownVdrFileType)
								{
									continue;
								}
								else if(vdrFileType == ImageVdrFileType)
                                {
                                    m_NbImages++;
                                }
                                else if(vdrFileType == SampleVdrFileType)
                                {
                                    m_NbSamples++;
                                }
                            }
                            oSeam.imageCount = m_NbImages;
                            oSeam.sampleCount = m_NbSamples;
                            oSeamSerie.seams.push_back(oSeam);
                            wmLog( eDebug, "AutoRunner: Scan Sequence (product %i seam series: %i, seam: %i images: %i samples: %i)\n", productNumber, seamseriesNumber, seamNumber, m_NbImages , m_NbSamples);
						}
						oProduct.seamSeries.push_back(oSeamSerie);
					}
					oSequence.products.push_back(oProduct);
				}
				m_Sequences.push_back(oSequence);
			}

		}
		catch (Poco::Exception const & _ex)
		{
			std::ostringstream oMsg;
			oMsg << __FUNCTION__ << " " << _ex.what() << " - " << _ex.message() << "\n";
			wmLog(eWarning, oMsg.str());
		}
		catch (const std::exception &p_rException)
		{
			std::ostringstream oMsg;
			oMsg << __FUNCTION__ << " " << p_rException.what() << "\n";
			wmLog(eWarning, oMsg.str());
		}
		catch (...)
		{
			std::ostringstream oMsg;
			oMsg << __FUNCTION__ << " Unknown exception encountered.\n";
			wmLog(eWarning, oMsg.str());
		}
}

bool AppMain::FindProductFolders(std::string & _folderName, ProductFolderMap_t  & _productFolderMap)
		{
			try
			{
				std::string regularExpression("SN-([0-9]+)$");
				Poco::File folder(_folderName);
				if (!folder.isDirectory())
				{
					return false; // skip all but folders
				}

				uint32_t serialNumber;
				if (ParseFolderName(_folderName, regularExpression, serialNumber))
				{
                    std::vector<Poco::File> files;
                    folder.list(files);
                    auto it = std::find_if(files.begin(), files.end(),
                        [] (const Poco::File &file)
                        {
                            return file.isFile() && Poco::Path{file.path()}.getExtension() == std::string("id");
                        }
                    );
                    Poco::UUID productInstanceId;
                    if (it != files.end())
                    {
                        productInstanceId = Poco::UUID(Poco::Path{it->path()}.getBaseName());
                    }
					_productFolderMap.insert(ProductFolderMap_t::ValueType(_folderName, std::make_pair(productInstanceId, serialNumber)));
					return true;
				}
				else
				{
					const Poco::SortedDirectoryIterator	oEndIt;
					Poco::SortedDirectoryIterator productIt(_folderName);
					bool foundOne = false;
					for (; productIt != oEndIt; ++productIt)
					{ // all product folder candidates in folder
						std::string folderName = productIt->path();
						if (FindProductFolders(folderName, _productFolderMap))
						{
							std::ostringstream oMsg;
							oMsg << __FUNCTION__ << " Found folder " << folderName << "\n";
							wmLog(eWarning, oMsg.str());
							foundOne = true;
						}
					}
					return foundOne;
				}
			}
			catch (Poco::Exception const & _ex)
			{
				std::ostringstream oMsg;
				oMsg << __FUNCTION__ << " " << _ex.what() << " - " << _ex.message() << "\n";
				wmLog(eWarning, oMsg.str());
			}
			catch (const std::exception &p_rException)
			{
				std::ostringstream oMsg;
				oMsg << __FUNCTION__ << " " << p_rException.what() << "\n";
				wmLog(eWarning, oMsg.str());
			}
			catch (...)
			{
				std::ostringstream oMsg;
				oMsg << __FUNCTION__ << " Unknown exception encountered.\n";
				wmLog(eWarning, oMsg.str());
			}

			return false;
		}

bool AppMain::ParseFolderName(std::string const & _folderName, std::string const & _regularExpression, uint32_t & _serialNumber)
		{
			try
			{
				Poco::File folder(_folderName);
				if (!folder.isDirectory())
				{
					return false; // skip all but folders
				}

				Poco::Path dirPath(_folderName);
				std::string dirName = dirPath.getFileName();
				//std::cout << dirName << std::endl;
                Poco::RegularExpression re(_regularExpression, Poco::RegularExpression::RE_CASELESS);
				Poco::RegularExpression::MatchVec matches;
				re.match(dirName, 0, matches);
				if (matches.size() == 0)
				{
					std::cout << "no match " << _regularExpression << std::endl;
                    return false;
				}

				uint32_t  serialNumber;
				Poco::RegularExpression::Match match = matches[matches.size() - 1];
				std::string strSerialNumber = dirName.substr(match.offset, match.length);

                serialNumber = std::stoul(strSerialNumber);
				_serialNumber = serialNumber;
				return true;
			}
			catch (Poco::Exception const & _ex)
			{
				std::ostringstream oMsg;
				oMsg << __FUNCTION__ << " " << _ex.what() << " - " << _ex.message() << "\n";
				wmLog(eWarning, oMsg.str());
			}
			catch (const std::exception &p_rException)
			{
				std::ostringstream oMsg;
				oMsg << __FUNCTION__ << " " << p_rException.what() << "\n";
				wmLog(eWarning, oMsg.str());
			}
			catch (...)
			{
				std::ostringstream oMsg;
				oMsg << __FUNCTION__ << " Unknown exception encountered.\n";
				wmLog(eWarning, oMsg.str());
			}

			return false;
		}

VdrFileType AppMain::parseVdrFileName(std::string const & _fileName, uint32_t & _vdrNumber)
		{
			try
			{
				VdrFileType answer = UnknownVdrFileType;
				Poco::File vdrFile(_fileName);
				if (!vdrFile.isFile())
				{
					return answer; // skip all but files
				}

				Poco::Path vdrPath(_fileName);
				std::string vdrName = vdrPath.getFileName();
				std::string fileExtension = vdrPath.getExtension();
				fileExtension = Poco::toLowerInPlace(fileExtension);
				if (fileExtension.compare("bmp") == 0)
				{
					answer = ImageVdrFileType;
				}
				else if (fileExtension.compare("smp") == 0)
				{
					answer = SampleVdrFileType;
				}
				else
				{
					return UnknownVdrFileType;
				}


				Poco::Path filePath(_fileName);
				std::string fileName = filePath.getFileName();
				std::string baseName = filePath.getBaseName();
				Poco::RegularExpression re("[^0-9]*([0-9]+)$", Poco::RegularExpression::RE_CASELESS);
				Poco::RegularExpression::MatchVec matches;
				re.match(baseName, 0, matches);
				uint32_t  vdrNumber;
				if (matches.size() == 0)
				{
					std::ostringstream oMsg;
					oMsg << __FUNCTION__ << " VDR file without number " << _fileName << "\n";
					wmLog(eDebug, oMsg.str());
					if (answer == ImageVdrFileType)
                        _vdrNumber = m_NbImages;
                    else
                        _vdrNumber = m_NbSamples;
				}
				else
				{
					Poco::RegularExpression::Match match = matches[matches.size() - 1];
					std::string strVdrNumber = fileName.substr(match.offset, match.length);
					std::ostringstream oMsg;
#if !defined(__QNX__)
					vdrNumber = std::stoul(strVdrNumber);
#else
					if (!stringToUint(strVdrNumber.c_str(), vdrNumber))
					{
						std::ostringstream oMsg;
						oMsg << __FUNCTION__ << " string to number conversion error " << strVdrNumber << "\n";
						wmLog(eWarning, oMsg.str());
						return UnknownVdrFileType;
					}
#endif
					_vdrNumber = vdrNumber;
				}

				return answer;
			}
			catch (Poco::Exception const & _ex)
			{
				std::ostringstream oMsg;
				oMsg << __FUNCTION__ << " " << _ex.what() << " - " << _ex.message() << "\n";
				wmLog(eWarning, oMsg.str());
			}
			catch (const std::exception &p_rException)
			{
				std::ostringstream oMsg;
				oMsg << __FUNCTION__ << " " << p_rException.what() << "\n";
				wmLog(eWarning, oMsg.str());
			}
			catch (...)
			{
				std::ostringstream oMsg;
				oMsg << __FUNCTION__ << " Unknown exception encountered.\n";
				wmLog(eWarning, oMsg.str());
			}

			return UnknownVdrFileType;
		}

		
void AppMain::runClientCode()
{
	std::cout << "runClientCode" << std::endl;
	notifyStartupFinished();
	AutoRunner autoRunner(m_control, m_strBasepath, m_Sequences);

 
    m_dbNotificationServer.fetchAllProductsData();
    
	if(m_simulateHW)
	{
		m_control.TriggerAutomatic(false, m_control.m_oProductType, m_control.m_oProductNumber, "no info");

		while(!markedForExit()){

			//system("clear");
			std::printf("***HW simulation mode ***\n");
			std::printf("(0)  Calibrate\n");
			std::printf("(1)  ContinuousMode\n");
			std::printf("(2)  AutomaticMode \n");
			std::printf("(3)  InspectInfo \n");
			std::printf("(4)  Start/Stop Inspection \n");
			std::printf("(5)  Set Product Number (%d) \n",m_control.m_oProductNumber);
			std::printf("(6)  Change productType (%d) \n",m_control.m_oProductType);
			std::printf("(7)  Change seamNr (%d) \n",m_control.m_oSeamNr);
			std::printf("(8)  Change numbers of seams to process (%d)\n", m_control.m_oNoSeams);
			std::printf("(9)  Change seamSeries (%d) \n",m_control.m_oSeamseries);
			std::printf("(10) Change number of seam series to process (%d)\n", m_control.m_oNoSeamSeries);
			std::printf("(11) Change number of products to process (%d)\n", m_control.m_oNoProducts);
			std::printf("(12) Change calibration method (%d)\n", m_control.m_oCalibrationType );
			std::printf("(a)  Auto Run \n");
            std::printf("(b)  Auto Run - calculated run time from sequences\n");
			std::printf("(l)  LWM PreStart \n");
			std::printf("(p)  ProductTeachInMode \n");

			std::printf("(r)  Enable/Disable SystemReadyStatus\n");
			std::printf("(s)  Set/Reset SumErrorLatched\n");

			std::printf("(e)  SignalSystemNotReady\n");
			std::printf("(f)  AckSystemFault\n");
			std::printf("(S)  SM_AcknowledgeStep \n");
			std::printf("(x)  EmergencyStop\n");

			std::string choose = "0";
			std::cin >> choose;
			if (std::cin.fail())
			{
				std::printf("Invalid command!\n"); 
				// clear error state
				std::cin.clear();
				// discard 'bad' character(s)
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			}
			else if (choose == "0")
			{
				//Start/Stop Calibration
				std::printf("\t(1) Start calibrating \n");
				std::printf("\t(2) Stop calibrating \n");
				char choose2 = '0';
				std::cin >> choose2;
				switch (choose2) {
				case '1':{
					m_control.TriggerCalibration(true, m_control.m_oCalibrationType);
					break;
				}
				case '2':{
					m_control.TriggerCalibration(false, m_control.m_oCalibrationType);
					break;
				}
				default:{
					std::printf("Invalid command!\n");
					break;
				}
				}
			}
			else if (choose == "1")
			{
				//ContinuousMode
				std::printf("\t(1) Start ContinuousMode \n");
				std::printf("\t(2) Stop ContinuousMode \n");
				char choose2 = '0';
				std::cin >> choose2;
				switch (choose2) {
					case '1':{
						m_control.TriggerContinuously(true, m_control.m_oProductType, m_control.m_oProductNumber);
						break;
					}
					case '2':{
						m_control.TriggerContinuously(false, m_control.m_oProductType, m_control.m_oProductNumber);
						break;
					}
					default:{
						std::printf("Invalid command!\n");
						break;
					}
				}
			}
			else if (choose == "2")
			{
				//AutomaticMode
				std::printf("\t(1) Start AutomaticMode \n");
				std::printf("\t(2) Stop AutomaticMode \n");
				std::printf("\t(3) Start AutomaticMode with ChangeToStandardMode \n");
				char choose2 = '0';
				std::cin >> choose2;
				switch (choose2) {
					case '1':{
						m_control.m_oChangeToStandardMode = false;
						m_control.TriggerAutomatic(true, m_control.m_oProductType, m_control.m_oProductNumber, "no info");
						break;
					}
					case '2':{
						m_control.TriggerAutomatic(false, m_control.m_oProductType, m_control.m_oProductNumber, "no info");
						break;
					}
					case '3':{
						m_control.m_oChangeToStandardMode = true;
						m_control.TriggerAutomatic(true, m_control.m_oProductType, m_control.m_oProductNumber, "no info");
						break;
					}
					default:{
						std::printf("Invalid command!\n");
						break;
					}
				}
			}
			else if (choose == "3")
			{
				//InspectInfo
				std::printf("\t(1) Start Takeover Seam Series \n");
				std::printf("\t(2) Stop Takeover Seam Series \n");
				char choose2 = '0';
				std::cin >> choose2;
				switch (choose2) {
					case '1':{
						m_control.TriggerInspectInfo(true, m_control.m_oSeamseries);
						break;
					}
					case '2':{
						m_control.TriggerInspectInfo(false, m_control.m_oSeamseries);
						break;
					}
					default:{
						std::printf("Invalid command!\n");
						break;
					}
				}
			}
			else if (choose == "4")
			{
				//Start/Stop Inspection
				std::printf("\t(1) Start inspection \n");
				std::printf("\t(2) Stop inspection \n");
				char choose2 = '0';
				std::cin >> choose2;
				switch (choose2) {
					case '1':{
						m_control.TriggerInspectStartStop(true, m_control.m_oSeamNr);
						break;
					}
					case '2':{
						m_control.TriggerInspectStartStop(false, m_control.m_oSeamNr);
						break;
					}
					default:{
						std::printf("Invalid command!\n");
						break;
					}
				}
			}
			else if (choose == "5")
			{
				//Set/Unset Product Number
				std::printf("\t(1) Set Product Number manually\n");
				std::printf("\t(2) Let Product Number be set automatically \n");
				char choose2 = '0';
				std::cin >> choose2;
				switch (choose2) {
					case '1':{
                        m_control.m_oProductNumberFromUserEnable=true;
                        std::printf("\tEnter new Product Number: \n");
                        std::cin >> m_control.m_oProductNumber;
						break;
					}
					case '2':{
                        m_control.m_oProductNumberFromUserEnable=false;
						break;
					}
					default:{
						std::printf("Invalid command!\n");
						break;
					}
				}
			}
			else if (choose == "6")
			{
				//new productType
				std::printf("\tEnter new productType: \n");
				std::cin >> m_control.m_oProductType;
			}
			else if (choose == "7")
			{
				//new seamNr
				std::printf("\tEnter new seamNr: \n");
				std::cin >> m_control.m_oSeamNr;
				autoRunner.adjustStartSeam();
			}
			else if (choose == "8")
			{
				//set number of seams to process
				std::printf("\tNumber of seams to process, starting at %d: \n", m_control.m_oSeamNr);
				std::cin >> m_control.m_oNoSeams;
			}
			else if (choose == "9")
			{
				//new seamSeries
				std::printf("\tEnter new seamSeries: \n");
				std::cin >> m_control.m_oSeamseries;
				autoRunner.adjustStartSeamSeries();
			}
			else if (choose == "10")
			{
				//set number of seamseries to process
				std::printf("\tNumber of seam series to process, starting at %d: \n", m_control.m_oSeamseries);
				std::cin >> m_control.m_oNoSeamSeries;
			}
			else if (choose == "11")
			{
				//set number of product-types to process
				std::printf("\tNumber of products to process, starting at %d: \n", m_control.m_oProductType);
				std::cin >> m_control.m_oNoProducts;
			}
			else if (choose == "12")
			{
				//set number of product-types to process
				std::printf("\tCurrent calibration method %d: \n", m_control.m_oCalibrationType);
				std::cin >> m_control.m_oCalibrationType;
			}
			else if (choose == "a")
			{
				//Auto Runner
				std::printf("\t(1) Start Auto Run\n");
				std::printf("\t(2) Stop Auto Run \n");
				std::printf("\t(3) Change Run Time (%d) ms \n", autoRunner.getRunTime());
				std::printf("\t(4) Change Break Time (%d) ms \n", autoRunner.getBreakTime());
				std::printf("\t(5) Change Seam Break Time (%d) ms \n", autoRunner.getSeamBreakTime());
				std::printf("\t(6) Change Loop Count (%d) \n", autoRunner.getLoopCount());
				char choose2 = '0';
				std::cin >> choose2;
				switch (choose2) {
					case '1':{
						autoRunner.setAutoRunnerMode(0);
                        if(autoRunner.activity().isStopped())
						{
							autoRunner.activity().start();
						}
						break;
					}
					case '2':{
						if(autoRunner.activity().isRunning())
						{
							autoRunner.activity().stop();
						}
						break;
					}
					case '3':
					{
						//new Runtime
						int iloc;
						std::printf("\tEnter new Run Time ms: \n");
						std::cin >> iloc;
						autoRunner.setRunTime(iloc);
						break;
					}
					case '4':
					{
						//new Breaktime when "restarting" the chosen seamseries
						int iloc;
						std::printf("\tEnter new Break Time between loops [ms]: \n");
						std::cin >> iloc;
						autoRunner.setBreakTime(iloc);
						break;
					}
					case '5':
					{
						//new Breaktime when "restarting" the chosen seam
						int iloc;
						std::printf("\tEnter new Break Time between seams [ms]: \n");
						std::cin >> iloc;
						autoRunner.setSeamBreakTime(iloc);
						break;
					}
					case '6':
					{
						//new Loop Count
						int iloc;
						std::printf("\tEnter new Loop Count: \n");
						std::cin >> iloc;
						autoRunner.setLoopCount(iloc);
						break;
					}
					default:{
						std::printf("Invalid command!\n");
						break;
					}
				}
			}
			else if (choose == "b")
			{
				//Auto Runner
				std::printf("\t(1) Start Auto Run with Sequences \n");
                std::printf("\t    Run Sequences in Folder \n\t    %s \n", autoRunner.getSequenceFolder().c_str());
                std::printf("\t(2) Stop Auto Run with Sequences \n");
                std::printf("\t(4) Scan Sequences Folder Structure again\n");
				std::printf("\t(5) Change Break Time (%d) ms \n", autoRunner.getBreakTime());
				std::printf("\t(6) Change Seam Break Time (%d) ms \n", autoRunner.getSeamBreakTime());
				std::printf("\t(7) Change Loop Count (%d) \n", autoRunner.getLoopCount());
				char choose2 = '0';
				std::cin >> choose2;
				switch (choose2) {
					case '1':{
						autoRunner.setAutoRunnerMode(1);
						m_control.m_oProductNumberFromUserEnable=true;
                        if(autoRunner.activity().isStopped())
						{
							ScanSequencesFolder();
                            autoRunner.activity().start();
						}
						break;
					}
					case '2':{
						if(autoRunner.activity().isRunning())
						{
							autoRunner.activity().stop();
						}
						autoRunner.setAutoRunnerMode(0);
						break;
					}
					case '3':
					{
						//change sequence folder
                        
                        std::string inputString;
                        std::printf("\tEnter new sequences folder path: \n");
                        std::cin >> inputString;
                        if (CheckSequencesPath(inputString))
                        {
                            autoRunner.setSequenceFolder(inputString);
                            m_strBasepath = inputString;
                        }
                        else
                        {
                            std::printf("\tnew sequences folder don't exist! \n");
                        }
                        break;
					}
					case '4':
					{
						// scan the sequences structure
                        ScanSequencesFolder();
                        break;
					}
					case '5':
					{
						//new Breaktime when "restarting" the chosen seamseries
						int iloc;
						std::printf("\tEnter new Break Time between sequences [ms]: \n");
						std::cin >> iloc;
						autoRunner.setBreakTime(iloc);
						break;
					}
					case '6':
					{
						//new Breaktime when "restarting" the chosen seam
						int iloc;
						std::printf("\tEnter new Break Time between seams [ms]: \n");
						std::cin >> iloc;
						autoRunner.setSeamBreakTime(iloc);
						break;
					}
					case '7':
					{
						//new Loop Count
						int iloc;
						std::printf("\tEnter new Loop Count: \n");
						std::cin >> iloc;
						autoRunner.setLoopCount(iloc);
						break;
					}
					default:{
						std::printf("Invalid command!\n");
						break;
					}
				}
			}
			else if (choose == "l")
			{
				//Start/Stop LWM PreStart
				std::printf("\t(1) Start LWM PreStart \n");
				std::printf("\t(2) Stop LWM PreStart \n");
				char choose2 = '0';
				std::cin >> choose2;
				switch (choose2) {
					case '1':{
						m_control.TriggerInspectionLWMPreStart(true, m_control.m_oSeamNr);
						break;
					}
					case '2':{
						m_control.TriggerInspectionLWMPreStart(false, m_control.m_oSeamNr);
						break;
					}
					default:{
						std::printf("Invalid command!\n");
						break;
					}
				}
			}
			else if (choose == "p")
			{
				//Start/Abort ProductTeachIn
				std::printf("\t(1) Start ProductTeachIn \n");
				std::printf("\t(2) Abort ProductTeachIn \n");
				char choose2 = '0';
				std::cin >> choose2;
				switch (choose2) {
				case '1': {
					m_control.StartProductTeachInMode();
					break;
				}
				case '2': {
					m_control.AbortProductTeachInMode();
					break;
				}
				default: {
					std::printf("Invalid command!\n");
					break;
				}
				}
			}
			else if (choose == "r")
			{
				//Enable/Disable SystemReadyStatus
				std::printf("\t(1) Enable \n");
				std::printf("\t(2) Disable \n");
				int choose2 = 0;
				std::cin >> choose2;
				switch (choose2) {
				case 1:{
					m_control.setSystemReady(true);
					break;
				}
				case 2:{
					m_control.setSystemReady(false);
					break;
				}
				default:{
					std::printf("Invalid command!\n");
					break;
				}
				}
			}
			else if (choose == "s")
			{
				//Set/Reset SumErrorLatched
				std::printf("\t(1) Set \n");
				std::printf("\t(2) Reset \n");
				int choose2 = 0;
				std::cin >> choose2;
				switch (choose2) {
				case 1:{
					m_control.setSumErrorLatched(true);
					break;
				}
				case 2:{
					m_control.setSumErrorLatched(false);
					break;
				}
				default:{
					std::printf("Invalid command!\n");
					break;
				}
				}
			}
			else if (choose == "e")
			{
				//Signal error
				std::printf("\tSet relatedException: \n");

				int choose2 = 0;
				std::cin >> choose2;

				m_control.SimulateSignalNotReady(choose2);
			}
			else if (choose == "f")
			{
				m_control.SimulateQuitSystemFault();
			}
			else if (choose == "S")
			{
				//SM_AcknowledgeStep
				std::printf("\t(1) SM_AcknowledgeStep high \n");
				std::printf("\t(2) SM_AcknowledgeStep low \n");
				char choose2 = '0';
				std::cin >> choose2;
				switch (choose2) {
					case '1':{
						m_control.TriggerSM_AcknowledgeStep(true);
						break;
					}
					case '2':{
						m_control.TriggerSM_AcknowledgeStep(false);
						break;
					}
					default:{
						std::printf("Invalid command!\n");
						break;
					}
				}
			}
			else if (choose == "x")
			{
				//Start/Stop Calibration
				std::printf("\t(1) EmergencyStop On \n");
				std::printf("\t(2) EmergencyStop Off \n");
				char choose2 = '0';
				std::cin >> choose2;
				switch (choose2) {
				case '1':{
					m_control.TriggerEmergencyStop(true);
					break;
				}
				case '2':{
					m_control.TriggerEmergencyStop(false);
					break;
				}
				default:{
					std::printf("Invalid command!\n");
					break;
				}
				}
			}
			else {
				std::printf("Invalid command!\n");
			}
		} // while
	} // if

	char wait = 0;
	std::cin >> wait;
} // runClientCode

} // namespace viInspectionControl
} // namespace precitec

