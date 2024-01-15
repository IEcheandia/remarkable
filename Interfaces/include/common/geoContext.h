/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			KIR, WoR, AB
 *  @date			2009
 *  @brief			Defines Trafo, TaskContext, ImageContext
 */

#ifndef GEO_CONTEXT_H_
#define GEO_CONTEXT_H_

#include <iostream>
#include <ostream>
#include <istream>
#include <string.h>
#include "Poco/SharedPtr.h"
#include "system/types.h"
#include "system/templates.h"
#include "system/timer.h"

#include "message/serializer.h"
#include "message/messageBuffer.h"
#include "common/measureTask.h"
#include "geo/size.h"
#include "geo/rect.h"
#include "geo/point.h"
#include "InterfacesManifest.h"

#include "Poco/UUIDGenerator.h"
#include "Poco/UUID.h"
#include "common/triggerContext.h"

namespace precitec
{
namespace interface
{
	class TaskContext;
	class ImageContext;

	typedef Poco::SharedPtr<TaskContext> SmpTaskContext;
	typedef Poco::SharedPtr<ImageContext> SmpImageContext;

	/// Koordinaten-Systeme haben Einheiten, folgende sind erstmal vorgesehen
	enum UnitTyp { UnitPixel, UnitMu, UnitNanoMeter, NumUnits };
	/// Image number in measure task enum
	enum MeasureTaskPos { noImagePos = -999, eFirstImage=-1, eMiddleImage=0, eLastImage=1 };
	enum TrafoTyp { TrafoIdent=0,  TrafoLinear, NumTypes };

	inline  std::ostream &operator <<(std::ostream &os, UnitTyp const& u) {
		switch (u) {
			case UnitPixel: os << "Pixel"; break;
			case UnitMu: os << "um"; break;
			case UnitNanoMeter: os << "nm"; break;
			default: os << "no Unit"; break;
		}
		 return os;
	}
	class Trafo;
	typedef Poco::SharedPtr<Trafo> SmpTrafo;

	class Trafo : public Serializable {
	public:
		// eine Minimalzahl von Trafos
		friend std::ostream &operator <<(std::ostream &os, TrafoTyp const& t) {
			switch (t) {
			case TrafoIdent: os << "Ident-Trafo"; break;
			case TrafoLinear: os << "Linear-Trafo"; break;
				default: os << "Unknown-Trafo"; break;
			}
			return os;
		}
	public:
		/// std:: CTor
		Trafo() : superTrafo_() {}
		/// copy-CTor
		Trafo(Trafo const& t) : superTrafo_(t.superTrafo_) {}
		/// deserialize CTor
		Trafo(system::message::MessageBuffer const& buffer) {}// : superTrafo_(Serializable::deMarshal<SmpTrafo>(buffer)) {}
		/// DTor virtual!
		virtual ~Trafo() {}

	public:
		void attachTo(Trafo const& t) { if (!superTrafo_) superTrafo_ = t.clone(); }
		void attachTo(SmpTrafo const& t) { if (!superTrafo_) superTrafo_ = t; }
		/// HinTrafo
		SmpTrafo operator() (Trafo const& t) const { return apply(t); }
		SmpTrafo operator() (SmpTrafo const& t) const { return apply(*t); }
		geo2d::Point operator() (geo2d::Point const& p) const { return apply(p); }
		geo2d::Rect operator() (geo2d::Rect const& r) const { return apply(r); }
		/// RueckTrafo
		SmpTrafo operator[] (Trafo const& t) const { return applyReverse(t); }
		//geo2d::Point operator[] (geo2d::Point const& p) const { return applyReverse(p); }
		//geo2d::Rect operator[] (geo2d::Rect const& r) const { return applyReverse(r); }

		virtual Trafo& operator = (Trafo const&rhs)  {	superTrafo_=rhs.superTrafo_; return *this; }
		virtual bool operator == (Trafo const& rhs) const	{
			if (superTrafo_.isNull() && rhs.superTrafo_.isNull()) return true;
			if (superTrafo_.isNull() || rhs.superTrafo_.isNull()) return false;
			return  superTrafo_->operator ==(*rhs.superTrafo_);
		}
		/// normlerweise kopiert sich Trafo per Referenz, hier wird einm subtyp-richtiger Deep-Copy erzwungen
		virtual SmpTrafo clone() const	{ return new Trafo(*this); }

