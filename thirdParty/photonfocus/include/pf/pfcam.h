/** \file pfcam.h
 *
 * This is the replacement for camdll.h. It will provide the API for
 * the property protocol only.
 * 
 *
 */

#include "pftypes.h"  // Photonfocus data types
#include "api.h"


/*!
 * \mainpage pfLib SDK Documentation
 * \version 1.0
 * \author Photonfocus AG
 * \date 11/2010
 *
 * \section intro   Introduction
 * 
 * The Photonfocus library 'pfLib' enables an application programmer
 * to control a Photonfocus Camera's features without direct access to
 * the CameraLink (or other communication layer) interface.
 * The access, in order to work with most of the frame grabbers, is
 * done by a separate COMDLL, which is a low level communication interface
 * to the frame grabber's RS232 emulation. This may be frame grabber specific,
 * and does not depend on the camera type.
 *
 * \note The API has changed from 0.x to 1.x. No 'old' function call
 *       is supported anymore. However, the 1.0 API is much simpler and
 *       flexible to handle.
 *
 * \section api   API Details
 *
 * The API interface uses the standard C convention (CDECL) interface.
 * Support for Visual Basic is not available.
 * The pfLib is portable among different operating systems such as
 * Microsoft Windows, Linux, QNX, and other Unix versions, however,
 * this is not tested and we cannot give any support for an operating
 * system other than Microsoft Windows.
 *
 * This 1.0 version API was designed as generic as possible for any
 * device type. Therefore, we ask the reader not to be confused about
 * abstract terms as 'devices' and 'properties'. For legacy reasons,
 * the term 'camera' still appears here and there. We apologize for
 * the inconsistence.
 *
 * \subsection props   Property concept
 *
 * The device property concept (so forth 'Property') of the previous 0.x PFLIB
 * API has been refined such that all device properties are basically
 * controlled via pfDevice_GetProperty() and  pfDevice_SetProperty().
 *
 * New in this API is a tree hierarchy, that covers the following property
 * features:
 *
 * \li Properties have more extended types such as structs and arrays and
 *     can contain other Property members
 * \li Max and min values are encoded the same way in a parent/children
 *     hierarchy
 * \li Mode selections can be encoded in this tree concept as well
 *
 * \image html properties.png  "Property hierarchy"
 * \image latex properties.pdf   "Property" width=15cm
 *
 *
 * The architecture also allows to integrate user defined children types
 * to a data type via structs that allow signalling events to a specific GUI
 * element when Property was changed, etc.
 *
 * To keep the API and the library code as compact and efficient as possible,
 * the Property access happens via tokens. The token is obtained by 
 * pfProperty_ParseName() via a given name, which is defined in the name
 * space of a certain parent node. The top root node of a Property
 * hierarchy is always the device node which is obtained by pfDevice_GetRoot().
 *
 * Value passing is done via a runtime data type information capable structure
 * 'PFValue'. This structure contains a data union field and an associated
 * data type. The value is retrieved from the PFValue struct according
 * to the type field. The Property getter and setter functions of the library
 * perform a type check and return an error (#PFERR_PROPERTY_TYPE_MATCH) in
 * case of a mismatch.
 *
 * The following code sequence demonstrates how to set the exposure time
 * of a camera:
 *

\code
int setExposure(int port, float time)
{
	PFValue val;
	int error;
	TOKEN t_exp = pfProperty_ParseName(port, "ExposureTime"));

	if (t_exp == INVALID_TOKEN) return PFERR_PROPERTY_UNKNOWN;

	// Initialize Value:
	val.value.f = time;
	val.type = PF_FLOAT;

	error = pfDevice_SetProperty(port, t_exp, &val);

	return error;
}
\endcode


 *
 *
 * \subsection commonfunc Common function parameters
 *
 * Generally, the first parameter of a function whose name starts with
 * 'pfDevice_' is the current camera device handle. The struct members of
 * the camera object must NEVER be accessed directly, to preserve future
 * binary compatibility.
 *
 * If not documented otherwise, the function's return value is 
 * \li  < 0 on failure
 * \li == 0 on success
 * \li  > 0 a warning
 *
 * Functions which set a value may return #PFWARN_PROPERTY_MODIFIED, meaning
 * that the given parameter was not valid or not according to certain
 * constraints. All 'Event' children of this property should be queried via
 * pfProperty_Select(), because they may have changed.
 * In this case, the user should also read back the value that was passed
 * to pfSetProperty() which was possibly modified 'in place'.
 *
 *
 * \see ErrorHandling
 *
 */


