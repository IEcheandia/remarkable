/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		kir, hs
 * 	@date		2010
 * 	@brief		classes Component, FilterParameter, InPipe, OutPipe, Filter, Graph
 */

#ifndef GRAPH_H_
#define GRAPH_H_

#include <memory>
#include <utility>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include "geo/range.h"
#include "Poco/UUID.h"
#include "Poco/DynamicAny.h"
#include "system/types.h"
#include "system/exception.h"
#include "system/templates.h"
#include "system/stdImplementation.h"
#include "message/messageBuffer.h"
#include "message/serializer.h"
#include "system/templates.h"
#include "InterfacesManifest.h"

namespace precitec
{
#undef interface
namespace interface
{
	// Eine Komponente enthaelt eine Anzahl Filter. Jeder Filter beschrebt in welcher Komponente er implementiert worden ist.
	// Die Komponente zeit mit dem Relativen Pfad auf die konkrete DLL oder .so
	class INTERFACES_API Component : public system::message::Serializable
	{
		public:
			Component() {}
			Component(Poco::UUID id, std::string filename) : id_(id), filename_(filename) {}
			virtual ~Component() {}

			Poco::UUID 		const& id()			const 	{ return id_;		}		//<- Liefert die ID der Komponente
			std::string 	const& filename() 	const	{ return filename_;	}		//<- Liefert den Name und Pfad der Komppnente
			friend  std::ostream &operator <<(std::ostream &os, Component const& c) {
				os << "Component: " << c.filename_; return os;
			}
		public:
			virtual void serialize ( system::message::MessageBuffer &buffer ) const
			{
				marshal(buffer, id_);
				marshal(buffer, filename_);
		 	}

			virtual void deserialize( system::message::MessageBuffer const&buffer )
			{
				deMarshal(buffer, id_);
				deMarshal(buffer, filename_);
			}

		private:
			Poco::UUID	id_;				///<- DB ID der Komponente
			std::string filename_;		///<- Relativer Pfad auf die DLL oder .so
	};
	typedef std::vector<Component> ComponentList;

	template<class T>
    class TFilterParameter;

    /// FilterParameter koennen unterschiedlicher Typen sein. Diese Klasse ist fuer alle Parameter gleich. Der konkrete
    /// Wert steckt im Template
    class INTERFACES_API FilterParameter : public system::message::Serializable
    {
    	public:
    		FilterParameter() {}
    		FilterParameter (Types p_oType) : type_(p_oType) {}
    		FilterParameter (Poco::UUID p_oparameterID, std::string p_oName, Types p_oType, Poco::UUID p_oInstanceID, Poco::UUID p_oTypID) :
    			parameterID_(p_oparameterID), name_(p_oName), type_(p_oType), instanceID_(p_oInstanceID), typID_(p_oTypID) {}
    		virtual ~FilterParameter() {}


			static FilterParameter* create(int type, system::message::MessageBuffer const &buffer );	// Factoryobjekt in graph.cpp implementiert

			std::string 		typeToStr() const;
    		inline Types 		type() 					const { return type_; 		}		//<- Liefert den Type
			inline Poco::UUID 	const& parameterID()	const { return parameterID_;	}		//<- Liefert die InstanceID des FilterParamters
			inline std:: string const& name()			const { return name_;		}		//<- Liefert den Namen des FilterParameters
			inline Poco::UUID 	const& instanceID()		const { return instanceID_;	}	//<- Liefert die VariantInstanceID des Paramters (z.Beispiel zum Unterscheiden mehrere Instanzen eines VariantTyps siehe SummError)
			inline Poco::UUID 	const& typID()			const { return typID_;	}		//<- Liefert die typID des Paramters (z.Beispiel zum Unterscheiden mehrere Typen eines VariantTyps verschiedene SummError oder SingleError)

			template<class T>
			inline T value() const { return dynamic_cast<const TFilterParameter<T>*>(this)->value(); }

			// Liefert Value als Any
			Poco::DynamicAny any() const;
			virtual void print(std::ostream &os) const {
				os << "name: " << name_
				<< " instanceID:" << parameterID().toString()
				<< " type:" << typeToStr();
			}
			void printStd()
			{
				std::cout << "name: " << name_
				<< " instanceID:" << parameterID().toString()
				<< " type:" << typeToStr() << std::endl;
			}

		public:
			virtual void serialize ( system::message::MessageBuffer &buffer ) const
			{
				marshal(buffer, int(type_));
				marshal(buffer, parameterID_);
				marshal(buffer, name_);
				marshal(buffer, instanceID_);
				marshal(buffer, typID_);
		 	}

