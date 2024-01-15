/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR, WOR, HS
 * 	@date		2012
 * 	@brief		Key value type.
 */

#ifndef DEVICE_H_
#define DEVICE_H_

// stl includes
#include <string>
#include <vector>
// project inlcudes
#include "system/types.h"
#include "system/templates.h"
#include "system/exception.h"
#include "message/messageBuffer.h"
#include "message/serializer.h"
#include "InterfacesManifest.h"
#include "module/moduleLogger.h"

namespace precitec {
	using system::message::MessageBuffer;
	using system::message::Serializable;
namespace interface {


typedef std::string 	Key;
typedef Types 	ValueTypes;


template<class T>
class TKeyValue;

/// TODO Doku
class INTERFACES_API KeyValue : public Serializable
{
public:
	KeyValue() : type_(TUnknown), key_("?"), handle_(-1), isHandleValid_(false) {}
	KeyValue(ValueTypes type) : type_(type), key_("?"), handle_(-1), isHandleValid_(false) {}
	KeyValue(ValueTypes type, Key key) : type_(type), key_(key), handle_(1), isHandleValid_(true) {}
	KeyValue(ValueTypes type, Key key, int handle) : type_(type), key_(key), handle_(handle), isHandleValid_(handle>=0) {}

	ValueTypes type() const { return type_; }		// Liefert den Type

	static KeyValue* create(int type, MessageBuffer const &buffer);

	virtual KeyValue* clone() const { return new KeyValue(*this); }

	template<class T>
	inline T value() const { return dynamic_cast<const TKeyValue<T>*>(this)->value(); }

	template<class T>
	inline T minima() const { return dynamic_cast<const TKeyValue<T>*>(this)->minima(); }

	template<class T>
	inline T setMinimum(T const& min) { dynamic_cast<TKeyValue<T>*>(this)->setMinimum(min); return dynamic_cast<const TKeyValue<T>*>(this)->minima(); }

	template<class T>
	inline T maxima() const { return dynamic_cast<const TKeyValue<T>*>(this)->maxima(); }

	template<class T>
	inline T setMaximum(T const& max) { dynamic_cast<TKeyValue<T>*>(this)->setMaximum(max); return dynamic_cast<const TKeyValue<T>*>(this)->maxima(); }

	template<class T>
	inline T defValue() const { return dynamic_cast<const TKeyValue<T>*>(this)->defValue(); }

	template<class T>
	inline void  setValue(T const& value) { dynamic_cast<TKeyValue<T>*>(this)->value(value); }


	Key key() const { return key_; }
	void setKey( Key p_oKey ) { key_ = p_oKey; }

	int handle() const { return handle_; }

	bool isHandleValid() const {return isHandleValid_;}

    bool isReadOnly() const
    {
        return m_readOnly;
    }

    void setReadOnly(bool readOnly)
    {
        m_readOnly = readOnly;
    }

    /**
     * @returns the recommended precision for representing floating point values in gui
     **/
    int precision() const
    {
        return m_precision;
    }
    /**
     * Sets the recommended precision for representing floating point values in gui to @p precision.
     * The default precision is @c 1.
     **/
    void setPrecision(int precision)
    {
        m_precision = precision;
    }

	virtual std::string toString() const  {wmLog(eDebug, "WARNING: Call to base not supported.\n"); return(" "); }

	virtual bool isValueValid() const { wmLog(eDebug, "WARNING: Call to base not supported.\n");  return false; } // should be pure

	virtual void resetToDefault()  { wmLog(eDebug, "WARNING: Call to base not supported.\n");  } // should be pure

	virtual void serialize ( MessageBuffer &buffer ) const
	{
		marshal(buffer, type_);
		marshal(buffer, key_);
		marshal(buffer, handle_);
		marshal(buffer, isHandleValid_);
        marshal(buffer, m_readOnly);
        marshal(buffer, m_precision);
	}

