#ifndef TIMER_H_
#define TIMER_H_

#	if defined __QNX__ || defined __linux__
#	 if defined __QNX__
#	  include <sys/neutrino.h>
#	  include <sys/syspage.h>
#	 endif
#	 if defined __linux__
#	  include <chrono>
#	 endif
#	 include <inttypes.h>
#	 include "system/types.h"
#	else
#    include <string>
#	 include <WTypes.h>
#	 include <BaseTsd.h>
#	 include "winbase.h"
typedef std::string PvString;
typedef long long xLong;
#	endif

#include <sstream>
#include <iostream>
#include <string>
#include <iomanip>// wg setprecision()

#include <chrono>

namespace precitec {
namespace system {

	class Timer {
	public:
		/// used for all 'Time'-value vs cycle values
		typedef xLong Time;
	public:
		/// you can name the timer for recognizable output
		Timer( PvString name="Timer: ");
	public:
		// die eingentlichen Timer-Funktionen
		/// Zeit wird genullt, Timer wird gestartet
		void start();
		/// Timer wird gestopt
		void stop();
		/// Zeit wird genommen; neue Messung sofort gestartet
		void restart(); // == stop(); start();
		/// Zwischenzeit wird gesetzt (ohne Anhalten): returns counter-value
		xLong elapsed();
	public:
		// fuer akkumulierende Messungen;
		/// Zeit wird nicht genullt, eine neue Messung gestartet
		void resume();
		/// Zeit wird genommen und aufaddiert
		void endResume();
	public:
		// Accessoren
		/// etwa fuer Timer-Arrays kann der Name auch nachgereict werden
		void setName(PvString name) { name_ = name; }
		/// etwa fuer UnitTest ;-)
		PvString name() const { return name_; }

		/// Zeitintervall-Accessor in Sekunden
		Time sec()const { return ( time_ + (cyclesPerSecond_>>1)) / cyclesPerSecond_; }
		/// Zeitintervall-Accessor in Millisekunden
		Time ms() const { return ( time_ * 1000 + (cyclesPerSecond_>>1)) / cyclesPerSecond_; }
		/// Zeitintervall-Accessor in Mikrosekunden
		Time us() const { return ( time_ * 1000 * 1000 + (cyclesPerSecond_>>1)) / cyclesPerSecond_; }
		/// Zeitintervall-Accessor in Nanosekunden
		Time ns() const { return ( time_ * 1000 * 1000 * 1000 ) / cyclesPerSecond_; }
		/// implicit conversion to Time is integal nanoseconds
		operator Time() const { return ns(); }
		/// Ausgabe auf Stream (wird von "return Timer::write(os, time_, name_);" verwendet) -- ??? prime candidate for protected
		std::ostream & write( std::ostream &os, xLong time, PvString text ) const;
		/// base fun for converting a Time into a string ??? use toString in write()
		static PvString toString(Time time);

		/// Stream-Ausgabe
		friend std::ostream & operator <<(std::ostream &os, const Timer &t) {	return t.write(os, t.time_, t.name_); }

	protected:
		xLong	    cyclesPerSecond_;	///< Umrechnungfaktor: Zyklen ->Zeit
		xLong	    start_;
		xLong	    stop_;
		xLong       time_;	///< wir merken uns Messzeiten (Akkumulation / Offline-Ausgabe)
		PvString    name_;	///< fuer weniger verwirrende Ausgaben
	private:
		static void getFrequency(xLong &freq);
		static void getTime(xLong & time);
	}; // Timer

/**
 * nur auf zwei Wert wirdSystemspezifisch zugeriffen:
 *  - die Tatfrequenz des Systems
 *  - der Instruction/Zyklus-Zaehler
 */
#if defined __QNX__
	inline void Timer::getFrequency(xLong & freq) { freq = SYSPAGE_ENTRY(qtime)->cycles_per_sec; }
	inline void Timer::getTime(xLong & time) 	 { time = ClockCycles(); }
#elif defined __linux__
	inline void Timer::getFrequency(xLong & freq) { freq = 1000000; } // microseconds
	inline void Timer::getTime(xLong & time)
	{
		auto now    = std::chrono::high_resolution_clock::now();
		auto now_us = std::chrono::time_point_cast<std::chrono::microseconds>(now);
		auto value  = now_us.time_since_epoch();
		time = value.count();
	}
#else
	inline void Timer::getFrequency(xLong & freq) { QueryPerformanceFrequency((LARGE_INTEGER *)&freq); }
	inline void Timer::getTime(xLong & time) { QueryPerformanceCounter((LARGE_INTEGER *)&time); }
#endif

	inline Timer::Timer(PvString name) 
    : 
    cyclesPerSecond_    ( 0 ),
    start_              ( 0 ),
    stop_               ( 0 ),
    time_               ( 0 ),
    name_               ( name )
    {
		getFrequency(cyclesPerSecond_);
	}

	inline void Timer::start() {
		getTime(start_);
		time_  = 0;
	} // Start

	inline void Timer::stop() {
		getTime(stop_);
		time_ = stop_ - start_;
	} // stop

	inline Timer::Time Timer::elapsed() {
		getTime(stop_);
		return time_ = stop_ - start_;
	} // Timer::elapsed