			virtual void deserialize( system::message::MessageBuffer const&buffer )
			{
				deMarshal(buffer, type_);
				deMarshal(buffer, parameterID_);
				deMarshal(buffer, name_);
				deMarshal(buffer, instanceID_);
				deMarshal(buffer, typID_);
			}

    	protected:
    		Poco::UUID 		parameterID_;
    		std::string 	name_;
    		Types			type_;
			Poco::UUID		instanceID_;
			Poco::UUID		typID_;
    };

	inline std::ostream &operator <<(std::ostream &os, FilterParameter const& c) {
		c.print(os);
		return os;
	}

	typedef std::shared_ptr<FilterParameter>							SpFilterParameter;
    typedef std::vector< SpFilterParameter >							ParameterList;
	typedef	std::map<Poco::UUID, interface::ParameterList>				paramSet_t;			///< All parmeters for one filter instance id
	typedef	std::map<Poco::UUID, paramSet_t>							paramSetMap_t;		///< All parmeters for one parameter set id

    /**
     * @short Small Container to transfer a filter id together with the ParameterList of the filter.
     **/
    class FilterParametersContainer : public system::message::Serializable
    {
    public:
        FilterParametersContainer() {}
        FilterParametersContainer(const Poco::UUID &filterID, const ParameterList &parameters)
            : filterID_(filterID)
            , parameters_(parameters)
        {
        }
        ~FilterParametersContainer() override {}

        const Poco::UUID &filterID() const
        {
            return filterID_;
        }

        const ParameterList &parameters() const
        {
            return parameters_;
        }

        void serialize( system::message::MessageBuffer &buffer ) const override
        {
            marshal(buffer, filterID_);
            marshal(buffer, parameters_);
        }

        void deserialize( system::message::MessageBuffer const&buffer ) override
        {
            deMarshal(buffer, filterID_);
            deMarshal(buffer, parameters_);
        }

    private:
        Poco::UUID filterID_;
        ParameterList parameters_;
    };
    typedef std::vector<FilterParametersContainer> FilterParametersList;

    // TFilterParameter enthaelt den Wert des Parameters. Alle anderen Informationen sind din FilterParameter gespeichert
    template<class T>
    class TFilterParameter : public FilterParameter
    {
    	public:
    		typedef T ParameterType;

    		TFilterParameter (
				Poco::UUID instanceID, 
				std::string name, T value, 
				Poco::UUID variantInstanceID, 
				Poco::UUID variantTypID) : FilterParameter (instanceID, name, FindTType<T>::TType, variantInstanceID, variantTypID), value_(value) {}
    		TFilterParameter(system::message::MessageBuffer const& buffer) : FilterParameter ( FindTType<T>::TType ) { deserialize( buffer ); }
    		virtual ~TFilterParameter () {}

    		// CCTor
			TFilterParameter(TFilterParameter const& rhs) : FilterParameter( rhs.parameterID_, rhs.name_, FindTType<T>::TType, rhs.instanceID_, rhs.typID_), value_( rhs.value_ )
			{
				if ( rhs.type() != this->type() ) {
					std::ostringstream oMsg;
					oMsg << __FUNCTION__ << ": cast from " << rhs.typeToStr() << " to " << typeToStr() << " not allowed";
					throw precitec::system::InvalidCastException(oMsg.str());
				}
			}

			virtual TFilterParameter &operator =(TFilterParameter const &rhs)
			{
				if ( rhs.type() != this->type() ) {
					std::ostringstream oMsg;
					oMsg << __FUNCTION__ << ": cast from " << rhs.typeToStr() << " to " << typeToStr() << " not allowed";
					throw precitec::system::InvalidCastException(oMsg.str());
				}
				value_ = rhs.value_;
				return *this;
			}
			virtual void print(std::ostream &os) const {
				FilterParameter::print(os);
				os << " value: " << value_;
			}
		public:
			virtual void serialize ( system::message::MessageBuffer &buffer ) const 	{ marshal(buffer, value_); 		FilterParameter::serialize(buffer);  	}
			virtual void deserialize( system::message::MessageBuffer const&buffer )	{ deMarshal(buffer, value_); 	FilterParameter::deserialize(buffer);	}


			T value() const
			{
				if ( FindTType<T>::TType != this->type() ) throw precitec::system::InvalidCastException("invalid value type");
				return value_;
			}
			inline void value(T value)	{ value_ = value;	} //<- Speicher den neuen Wert des Parameters

			// Liefert den Wert des Parameters
			T operator()() { return value_; }

    	private:
    		T value_;
    };

    // Die Klasse InPipe beschreibt einen Pipe Eingang eines Filters und deren Verbindungsinformationen
    class INTERFACES_API InPipe : public system::message::Serializable
    {
    	public:
    		InPipe() {}
    		InPipe (Poco::UUID sender, std::string name, int group) : sender_(sender), name_(name), group_(group), tag_("") {}
    		InPipe (Poco::UUID sender, std::string name, int group, std::string tag) : sender_(sender), name_(name), group_(group), tag_(tag) {}
    		~InPipe() {}