	virtual void deserialize( MessageBuffer const&buffer )
	{
		deMarshal(buffer, type_);
		deMarshal(buffer, key_);
		deMarshal(buffer, handle_);
		deMarshal(buffer, isHandleValid_);
        deMarshal(buffer, m_readOnly);
        deMarshal(buffer, m_precision);
	}

private:
	ValueTypes	 	type_;
	Key				key_;
	int				handle_;
	bool			isHandleValid_;
    bool m_readOnly = false;
    int m_precision = 1;

};

typedef Poco::SharedPtr<KeyValue> SmpKeyValue;

/// TODO Doku
template <class T>
class TKeyValue : public	KeyValue
{
public:
	TKeyValue() : KeyValue( FindTType<T>::TType) {}
	TKeyValue(Key key) : KeyValue( FindTType<T>::TType, key ) {}
	TKeyValue(Key key, T value) : KeyValue( FindTType<T>::TType, key ), value_( value ) { }
	TKeyValue(Key key, T value,T min,T max, T defVal, int precision = 1) : KeyValue( FindTType<T>::TType, key ), value_( value ),min_(min),max_(max),defVal_(defVal)
    {
        setPrecision(precision);
    }
	TKeyValue(MessageBuffer const& buffer) : KeyValue ( FindTType<T>::TType ) { deserialize( buffer ); }
	/*virtual*/ KeyValue* clone() const { return new TKeyValue(*this); }

	T value() const
	{
		if ( FindTType<T>::TType != this->type() ) 
        {
            std::ostringstream oMsg;
            oMsg << __FUNCTION__ << ": invalid value type: " << this->type() << " key: " << this->key();
            throw precitec::system::InvalidCastException(oMsg.str());
        }
		return value_;
	}
	T minima() const
	{
		return min_;
	}
	void setMinimum( T const& min )
    {
        min_ = min;
    }
	T maxima() const
	{
		return max_;
	}
	void setMaximum( T const& max )
    {
        max_ = max;
    }
	T defValue() const
	{
		return defVal_;
	}

	void value(T const& value) { value_ = value; }

	std::string toString() const override
	{
        return actualToString();
    }

	/**
	 * @brief	Checks if the given value lies between minimum and maximum.
	 * @return	bool			If the value lies between minimum and maximum.
	 */
	/*virtual*/ bool isValueValid() const {
		return min_ <= value_ && value_ <= max_;
	} // isValueValid

	/**
	 * @brief	Sets the value to the default value.
	 * @return	void
	 */
	/*virtual*/ void resetToDefault() {
		value_ = defVal_;
	} // resetToDefault

	virtual void serialize ( MessageBuffer &buffer ) const
	{
		KeyValue::serialize(buffer);
		marshal(buffer, value_);
		marshal(buffer, min_);
		marshal(buffer, max_);
		marshal(buffer, defVal_);
	}

	virtual void deserialize( MessageBuffer const&buffer )
	{
		KeyValue::deserialize(buffer);
		deMarshal(buffer, value_);
		deMarshal(buffer, min_);
		deMarshal(buffer, max_);
		deMarshal(buffer, defVal_);
	}
private:
	//siehe templates.h
	template<class T1 = T>
	std::string actualToString(typename std::enable_if<!std::is_convertible<T1, std::string>::value>::type* = 0) const { return key() + " = " + std::to_string(value_)
					+ ", min: " + std::to_string(min_) + ", max: " + std::to_string(max_) + ", default: " + std::to_string(defVal_); }

	template<class T1 = T>
	std::string actualToString(typename std::enable_if<std::is_convertible<T1, std::string>::value>::type* = 0) const { return key() + " = " + value_
					+ ", min: " + min_ + ", max: " + max_ + ", default: " + defVal_; }
	T	value_;
	T   min_;
	T   max_;
	T   defVal_;
}; // TKeyValue


class INTERFACES_API KeyHandle {
public:
	KeyHandle() : handle_(-1) {}
	KeyHandle(int num) : handle_(num) {}
	int handle() const { return handle_; }
private:
	int handle_;
}; // KeyHandle



typedef std::vector<SmpKeyValue> Configuration;
inline std::ostream &operator <<(std::ostream &os, Configuration const& c) {
	for (uInt i=0; i< c.size(); ++i) {
		os <<"--"<< c[i]->toString()<<std::endl;
	}
	return os;
} // operator <<



/**
 * @ingroup interface
 * @brief	Writes configuration to given file path.
 * @details	ATTENTION: Poco::XMLConfiguration writes only string, int, double and bool - which is a subset of precitec::Types.
 * 			Using casts may help to overcome this limitation. A saved configuration can be read:
 * 				AutoPtr<Util::XMLConfiguration> pConfIn(new XMLConfiguration("test.xml"));
 *				std::string date_time	= pConfIn->getString("date_time", "default");
 *				int product_type		= pConfIn->getInt("product_type", 0);
 * @param	p_rConfigFilePath	File path to a writable configuration file (xml).
 */
void INTERFACES_API writeToFile(const std::string &p_rFilePath, const Configuration &p_rConfiguration);



} // namespace interface
} // namespace precitec


#endif /*DEVICE_H_*/
