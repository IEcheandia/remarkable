#ifndef SERIALIZER_H_
#define SERIALIZER_H_


#include <vector>

#include "SystemManifest.h"

#include "Poco/UUID.h"
#include "Poco/SharedPtr.h"
#include "system/templates.h"
#include "message/messageBuffer.h"

inline std::ostream &operator <<(std::ostream &os, Poco::UUID const& id) {
	os << id.toString(); return os;
}

namespace precitec 	{
namespace system 	{
namespace message 	{


	enum SerializeMethode { PodMethod, ClassMethod, PolymorphicMethod, SharedPtrMethod, StdVectorMethod,
							// int a[n],	 class a[n],        SharedPtr<Serializable>[n] ,    SharedPtr<class a[n]>
							PodVectorMethod, ClassVectorMethod, PolymorphicVectorMethod, SharedPtrVectorMethod};

	template <class T, SerializeMethode M>
	struct Serializer;

	//enum TransferTypes { Global=0, InterProcess=1, IntraProcess=2 };

	const bool Single(false);
	const bool Vector(true);

	template <typename T, bool IsArray= false>
	struct FindSerializer;


	class SYSTEM_API Serializable
	{
	public:
		enum { NullPtrType=-1 };

		Serializable() {}
		virtual ~Serializable() {}

		virtual void serialize	( MessageBuffer &buffer ) const = 0;
		virtual void deserialize( MessageBuffer const&buffer ) = 0;

		/// serialisiert einen POD/Klasse(von Serializable abgeleitet) oder einen SharedPtr der auf Serializable zeigt
		template <class T>
		void marshal( MessageBuffer &buffer, T const& value) const {
			//std::cout << "Serializable marshal" << std::endl;
			Serializer<T, FindSerializer<T, Single>::Value>::serialize( buffer, value );
		}

		/// serialisiert eine Array von POD/Klasse(von Serializable abgeleitet) oder einen SharedPtr der auf Serializable zeigt
		template <class T>
		void marshal( MessageBuffer &buffer, T const& value, std::size_t length) const {
			Serializer<T, FindSerializer<T, Vector>::Value>::serialize( buffer, value, length);
		}

		/// deserialisiert einen POD/Klasse(von Serializable abgeleitet) oder einen SharedPtr der auf Serializable zeigt
		template <class T>
		void deMarshal( MessageBuffer const&buffer, T& value) const {
			//std::cout << "Serializable demarshal value" << std::endl;
			Serializer<T, FindSerializer<T, Single>::Value>::deserialize(	buffer, value );
		}

		/// fuer Initialisierungslisten im CTor
		template <class T>
		T deMarshal( MessageBuffer const&buffer) const {
			//std::cout << "Serializable demarshal" << std::endl;
			T value;
			Serializer<T, FindSerializer<T, Single>::Value>::deserialize(	buffer, value );
			return value;
		}

		/// serialisiert eine Array von POD/Klasse(von Serializable abgeleitet) oder einen SharedPtr der auf Serializable zeigt
		template <class T>
		void deMarshal( MessageBuffer const&buffer, T& value, std::size_t length) const
		{
			//std::cout << "Serializable demarshal value, length" << std::endl;
			Serializer<T, FindSerializer<T, Vector>::Value>::deserialize( buffer, value, length );
		}

		// nun folgen Debug-Varianten, die noc zusatzinfo einblenden
		/// deserialisiert einen POD/Klasse(von Serializable abgeleitet) oder einen SharedPtr der auf Serializable zeigt
		template <class T>
		void deMarshal( MessageBuffer const&buffer, T& value, PvString const& name) const {
			Serializer<T, FindSerializer<T, Single>::Value>::deserialize(	buffer, value );
		}

		/// fuer Initialisierungslistenmim CTor
		template <class T>
		T deMarshal( MessageBuffer const&buffer, PvString const& name) const {
			T value;
			Serializer<T, FindSerializer<T, Single>::Value>::deserialize(	buffer, value );
			return value;
		}

		/// serialisiert eine Array von POD/Klasse(von Serializable abgeleitet) oder einen SharedPtr der auf Serializable zeigt
		template <class T>
		void deMarshal( MessageBuffer const&buffer, T& value, int length, PvString const& name) const {
			Serializer<T, FindSerializer<T, Vector>::Value>::deserialize( buffer, value, length );
			std::cout << value << std::endl;
		}
	};


	class SerializerTool : public  Serializable
	{
		public:
		SerializerTool() {}
		virtual ~SerializerTool() {}

		virtual void serialize	( MessageBuffer &buffer ) const {}
		virtual void deserialize( MessageBuffer const&buffer ) {}
	};



