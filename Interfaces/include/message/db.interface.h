/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			kir, ged, es, hs
 *  @date			2009
 *  @brief			Database message interface. Provides data stored in db, e.g. product specific data.
 */

#ifndef DB_INTERFACE_H_
#define DB_INTERFACE_H_


#include  "server/interface.h"
#include  "module/interfaces.h" // wg appId
#include  "protocol/protocol.info.h"
#include  "message/db.h"

namespace precitec
{
namespace interface
{

	template <int CallType>
	class TDb;

	/**
	 *	AbstrakteBasisklasse des Module Servers
	 * 	Der Modul-Server kommuniziert mit dem Module-Manager,
	 * 		d.h. er wartet auf Befehler dessselben
	 */
	template <>
	class TDb<AbstractInterface>
	{
	public:
		TDb() {}
		virtual ~TDb() {}
	public:
		typedef Poco::UUID	PocoUUID;
		// liefert Servername und DB Name fuers Log
		virtual std::string getDBInfo() = 0;
		// fordert eine Liste mit Produktdaten in Abhaengigkeit der Stationsid an
		virtual ProductList getProductList(PocoUUID stationID) = 0;
		// fordet eine Liste mit Messaufgaben in Abhaengigkeit des Produkts an
		virtual MeasureTaskList getMeasureTasks(PocoUUID stationID, PocoUUID productID) = 0;
		// fordert einen Graph in Abhaengkigkeit der Messaufgabe an
		virtual GraphList getGraph(PocoUUID measureTaskID, PocoUUID graphID) = 0;
		// fordert aktualsierte FilterParameter an
		virtual ParameterList getFilterParameter(PocoUUID filterID, PocoUUID measureTaskID) = 0;
		// fordert einen kompletten Parametersatz zu einer Filterinstanz an.
		virtual ParameterList getParameterSatz(PocoUUID parametersatzID, PocoUUID filterInstanceID) = 0;
		// fordert alle Produktparameter an
		virtual ParameterList getProductParameter(PocoUUID produktID) = 0;
		// fordert alle ProduktHardwareParameter an
		virtual ParameterList getHardwareParameterSatz(PocoUUID id) = 0;
		// fordert einen HwParameter an
		virtual ParameterList getHardwareParameter(PocoUUID parametersatzID, Key key) = 0;
		// fordert Kurve aus der DB
		virtual Product1dParameter getEinsDParameter(PocoUUID id) = 0;
        /**
         * Gets all parameters for the @p parameterSatzId.
         * The returned list is for all filters with their respective parameters.
         **/
        virtual FilterParametersList getParameterSatzForAllFilters(PocoUUID parametersatzID) = 0;

		virtual ProductCurves getProductCurves(PocoUUID id) = 0;

        virtual ReferenceCurveSet getReferenceCurveSet(PocoUUID productId, PocoUUID referenceId) = 0;
	};

    struct TDbMessageDefinition
    {
		MESSAGE(std::string, 		GetDBInfo, void);
		MESSAGE(ProductList, 		GetProductList, 			Poco::UUID);
		MESSAGE(MeasureTaskList, 	GetMeasureTasks,			Poco::UUID, Poco::UUID);
		MESSAGE(GraphList, 			GetGraph, Poco::UUID, 		Poco::UUID);
		MESSAGE(ParameterList, 		GetFilterParameter, 		Poco::UUID, Poco::UUID);
		MESSAGE(ParameterList, 		GetParameterSatz, 			Poco::UUID, Poco::UUID);
		MESSAGE(ParameterList, 		GetProductParameter,		Poco::UUID);
		MESSAGE(ParameterList, 		GetHardwareParameterSatz,	Poco::UUID);
		MESSAGE(ParameterList, 		GetHardwareParameter, 		Poco::UUID, Key);
		MESSAGE(Product1dParameter, 	GetEinsDParameter,		Poco::UUID);
		MESSAGE(FilterParametersList, GetParameterSatzForAllFilters, Poco::UUID);
        MESSAGE(ProductCurves, GetProductCurves, Poco::UUID);
		MESSAGE(ReferenceCurveSet, GetReferenceCurveSet, Poco::UUID, Poco::UUID);

		MESSAGE_LIST(
			GetDBInfo,
			GetProductList,
			GetMeasureTasks,
			GetGraph,
			GetFilterParameter,
			GetParameterSatz,
			GetProductParameter,
			GetHardwareParameterSatz,
			GetHardwareParameter,
			GetEinsDParameter,
			GetParameterSatzForAllFilters,
            GetProductCurves,
            GetReferenceCurveSet
		);
    };

	template <>
	class TDb<Messages> : public Server<Messages>, public TDbMessageDefinition
	{
	public:
		TDb() : info(system::module::Db, sendBufLen, replyBufLen, MessageList::NumMessages) {}
		MessageInfo info;
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 500*KBytes, replyBufLen = 10*MBytes };
	};

	typedef TDb<interface::AbstractInterface>	db_interface_t;

} // namespace interface
} // namespace precitec



#endif /*DB_INTERFACE_H_*/