		/// die reine Trafo hat ncx ausser der Supertrafo
		virtual void copyFrom(Trafo const& t)
		{
			if (!t.superTrafo_.isNull()) superTrafo_ = t.superTrafo_->clone();
		}

		// der Serialize-Block
		/// Subtyp identifizieren
		virtual TrafoTyp type() const
		{
			return TrafoIdent;
		}

		/// in den Puffer
		virtual void serialize ( system::message::MessageBuffer &buffer ) const
		{
			/*
			 * As of June 2012, the linearTrafo is the most complex one and highest in hierarchy. As we do not want another polymorphism and managedCode interface on Windows,
			 * I try to always (de)serialize just ONE kind of matrix/trafo, which should encompass all. That said, this so far is the linearTrafo, and the contract() function
			 * breaks it down to computing the final matrix. Thus, no more marshalling of the supertrafo here.
			 */
			//marshal(buffer, superTrafo_);
		}

		/// der Buffer-CTor macht die eigentliche Arbeit
		virtual void deserialize( system::message::MessageBuffer const&buffer )
		{
			/*
			 * As of June 2012, the linearTrafo is the most complex one and highest in hierarchy. As we do not want another polymorphism and managedCode interface on Windows,
			 * I try to always (de)serialize just ONE kind of matrix/trafo, which should encompass all. That said, this so far is the linearTrafo, and the contract() function
			 * breaks it down to computing the final matrix. Thus, no more marshalling of the supertrafo here.
			 */
			//Trafo tmp(buffer); swap(tmp);
		}

		// die create der Basisklasse: muss so heissen und diese Signatur haben
		static Trafo * create(int type, system::message::MessageBuffer const&buffer );
		// Fabrik der Basisklasse
		static Trafo * factory(system::message::MessageBuffer const&buffer)
		{
			return new Trafo(buffer);
		}

		/// wg nice-Class und deSerialize
		void swap(Trafo &rhs )
		{
			superTrafo_.swap(rhs.superTrafo_);
		}

		friend  std::ostream &operator <<(std::ostream &os, Trafo const& t)
		{
			os << t.type(); t.toStream(os);
			if (t.superTrafo_.isNull())		{ os << " ->NULL"; }
			else							{ os << " ->" << *t.superTrafo_; }
			return os;
		}
		virtual void toStream(std::ostream & os) const {}
	public:
		static SmpTrafo createLinear(int offsetX, int offsetY);
		static SmpTrafo createIdent();
		/// Trafo-Accessor
		virtual int dx () const{ return 0; }
		virtual int dy () const{ return 0; }
	protected:
public:
		/// forward trafo: create _new_ trafo of this trafo applied to t
		virtual SmpTrafo apply (Trafo const& t) const {	return new Trafo(t); }
		virtual geo2d::Point apply (geo2d::Point const& p) const { return p; }
		virtual geo2d::Range applyX(geo2d::Range const& r) const { return r; }
		virtual geo2d::Range applyY(geo2d::Range const& r) const { return r; }
		virtual geo2d::Rect apply (geo2d::Rect const& r) const { return r; }
		/// reverse trafo: create new trafo of inverse trafo applied to t (we are ident!, so no problem)
		virtual SmpTrafo applyReverse (Trafo const& t) const { return new Trafo(t); }
		virtual geo2d::Point applyReverse(geo2d::Point const& p) const { return p; }
		virtual geo2d::Range applyReverseX(geo2d::Range const& r) const { return r; }
		virtual geo2d::Range applyReverseY(geo2d::Range const& r) const { return r; }
		virtual geo2d::Rect applyReverse(geo2d::Rect const& r) const { return r; }
		virtual SmpTrafo contract() const
		{
			// The if-clause out commented here MUST be included into the contract() function of all derivations! Reason: See above, "As of June..."
			return clone();
		}

protected:
		/// Verweis auf Trafokette: Bildanfang, Taskanfang, Produktanfang
		SmpTrafo	superTrafo_;
	};