// This trick is used for easy function wrapping..
#ifndef APIFUNC
#define APIFUNC(retval, name, parms)           \
	CAMDLL_API retval APIDECL pf##name parms;
#endif


/**
 * \defgroup PortAccess Port Access, Device Initialization
 *
 * This module contains the initialization routines for the
 * camera ports
 */

/**
 * \example example.c
 *
 * This example works with a MV-D1024E-40 camera. 
 * For other cameras, check propertylist first and change code.\n
 * Propertylist: Start PFRemote, press F1 for help, menu Camera Properties, select camera
 *
 */
 
/**
 * \example main.c
 *
 * A simple command line program to query and set properties.
 * 
 * \b Usage
 * \verbatim
   <programname>\endverbatim
 *                       Lists all top level properties of the device
 *
 * \verbatim
   <programname> <property_name> \endverbatim
 *
 *                       Query specified Property with name 'property'.
 *                       Children are displayed as well as its value,
 *                       if applicable.
 *
 * \verbatim
   <programname> <property> <value> \endverbatim
 *
 *                       Set specified Property with name 'property'
 *                       to value 'value'. Illegal values are caught.
 *
 *
 *
 */

/**
 * \addtogroup PortAccess
 * \{
 */

 
/** 
 *
 *  Init of all camera ports. This has to be called as first
 *
 * \param numOfPorts	Number of available camera ports
 * 
 * \return		0: successful\n
 * 			< 0: error
 *
 */
APIFUNC(int, PortInit, (int *numOfPorts))

	
/** 
 *
 *  Get information about a port (manufacturer, name, etc)
 *
 * \param port		Port number of the camera
 *
 * \param manu		Name of the manufacturer of this port
 *
 * \param mBytes	Lenght of (manu+1)
 *
 * \param name		Name of the manufacturer of this port
 *
 * \param nBytes	Lenght of (name+1)
 *
 * \param version	Version of CLALLSERIAL (0 if no ClAllSerial port)\n
 *			1 : CL_DLL_VERSION_NO_VERSION	Not a CL dll\n
 *			2 : CL_DLL_VERSION_1_0		Oct 2000 compliant\n
 *			3 : CL_DLL_VERSION_1_1  	Oct 2001 compliant\n

 * \param type		Type of this port\n
 * 			0 : ClAllSerial port\n
 * 			1 : port with clser.dll at pfremote directory\n
 *			2 : USB port\n
 *			3 : RS-232 port\n
 * 
 * \return		0: successful\n
 * 			< 0: error
 *
 */
APIFUNC(int, PortInfo, (int port, char *manu, int *mBytes, char *name, int *nBytes, int *version, int *type))


/**
 * Check if required baud rate is possible. The camera port must be opened, before checking the required baud rate
 * 
 * \param port		Port number of the camera
 *
 * \param baudrate	Baud rate to check (eg. 57600)
 *                       
 * \return		1: baud rate suporrted\n
 * 			0: baud rate not supported\n
 * 			< 0: error.
 */
APIFUNC(int, IsBaudRateSupported, (int port, int baudrate))

	
/**
 * Get current baud rate
 * 
 * \param port		Port number of the camera
 *
 * \param baudrate	Baud rate
 *                       
 * \return		0: successful\n
 * 			< 0: error.\n
 */
APIFUNC(int, GetBaudRate, (int port, int *baudrate))


/**
 * Set baud rate. The camera port must be opened, before setting the baud rate
 * 
 * \param port		Port number of the camera
 *
 * \param baudrate	Baud rate to set (eg. 57600)
 *                       
 * \return		0: successful\n
 * 			< 0: error.\n
 */
