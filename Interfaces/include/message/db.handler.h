/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			kir, ged, es, hs
 *  @date			2009
 *  @brief			Database message interface. Provides data stored in db, e.g. product specific data.
 */

#ifndef DB_HANDLER_H_
#define DB_HANDLER_H_

#include  "Poco/UUID.h"
#include  "Poco/Process.h"

#include  "server/handler.h"
#include  "message/db.interface.h"

namespace precitec
{
namespace interface
{

	template <>
	class TDb<MsgHandler> : public Server<MsgHandler>, public TDbMessageDefinition
	{
	public:
		MSG_HANDLER(TDb );

		typedef Poco::UUID	PocoUUID;

		void registerCallbacks()
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			REGISTER_MESSAGE(GetDBInfo, getDBInfo );
			REGISTER_MESSAGE(GetProductList, getProductList);
			REGISTER_MESSAGE(GetMeasureTasks, getMeasureTasks);
			REGISTER_MESSAGE(GetGraph, getGraph);
			REGISTER_MESSAGE(GetFilterParameter, getFilterParameter);
			REGISTER_MESSAGE(GetParameterSatz, getParameterSatz);
			REGISTER_MESSAGE(GetProductParameter, getProductParameter);
			REGISTER_MESSAGE(GetHardwareParameterSatz, getHardwareParameterSatz);
			REGISTER_MESSAGE(GetHardwareParameter, getHardwareParameter);
			REGISTER_MESSAGE(GetEinsDParameter, getEinsDParameter);
			REGISTER_MESSAGE(GetParameterSatzForAllFilters, getParameterSatzForAllFilters);
            REGISTER_MESSAGE(GetProductCurves, getProductCurves);
            REGISTER_MESSAGE(GetReferenceCurveSet, getReferenceCurveSet);

		}

		void getDBInfo(Receiver &receiver)
		{
			std::string info = getServer()->getDBInfo();
			receiver.marshal(info);
			receiver.reply();

		}

		void getProductList(Receiver &receiver)
		{
			PocoUUID stationID; receiver.deMarshal(stationID);
			ProductList productList = getServer()->getProductList( stationID );

			receiver.marshal(productList);
			receiver.reply();
		}
		void getMeasureTasks(Receiver &receiver)
		{
			PocoUUID stationID; receiver.deMarshal(stationID);
			PocoUUID productID; receiver.deMarshal(productID);
			MeasureTaskList measureTaskList = getServer()->getMeasureTasks( stationID, productID );
			receiver.marshal(measureTaskList);
			receiver.reply();
		}
		void getGraph(Receiver &receiver)
		{
			PocoUUID measureTaskID; receiver.deMarshal(measureTaskID);
			PocoUUID graphID; receiver.deMarshal(graphID);
			GraphList graphList = getServer()->getGraph( measureTaskID, graphID );
			receiver.marshal(graphList, graphList.size());
			receiver.reply();
		}
		void getFilterParameter(Receiver &receiver)
		{
			PocoUUID filterID; receiver.deMarshal(filterID);
			PocoUUID measureTaskID; receiver.deMarshal(measureTaskID);
			ParameterList parameterList = getServer()->getFilterParameter( filterID, measureTaskID );
			receiver.marshal(parameterList, parameterList.size());
			receiver.reply();
		}

		void getParameterSatz(Receiver &receiver)
		{
			PocoUUID parametersatzID; receiver.deMarshal(parametersatzID);
			PocoUUID filterInstanceID; receiver.deMarshal(filterInstanceID);
			ParameterList parameterList = getServer()->getParameterSatz( parametersatzID, filterInstanceID);
			receiver.marshal(parameterList, parameterList.size());
			receiver.reply();
		}

		void getProductParameter(Receiver &receiver)
		{
			PocoUUID productID; receiver.deMarshal(productID);
			ParameterList parameterList = getServer()->getProductParameter( productID);
			receiver.marshal(parameterList, parameterList.size());
			receiver.reply();
		}

		void getHardwareParameterSatz(Receiver &receiver)
		{
			PocoUUID id; receiver.deMarshal(id);
			ParameterList parameterList = getServer()->getHardwareParameterSatz( id);
			receiver.marshal(parameterList, parameterList.size());
			receiver.reply();
		}

		void getHardwareParameter(Receiver &receiver)
		{
			PocoUUID parametersatzID; receiver.deMarshal(parametersatzID);
			Key key; receiver.deMarshal(key);
			ParameterList parameterList = getServer()->getHardwareParameter( parametersatzID, key);
			receiver.marshal(parameterList, parameterList.size());
			receiver.reply();
		}

		void getEinsDParameter(Receiver &receiver)
		{
			PocoUUID id; receiver.deMarshal(id);
			Product1dParameter parameter = getServer()->getEinsDParameter(id);
			receiver.marshal(parameter);
			receiver.reply();
		}

		void getParameterSatzForAllFilters(Receiver &receiver)
		{
			PocoUUID measureTaskID; receiver.deMarshal(measureTaskID);
			auto parameterList = getServer()->getParameterSatzForAllFilters(measureTaskID);
			receiver.marshal(parameterList);
			receiver.reply();
		}

        void getProductCurves(Receiver &receiver)
        {
            PocoUUID id; receiver.deMarshal(id);
            ProductCurves parameter = getServer()->getProductCurves(id);
            receiver.marshal(parameter);
            receiver.reply();
        }

        void getReferenceCurveSet(Receiver& receiver)
        {
            PocoUUID productId; receiver.deMarshal(productId);
            PocoUUID referenceId; receiver.deMarshal(referenceId);
            ReferenceCurveSet parameter = getServer()->getReferenceCurveSet(productId, referenceId);
            receiver.marshal(parameter);
            receiver.reply();
        }

	private:
		TDb<AbstractInterface> * getServer()
		{
			return server_;
		}




	};

} // namespace interface
} // namespace precitec

#endif /*DB_HANDLER_H_*/