	class LinearTrafo : public Trafo {
	public:
		/// copy cTor for all non linear trafos
		LinearTrafo(Trafo const& rhs) : Trafo(rhs), dx_(rhs.dx()), dy_(rhs.dy()) {}
		/// is necessary!!!, compiler will not call the Trafo& cTor but rather construct his own
		LinearTrafo(LinearTrafo const& rhs) : Trafo(rhs), dx_(rhs.dx()), dy_(rhs.dy()) {}
		/// CTor aus x, y-Offset]
		LinearTrafo(int offsetX=0, int offsetY=0) : dx_(offsetX), dy_(offsetY) {}
		/// CTor aus Offset-Punkt
		LinearTrafo(geo2d::Point const& offset): dx_(offset.x), dy_(offset.y) {}
		/// Deserialisierungs-CTor
		LinearTrafo(system::message::MessageBuffer const& buffer)
			: Trafo(buffer), dx_(Serializable::deMarshal<int>(buffer)), dy_(Serializable::deMarshal<int>(buffer)) {}
		virtual ~LinearTrafo() {}

	public:
		virtual SmpTrafo clone() const { return new LinearTrafo(*this);	}
		virtual LinearTrafo& operator = (Trafo const&rhs)  {	dx_=rhs.dx(); dy_=rhs.dy(); Trafo::operator =(rhs); return *this; }
		virtual bool operator == (LinearTrafo const&rhs) const {
			return ( (dx_==rhs.dx_) && (dy_==rhs.dy_) && Trafo::operator ==(rhs) );
		}
		virtual bool operator == (Trafo const&rhs) const {
			if (rhs.type() != TrafoLinear)
			{
				return false;
			}
			LinearTrafo oRhs = (LinearTrafo)(rhs);
			return ( (dx_==oRhs.dx_) && (dy_==oRhs.dy_) && Trafo::operator ==(oRhs) );
		}
		virtual TrafoTyp type() const {	return TrafoLinear;	}
		virtual void serialize ( system::message::MessageBuffer &buffer ) const {
			Trafo::serialize(buffer);
			marshal(buffer, dx_);
			marshal(buffer, dy_);
		}
		/// der Buffer-CTor macht die eigentliche Arbeit
		virtual void deserialize( system::message::MessageBuffer const&buffer )
		{
			LinearTrafo tmp(buffer);
			swap(tmp);
			/* deser of dx_ and dy_ is done in LinearTrafo(buffer) constructor */
		}
		/// wg nice-Class und deSerialize
		void swap(LinearTrafo &rhs ) { Trafo::swap(rhs); std::swap(dx_, rhs.dx_); std::swap(dy_, rhs.dy_); }
		// Fabrik wird  von create der Basisklasse aufgerufen
		static Trafo * factory(system::message::MessageBuffer const &buffer) {	return new LinearTrafo(buffer);	}
		/// virtuelle Ausabe-Funktion fuer Basisklassen <<
		virtual void toStream(std::ostream & os) const { os << "(" << dx_ << ", " << dy_ << ")";	}
		virtual SmpTrafo contract() const
		{
			if ( !superTrafo_.isNull() )
			{
				return this->apply(superTrafo_);
			}
			return clone();
		}
		/// HinTrafo
		virtual SmpTrafo apply (Trafo const& t) const {	return new LinearTrafo(dx_+t.dx(), dy_+t.dy()); }
		virtual SmpTrafo apply (SmpTrafo t) const { return new LinearTrafo(dx_+t->dx(), dy_+t->dy()); }
		virtual geo2d::Point apply(geo2d::Point const& p) const { return geo2d::Point(p.x+dx(), p.y+dy()); }
		virtual geo2d::Range applyX(geo2d::Range const& r) const { return geo2d::Range(r.start()+dx_, r.end()+dx_); }
		virtual geo2d::Range applyY(geo2d::Range const& r) const { return geo2d::Range(r.start()+dy_, r.end()+dy_); }
		virtual geo2d::Rect apply(geo2d::Rect const& r) const { return geo2d::Rect(applyX(r.x()), applyY(r.y())); }
		/// RueckTrafo
		virtual SmpTrafo applyReverse (Trafo const& t) const { return SmpTrafo(new LinearTrafo(dx_-t.dx(), dy_-t.dy())); }
		virtual geo2d::Point applyReverse(geo2d::Point const& p) const { return geo2d::Point(p.x-dx(), p.y-dy()); }
		virtual geo2d::Range applyReverseX(geo2d::Range const& r) const { return geo2d::Range(r.start()-dx_, r.end()-dx_); }
		virtual geo2d::Range applyReverseY(geo2d::Range const& r) const { return geo2d::Range(r.start()-dy_, r.end()-dy_); }
		virtual geo2d::Rect applyReverse(geo2d::Rect const& r) const{ return geo2d::Rect(applyReverseX(r.x()), applyReverseY(r.y())); }

