#ifndef CALIBRATIONPARAMMAP_H_
#define CALIBRATIONPARAMMAP_H_

// stl includes
#include <string>
#include <vector>
#include <map>

// project includes
#include <message/device.h>
#include <module/moduleLogger.h>

#include <Analyzer_Interface.h>

#include "Poco/DOM/Document.h"
#include "Poco/DOM/AutoPtr.h"


#include "message/serializer.h"
#include "message/messageBuffer.h"


namespace precitec
{
namespace math
{

	class ANALYZER_INTERFACE_API CalibrationParamMap: public system::message::Serializable
	{
	private:
		interface::Configuration						m_oParameters;			///< All user accessible calibration parameters.
		std::map<std::string, interface::SmpKeyValue>	m_oParamMap;			///< Map that references all parameters for quick access by key.
		std::vector<std::string>	m_oProtectedParameterKeys;			///< list non-modifiable parameters (to write in xml config file, but disabled for the user -  computed parameters, used only as a mechanism to initialize Analyzer)
		std::vector<std::string> m_oProcedureParameterKeys;  ///< list of keys used only during calibration procedure 
		bool m_oProcedureParametersModified; /// true if values have been modified but not written to file
		bool m_oCalibrationParametersModified; //former isCalibrationGraphParameter
		std::string m_oConfigFilename;

		void addCalibrationProcedureParameters(); ///< Add parameters for the calibration graph to the KeyValue map

		// KeyValue handling
		template<typename T> // any nullable type...
		bool getTagValue(T &p_rValue, Poco::XML::AutoPtr<Poco::XML::Document> &p_rDoc, std::string p_oTag) const;
		
        void updateModifiedFlag(const std::string & p_rKey);
		void setModifiedFlag(bool val); // to be used only by initialization or print
		void initializeCalibrationKeyValues();
		/**
		* @brief Save parameter set to the xml configuration file.
		*/
		bool exportToFile(const std::string oConfigFilename) const;
    
	public:
		CalibrationParamMap();
		CalibrationParamMap(const std::string oConfigFilename);
        
        virtual void serialize ( system::message::MessageBuffer &buffer ) const;
        virtual void deserialize( system::message::MessageBuffer const&buffer );
        
        
		size_t size() const;
		bool hasModifiedParameters() const;
		bool hasModifiedCalibrationParameter() const;
		bool readParamMapFromFile(const std::string oConfigFilename);
		bool setFile(const std::string oConfigFilename);
        bool hasKey(const std::string& key) const;
        const std::vector<std::string> & getProcedureParameterKeys() const;
        std::vector<std::string> getScanMasterKeys(bool withCalibrationProcedureParameters) const;
        bool isProtectedParamKey(const std::string & p_rKey) const;
        bool isProcedureParamKey (const std::string & p_rKey) const;
        const std::string & getConfigFilename() const;
        

		/**
		* @brief Set a single key.
		* @param p_pKeyValue Shared pointer to key value object.
		* @return Returns a handle to the key (the handle is not implemented correctly in the device servers right now).
		*/
		interface::KeyHandle set(interface::SmpKeyValue keyValue, bool overrideProtected=false);

		/**
		* @brief Get a single parameter.
		* @param p_oKey Key object (e.g. 'ExposureTime').
		* @return
		*/
		interface::SmpKeyValue get(interface::Key key) const;

    public:
        //used by DeviceServer::get overriden by CalibrationData::getConfiguration()
		/**
		* @brief Get all parameters of the calibration
		* @return Configuration object (std::vector of key-value objects).
		*/
		//interface::Configuration get() const;



		/**
		* @brief Convenience function to retrieve a bool parameter.
		* @param p_oName std::string with the name of the parameter.
		* @return int value of the parameter.
		*/
		bool getBool(std::string p_oName) const;

		/**
		* @brief Convenience function to retrieve an integer parameter.
		* @param p_oName std::string with the name of the parameter.
		* @return int value of the parameter.
		*/
		int getInt(std::string p_oName) const;

		/**
		* @brief Convenience function to change an integer parameter.
		* @param p_oName std::string with the name of the parameter.
		* @param p_oValue new int value of the parameter.
		*/
		void setInt(std::string p_oName, int p_oValue);

		/**
		* @brief Convenience function to retrieve a double parameter.
		* @param p_oName std::string with the name of the parameter.
		* @return double value of the parameter.
		*/
		double getDouble(std::string p_oName) const;

		/**
		* @brief Convenience function to change a double parameter.
		* @param p_oName std::string with the name of the parameter.
		* @param p_oValue new double value of the parameter.
		*/
		void setDouble(std::string p_oName, double p_oValue);


		std::string getString(std::string p_oName) const;


		void setString(std::string p_oName, std::string p_oValue);


		/**
		* @brief Print all calibration values.
		*/
		void print() const;

		interface::SmpKeyValue prepareKeyValueForModification(std::string p_oName, bool overrideProtected);

		//set value (Casts if necessary)
		template <typename tKeyType>
		void setValue(std::string p_oName, tKeyType p_oValue, bool overrideProtected=false)
		{
			auto oSmpKey = prepareKeyValueForModification(p_oName, overrideProtected);
			if (oSmpKey.isNull())
			{
				return;
			}

			const Types oType(oSmpKey->type());

			switch ( oType )
			{
				case TInt: 
					oSmpKey->setValue<int>(int(p_oValue));
					break;
				case TDouble:
					oSmpKey->setValue<double>(double(p_oValue));
					break;
				case TBool:
					oSmpKey->setValue<bool>(bool(p_oValue));
					break;
				case TString:
					// see corresponding template specialization
					//FALLTHROUGH
				default:
					std::ostringstream oMsg;
					oMsg << __FUNCTION__ << ": invalid value type: " << oType << "\n";
					wmLogTr(eWarning, "QnxMsg.Calib.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str());
					break;
			} // switch

		};

		template <typename tKeyType>
		tKeyType getValue(std::string p_oName) const;


		/**
		* @brief Save parameter set to the xml configuration file only when parameters are modified.
		* Returns false in case of invalid file
		*/
		bool saveToFile();
		bool syncXMLContent(std::string & rXMLContent);
	};

	
template <> 
void CalibrationParamMap::setValue<std::string>(std::string p_oName, std::string p_oValue, bool overrideProtected) ;


}
}


#endif