    		inline Poco::UUID const& sender()	const { return sender_; 	}	//<- Liefert die ID des SenderFilter
    		inline std::string const& name() const { return name_; 		}	//<- Liefert den Namen der Pipe
    		inline int group() 			const { return group_; 		} 	//<- Liefert die Groupierungsnummer
    		inline std::string const& tag()  const { return tag_; 		}	//<- Liefert die Optionale Tag Information der Pipe
  			friend  std::ostream &operator <<(std::ostream &os, InPipe const& c) {
  				os << "InPipe: " << c.name_; return os;
  			}
    	public:
    		virtual void serialize ( system::message::MessageBuffer &buffer ) const
			{
				marshal(buffer, sender_);
				marshal(buffer, name_);
				marshal(buffer, group_);
				marshal(buffer, tag_);
			}

			virtual void deserialize( system::message::MessageBuffer const&buffer )
			{
				deMarshal(buffer, sender_);
				deMarshal(buffer, name_);
				deMarshal(buffer, group_);
				deMarshal(buffer, tag_);
			}

    	private:
    		Poco::UUID	sender_;
    		std::string name_;
    		int			group_;
    		std::string tag_;
    };
    typedef std::vector<InPipe> InPipeList;


    enum PipeType { SynchronePipe, AsynchronePipe, NumPipeType };
    // Die Klasse OutPipe beschreibt einen Pipe Ausgang eines Filters.
    class INTERFACES_API OutPipe : public system::message::Serializable
    {
    	public:
    		OutPipe() {}
    		OutPipe (std::string name, PipeType type, std::string contentType) : name_(name), contentType_(contentType), type_(type), tag_("") {}
    		OutPipe (std::string name, PipeType type, std::string contentType, std::string tag) : name_(name), contentType_(contentType), type_(type), tag_(tag) {}
    		~OutPipe() {}

    		inline std::string	const& name() 			const { return name_; 			}	//<- Liefert den Namen der Pipe
    		inline std::string	const& contentType()	const { return contentType_; 	}	//<- Liefert den ContetType "string", "int", Image" etc
    		inline PipeType		type() 					const { return type_; 			} 	//<- Liefert den PipeType
    		inline std::string	const& tag()  			const { return tag_; 			}	//<- Liefert die Optionale Tag Information der Pipe

    		friend  std::ostream &operator <<(std::ostream &os, OutPipe const& op) {
					os << "oPipe: " << op.name_; return os;
				}
    	public:
    		virtual void serialize ( system::message::MessageBuffer &buffer ) const
			{
				marshal(buffer, name_);
				marshal(buffer, contentType_);
				marshal(buffer, type_);
				marshal(buffer, tag_);
			}

			virtual void deserialize( system::message::MessageBuffer const&buffer )
			{
				deMarshal(buffer, name_);
				deMarshal(buffer, contentType_);
				deMarshal(buffer, type_);
				deMarshal(buffer, tag_);
			}

    	private:
    		std::string name_;
    		std::string contentType_;
    		PipeType type_;
    		std::string tag_;
    };
    typedef std::vector<OutPipe> OutPipeList;

    // Die Klasse Filter enthaelt saemtliche Daten zum realien Filter, wie ID, InPipe, OutPipe, FilterParameters etc;
    class INTERFACES_API Filter : public system::message::Serializable
    {
    	public:
    		Filter () {}
    		Filter (Poco::UUID instanceID, std::string name, Poco::UUID component) : instanceID_(instanceID), name_(name), component_(component) {}
    		~Filter() {}


			Filter(Filter const& rhs) :
				instanceID_ (rhs.instanceID_),
				name_(rhs.name_),
				component_(rhs.component_),
				inPipeList_(rhs.inPipeList_),
				outPipeList_(rhs.outPipeList_),
				parameterList_(rhs.parameterList_)
			{
				// SharedPtr umkopieren
				//for(unsigned int i=0;i<parameterList_.size();i++) parameterList_[i] = rhs.parameterList_[i];
			}

			Filter& operator=(Filter const& rhs)
			{
				instanceID_ = rhs.instanceID_;
				name_ = rhs.name_;
				component_ = rhs.component_;
				inPipeList_ = rhs.inPipeList_;
				outPipeList_ = rhs.outPipeList_;
				parameterList_ = rhs.parameterList_;
				// SharedPtr umkopieren
				//for(unsigned int i=0;i<parameterList_.size();i++) parameterList_[i] = rhs.parameterList_[i];
				return *this;
			}

    	public:

    		inline Poco::UUID const& instanceID()		const { return instanceID_; }	//<- Liefert die ID des Filters
    		inline std::string const& name() 			const { return name_; 		}	//<- Liefert den Namen des Filters
    		inline Poco::UUID const& component() 		const { return component_; 	}	//<- Liefert die ID der Komponenten in der der Filter impl. ist

    		inline InPipeList const& inPipeList() 		const 	{ return inPipeList_; 		}
    		inline InPipeList & inPipeList() 		 			{ return inPipeList_; 		}
    		inline OutPipeList const& outPipeList() 	const 	{ return outPipeList_;		}
    		inline OutPipeList & outPipeList() 	 				{ return outPipeList_;		}
    		inline ParameterList const& parameterList() const 	{ return parameterList_;	}
    		inline ParameterList & parameterList()  			{ return parameterList_;	}
  			friend  std::ostream &operator <<(std::ostream &os, Filter const& c) {
  				os << "Filter: " << c.name_; return os;
  			}

            void setInPipeList(InPipeList &&list)
            {
                inPipeList_ = std::move(list);
            }

            void setParameterList(ParameterList &&list)
            {
                parameterList_ = std::move(list);
            }

    	public:
			virtual void serialize ( system::message::MessageBuffer &buffer ) const
			{
				marshal(buffer, instanceID_);
				marshal(buffer, name_);
				marshal(buffer, component_);
				marshal(buffer, inPipeList_);
				marshal(buffer, outPipeList_);
				marshal(buffer, parameterList_);
			}

			virtual void deserialize( system::message::MessageBuffer const&buffer )
			{
				deMarshal(buffer, instanceID_);
				deMarshal(buffer, name_);
				deMarshal(buffer, component_);
				deMarshal(buffer, inPipeList_);
				deMarshal(buffer, outPipeList_);
				deMarshal(buffer, parameterList_);
			}


    	private:
    		Poco::UUID 		instanceID_;
    		std::string 	name_;
    		Poco::UUID 		component_;

    		InPipeList inPipeList_;
    		OutPipeList outPipeList_;
    		ParameterList parameterList_;
    };
    typedef std::vector<Filter> FilterList;

	// Die Klasse Graph enthaelt eine Liste mit Filtern und Komponenten.
	class INTERFACES_API Graph : public system::message::Serializable
	{
		public:
			Graph () {}
			Graph (Poco::UUID const &instanceID) : instanceID_(instanceID), pathComponents_("") {}
			Graph (Poco::UUID const &instanceID, std::string pathComponents) : instanceID_(instanceID), pathComponents_(pathComponents) {}
			virtual ~Graph() {}

			Graph(Graph const& rhs) :
				instanceID_ (rhs.instanceID_),
				pathComponents_(rhs.pathComponents_),
				componentList_(rhs.componentList_),
				filterList_(rhs.filterList_)
			{}

			Graph& operator=(Graph const& rhs)
			{
				instanceID_ = rhs.instanceID_;
				pathComponents_ = rhs.pathComponents_;
				componentList_ = rhs.componentList_;
				filterList_ = rhs.filterList_;
				return *this;
			}

		public:
			inline ComponentList 	const& components() const	{ return componentList_; 		}	// Liste mit Komponenten
			inline ComponentList 	& components() 				{ return componentList_; 		}	// Liste mit Komponenten
			inline FilterList 		const& filters() const		{ return filterList_; 			}	// Liste mit Filter
			inline FilterList 		& filters() 				{ return filterList_; 			}	// Liste mit Filter
			inline Poco::UUID 	   const& id() const			{ return instanceID_; 			}	//<- DB ID Graph
			inline std::string const& pathComponents() const	{ return pathComponents_; 		}

            void setComponents(ComponentList &&list)
            {
                componentList_ = std::move(list);
            }
            void setFilters(FilterList &&list)
            {
                filterList_ = std::move(list);
            }

		public:
			virtual void serialize ( system::message::MessageBuffer &buffer ) const
			{
				marshal(buffer, instanceID_);
				marshal(buffer, pathComponents_);
				marshal(buffer, componentList_);
				marshal(buffer, filterList_);
			}

			virtual void deserialize( system::message::MessageBuffer const&buffer )
			{
				deMarshal(buffer, instanceID_);
				deMarshal(buffer, pathComponents_);
				deMarshal(buffer, componentList_);
				deMarshal(buffer, filterList_);
			}

		private:
			Poco::UUID 		instanceID_;
			std::string 	pathComponents_;
			ComponentList	componentList_;
			FilterList 		filterList_;

	};

	// Liste aller Module
	typedef std::vector<Graph> GraphList;

	INTERFACES_API std::ostream& operator<<( std::ostream& os, const Graph& g );


}	// precitec
}	// interface


#endif /*GRAPH_H_*/