	protected:
		/// Trafo-Accessor
		virtual int dx () const{ return dx_; }
		virtual int dy () const{ return dy_; }

	private:
		int	dx_;
		int	dy_;
	};

	inline Trafo * Trafo::create(int type, system::message::MessageBuffer const&buffer ) {
		typedef Trafo* (*TrafoFactory)(system::message::MessageBuffer const&buffer);
		/// factory list is initialized at first call and then statically availiable
		static const TrafoFactory trafoFactoryList[NumTypes] = {
			Trafo::factory,
			LinearTrafo::factory
		};
		return trafoFactoryList[type](buffer);
		//return trafoFactoryList[1](buffer);
	}

	inline SmpTrafo Trafo::createLinear(int offsetX, int offsetY) { return new LinearTrafo(offsetX, offsetY); }
	inline SmpTrafo Trafo::createIdent() { return new Trafo(); }



	// *********************************************************************************************


	/**
		 * Der Aufgaben/Task(-Koordinaten)-Kontext beziehen sich auf ein Produkt-Koordinatensystem
		 * hat also IMMER einen supercontext 8auch wenn der anfangs ein Ident-Context sein kann).
		 * Der Task-Kontext kann beliebig komplex sein, wird aber sinnvollerweise (wenn er nicht
		 * zum Ident verkuemmert) eine 2D-Verschiebung und eine Drehung (in dieser Reihenfolge) beinhalten.
		 * Die darauf aufbauenden Bild-Kontexte haben (anfngs/typischweise) eine lineare Verschiebung
		 * entlang der x-Achse gegenueber diesem Kontext.
		 */
		class TaskContext : public Serializable {
			typedef Timer::Time Time;
		public:
			/// leer-CTor
			TaskContext() : trafo_(new Trafo), productInstanceId_(), measureTaskId_(), measureTask_(new MeasureTask) {}
			/// der Std-CTor kombiniert die Elemente aus TC und IC
			TaskContext(SmpTrafo p_oSpTrafo, Poco::UUID const&  productInstanceId) :
								trafo_(p_oSpTrafo), productInstanceId_(productInstanceId), measureTaskId_(), measureTask_(new MeasureTask) {}
			TaskContext(SmpTrafo p_oSpTrafo, Poco::UUID const&  productInstanceId, SmpMeasureTask p_oSpMeasureTask) :
					trafo_(p_oSpTrafo), productInstanceId_(productInstanceId), measureTaskId_(p_oSpMeasureTask->taskID()),
					measureTask_(p_oSpMeasureTask) {}
			/// copy-CTor
			TaskContext(const TaskContext& rhs) : trafo_(rhs.trafo_),
					productInstanceId_(rhs.productInstanceId_),
					productId_(rhs.productId_),
					stationId_(rhs.stationId_),
					measureTaskId_(rhs.measureTask_->taskID()),
					measureTask_(rhs.measureTask_)
			{}
			/// Buffer-CTor fuer Deserialisierung
			TaskContext(system::message::MessageBuffer const& buffer)
					//trafo_(Serializable::deMarshal<SmpTrafo>(buffer)),

				{
					trafo_ = new LinearTrafo(buffer);
					deMarshal(buffer, productInstanceId_);
					//productInstanceId_deMarshal<Poco::UUID>(buffer));
					deMarshal(buffer, productId_);
					deMarshal(buffer, stationId_);
					deMarshal(buffer, measureTaskId_);
					measureTask_ = new MeasureTask(Serializable::deMarshal<MeasureTask>(buffer));
				}


