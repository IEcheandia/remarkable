/*
 * dbNotification.interface.h
 *
 *  Created on: 22.06.2010
 */

#ifndef DBNOTIFICATION_INTERFACE_H_
#define DBNOTIFICATION_INTERFACE_H_



#include  "server/interface.h"
#include  "module/interfaces.h" // wg appId
#include  "protocol/protocol.info.h"
#include  "event/dbNotification.h"
#include "message/device.h"

namespace precitec
{
	using namespace  system;
	using namespace  message;

namespace interface
{

	template <int CallType>
	class TDbNotification;

	enum DbStatus { eUnchanged, eProduct, eMeasureTask, eFilterParameter };

	/**
	 * Class that represents a single database change that was announced by the GUI.
	 */
	class DbChange
	{
	public:

		/**
		 * CTor.
		 */
		DbChange( DbStatus p_oStatus ) : m_oStatus( p_oStatus ),
			m_oProductID( Poco::UUID() ),
			m_oMeasureTaskID( Poco::UUID() ),
			m_oFilterID( Poco::UUID() )
		{
		}

		/**
		 * What has changed?
		 */
		DbStatus getStatus() const
		{
			return m_oStatus;
		}

		/**
		 * Set product ID
		 */
		void setProductID( const Poco::UUID& p_rProductID )
		{
			m_oProductID = p_rProductID;
		}
		/**
		 * Get product ID.
		 */
		const Poco::UUID& getProductID() const
		{
			return m_oProductID;
		}

		/**
		 * Set measuretaskID
		 */
		void setMeasureTaskID( const Poco::UUID& p_rMeasureTaskID )
		{
			m_oMeasureTaskID = p_rMeasureTaskID;
		}
		/**
		 * Get measuretask ID.
		 */
		Poco::UUID& getMeasureTaskID()
		{
			return m_oMeasureTaskID;
		}

		/**
		 * Set filter ID
		 */
		void setFilterID( const Poco::UUID& p_rFilterID )
		{
			m_oFilterID = p_rFilterID;
		}
		/**
		 * Get filter ID.
		 */
		Poco::UUID& getFilterID()
		{
			return m_oFilterID;
		}

	private:

		DbStatus 		m_oStatus;			///< What has changed? Product, MeasureTask or FilterParameter?
		Poco::UUID 		m_oProductID;		///< ProductID of affected product.
		Poco::UUID		m_oMeasureTaskID;	///< MeasureTaskID of the updated measuretask.
		Poco::UUID		m_oFilterID;		///< ID of the filter that has been changed.
	};

	/**
	 *	AbstrakteBasisklasse der DB Notification
	 * 	Die Datenbank meldet Aenderungen in Daten.
	 */
	template <>
	class TDbNotification<AbstractInterface>
	{
	public:
		TDbNotification() {}
		virtual ~TDbNotification() {}
	public:
		// Die Produktaten wurden geaendert.
		virtual void setupProduct(const Poco::UUID& productID) = 0;
		// Die Messaufgaben wurden geaendert (inkl. Graphen)
		virtual void setupMeasureTask(const Poco::UUID& measureTaskID) = 0;
		// die Filter Parameter wurden geaendert
		virtual void setupFilterParameter(const Poco::UUID& measureTaskID, const Poco::UUID& filterID) = 0;
		// ein HW-Parameter wurde geaendert
		virtual void setupHardwareParameter(const Poco::UUID& hwParameterSatzID, const Key key) = 0;
		// calibration configuration was changed
		virtual void resetCalibration(const int sensorId) = 0;
	};

    struct TDbNotificationMessageDefinition
    {
		EVENT_MESSAGE(SetupProduct, 			Poco::UUID);
		EVENT_MESSAGE(SetupMeasureTask, 		Poco::UUID);
		EVENT_MESSAGE(SetupFilterParameter, 	Poco::UUID, Poco::UUID);
		EVENT_MESSAGE(SetupHardwareParameter,	Poco::UUID, Key);
		EVENT_MESSAGE(ResetCalibration, int);

		MESSAGE_LIST(
			SetupProduct,
			SetupMeasureTask,
			SetupFilterParameter,
			SetupHardwareParameter,
			ResetCalibration);
    };

	template <>
	class TDbNotification<Messages> : public Server<Messages>, public TDbNotificationMessageDefinition
	{
	public:
		TDbNotification() : info(system::module::DbNotification, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
		MessageInfo info;
	private:
		/// Kontanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 500*Bytes, replyBufLen = 100*KBytes, NumBuffers = 64 };

	};

} // namespace interface
} // namespace precitec


#endif /* DBNOTIFICATION_INTERFACE_H_ */
