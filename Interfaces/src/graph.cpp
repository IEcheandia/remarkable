/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		kir, hs
 * 	@date		2010
 * 	@brief		classes Component, FilterParameter, InPipe, OutPipe, Filter, Graph
 */

#include "system/types.h"
#include "common/graph.h"
#include "message/messageBuffer.h"

namespace precitec
{
	using namespace system::message;
	namespace interface
	{
		///
		/// Dies ist die Factoryklasse zum erzeugen der FilterParameter
		/// Es koennen saemtliche Typen die in Types definiert sind instanziert werden.

		// aus types enum Types { TChar, TByte, TInt, TBool, TFloat, TString, TNumTypes};

		template <class T>
		FilterParameter* CreateFilterParameter(MessageBuffer const &buffer)
		{
			return new TFilterParameter<T>( buffer );
		}
		typedef FilterParameter* (*FilterParameterFactory)(MessageBuffer const &buffer);


		FilterParameterFactory filterParameterFactoryList[TNumTypes] = {
			CreateFilterParameter<char>,
			CreateFilterParameter<byte>,
			CreateFilterParameter<int>,
			CreateFilterParameter<unsigned int>,
			CreateFilterParameter<bool>,
			CreateFilterParameter<float>,
			CreateFilterParameter<double>,
			CreateFilterParameter<std::string>
		};

		FilterParameter* FilterParameter::create(int type, MessageBuffer const &buffer )
		{
			return filterParameterFactoryList[type](buffer);
		}

		std::string FilterParameter::typeToStr() const
		{
			switch ( type_ )
			{
				case TChar 		: return "char"; break;
				case TByte 		: return "byte"; break;
				case TInt 		: return "int"; break;
				case TUInt 		: return "uint"; break;
				case TBool 		: return "bool"; break;
				case TFloat 	: return "float"; break;
				case TDouble 	: return "double"; break;
				case TString 	: return "string"; break;
				default: return "?";
			}
			return "?";
		}

		Poco::DynamicAny FilterParameter::any() const
		{
			switch ( type_ )
			{
				case TChar 		: return Poco::DynamicAny( value<char>() ); break;
				case TByte 		: return Poco::DynamicAny( value<byte>() ); break;
				case TInt 		: return Poco::DynamicAny( value<int>() ); break;
				case TUInt 		: return Poco::DynamicAny( value<unsigned int>() ); break;
				case TBool 		: return Poco::DynamicAny( value<bool>() ); break;
				case TFloat 	: return Poco::DynamicAny( value<float>() ); break;
				case TDouble 	: return Poco::DynamicAny( value<double>() ); break;
				case TString 	: return Poco::DynamicAny( value<std::string>() ); break;
				default: return Poco::DynamicAny();
			}
			return Poco::DynamicAny();
		}

		std::ostream& operator<<( std::ostream& os, const Graph& graph )
		{
			os << "graph id=" << graph.id().toString() << " component path='" << graph.pathComponents() << "'";

			ComponentList const& components = graph.components();
			for (ComponentList::const_iterator comp=components.begin(); comp!=components.end(); ++comp)
			{
				os << "\tcomponent id=" << comp->id().toString() << " filename=" << comp->filename() << std::endl;
			}

			os << std::endl;

			FilterList const& filters = graph.filters();
			// Durch saemtliche Filter traversieren
			for (FilterList::const_iterator filter=filters.begin(); filter!= filters.end(); ++filter)
			{
				os << "\t\tfilter instanceID="
				<< filter->instanceID().toString()
				<< " name=" << filter->name()
				<< " component=" << filter->component().toString() << std::endl;

				InPipeList const& inPipes = filter->inPipeList();
				for (InPipeList::const_iterator inPipe=inPipes.begin(); inPipe!= inPipes.end(); ++inPipe)
				{
					os << "\t\t\tinPipe name=" << inPipe->name()
					<< " sender=" << inPipe->sender().toString()
					<< " tag=" << inPipe->tag()
					<< " group=" << inPipe->group() << std::endl;
				}

				OutPipeList const& outPipes = filter->outPipeList();
				for (OutPipeList::const_iterator outPipe=outPipes.begin(); outPipe!= outPipes.end(); ++outPipe)
				{
					os << "\t\t\toutPipe name=" << outPipe->name()
					<< " contentType=" << outPipe->contentType()
					<< " tag=" << outPipe->tag() << " type=" 	<< outPipe->type() << std::endl;
				}

				ParameterList const& params = filter->parameterList();
				for (ParameterList::const_iterator param=params.begin(); param!= params.end(); ++param)
				{
					os << "\t\t\tparameter " << *(*param) << " value:" <<  (*param)->any().convert<std::string>() << std::endl;
				}

			}
			return os;
		}

	}
}