			void setMeasureTask(MeasureTask *p_rMeasureTask)
			{
				measureTaskId_ = p_rMeasureTask->taskID();
				measureTask_ = new MeasureTask(*p_rMeasureTask);
			}

			TaskContext& operator=(TaskContext const& rhs)
			{
				TaskContext tmp(rhs);
				swap(tmp);
				return *this;
			}
			bool operator == (TaskContext const& rhs) const;
			friend  std::ostream &operator <<(std::ostream &os, TaskContext const& c) {
				os << *(c.trafo_) << "\n  measure task: " << c.measureTask_ << "\n"
						 << "\n  product instance id: " << c.productInstanceId_.toString() << "\n";
				return os;
			}
		public:
			// the serializer block
			virtual void serialize ( system::message::MessageBuffer &buffer ) const;
			/// der Buffer-CTor macht die eigentliche Arbeit
			virtual void deserialize( system::message::MessageBuffer const&buffer ) { TaskContext tmp(buffer); swap(tmp); }
			void swap(TaskContext &rhs );

			/// std-accessors
			Poco::UUID measureTaskId() const { return measureTask_->taskID();	}
			Poco::UUID productInstanceId() const { return productInstanceId_; }
			void setProductInstanceId(Poco::UUID p_oPIid) { productInstanceId_ = p_oPIid; }
			Poco::UUID productId() const { return productId_; }
			void setProductId(Poco::UUID p_oPid) { productId_ = p_oPid; }
			Poco::UUID stationId() const { return stationId_; }
			void setStationId(Poco::UUID p_oSid) { stationId_ = p_oSid; }
			SmpTrafo trafo() const { return trafo_; }
			SmpMeasureTask measureTask() const { return measureTask_; }

		private:
			SmpTrafo				trafo_; // taskTrafo = task-coordSys relative to product-coordSys
			Poco::UUID			productInstanceId_;
			Poco::UUID      productId_;
			Poco::UUID      stationId_;
			Poco::UUID			measureTaskId_;  ///< additional copy of the measureTaskID so win does not need to store the measureTask twice and can access it via its GUID
			SmpMeasureTask  measureTask_;
		}; // TaskContext

		typedef Poco::SharedPtr<TaskContext> SmpTaskContext;



		inline void TaskContext::serialize ( system::message::MessageBuffer &buffer ) const {
			/*
			 * As of June 2012, the linearTrafo is the most complex one and highest in hirarchy. As we do not want another polymorphism and managedCode interface on Windows,
			 * I try to always (de)serialize just ONE kind of matrix/trafo, which should encompass all. That said, this so far is the linearTrafo, and the contract() function
			 * breaks it down to computing the final matrix. If new kinds of trafos are implemented, this will have to be adjusted here and on Windows, too!!
			 */
			if ( trafo_->type() != TrafoLinear )
			{
				LinearTrafo lt(*trafo_);
				marshal(buffer, lt);
			} else {
				SmpTrafo pTrafo(trafo_->contract());
				marshal(buffer, *pTrafo);
			}
			marshal(buffer, productInstanceId_);
			marshal(buffer, productId_);
			marshal(buffer, stationId_);
			marshal(buffer, measureTaskId_);
			measureTask_->serialize(buffer);
		}

		inline void TaskContext::swap( TaskContext &rhs) {
			trafo_.swap(rhs.trafo_);
			productInstanceId_.swap(rhs.productInstanceId_);
			productId_.swap(rhs.productId_);
			stationId_.swap(rhs.stationId_);
			measureTaskId_.swap(rhs.measureTaskId_);
			measureTask_.swap(rhs.measureTask_);
		}

		inline bool TaskContext::operator == (TaskContext const& rhs) const {
		return (
				(*trafo_==*rhs.trafo_)
				&& (productInstanceId_==rhs.productInstanceId_)
				&& (measureTaskId_ == rhs.measureTaskId_)
				&& (productId_ == rhs.productId_)
				&& (stationId_ == rhs.stationId_)
				&& (measureTask_ == rhs.measureTask_)
				);
		}


