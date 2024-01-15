/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			kir, ged, es, hs
 *  @date			2009
 *  @brief			Database message interface. Provides data stored in db, e.g. product specific data.
 */

#ifndef DB_SERVER_H_
#define DB_SERVER_H_

#include <string>
#include <iostream>
#include <map> // wg HandlerList
#include "Poco/NamedMutex.h"
//#include "Poco/ScopedLock.h"
#include "Poco/Process.h"
#include "Poco/Path.h"

#include "message/db.interface.h"


namespace precitec
{
namespace interface
{

	template <>
	class TDb<MsgServer> : public TDb<AbstractInterface>
	{
	public:
		// liefert Servername und DB Name fuers Log
		virtual std::string getDBInfo() { return "Server:TDb<MsgServer> DB:?"; }
		// fordert eine Liste mit Produktdaten in Abhaengigkeit der Stationsid an
		virtual ProductList getProductList(Poco::UUID stationID) { return ProductList(100); }
		// fordet eine Liste mit Messaufgaben in Abhaengigkeit des Produkts an
		virtual MeasureTaskList getMeasureTasks(Poco::UUID stationID, Poco::UUID productID) { return MeasureTaskList(10); }
		// fordert einen Graph in Abhaengkigkeit der Messaufgabe an
		virtual GraphList getGraph(Poco::UUID measureTaskID, Poco::UUID graphID) { return GraphList(100); }
		// fordert aktualsierte FilterParameter an
		virtual ParameterList getFilterParameter(Poco::UUID filterID, Poco::UUID measureTaskID) { return ParameterList(100); }
		// fordert einen kompletten Parametersatz zu einer Filterinstanz an.
		virtual ParameterList getParameterSatz(Poco::UUID parametersatzID, Poco::UUID filterInstanceID) { return ParameterList(100); }
		// fordert alle Produktparameter an
		virtual ParameterList getProductParameter(Poco::UUID produktID) { return ParameterList(100); }
		// fordert einen HardwareParameterSatz an
		virtual ParameterList getHardwareParameterSatz(Poco::UUID id) { return ParameterList(100); }
		//fordert einen HWardware-Parameter an
		virtual ParameterList getHardwareParameter(Poco::UUID parametersatzID, Key key){ return ParameterList(100); }
		// fordert Kurve aus der DB
		virtual Product1dParameter getEinsDParameter(Poco::UUID id){ return Product1dParameter(); }

		FilterParametersList getParameterSatzForAllFilters(Poco::UUID parametersatzID) override { return FilterParametersList(100); }

        virtual ProductCurves getProductCurves(Poco::UUID id){ return ProductCurves(); }

        virtual ReferenceCurveSet getReferenceCurveSet(Poco::UUID productId, Poco::UUID referenceId){ return ReferenceCurveSet(); }
	};


} // namespace interface
} // namespace precitec


#endif /*DB_SERVER_H_*/