APIFUNC(int, SetBaudRate, (int port, int baudrate))


/**
 * Open Device
 * 
 * \param port		Port number of the camera
 *                       
 * \return		0: successful\n
 * 			< 0: error\n
 *                      A value > 0 means, the device was opened, but
 *                      some non blocking failure was detected (bad
 *                      device identity, etc.). In this case, some int index
 *                      device features may not be accessible, but the
 *                      it can be normally fixed via an update.
 */
APIFUNC(int, DeviceOpen, (int port))	

	
/**
 * Close device communication.
 * This should be called at the end of an application,
 * when the device is no longer accessed.
 *
 * \param port		Port number of the camera
 *
 * \return		0: success\n
 * 			< 0: error\n
 */
APIFUNC(int, DeviceClose, (int port))

/* \} */


////////////////////////////////////////////////////////////////////////////
// Prototyping NEW API (1.0)
////////////////////////////////////////////////////////////////////////////

// DLL top level stuff

APIFUNC(const char *, DeviceGetDllVersion, (int port, int *major, int *minor))	

////////////////////////////////////////////////////////////////////////////
// PROPERTY API

/**
 * \defgroup PropControl Generic Device property control
 *
 * This contains the function set necessary to control arbitrary
 * device properties.
 * 
 */

/**
 * \addtogroup PropControl
 * \{
 */

	
/**
 * Get root namespace descriptor token of current device
 *
 * This function always returns a valid token.

 * \param port	Port number of the camera 
 * 
 * \return      The root descriptor token
 *
 **/
APIFUNC(TOKEN, Device_GetRoot, (int port))


/**
 * Select a property in the property and feature hierarchy.
 *
 * This function is used to query a property inside the property hierarchy
 * which is organized in a tree. A property can have children and has always
 * a parent, except if it is the root node.
 *
 * Quick overview: To select the first child of a node, use:
 *
 * \code
     child = pfProperty_Select(port, node, node); \endcode
 *
 * To select the next children of that node:
 *
 * \code
     next = pfProperty_Select(port, node, child); \endcode
 *
 * \param port		Port number of the camera 
 * \param parent	The parent node to be queried 
 * \param prev		The node whose following (next) node shall be selected
 *                  	If the parent node is specified, its first children is
 *                  	selected.
 *
 * \return          	The next or children token, if existing, INVALID_TOKEN
 *                  	otherwise.
 **/
APIFUNC(TOKEN, Property_Select, (int port, TOKEN parent, TOKEN prev))

	
/**
 * Get a property handle (TOKEN) by name.
 *
 * \param port		Port number of the camera * 
 * \param propname	Pointer to a string containing the property name
 * 
 * \return          	The TOKEN of the property. If not existing, INVALID_TOKEN.
 *
 **/	
APIFUNC(TOKEN, Property_ParseName, (int port, const char *propname))
	

/**
 * Get the name of a property by token
 *
 * \param port		Port number of the camera
 * \param p	        The token of the property whose name is to be
 *	                queried
 *
 * \return          	Pointer to the constant property string
 * 
 **/
APIFUNC(const char *, Property_GetName, (int port, TOKEN p))


/**
 * Get the property data type by token
 *
 * \param port		Port number of the camera
 * \param p          	The property token whose type are to be queried
 *
 * \return           	The type of the property 
 *
 **/
APIFUNC(PropertyType, Property_GetType, (int port, TOKEN p))

	
/**
 * Get the property flags by token
 *
 * \param port		Port number of the camera
 * \param p          	The property token whose flags are to be queried
 *
 * \return           	The property flags
 *
 **/
APIFUNC(unsigned long, Property_GetFlags, (int port, TOKEN p))