    struct ScannerContextInfo
    {
        bool m_hasPosition = false;
        double m_x = 0.0;
        double m_y = 0.0;
        friend  std::ostream &operator <<(std::ostream &os, ScannerContextInfo const& v) {
            if (v.m_hasPosition)
            {
                os << "[]";
            }
            else
            {
                os << "[ " << v.m_x <<", " << v.m_y << " ]";
            }
            return os;
        }
    };
		
	/**
	 * Der Image-Kontext wird von der Analyse verwendet
	 * ueber den id koennen eindeutige Zuordnungen gemacht werden.
	 */
	class ImageContext : public Serializable {
	public:
		/// eigentlich nur fuer Context-Arrays sinnvoll
		ImageContext()
		: 
        measureTaskPos_ (noImagePos), 
        trafo_          (new Trafo), 
        imageNumber_    (0), 
        position_       (0),
        HW_ROI_x0       (0), 
        HW_ROI_y0       (0), 
        SamplingX_      { 1 },
        SamplingY_      { 1 },
        m_ScannerInfo{false},
        m_transposed{false},
         taskContext_   (new TaskContext) 
        {}
		/// CTor fuer Bilderzeuger (Senor)
		ImageContext(ImageContext const& rhs)
		: 
        measureTaskPos_ (rhs.measureTaskPos_),
	    trafo_          (rhs.trafo()->clone()), //trafo_(rhs.trafo_)
		imageNumber_    (rhs.imageNumber()), 
        position_       (rhs.position_),
		relTime_        (rhs.relativeTime()), 
        id_             (rhs.id_), 
        HW_ROI_x0       (rhs.HW_ROI_x0), 
        HW_ROI_y0       (rhs.HW_ROI_y0),
        SamplingX_      {rhs.SamplingX_},
        SamplingY_      {rhs.SamplingY_},
        m_ScannerInfo {rhs.m_ScannerInfo},
        m_transposed {rhs.m_transposed},
		taskContext_    (rhs.taskContext_) 
        {}
		/// der "ROI"-CTor, alles wird uebernommen, nur die Trafo neu gesetzt
		ImageContext(ImageContext const& rhs, SmpTrafo trafo)
		: 
        measureTaskPos_ (rhs.measureTaskPos_), 
        trafo_          (trafo->clone()), 
        imageNumber_    (rhs.imageNumber()), 
        position_       (rhs.position_),
		relTime_        (rhs.relativeTime()), 
        id_             (rhs.id_),  
        HW_ROI_x0       (rhs.HW_ROI_x0), 
        HW_ROI_y0       (rhs.HW_ROI_y0),
        SamplingX_      { rhs.SamplingX_ },
        SamplingY_      { rhs.SamplingY_ },
        m_ScannerInfo {rhs.m_ScannerInfo},
        m_transposed {rhs.m_transposed},
		taskContext_    (rhs.taskContext_) 
        {}
		/// CTor fuer Bilderzeuger (Sensor)
		ImageContext(TriggerContext const& tc)
		: 
        measureTaskPos_ { 0 },
        trafo_          (new LinearTrafo), 
        imageNumber_    (tc.imageNumber()), 
        position_       (0), 
        relTime_        (tc.relativeTime()),
		id_             (Poco::UUIDGenerator::defaultGenerator().createOne()), 
        SamplingX_      { 1 },
        SamplingY_      { 1 },
        m_ScannerInfo{false},
        m_transposed {false},
        taskContext_    (new TaskContext)
		{
	//		measureTaskPos_ = getMeasureTask(tc.imageNumber());
			HW_ROI_x0=tc.HW_ROI_x0;
			HW_ROI_y0=tc.HW_ROI_y0;
		}
		/// der Deserialisierung-CTor
		ImageContext(system::message::MessageBuffer const& buffer) 	
        :
        SamplingX_  { 1 },
        SamplingY_  { 1 }
        {
			//buffer.dump(std::cout);
			//buffer.dump();
			measureTaskPos_ = Serializable::deMarshal<int>(buffer);
			trafo_ = new LinearTrafo(buffer);
			imageNumber_ = Serializable::deMarshal<int>(buffer);
			position_ = Serializable::deMarshal<int>(buffer);
			relTime_ = Serializable::deMarshal<Timer::Time>(buffer);
			id_ = Serializable::deMarshal<Poco::UUID>(buffer);
			HW_ROI_x0= Serializable::deMarshal<int>(buffer);
			HW_ROI_y0 = Serializable::deMarshal<int>(buffer);
            // sampling does not get serialized
            m_ScannerInfo = Serializable::deMarshal<ScannerContextInfo>(buffer);
            m_transposed = Serializable::deMarshal<bool>(buffer);
			taskContext_ = new TaskContext(Serializable::deMarshal<TaskContext>(buffer));
		}

