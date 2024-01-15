///////////////////////////////////////////////////////////
//  Parameter.h
//  Implementation of the Class Parameter
//  Created on:      11-Dez-2007 17:22:24
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#if !defined(EA_FC39EC30_D0C8_48bc_94CA_1B320376E08E__INCLUDED_)
#define EA_FC39EC30_D0C8_48bc_94CA_1B320376E08E__INCLUDED_

#include <string>
#include <iostream>
#include "Poco/UUID.h"
#include "Poco/DynamicAny.h"

namespace fliplib
{
	/**
	 * Stellt eine Gruppe von Eigenschaften und Methoden bereit, mit denen Parameter verwaltet werden.
	 */
	class FLIPLIB_API Parameter
	{

	public:
		static const std::string TYPE_Int8;
		static const std::string TYPE_Int16;
		static const std::string TYPE_Int32;
		static const std::string TYPE_Int64;
		static const std::string TYPE_UInt8;
		static const std::string TYPE_UInt16;
		static const std::string TYPE_UInt32;
		static const std::string TYPE_UInt64;
		static const std::string TYPE_int;
		static const std::string TYPE_uint; // equivalent to TYPE_UInt32
		static const std::string TYPE_bool;
		static const std::string TYPE_float;
		static const std::string TYPE_double;
		static const std::string TYPE_char;
		static const std::string TYPE_string;
		static const std::string TYPE_text;

		/* ---------------- NIO parameter types ---------------- */
		// limits
		static const std::string TYPE_limitCoordinate;    // coordinate out of bound error
		static const std::string TYPE_limitPosSum;        // sum of min and max position errors

		/*
		static const std::string TYPE_limitWidthMin;      // min width error (lower limit)
		static const std::string TYPE_limitWidthMax;      // max width error (upper limit)
		static const std::string TYPE_limitMismatchMin;   // min mismatch error (lower limit)
		static const std::string TYPE_limitMismatchMin;   // max mismatch error (upper limit)
		*/
		// NIOs
		static const std::string TYPE_nioLocal_posMax;
		static const std::string TYPE_nioLocal_posMin;
		static const std::string TYPE_nioGlobal_position;
		static const std::string TYPE_nioLocal_width;
		static const std::string TYPE_nioLocal_mismatch;

	public:
		/**
		 * Initialisiert eine neue Instanz der Parameterklasse
		 *
		 * \param [in] name Name der Parameterinstanz
		 * \param [in] value Parameterwert.
		 */
		Parameter(const std::string& name, const std::string& type, const Poco::DynamicAny& value);
		Parameter(const std::string& name, const std::string& type, const std::string& value);

		/**
		 * allow copy
		 */
		Parameter(const Parameter& obj);

		/**
		 * Gibt die Resourcen der Instanz wieder frei
		 */
		virtual ~Parameter();

		/**
		 * allow asignment
		 */
		Parameter& operator=(const Parameter& obj);

		/**
		 * Liefert den Namen des Parameters
		 */
		const std::string& getName() const;


		/**
		 * Liefert den untypisierten Inhalt des Parameters
		 */
		Poco::DynamicAny& getValue();

		/**
		 * Liefert den untypisierten Inhalt des Parameters
		 */
		const Poco::DynamicAny& getValue() const;

		/**
		 * Speichert den Wert des Parameters
		 *
		 * \param [in] value Neuer Parameterwert
		 */
		void setValue(const Poco::DynamicAny& value);

		inline bool isUpdated() const { return updated_; }

		/**
		 * Liefert den Type des Parameters
		 * */
		const std::string& getType() const;

		/**
		 * convert to any type int i = parameter.convert<string>();
		 */
		template <typename T>
		T convert() const
		{
			return value_.convert<T>();
		}
		std::string toXml() const;

		static Poco::DynamicAny convertToType(const std::string& type, const Poco::DynamicAny& value);

		friend std::ostream &operator <<(std::ostream &os, Parameter const& p) {
			os << "Param:<" << p.name_ << " " << p.type_ << " : " << (p.convert<std::string>()).c_str() << ">"; return os;
		}
	private:
		Parameter();

		std::string name_;
		std::string type_;
		Poco::UUID id_;
		Poco::DynamicAny value_;
		mutable bool updated_;

		friend class ParameterContainer;

	};



} // namespace fliplib
#endif // !defined(EA_FC39EC30_D0C8_48bc_94CA_1B320376E08E__INCLUDED_)
