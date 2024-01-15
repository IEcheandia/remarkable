/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			kir, ged, es, hs
 *  @date			2009
 *  @brief			Database message interface. Provides data stored in db, e.g. product specific data.
 */

#ifndef DB_PROXY_H_
#define DB_PROXY_H_

#include <sstream>
// WM includes
#include "message/db.interface.h"
#include "server/proxy.h"
#include "module/moduleLogger.h"

namespace precitec
{
namespace interface
{
	using namespace  message;
	using namespace  system;

	template <>
	class TDb<MsgProxy> : public Server<MsgProxy>, public TDb<AbstractInterface>, public TDbMessageDefinition
	{
	public:
		TDb() : PROXY_CTOR(TDb), TDb<AbstractInterface>()
		{
		}

		/// normalerweise wird das Protokoll gleich mitgeliefert
		TDb(SmpProtocolInfo &p) : PROXY_CTOR1(TDb,  p), TDb<AbstractInterface>()
		{
		}

	public:

		virtual std::string getDBInfo()
		{
			INIT_MESSAGE(GetDBInfo);
			sender().send();
			std::string info; sender().deMarshal(info);
			return info;
		}

		virtual ProductList getProductList(PocoUUID stationID)
		{
			INIT_MESSAGE(GetProductList);
			sender().marshal(stationID);
			sender().send();
			ProductList ret;
			sender().deMarshal(ret);

			return ret;
		}

		virtual MeasureTaskList getMeasureTasks(PocoUUID stationID, PocoUUID productID)
		{
			INIT_MESSAGE(GetMeasureTasks);
			sender().marshal(stationID);
			sender().marshal(productID);
			sender().send();
			MeasureTaskList ret; sender().deMarshal(ret);
			return ret;
		}

		virtual GraphList getGraph(PocoUUID measureTaskID, PocoUUID graphID)
		{
			INIT_MESSAGE(GetGraph);
			sender().marshal(measureTaskID);
			sender().marshal(graphID);
			sender().send();
			GraphList ret; sender().deMarshal(ret, 0);
			return ret;
		}

		virtual ParameterList getFilterParameter(PocoUUID filterID, PocoUUID measureTaskId)
		{
			INIT_MESSAGE(GetFilterParameter);
			sender().marshal(filterID);
			sender().marshal(measureTaskId);
			sender().send();
			ParameterList ret; sender().deMarshal(ret, 0);
			return ret;
		}

		virtual ParameterList getParameterSatz(PocoUUID parametersatzID, PocoUUID filterInstanceID)
		{
			INIT_MESSAGE(GetParameterSatz);
			sender().marshal(parametersatzID);
			sender().marshal(filterInstanceID);
			sender().send();
			ParameterList ret; sender().deMarshal(ret, 0);
			return ret;
		}

		virtual ParameterList getProductParameter(PocoUUID produktID)
		{
			INIT_MESSAGE(GetProductParameter);
			sender().marshal(produktID);
			sender().send();
			ParameterList ret; sender().deMarshal(ret, 0);
			return ret;
		}

		virtual ParameterList getHardwareParameterSatz(PocoUUID id)
		{
			INIT_MESSAGE(GetHardwareParameterSatz);
			sender().marshal(id);
			sender().send();
			ParameterList ret; sender().deMarshal(ret, 0);
			return ret;
		}

		virtual ParameterList getHardwareParameter(PocoUUID parametersatzID, Key key)
		{
			INIT_MESSAGE(GetHardwareParameter);
			sender().marshal(parametersatzID);
			sender().marshal(key);
			sender().send();
			ParameterList ret; sender().deMarshal(ret, 0);
			return ret;
		}

		virtual Product1dParameter getEinsDParameter(PocoUUID id)
		{
			INIT_MESSAGE(GetEinsDParameter);
			sender().marshal(id);
			sender().send();
			Product1dParameter ret; sender().deMarshal(ret);
			return ret;
		}

        FilterParametersList getParameterSatzForAllFilters(Poco::UUID measureTaskID) override
        {
            INIT_MESSAGE(GetParameterSatzForAllFilters);
            sender().marshal(measureTaskID);
            sender().send();
            FilterParametersList ret;
            sender().deMarshal(ret);
            return ret;
        }

        virtual ProductCurves getProductCurves(PocoUUID id)
        {
            INIT_MESSAGE(GetProductCurves);
            sender().marshal(id);
            sender().send();
            ProductCurves ret;
            sender().deMarshal(ret);
            return ret;
        }

        virtual ReferenceCurveSet getReferenceCurveSet(PocoUUID productId, PocoUUID referenceId)
        {
            INIT_MESSAGE(GetReferenceCurveSet);
            sender().marshal(productId);
            sender().marshal(referenceId);
            sender().send();
            ReferenceCurveSet ret;
            sender().deMarshal(ret);
            return ret;
        }

	};


} // namespace interface
} // namespace precitec


#endif /*DB_PROXY_H_*/