		~ImageContext()=default;

		friend  std::ostream &operator <<(std::ostream &os, ImageContext const& v) {
			os << "ImageContext:\n imgNr:" << v.imageNumber_
			<< "; Pos.: " << v.position_
			<< "\n  HWRoi:(" << v.HW_ROI_x0 << ", "<< v.HW_ROI_y0
            << ")\n  sampling:(" << v.SamplingX_ << ", "<< v.SamplingY_
			<< "); relTime: " << v.relTime_
			<< "\n  Trafo:  " << *v.trafo_
			<< "\n  Scanner Position:  " << v.m_ScannerInfo
			<< "\n  Transposed:  " << v.m_transposed
			<< "\n  TaskContext: " << *v.taskContext_ << "\n";
			return os;
		}
	public:
		/// nur Trafo ist hier nicht podbar
		virtual void serialize ( system::message::MessageBuffer &buffer ) const;
		/// der Buffer-CTor macht die eigentliche Arbeit
		virtual void deserialize( system::message::MessageBuffer const&buffer ) { ImageContext tmp(buffer); swap(tmp);	}

		/// wg nice-Class und deSerialize
		void swap(ImageContext &rhs );
		bool operator == (ImageContext const& rhs) const;
		bool operator != (ImageContext const& rhs) const { return ! (*this == rhs); }
		void setTaskContext(TaskContext &context) { taskContext_ = new TaskContext(context); }

		ImageContext& operator=(ImageContext const& rhs)
		{
			ImageContext tmp(rhs);
			swap(tmp);
			return *this;
		}

		/// sets measureTaskFlag w.r.t current imagenumber no and current. Works only for nonparallel graphs!!
		void setMeasureTaskPositionFlag(MeasureTaskPos p_oMeasureTaskPos)
		{
			// sets flag: -1 for p_oCurrentNr==p_oStart, 1 for p_oCurrentNr==p_oEnd, 0 for p_oSart < p_oCurrentNr < p_oEnd, noImagePos otherwise
			measureTaskPos_ = p_oMeasureTaskPos;
		} // setMeasureTaskPositionFlag

		inline int measureTaskPosFlag() const { return measureTaskPos_; }

		/// sets absolute position [um] w.r.t. start of seam
		inline void setPosition(long p_oPos)
		{
			position_ = p_oPos;
		}

		/// returns absolute position [um] w.r.t. start of seam
		long position() const
		{
			//return position_;
			return position_;
		}

	public:
		SmpTrafo trafo() const {	return (trafo_.isNull()) ? SmpTrafo(new Trafo) : trafo_; }
		void setImageNumber(int i) 	{ imageNumber_ = i; }
		int imageNumber() const { return imageNumber_; }
		void setTime(int t)	{ relTime_ = t; }
		Timer::Time relativeTime()	const { return relTime_; }
		Poco::UUID id()			const { return id_; }
		TaskContext const & taskContext() const { return *taskContext_; }
		SmpTaskContext const getSmpTaskContext() { return taskContext_; }
		int getTrafoX() const {return (trafo_.isNull()) ? 0 : trafo_->dx();}
		int getTrafoY() const {return (trafo_.isNull()) ? 0 : trafo_->dy();}

