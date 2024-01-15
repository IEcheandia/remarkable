/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2011
 * 	@brief		arm states for filter arm 
 */


#ifndef ARMSTATES_20110905_H_
#define ARMSTATES_20110905_H_


namespace precitec {
	namespace filter {


	/// arm states
	enum ArmState {
		eSeamStart				= 0,					///< seam start
		eSeamEnd,										///< seam end
		eSeamIntervalChange,							///< seam change (includes seam start)
		eSeamIntervalStart,								///< seam interval start (only signaled to sum errors)
		eSeamIntervalEnd,								///< seam interval end (only signaled to sum errors)
		eSeamSeriesStart,								///< seam series start
		eSeamSeriesEnd,									///< seam series end
		eCycleStart,									///< automatic cycle start
		eCycleEnd,										///< automatic cycle end
		eArmStateMin			= eSeamStart,			///< delimiter
		eArmStateMax			= eCycleEnd				///< delimiter
	}; // ArmState



	} // namespace filter
} // namespace precitec


#endif // ARMSTATES_20110905_H_