	template <class T>
	struct Serializer<T, PodMethod> // int a -> T=int
	{
		static void serialize(	MessageBuffer &buffer, T const& v)
		{
			marshalBuffer(v, buffer);
		}

		static void deserialize( MessageBuffer const& buffer, T &v)
		{
			deMarshalBuffer(v, buffer);
		}
	};

	template <class T>
	struct Serializer<T, PodVectorMethod>  // int a[n] -> T=int[0]
	{
		typedef typename DeRefPtr<T>::RefType rt;

		static void serialize(	MessageBuffer &buffer, T const& v, std::size_t length)
		{
			marshalBuffer(length, buffer);
			marshalBuffer(v, length * sizeof(rt), buffer);
		}

		static void deserialize( MessageBuffer const&buffer, T &v, std::size_t size)
		{
			std::size_t length(0);
			deMarshalBuffer(length, buffer);
			deMarshalBuffer(v, iMin(size, length) * sizeof(rt), buffer);
			if (size < length) buffer.skip((length - size) * sizeof(rt));
		}
	};


	template <class T>
	struct Serializer<T, SharedPtrMethod> // ! IsDerivedFromSerializeable
	{
		static void serialize(	MessageBuffer &buffer, T const& v)
		{
			marshalBuffer(*v, buffer);
		}

		static void deserialize( MessageBuffer const& buffer, T &v)
		{
			typedef typename SharedPtrType<T>::Type TypeOfSharedPtr;
			TypeOfSharedPtr *ptr = new TypeOfSharedPtr;
			deMarshalBuffer(*ptr, buffer);
			v = T(ptr);
		}
	};


	template <class T>
	struct Serializer<T, SharedPtrVectorMethod> // ! IsDerivedFromSerializeable
	{
		static void serialize(	MessageBuffer &buffer, T const& v, std::size_t length)
		{
			marshalBuffer(length, buffer);
			marshalBuffer(*v, length * sizeof(T*), buffer);
		}

		static void deserialize( MessageBuffer const& buffer, T &v, std::size_t size)
		{
			std::size_t length(0);
			deMarshalBuffer(length, buffer);
			if (length > 0)
			{
				typedef typename SharedPtrType<T>::Type TypeOfSharedPtr;
				TypeOfSharedPtr *ptr = new TypeOfSharedPtr[length];
				deMarshalBuffer(*ptr, length * sizeof(T*), buffer);
				v = T(ptr);
			}
		}
	};


	template <class T>
	struct Serializer<T, ClassMethod> // IsDerivedFromSerializeable
	{
		static void serialize( MessageBuffer &buffer, T const& v )
		{
			v.serialize( buffer );
		}

		static void deserialize( MessageBuffer const& buffer, T &v )
		{
			v.deserialize( buffer);
		}
	};

	template <class T>
	struct Serializer<T, ClassVectorMethod> // class Serializable[] -> T=Serializable[0]
	{
		typedef T const * const_pointer;
		typedef T * pointer;

		static void serialize( MessageBuffer &buffer, T const& v, std::size_t length)
		{
			const_pointer ptr = &v;
			for (std::size_t i = 0; i<length; ++i, ++ptr) {
				ptr->serialize(buffer);
			}
		}

		static void deserialize( MessageBuffer const&buffer, T &v, std::size_t length )
		{
			pointer ptr=&v;
			for(std::size_t i=0;i<length;++i, ++ptr)
				ptr->deserialize(buffer);
		}

	};


	template <class T>
	struct Serializer<T, PolymorphicMethod> // IsDerivedFromSerializeable
	{
		// T = SharedPtr <XY>
		static void serialize( MessageBuffer &buffer, T const& v )
		{
			// wenn der Ptr Null ist, wird -1 gespeichert
			if (v.isNull())
				marshalBuffer(int(Serializable::NullPtrType), buffer);
			else
			{
				marshalBuffer(v->type(), buffer);
				v->serialize( buffer );
			}
		}

		// T = SharedPtr <XY>
		static void deserialize( MessageBuffer const&buffer, T &v )
		{
			int type = Serializable::NullPtrType;
			deMarshalBuffer(type, buffer);

			// is it a NullPtr?
			if (type == Serializable::NullPtrType)
			{
				v = T(NULL);
				return;
			}

			// SharedPtr zeigt auf eine abgeleitete Klasse die wieder hergestellt werden soll. Dazu wird
			// im ersten Schritt die Klassen ID ausgelesen und danach via Factory Funktion die Klasse instanziert.
			// Die Factory Funktion wird in der Basisklasse realisiert und muss create heissen.
			//
			// Damit Polymorphic funktioniert muss die Basisklasse folgende Schnittstelle implementieren

			// !!Policie fuer abgeleitete Klassen sieht so aus!!
			//	class PolymorphicSerializable : public message::Serializable
			//	{
			//	public:
			//		PolymorphicSerializable() {}
			//		virtual ~PolymorphicSerializable() {}
			//		virtual unsigned int type() const {...};		// Liefert den Type
			//		static PolymorphicSerializable * create(int type, buffer) { ...}	// Erzeugt via Factory eine neue Version
			//	};

			typedef typename SharedPtrType<T>::Type TypeOfSharedPtr;	// Type aus Shared Ptr bestimmen
			v = T( TypeOfSharedPtr::create( type, buffer ) );			// Erzeuge Instanz von Type mit SharedPtr ref
		}
	};


