/**
 * @file
 * @brief  camera Klasse realisiert Kamera Zugriff, Zugriff auf Photonfocus SDK
 * Camera realisiert den Kamerazugriff.
 *
 * @author JS
 * @date   20.05.10
 * @version 0.1
 * Erster Wurf
 *
 */



#ifndef CAMERA_HEADER_FILE
#define CAMERA_HEADER_FILE

#include <cstdlib> // WoR: wg std::printf

#include <iostream>
#include <sstream>
#include <set>
#include <vector>
#include "pf/photonFocus.h"

#include "system/types.h"
#include "sensor.h"




/**
*  Kamera Klasse
*  Zweck: Kommunikation mit der Kamera
*
*  Stellt direkt implementierte Zugriffsfunktion
*  und die allgemeinen Zugriffsfunktion SetProperty und GerProperty
*  zur Verfuegung
*
 */
/// Kamera - Photonfocus Zugriff
class Camera
{
	enum TriggerType { FreeRunning = 0, GrabberControlled = 1};
public:
	Camera(int portNum=DefaultPort__);
	~Camera();
	///der test liesst den Kameranamen aus
	int test();
	/// Fehlerausgabe und Beschreibung
	int handleError(int error);
	/// Kamera schliessen
	void close(void);
	/// Kammera schliesen und wieder oeffnen
	int reOpen(void);

	/// leere Kommunikations Scnittstelle
	int flushPort(void);

	/// Konfiguration speichern
	void saveConfig(void);
	/// Kamera initialisiseren
	int  init(int portNum);
	/// ok Variable liefern
	bool isOk(void) const { return ok; }

	/// Property als String setzen
	int SetPropertyString(int port, char* property, char* val);
	/// Property als String liefern
	int GetPropertyString(int port, const char* property, char* val);

	/// Property als int setzen
	int SetPropertyInt(int port, char* property, int val);
	/// Property als int liefern
	int GetPropertyInt(int port, char* property, int* val);

	/// Property als float setzen
	int SetPropertyFloat(int port, char* property, float val);
	/// Property als float liefern
	int GetPropertyFloat(int port, char* property, float* val);

	/// Typ des Properties holen
	precitec::Types GetPropertyType(int port, const char* property);

	/// Property setzen
	template<typename T>
	inline int SetProperty(int port,char* property, T val);

	/// Property lesen
	template<typename T>
	inline int GetProperty(int port,char* property, T* val);

    /// Belichtungszeit setzen
	void setExposure(bool usePulseWidthExposure,int exposure);
	int  getExposure(void) ;

	///skim Wert setzen
	int  setSkimValue(int iSkimValue);
	/// skim Wert lesen
	int  getSkimValue(void) ;
    /// globalen Offset ( Schwarzabgleich) setzen
	int  setOffset(int iOffset);
	/// globalen Offset lesen
	int  getOffset(void) ;
	/// Helligkeitsabgleich lesen
	int  getCalibOffset(void) ;
    /// Frametime setzen : Feste Zeit von Bild zu Bild setzen
	int  setFrameTime(int iFrameTime);
	/// Frametime lesen
	int  getFrameTime(void) ;

	/// z.Zt. nicht unterstuetzt
	int  importSettings(const char *filename, int& endCounter);
	int  exportSettings(const char *filename, int& endCounter) const;

	/// skim Flag setzen
	int  setSkimFlag(int flag);
	/// Bild Rotations Flag setzen: Z.Zt. nicht unterstuetzt
	int  setRotateFlag(int flag);
	/// konstante Bildrate setzen
	int  setConstFrameRate(int flag);
	/// Belichtungszeit Modus setzen
	int  setExposureMode( int flag);
	/// skim Flag lesen
	int  getSkimFlag(void);
	/// rotate flag lesen
	int  getRotateFlag(void);
	/// flag lesen
	int  getConstFrameRate(void);
	///Belichtungszeit Modus lesen
	int  getExposureMode(void);
	/// Look Up Tabelle setzen z.Zt. nicht implementiert
	int  setLUT( unsigned char* lut,int number);
	/// linlog Werte setzen
	int  setLinLog(int linlog1,int linlog2,int time1,int time2);
	/// linlog Werte lesen
	int  getLinLog(int &linlog1,int &linlog2,int &time1,int &time) ;
	/// Wird ein ROI Parameter geaendert ?
	int isRoiParameter(std::string const &key,int value);

	/// ROI setzen
	int	 setRoi(int x, int y, int dx, int dy, bool useOffset);
	/// Roi Werte aus der Kamera lesen
	int  getRoi(int& x, int& y, int& dx, int& dy);

	/// interne ROI Variablen zurueckgeben
	void getCamRoi(int& x, int& y, int& dx, int& dy){x =camRoiX_;y = camRoiY_;dx=camRoiDX_;dy=camRoiDY_; }

	/// Werte aus EEPROM in Kamera FPGA schreiben
	void reset(void);
	/// Kamera auf Auslieferungszustand setzen
	void factoryReset(void);
	/// trigger MOdus setzen
	void	setTrigger(TriggerType trig);
	/// Liste mit den Kamera Properties lesen
    //StringSet getKeys(void){ return cameraKeys_; }
    SensorConfigVector getProps(void){return cameraProps_; }

