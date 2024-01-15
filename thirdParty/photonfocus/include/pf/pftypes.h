/** \file pftypes.h
 *
 * Photonfocus API data types
 *
 * These are datatypes that have to be exported to the API.
 *
 * WARNING: Changes in these data structures may affect
 * API binary compatibility. If this is the case, the major library
 * version number must be increased.
 */

#ifndef PFTYPES_H_INCLUDED_260872
#define PFTYPES_H_INCLUDED_260872

#define TOKEN unsigned long
#if defined (SM2_DSP)
	#define MAX_CAMERAS 7
#else
	#define MAX_CAMERAS 32
#endif
/** Invalid token value. When returned from pfProperty_ParseName(),
 * a property of that name was not found. Note that pfDevice_GetRoot()
 * may return the same value as INVALID_TOKEN, which is normal. */
#define INVALID_TOKEN 0


/** 
 *  Property data types
 */

typedef enum {
	PF_INVALID,
	PF_ROOT,     ///< ROOT NODE TYPE
	PF_INT,      ///< A 32 bit signed integer
	PF_FLOAT,    ///< A 4 byte float, single precision
	PF_BOOL,     ///< A boolean value (1: true, 0: false)
	PF_MODE,     ///< A mode value. Only the values in the choice are valid.
	PF_REGISTER, ///< A register value (direct register export)
	PF_STRING,   ///< A string value (constant or fixed lengh, 0 terminated)
	PF_BUFFER,   ///< A buffer value. The length is specified in the len field.
	// Meta types:
	PF_STRUCT,   ///< A struct value containing other properties
	PF_ARRAY,    ///< An array value, containing a struct or property
	// Special:
	PF_COMMAND,  ///< A command
	PF_EVENT    ///< An event node.
} PropertyType;


/** Property flags
 *
 * These flags are queried via pfProperty_GetFlags().
 *
 * All other bits are reserved for internal purposes.
 *
 */

#define F_PRIVATE  0x02    ///< Property is private
#define F_BIG      0x04    ///< Big endian, if Register node
#define F_RW       0x00    ///< Readable/Writeable
#define F_RO       0x10    ///< Readonly
#define F_WO       0x20    ///< Writeonly
#define F_INACTIVE 0x40    ///< Property is currently inactive


/** Run time information data type.
 *
 * This contains a value union and a type field
 *
 * A PFValue is to be initialized as follows:
 * 
   \code v.type = <type>; v.value.<valuefield> = <value> \endcode
 *
 * For example, see macros #SET_FLOAT, etc.
 *
 * In case of a string or buffer value, it is important to know that
 * the data buffer is not owned by the property structure, i.e. the
 * PFValue only serves as a descriptor and no value copying actually
 * happens.
 * Therefore, the programmer has to distinguish between two cases
 * before querying a property via pfDevice_GetProperty()
 *
 * \li The property is static (fixed length): The caller does not have to
 *     worry about pointer initialization of the PFValue before calling
 *     pfDevice_GetProperty()
 * \li The property is dynamic (read/write) and the caller *must*
 *     initialize the value buffer.
 *
 * For a dynamic string, the initialization would look as follows:
 *
\code 
#define BUF_LEN 80
	PFValue v;
	char buffer[BUF_LEN];

	v.type = PF_STRING;
        v.len = BUF_LEN;
	v.value.p = buffer;

	pfDevice_GetProperty(d, property, &v);
	// ...
	//
\endcode
 * Note that in this example, the (zero terminated) string has a maximum
 * length of 79.
 *
 * If there was not enough space reserved for the property buffer,
 * an error #PFERR_PROPERTY_SIZE_MATCH is returned from pfGetProperty()
 *
 * Data type checking is performed when setting properties.
 */

typedef struct 
{
	union{
		long i;		///< Union: Integer value
		float f;	///< Union: Float value
		void *p;	///< Union: Generic Pointer value
	} value;
	PropertyType type;	///< Value Type
	int len;		///< length of array, if type is a buffer
} PFValue;

/**
 * \ingroup Misc
 *
 * Initialize float PFValue */
#define SET_FLOAT(v, f) v.type = PF_FLOAT; v.value.f = f
/**
 * \ingroup Misc
 *
 * Initialize integer PFValue */
#define SET_INT(v, i)   v.type = PF_INT; v.value.f = i

/**
 * \ingroup Misc
 *
 * Initialize string PFValue */
#define SET_STRING(v, s, l)   v.type = PF_STRING; v.value.p = (void *) s; \
                              v.len = l;


/** 
 * \ingroup Misc
 *
 * Property callback function for recursive node walker.
 * See source code for detailed information.
 *
 * \param t      The current node token
 *
 * \return 1: success, continue walking
 *         0: quit walking.
 */

typedef int (PropCallback)(TOKEN t);

/**
 * Feedback function pointer definition
 *
 * See pfSetFeedback() in pfcam.h
 */

typedef int (*FeedbackFuncP)(int i);

#endif // PFTYPES_H_INCLUDED_260872
