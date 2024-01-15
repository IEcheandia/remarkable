///////////////////////////////////////////////////////////
//  SourceFilter.h
//  Implementation of the Class SourceFilter
//  Created on:      17-Okt-2007 17:45:04
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#if !defined(EA_9DB6BD8D_D2CC_4c7f_B735_A458937ABB64__INCLUDED_)
#define EA_9DB6BD8D_D2CC_4c7f_B735_A458937ABB64__INCLUDED_

#include "Poco/UUID.h"
#include "fliplib/Fliplib.h"
#include "fliplib/BaseFilter.h"
#include "fliplib/FilterControlInterface.h"

namespace fliplib
{
	/**
	 * Von der SinkFilter Klasse leiten alle SourceFilterimplementationen ab.
	 */
	class FLIPLIB_API SourceFilter : public BaseFilter
	{
	public:
		static const std::string PARAM_SOURCEFILTER_SENSORID;

	public:
		/**
		 * Konstruktor
		 *
		 * \param [in] name Name des Filters
		 * \param [in] GUID des InstanzeFilters (DB)
		 * \param [in] DeviceID. Kann auch ueber die Parameterschnittstelle gesetzt werden
		 */
		SourceFilter(const std::string& name);
		SourceFilter(const std::string& name, const Poco::UUID & filterID);
		SourceFilter(const std::string& name, const Poco::UUID & filterID, int sensorID);

		/**
		 * Destruktor
		 */
		virtual ~SourceFilter();

		/**
		 * @brief Get filter type.
		 * @return int FilterType enum - here SOURCE.
		 */
		virtual int getFilterType() const;

		/**
		 * Fire Source Filter
		 */
		virtual void fire() {};

		/**
		 * Get Device ID
		 */
		inline int getSensorID() const	{	return sensorID_;	}
		/**
		 * Set Device ID
		 *
		 * TODO: Remove function again once the Locate column in the field table is interpreted and send to QNX ...
		 */
		inline void setSensorID( int sensorID ) 	{	sensorID_ = sensorID;	}


	protected:
		/*virtual*/ void setParameter();		// set base filter parameter that all filters have in common

	private:
		// hide constructor
		SourceFilter();
		SourceFilter(const BaseFilter &);
		SourceFilter& operator=(SourceFilter&);

		int sensorID_;
	};

	class FLIPLIB_API ParameterFilter : public SourceFilter
	{
	};

}
#endif // !defined(EA_9DB6BD8D_D2CC_4c7f_B735_A458937ABB64__INCLUDED_)