    void  setCameraTmpROI(int &x,int &y, int&dx, int&dy);

    bool isCommand(std::string const & key);

    int   createCameraProperties(void);
    
    std::vector<std::uint8_t> getUserFlash(int startBit=0, int numBits=2048);
    bool setUserFlash(const std::vector<std::uint8_t> & rContent);

    unsigned int getMaxSensorWidth() const
    {
        return MaxSensorWidth;
    }

    unsigned int getMaxSensorHeight() const
    {
        return MaxSensorHeight;
    }
private:
	static const int DefaultPort__ = 0;						///< default Kamera Port, anhaengig von Grabberanzahl und Anschluss
	int port_;												///< port Nummer (Anschlussnummer der kamera)
	bool ok;												///< ok

	static const unsigned short EEPROM_BUFLEN ;	///<EEPROM Laenge
	static const unsigned short MAXDACVAL;		/// Maximaler DAC Wert
	static const double 		SKIM_FACTOR; 	///< Umrechnungsfaktor fuer skimming
	static const unsigned short SKIM_HEADROOM;	///<skim Wertebereich

	unsigned int  MaxSensorWidth;	///<Sensorgroesse
	unsigned int  MaxSensorHeight;	///<Sensorgroesse


	///tatsaechliche Kamera ROI Groesse
	int camRoiX_;												///<eingesteller Kamera ROI x Offset Wert
	int camRoiY_;												///<eingesteller Kamera ROI y Offset Wert
	int camRoiDX_;												///<eingesteller Kamera ROI dx Offset Wert
	int camRoiDY_;												///<eingesteller Kamera ROI dy Offset Wert

	int camTmpRoiX_;											///< Kamera ROI x Offset Wert - temporaer gespeicherter Wert, noch nicht in der Kamera gesetzt.
	int camTmpRoiY_;											///< Kamera ROI y Offset Wert - temporaer gespeicherter Wert, noch nicht in der Kamera gesetzt.
	int camTmpRoiDX_;											///< Kamera ROI dx Offset Wert - temporaer gespeicherter Wert, noch nicht in der Kamera gesetzt.
	int camTmpRoiDY_;											///< Kamera ROI dx Offset Wert - temporaer gespeicherter Wert, noch nicht in der Kamera gesetzt.

	//Vector mit den  Eintraegen Value, Min,Max,Default und dataType
	SensorConfigVector cameraProps_;							///< zur VErfuegung gestellte Kameraeigenschaften

	std::string m_oCamName;

	/// Hilfsfunktionen-werden momentan nicht unterstuetzt
	void initDumpCache(unsigned char *cache, unsigned char *dirty, int n);
	int  EepromToFile(unsigned char *buf, int buflen, const char *filename, int& endCounter) const;
	int  FileToEeprom(unsigned char *buf, unsigned char *dirty, int buflen, const char *filename, int& endCounter);

}; // class Camera

/**
*  property an Kamera uebertragen
*
* \param   port - Kameraport
* \param   property - Eigenschaft
* \param  val - Wert
* \return  error Fehlernummer
*
 */
template<typename T>
int Camera::SetProperty(int port, char* property, T val)
{
	TOKEN t;
	int error=0;
//	PFValue v;

	//auf ostringstream
	std::ostringstream oss; //(ostringstream::out);
	oss << val;

	//auf string
	std::string s=oss.str();
	//string auf char* konvertieren
	const char *cVal = s.c_str();

	//char* auf string
	std::string keyString = property;


//	std::cout<<"camera.h setProperty Template: "<<os<<std::endl;
	t = pfProperty_ParseName(port, property);


	if(t == INVALID_TOKEN){
		std::cout << "SetProperty: Property not found or bad index: (" << property << ")\n" << std::endl;
		error = -1;
	}
	else{

		error = pfDevice_SetProperty_String(port,t,cVal);

        // wert zuruecklesen:
		char val[256];
		pfDevice_GetProperty_String(port, t, val, 64);
		std::string valueString=val;

// die float Uebergabe gibt immer einen out of range error mit der 80er Kamera
//		v.type = pfProperty_GetType(port, t);
//		v.value.i = val;
//		error = pfDevice_SetProperty(port, t, &v);
//		if(error < 0){
//			const char *s;
//				s = pfGetErrorString(error);
//			    std::printf("Error: %s\n", s);
//
//			//Camera::handleError(error);
//		}
	}
	return error;
}


/**
*  property aus Kamera lesen
*
* \param   port - Kameraport
* \param   property - Eigenschaft
* \param  val - Wert
* \return  error Fehlernummer
*
 */
template<typename T>
int Camera::GetProperty(int port, char* property, T* val)
{
	TOKEN t;
	int error;
	PFValue v;

	error = 0;
	std::cout << "Get property " << property << std::endl;
	t = pfProperty_ParseName(port, property);
	if(t == INVALID_TOKEN){
		std::cout << "GetProperty: Property not found or bad index: (" << property << ")\n" << std::endl;
	}
	else{
		error = pfDevice_GetProperty(port, t, &v);
		if(error < 0){
			const char *s;
			s = pfGetErrorString(error);
			std::cout << "Error: " << s << std::endl;

			//Camera::handleError(error);
		}
		*val = v.value.i;
	}
	return error;
}



#endif // CAMERA_HEADER_FILE