	inline void Timer::restart() {
		getTime(stop_);
		time_  = stop_ - start_;
		start_ = stop_;
	} // Timer::restart

	inline void Timer::resume() {
		getTime(stop_);
		start_ = stop_;
	} // Timer::resume

	inline void Timer::endResume() {
		getTime(stop_);
		time_ += stop_ - start_;
		start_ = stop_;
	} // Timer::resume

	inline std::ostream & Timer::write( std::ostream &os, xLong time, PvString text ) const {
		const unsigned int oSizeUnits = 5;
		char const * units[oSizeUnits] = { "s\0", "ms\0", "us\0", "ps\0", "ns\0" };
		xLong cps;
		Timer::getFrequency(cps); // fuer die Umrechnung Zyklen -> Zeit

		os << text << " ";
		// Wir teilen die Cycles solange, bis wir einen von Null verschiedenen Wert
		// gefunden haben. Dieser wird auf 3 gueltige Ziffern genau ausgegeben.
		// i=0 ist Sekunden, i=1 Millisekunden, i=2 Picosekunden, i=3 Nanosekunden
		for( unsigned int i=0; i < oSizeUnits; i++ ) 	{
			// Ist die ganzzahlige Division gleich Null, muessen wir eine kleinere
			// Einheit versuchen.
			if( (time / cps) == 0 )	{
				// Dazu Multiplizieren wir die Cycles mit 1000. Ein Ueberlauf findet
				// nicht statt, sonst waere die Division nicht Null gewesen.
				time *= 1000;
			}	else {// Die Division ist nicht Null, die groesste Einheit ist gefunden
				// Umrechnen in eine echte Gleitkommazahl
				double printTime = double(time) / cps;
				// Formatierte Ausgabe auf 3 gueltige Ziffern mit Einheit
				os <<  std::setprecision(3) << printTime << " " << units[i];
				break;
			}
		}
		return  os;
	}

	/**
	 * converts a Timevalue in ns to a unit-adapted version
	 * @param time in nanoseconds
	 * @return timeString with unit
	 */
	inline PvString Timer::toString(Time time) {
		const unsigned int oSizeUnits = 5;
		char const * units[oSizeUnits] = { "s\0", "ms\0", "us\0", "ps\0", "ns\0" };
		xLong powersOfThousand [oSizeUnits] = { 1000*1000*1000*1000LL, 1000*1000*1000LL, 1000*1000LL, 1000LL, 1 };
		// Wir teilen die Cycles solange, bis wir einen von Null verschiedenen Wert
		// gefunden haben. Dieser wird auf 3 gueltige Ziffern genau ausgegeben.
		// i=0 ist Sekunden, i=1 Millisekunden, i=2 Picosekunden, i=3 Nanosekunden
		for( unsigned int i=0; i < oSizeUnits; i++ ) 	{
			// Ist die ganzzahlige Division gleich Null, muessen wir eine kleinere
			// Einheit versuchen.
			if( (time/powersOfThousand[i]) == 0 )	{
				// Dazu Multiplizieren wir die Cycles mit 1000. Ein Ueberlauf findet
				// nicht statt, sonst waere die Division nicht Null gewesen.
				continue;
			}	else {// Die Division ist nicht Null, die groesste Einheit ist gefunden
				// Umrechnen in eine echte Gleitkommazahl
				double printTime(time/double(powersOfThousand[i]));
				// Formatierte Ausgabe auf 3 gueltige Ziffern mit Einheit
				std::stringstream ss; ss << std::setprecision(3) << printTime << " " << units[i];
				return ss.str();
			}
		}
		// we only end here, if time is zero by our reckoning
		return "0.00 ns";
	}


	/**
	 * ScopedTimer ist die Einzeilen-Variante der Timerklasse
	 * Der ScopedTimer wird mit einem Namen erzeugt, sofort gestartet,
	 * und gibt im Destruktor (am dende des Scope) die Zeit aus
	 */
	class ScopedTimer : protected Timer {
	public:
		ScopedTimer(PvString name, std::ostream &os=std::cout) : Timer(name), os_(os) { start(); }
		~ScopedTimer() { stop(); write( os_, time_, name_); os_<< std::endl; }
	private:
		std::ostream &os_;
	}; // ScopedTimer

/**
 * Simple class which can measure the time elapsed.
 * Example usage:
 * @code
 * ElapsedTimer timer;
 * while (true)
 * {
 *     // do something which takes long time
 * }
 * std::cout << "This took " << timer.elapsed().count() << " nanoseconds" << std::endl;
 * @endcode
 **/
class ElapsedTimer
{
public:
#ifdef __linux__
    typedef std::chrono::steady_clock Clock;
#else
    typedef std::chrono::high_resolution_clock Clock;
#endif
    ElapsedTimer()
        : m_startTime(Clock::now())
    {
    }
    
    std::chrono::nanoseconds elapsed() const
    {
        return Clock::now() - m_startTime;
    }
    
    /**
     * Restarts the ElapsedTimer.
     **/
    void restart()
    {
        m_startTime = Clock::now();
    }
private:
    std::chrono::time_point<Clock> m_startTime;
};

}	// namesspace system
} // namespace precitec

#endif // TIMER_H_