	template <class T>
	struct Serializer<T, PolymorphicVectorMethod> // T = SharedPtr < Serializable >[n]
	{
		static void serialize( MessageBuffer &buffer, T const& v, std::size_t length )
		{
			T const *ptr = &v;
			marshalBuffer(length, buffer);
			for(std::size_t i=0; i<length; ++i, ++ptr)
			{
				if ((*ptr).get() == nullptr) {
					marshalBuffer(int(Serializable::NullPtrType), buffer);
				} else {
					marshalBuffer((*ptr)->type(), buffer);
					(*ptr)->serialize( buffer );
				}
			}
		}

		static void deserialize( MessageBuffer const&buffer, T &v, std::size_t size )
		{
			std::size_t length(0);
			deMarshalBuffer(length, buffer);
			length = (length < size)?length:size;

			T *ptr = &v;

			// Jedes fuer sich deserialisieren
			for(std::size_t i=0;i<length;++i, ++ptr)
			{
				int type = Serializable::NullPtrType;
				deMarshalBuffer(type, buffer);

				// is it a NullPtr?
				if (type != Serializable::NullPtrType)
				{
					typedef typename SharedPtrType<T>::Type TypeOfSharedPtr;	// Type aus Shared Ptr bestimmen
					(*ptr) = T(TypeOfSharedPtr::create( type, buffer ));			// Erzeuge Instanz von Type mit SharedPtr ref
				}
			}
		}
	};



	template <class T>
	struct Serializer<T, StdVectorMethod>
	{
		typedef typename T::value_type value_type;
		typedef typename std::vector<value_type> vector;
		typedef typename vector::const_reference cref;

		// T = vector<XY>
		static void serialize( MessageBuffer &buffer, T const& v )
		{
			// Ganze Liste schreiben
			serialize( buffer, v, v.size() );
		}

		static void serialize( MessageBuffer &buffer, T const& v, std::size_t length)
		{
			marshalBuffer(length, buffer);
			if (length > 0)	{
				SerializerTool tool;
				value_type const * ptr = &(*(v.begin()));
				tool.marshal(buffer, *ptr, length);
			}
		}

		static void deserialize( MessageBuffer const &buffer, T &v )
		{
			// Laenge der Liste ist noch unbekannt
			deserialize(buffer, v, 0);
		}

		static void deserialize( MessageBuffer const&buffer, T &v, std::size_t size )
		{
			std::size_t length(0);
			deMarshalBuffer(length, buffer);
			if (length > 0)
			{
				v.resize(length);
				SerializerTool tool;
				tool.deMarshal(buffer, *(v.begin()), length);
			}
		}

	};


	// Vector-Methoden
	template <typename T>
	struct FindSerializer<T, Vector> {
		static const SerializeMethode Value =
							(
									IsSharedPtr<T>::Value==true && IsSharedPtrFrom< T, Serializable >::Value == true
									? PolymorphicVectorMethod	// SharedPtr<Serializable>[]
									: IsSharedPtr<T>::Value==true
										? SharedPtrVectorMethod		// SharedPtr<class[]>
										: IsSubClassOf<T, Serializable >::Value==true
											? ClassVectorMethod		// class[]
											: IsStdVector<T>::Value==true
												? StdVectorMethod	// std::vector<class>
												: PodVectorMethod	// pod[]
							);
	};


	// Nicht-Vector-Methoden
	template <typename T>
	struct FindSerializer<T, Single> {
		static const SerializeMethode Value =
							(
								IsSharedPtr<T>::Value==true && IsSharedPtrFrom< T, Serializable >::Value == true
								? PolymorphicMethod
								: IsSharedPtr<T>::Value==true
									? SharedPtrMethod
									: IsSubClassOf<T, Serializable >::Value==true
										? ClassMethod
										: IsStdVector<T>::Value==true
											? StdVectorMethod
											: PodMethod
							);
	};


}
}
}

#endif /*SERIALIZER_H_*/