	private:
		int 			measureTaskPos_;		///< 3(4)-state variable: frist Image (<-1), last image(>1) or in between (0), undefined (-999)
		SmpTrafo		trafo_;					///< Transformation gegen vorheriges Koordinatensystem
		int 			imageNumber_;			///< Bildnummer ab Produktstart
		int32_t			position_;              ///< position relative to seam [um]
		Timer::Time 	relTime_;				///< Zeit ab Taskstart?
		Poco::UUID		id_;					///< image id
	public:
		int             HW_ROI_x0;
		int             HW_ROI_y0;
        double          SamplingX_;             ///< over-/subsampling factor in x - 1024 -> 512: SamplingX_ = 0.5
		double          SamplingY_;             ///< over-/subsampling factor in y - 1024 -> 2048: SamplingY_ = 2.0
		ScannerContextInfo m_ScannerInfo;
        bool m_transposed;
	private:
		SmpTaskContext taskContext_;    ///< task context
	}; // ImageContext

	/// Std-Implementierung
	inline void ImageContext::serialize ( system::message::MessageBuffer &buffer ) const
	{
		marshal(buffer, measureTaskPos_);
		/*
		 * As of June 2012, the linearTrafo is the most complex one and highest in hirarchy. As we do not want another polymorphism and managedCode interface on Windows,
		 * I try to always (de)serialize just ONE kind of matrix/trafo, which should encompass all. That said, this so far is the linearTrafo, and the contract() function
		 * breaks it down to computing the final matrix. If new kinds of trafos are implemented, this will have to be adjusted here and on Windows, too!!
		 */

		if ( trafo_->type() != TrafoLinear )
		{
			LinearTrafo lt(*trafo_);
			marshal(buffer, lt);
		} else {
			SmpTrafo pTrafo(trafo_->contract());
			marshal(buffer, *pTrafo);
		}
		marshal(buffer, imageNumber_);
		marshal(buffer, position_);
		marshal(buffer, relTime_);
		marshal(buffer, id_);
		marshal(buffer, HW_ROI_x0);
		marshal(buffer, HW_ROI_y0);
        // sampling does not get serialized
        marshal(buffer, m_ScannerInfo);
        marshal(buffer, m_transposed);
		marshal(buffer, *taskContext_);

		//buffer.dump();
	}


	inline bool ImageContext::operator == (ImageContext const& rhs) const {
		return (*trafo_==*rhs.trafo_)
				&& (imageNumber_== rhs.imageNumber_)
				&& (position_ == rhs.position_)
				&& (HW_ROI_x0== rhs.HW_ROI_x0)
				&& (HW_ROI_y0== rhs.HW_ROI_y0)
                && (SamplingX_== rhs.SamplingX_)
                && (SamplingY_== rhs.SamplingY_)
                && (m_ScannerInfo.m_hasPosition == rhs.m_ScannerInfo.m_hasPosition)
                && (!m_ScannerInfo.m_hasPosition || 
                    (m_ScannerInfo.m_x == rhs.m_ScannerInfo.m_x && m_ScannerInfo.m_y == rhs.m_ScannerInfo.m_y))
                && (m_transposed == rhs.m_transposed)
				&& (relTime_==rhs.relTime_)
				&& (id_==rhs.id_)
				&& (measureTaskPos_ == rhs.measureTaskPos_)  /*AB: muss irgendwann weg*/
				&& (*taskContext_ == *(rhs.taskContext_));
	}

	/// Std-Implementierung
	inline void ImageContext::swap(ImageContext &rhs ) {
		trafo_.swap(rhs.trafo_);
		std::swap(imageNumber_, rhs.imageNumber_);
		std::swap(position_, rhs.position_);
		std::swap(relTime_, rhs.relTime_);
		id_.swap(rhs.id_);
		std::swap(measureTaskPos_, rhs.measureTaskPos_);
		std::swap(HW_ROI_x0, rhs.HW_ROI_x0);
		std::swap(HW_ROI_y0, rhs.HW_ROI_y0);
        std::swap(SamplingX_, rhs.SamplingX_);
		std::swap(SamplingY_, rhs.SamplingY_);
        std::swap(m_ScannerInfo, rhs.m_ScannerInfo);
        std::swap(m_transposed, rhs.m_transposed);
		taskContext_.swap(rhs.taskContext_);
	}

	inline std::ostream &operator <<(std::ostream &os, Poco::UUID const& u) {
		os << u.toString(); return os;
	}


} // namespace interface
} // namespace precitec

#endif /* GEO_CONTEXT_H_ */