/**
 * Get the value of a property.
 *
 * By default, a property value is owned by the caller, i.e. in case of
 * a dynamic string property, the caller is responsible for allocating the
 * proper memory and initializing the PFValue pointer field.
 * In case of a static string property, the string is owned by the
 * library, so the PFValue does not need to be initialized. See also
 * PFValue documentation.
 *
 * \param port		Port number of the camera
 * \param p          	The property token whose value is to be requested
 * \param value      	Pointer to a PFValue which contains the property value
 *                  	after successful return of this function. Note that
 *                  	in case of a non static string property, the property
 *                  	value must be initialized first.
 *
 * \return           	A standard return code
 *
 **/
APIFUNC(int, Device_GetProperty, (int port, TOKEN p, PFValue *value))

	
/**
 * Get the value of a property by string
 *
 *
 * \param port		Port number of the camera
 * \param p          	The property token whose value is to be requested
 * \param outs       	Pointer to a string which contains the property value
 *                   	string conversion after successful return of this function.
 *                   	The caller must reserve a string of the length of the
 *                   	expected value string.
 * \param len        	Length of the string that was reserved by caller
 *
 * \note             	This function only applies to simple datatypes, resp.
 *                   	only displays debugging information about some
 *                   	complex data types.
 *
 * \return          	The standard return code
 *
 **/
APIFUNC(int, Device_GetProperty_String, (int port, TOKEN p, char *outs, int len))	


/**
 * Set the value of a property.
 *
 * \param port		Port number of the camera
 * \param p          	The property token whose value is to be requested
 * \param value      	Pointer to a PFValue containing the value to be set.
 *
 * \return           	The standard return code
 *
 **/
APIFUNC(int, Device_SetProperty, (int port, TOKEN p, PFValue *value))

	
/**
 * Set the value of a property by string.
 *
 * This function parses a string and tries to match it to the
 * specified property. 
 *
 * \param port		Port number of the camera
 * \param p          	The property token whose value is to be set
 * \param string     	Pointer to a string containing the value to be set.
 *
 * \return           	The standard return code
 *
 **/
APIFUNC(int, Device_SetProperty_String, (int port, TOKEN p, const char *string))	


/* \} */

////////////////////////////////////////////////////////////////////////////
/* Non exposed low level burst read/write commands */
APIFUNC(int, Write, (int port, unsigned short addr, const unsigned char *buf, unsigned int size))
APIFUNC(int, Read, (int port, unsigned short addr, unsigned char *buf, int size))

APIFUNC(int, Write2, (int port, unsigned long addr, const unsigned char *buf, unsigned int size))
APIFUNC(int, Read2, (int port, unsigned long addr, unsigned char *buf, int size))

/**
 * \defgroup Misc Auxiliary and miscellaneous functions/macros
 *
 * Auxiliary functions for handling & reporting errors.
 *
 */

/**
 * \defgroup ErrorHandling Error Handling
 *
 * Auxiliary functions for handling & reporting errors.
 *
 */

/**
 * \ingroup ErrorHandling
 * 
 **/

/**
 * Verbose error string query of an error code.
 *
 * \param error          The error code returned by a function.
 *
 * \return               Pointer to a constant string containing the error
 *                       message to the specified error code.
 *
 **/
APIFUNC(const char *, GetErrorString, (int error))


/**
 * \defgroup Aux Auxiliary functions
 *
 *
 */

/** \ingroup Aux
 *
 */

APIFUNC(int, Value_ToString, (PFValue *val, char *outs, int len))

	
/** \ingroup Aux
 * Sets camera feedback function
 *
 * The feedback function must return a value = 0, if the caller should
 * proceed. For example, a GUI based feedback routine may allow the user
 * to cancel the procedure by hitting the ESC key.
 * If the procedure was cancelled, the caller returns #PFERR_CANCEL.
 *
 * \param func pointer to a feedback function of type FeedbackFuncP
 *             If equal 0, the default dummy routine (no feedback) is set.
 * \return     the previous feedback function pointer
 */
APIFUNC(FeedbackFuncP, SetFeedback, (int port, FeedbackFuncP func))

/** \ingroup Aux
 * Flush serial port
 *
 * \param port		Port number of the camera
 *
 * \return		The standard return code
 */
APIFUNC(int, FlushPort, (int port))

