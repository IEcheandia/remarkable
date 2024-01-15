#include "fgrab_prototyp.h"
#include "fgrab_struct.h"
#include "fgrab_define.h"

namespace me4Globals {

}

namespace Process {

}

namespace DmaToPC {

}

namespace CameraGrayAreaBase {
enum DvalMode {DVAL_Enabled=1,DVAL_Disabled=0};
enum CLformat {SingleTap8Bit=0,SingleTap10Bit=1,SingleTap12Bit=2,SingleTap14Bit=3,SingleTap16Bit=4,DualTap8Bit=5,DualTap10Bit=6,DualTap12Bit=7};

}

namespace ImageBuffer {

}

namespace TrgPortArea {
	enum TriggerModeN {
			GrabberControlled	= 1,
			ExternSw_Trigger	= 2
	};
	enum EnableN { OFF=0, ON=1};
	enum ImageTrgInSourceN {
			InSignal0				= 0,
			InSignal1				= 1,
			InSignal2				= 2,
			InSignal3				= 3,
			SoftwareTrigger	= 4
	};
	enum PolarityN {
			HighActive			= 0,
			LowActive				= 1
	};
	enum CCsourceN {
			Exsync=0,
			ExsyncInvert=1,
			Hdsync=2,
			HdsyncInvert=3,
			Flash=4,
			FlashInvert=5,
			Clk=6,
			Gnd=7,
			Vcc=8
	};

} // TrgPortArea

