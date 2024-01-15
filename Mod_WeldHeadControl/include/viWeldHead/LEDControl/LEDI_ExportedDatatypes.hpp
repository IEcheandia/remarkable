#ifndef _LEDI_EXPORTED_DATATYPES_HPP_
#define _LEDI_EXPORTED_DATATYPES_HPP_

//#define LED_DRIVER_CHATTY
#undef LED_DRIVER_CHATTY

//#define LED_DRIVER_DISABLE_COMPLETE_CODE
#undef LED_DRIVER_DISABLE_COMPLETE_CODE


enum LEDControllerType {eLEDTypeNone = 0, eLEDTypePP420F = 1, eLEDTypePP520 = 2, eLEDTypePP820 = 3};

class LEDdriverParameterT
{
	public:
		bool useDriverInternalTrigger;
};

#define ANZ_LEDI_PARAMETERS 8

class LEDI_ParametersT
{
	public:
		int led_enable;
		int led_brightness;
		int led_pulse_width;
};

#ifndef LED_DRIVER_DISABLE_COMPLETE_CODE

class LEDI_ParametersInternT
{
	public:
		LEDI_ParametersInternT(int in_pulselength_us, int in_current_mA ,int in_retriggerdelay_ms) \
			{ led_pulselength_us=in_pulselength_us; led_current_mA=in_current_mA; retriggerdelay_ms=in_retriggerdelay_ms;};

		LEDI_ParametersInternT()\
			{led_pulselength_us=80; led_current_mA=200; retriggerdelay_ms=2; };

		int led_pulselength_us;
		int led_current_mA;
		int retriggerdelay_ms;
};

struct lightRatingStructureT
{
	public:
		double AmpereLimit;
		double VoltageLimit;
};

#endif // LED_DRIVER_DISABLE_COMPLETE_CODE

#endif // _LEDI_EXPORTED_DATATYPES_HPP_

