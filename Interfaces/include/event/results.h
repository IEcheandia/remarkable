/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		WoR, AB, HS
 * 	@date		2011
 * 	@brief		Provides result- and  typ enum as well as base classes necessary for handling results (TResultArgs, ...)
 */

#ifndef RESULTS_H_
#define RESULTS_H_

#include "Poco/UUID.h"
#include "geo/geo.h"
#include "geo/range.h"
#include "event/resultType.h"
#include "system/stdImplementation.h"
#include "geo/array.h"
#include <iostream>
#include <vector>
#include <queue>

namespace precitec {
namespace interface {
	typedef std::vector<Poco::UUID> MeasureTaskIDs;
	typedef geo2d::TRange<int> IntRange;

	template<class T>
	class TResultArgsImpl;

	class INTERFACES_API ResultArgsImpl : public Serializable {
	public:
		virtual ResultArgsImpl* clone() const=0;
		virtual PvString toString() const=0;
		virtual bool empty() const=0;

		template <typename T>
		const std::vector<T>& value() const 
		{
            assert(dynamic_cast<const TResultArgsImpl<T>*>(this));
			return dynamic_cast<const TResultArgsImpl<T>*>(this)->value();
		}
		virtual const std::vector<int>& rank() const = 0;
		
	};

	template <typename T>
	class TResultArgsImpl : public ResultArgsImpl {
	public:
		TResultArgsImpl()
			:
			m_oValueArray	(1)	// prevent empty vector
		{}

		TResultArgsImpl(const geo2d::TArray<T>& p_rValue, geo2d::TRange<T> p_oDeviation)
			:
			m_oValueArray	(p_rValue), 
			m_oDeviation	(p_oDeviation) 
		{}

		TResultArgsImpl(MessageBuffer const& buffer) 
		{  
			//handle data and rank separately because message::FindSerializer does not recognize TArray 
			 Serializable::deMarshal<std::vector<T>>(buffer, m_oValueArray.getData()); 
			 Serializable::deMarshal<std::vector<int>>(buffer, m_oValueArray.getRank()); 
			 Serializable::deMarshal<geo2d::TRange<T> >(buffer, m_oDeviation);
        }
		
		/*virtual*/ TResultArgsImpl* clone() const { 
			return new TResultArgsImpl(m_oValueArray, m_oDeviation) ;
		}
		
		/*virtual*/ PvString toString() const  {             
            auto pVec = value();
            auto pRank = rank();
    		std::stringstream pOStream;
            for (int n=0; n< int(pVec.size()); n++)
            {
                if (n%20== 0) 
                {
                    pOStream << "\n\t ";
                }
                pOStream << pVec[n] << "(" << pRank[n] << ") " ;
            }
            pOStream << "\n";
     
			return pOStream.str(); 
		}
		
		/*virtual*/ bool empty() const {
			return value().empty();
		}

		const std::vector<T>& value() const { 
			return m_oValueArray.getData(); 
		}
        
        const std::vector<int>& rank() const 
        { 
			return m_oValueArray.getRank(); 
		}	

		static  ResultArgsImpl* create(MessageBuffer const& buffer) { 
			return new TResultArgsImpl(buffer); 
		}

		geo2d::TRange<T> deviation() const { 
			return m_oDeviation; 
		}

		void setdeviation(T &lower, T &upper) { m_oDeviation.start() = lower; m_oDeviation.end() = upper; }

		
		/*virtual*/ void serialize ( MessageBuffer &buffer ) const {
			//handle data and rank separately because message::FindSerializer does not recognize TArray 
			marshal(buffer, m_oValueArray.getData());
			marshal(buffer, m_oValueArray.getRank());
			marshal(buffer, m_oDeviation);
		}
		
		/*virtual*/ void deserialize( MessageBuffer const& buffer ) { 
			//handle data and rank separately because message::FindSerializer does not recognize TArray 
			Serializable::deMarshal<std::vector<T>>(buffer, m_oValueArray.getData());
			Serializable::deMarshal<std::vector<int>>(buffer, m_oValueArray.getRank());
			Serializable::deMarshal<geo2d::TRange<T> >(buffer, m_oDeviation); 
		}

	private:
		geo2d::TArray<T>	m_oValueArray;
		geo2d::TRange<T>	m_oDeviation;
	};


	template<typename T>	class TResultArgs;

	class INTERFACES_API ResultArgs : public Serializable {
	public:
		ResultArgs() 
			: 
			ra_			(nullptr), 
			isValid_	(false), 
			isNio_		(false) 
		{}
		virtual ~ResultArgs()
		{
			if (ra_)
			{
				delete ra_;
			}
		}
		ResultArgs(ResultArgs const& rhs)
		{
			ra_ = rhs.ra_? rhs.ra_->clone() : nullptr; copyValues(rhs);
		}
		template<typename T>
		ResultArgs(TResultArgs<T> const& rhs)
		{
			ra_ = rhs.ra_? rhs.ra_->clone() : NULL; copyValues(rhs);
		}
		ResultArgs & operator =(ResultArgs const& rhs)
		{
			if (ra_) {  delete ra_; }
			if (rhs.ra_)
			{
				 ra_ = rhs.ra_->clone();
			} else
			{
				// should never really happen
				ra_ = NULL;
			}
			copyValues(rhs);
			return *this;
		}
		ResultArgs(MessageBuffer const& buffer) { deserialize(buffer);	}

		ResultArgs(
			Poco::UUID const&		filterID,
			ResultType				rs, 
			ResultType				nio, 
			ImageContext const&		c, 
			bool					isValidFlag, 
			bool					isNioFlag, 
			RegTypes				type, 
			ResultArgsImpl*			ra);

		template<typename T>
		friend class TResultArgs;

		// check for correct type before calling this!!
		template<typename T>
        const std::vector<T>& value() const
        {
            assert(ra_ != nullptr);
            return ra_->value<T>();
        }
        // check for validity before calling this!!
        const std::vector<int>& rank() const
        {            
            assert(ra_ != nullptr);
            return ra_->rank();
        }
        template<typename T>
        geo2d::TRange<T> deviation() const 
        { 
            assert(dynamic_cast<TResultArgsImpl<T>*>(ra_));
            return dynamic_cast<TResultArgsImpl<T>*>(ra_)->deviation(); 
        }

    public:
		// accessors
		/// generisch kann der Wert nur als String ausgelesen werden
		operator const PvString() const 
		{ 
            if (ra_ == nullptr) 
                return "NULL"; 
            return ra_->toString(); 
        }
		const PvString getString() const 
		{ 
            if (ra_ == nullptr) 
                return "NULL";
            return ra_->toString(); 
        }
		const TaskContext& taskContext() const { return context_.taskContext(); }
		const ImageContext& context() const { return context_; }

		virtual void serialize ( MessageBuffer &buffer ) const;
		virtual void deserialize( MessageBuffer const& buffer );
		const Poco::UUID& filterId() const { return filterID_; }
		ResultType resultType() const { return resultType_; } // das ist der ResultatTyp, also WM-spezifisch (GapMismatch, ....)
		void setResultType(const ResultType rt) {resultType_ = rt; }
		void setNioType(const ResultType rt) { nioType_ = rt; } /// necessary for global SumErrors, they need to change the results result type!
		ResultType nioType() const { return nioType_; }
		void setNio(const bool oValue) { isNio_ = oValue; }
		bool isNio() const { return isNio_; }
        void setValid(const bool oValue) 
        { 
            isValid_ = oValue; 
            assert (!isValid_ || ra_ != nullptr);
        }
        bool isValid() const { return isValid_ && ra_->empty() == false; }
		void setContext(ImageContext const& cxt) { context_ = cxt; }
		int type() const { return valueType_; } // das ist der Typ des Wertes (float, int, ...)
		const PvString toString() const 
		{ 
            if (ra_ == nullptr)
                return "NULL";
            return ra_->toString(); 
        }
        const std::vector<float>& signalQuality() const { return signalQuality_; }
        const geo2d::VecDPoint& nioPercentage() const { return nioPercentage_; }
        const geo2d::VecDPoint& upperReference() const { return upperReference_; }
        const geo2d::VecDPoint& lowerReference() const { return lowerReference_; }
        void setQuality(const std::vector<float>& quality) { signalQuality_ = quality; }
        void setNioResult(const geo2d::VecDPoint& nioPercentage) { nioPercentage_ = nioPercentage; }
        void setUpperReference(const geo2d::VecDPoint& upperReference) { upperReference_ = upperReference; }
        void setLowerReference(const geo2d::VecDPoint& lowerReference) { lowerReference_ = lowerReference; }
        
		friend  std::ostream &operator <<(std::ostream &os, ResultArgs const& a) {
			os << RegTypes(a.type()); os.flush();
			os << " FilterID: " << a.filterID_.toString(); os.flush();
			os << " Valid: " << a.isValid_ << "\t";
			os << " ResultType: " << a.resultType_ << "\t";
			os << " NioType: " << a.nioType_ << "\t";
			os << " IsNio: " << a.isNio_ << "\t";
			os << "@ " << a.context_ << "\t";
			os << " = " << a.toString();
			return os;
		}
	protected:
		void copyValues(ResultArgs const& rhs);
	private:
		ResultArgsImpl* deserializeRa( MessageBuffer const& buffer, RegTypes valueType );
	protected:
		RegTypes			valueType_;     ///< type id of value_; deviation is Range<typeof(value)>
		ResultArgsImpl*		ra_;			///< holds the typedependent values
		Poco::UUID			filterID_;		///< Das Resultat wurde in diesem Filter berechnet; Gesetz durch ResultFilter
		ResultType			resultType_;	///< (wm) semantic type of result
		ResultType			nioType_;		///< nio type
		bool				isValid_;		///< Validity - processing flag for windows
		bool				isNio_;         ///< is nio?
		ImageContext		context_;		///< image context and image data
		std::vector<float>	signalQuality_;	///< quality of each signal value (0.0 means equal to reference, 100.0 means at bottom/top reference and just counted as NIO, >=100.0 means NIO)
		geo2d::VecDPoint nioPercentage_;
        geo2d::VecDPoint upperReference_; ///< values of the upper reference curve (NULL if no reference is used to check this signal)
        geo2d::VecDPoint lowerReference_; ///< values of the upper reference curve (NULL if no reference is used to check this signal)
	};

	template<typename T>
	class TResultArgs : public ResultArgs {
	public:
		TResultArgs(	const Poco::UUID&		p_rFilterId, 
						ResultType				p_oResultType, 
						ResultType				p_oNioType, 
						const ImageContext&		p_rImageContext, 
						const TGeo<T>&			p_rGeoValue, 
						geo2d::TRange<T>		p_oDeviation,
						bool					p_oIsNio = false)
			: 
			ResultArgs	{	p_rFilterId, 
							p_oResultType, 
							p_oNioType, 
							p_rImageContext,
							true,						// result is always valid
							p_oIsNio,
							RegTypes					( RegisteredType<T>::Value ), 
							new TResultArgsImpl<T>		( p_rGeoValue.ref(), p_oDeviation ) }
		{}

		TResultArgs(	const Poco::UUID&				p_rFilterId, 
						ResultType						p_oResultType, 
						ResultType						p_oNioType, 
						const ImageContext&				p_rImageContext, 
						const TGeo<geo2d::TArray<T>>&	p_rGeoArray, 
						geo2d::TRange<T>				p_oDeviation,
						bool							p_oIsNio = false)
			: 
			ResultArgs	{	p_rFilterId, 
							p_oResultType, 
							p_oNioType, 
							p_rImageContext,
							true,						// result is always valid
							p_oIsNio,
							RegTypes					( RegisteredType<T>::Value ), 
							new TResultArgsImpl<T>		( p_rGeoArray.ref(), p_oDeviation ) } 
		{}

		TResultArgs() {};

		TResultArgs(ResultArgs const& rhs) {
			ra_ = rhs.ra_ ? rhs.ra_->clone() : NULL;
			copyValues(rhs);
		}

		TResultArgs(MessageBuffer const& buffer) : ResultArgs(buffer) {}
		TResultArgs & operator = (ResultArgs const& rhs) {
			// TResultArgs tmp(rhs); swap(tmp); return *this;
			if (ra_) delete ra_;
			ra_ = rhs.ra_ ? rhs.ra_->clone() : NULL;
			copyValues(rhs);
			return *this;
		}

		const std::vector<T>& value() const { return dynamic_cast<TResultArgsImpl<T>*>(ra_)->value(); }
		geo2d::TRange<T> deviation() const { return dynamic_cast<TResultArgsImpl<T>*>(ra_)->deviation(); }
		void setdeviation(T &lower, T &upper) { dynamic_cast<TResultArgsImpl<T>*>(ra_)->setdeviation(lower, upper); }
	};

	// Standardtypen fuer Resultate
	typedef std::vector<ResultArgs>			ResultArgsList;
	typedef TResultArgs	< int >				ResultIntArray;
	typedef TResultArgs	< double >			ResultDoubleArray;
	typedef TResultArgs	< geo2d::Range >	ResultRangeArray;
	typedef TResultArgs	< geo2d::Range1d >	ResultRange1dArray;
	typedef TResultArgs < geo2d::Point >	ResultPointArray;
	typedef ResultArgs NIOResult;



	inline void ResultArgs::deserialize( MessageBuffer const& buffer )	{
		filterID_	= Serializable::deMarshal<Poco::UUID>(buffer); // Id
		resultType_ = Serializable::deMarshal<ResultType>(buffer); // ResultType and NIOType
		nioType_	= Serializable::deMarshal<ResultType>(buffer);
		isNio_		= Serializable::deMarshal<bool>(buffer);
		context_	= Serializable::deMarshal<ImageContext>(buffer); // context
		isValid_	= Serializable::deMarshal<bool>(buffer);
		valueType_ = RegTypes(Serializable::deMarshal<uInt>(buffer));
		// to avoid mem-leak wenn ra_ is overwritten
		if (ra_)
		{
			delete ra_;
			ra_ = NULL;
		}

		bool paramAvailable = Serializable::deMarshal<bool>(buffer);
		if (paramAvailable)
		{
			ra_ = deserializeRa(buffer, valueType_);
		} else
		{
			ra_ = NULL;
		}
        paramAvailable = Serializable::deMarshal<bool>(buffer);
        if (paramAvailable)
        {
            std::size_t size = Serializable::deMarshal<std::size_t>(buffer);
            Serializable::deMarshal(buffer, signalQuality_, size);
        }
        paramAvailable = Serializable::deMarshal<bool>(buffer);
        if (paramAvailable)
        {
            std::size_t size = Serializable::deMarshal<std::size_t>(buffer);
            Serializable::deMarshal(buffer, nioPercentage_, size);
        }
        paramAvailable = Serializable::deMarshal<bool>(buffer);
        if (paramAvailable)
        {
            std::size_t size = Serializable::deMarshal<std::size_t>(buffer);
            Serializable::deMarshal(buffer, upperReference_, size);
        }
        paramAvailable = Serializable::deMarshal<bool>(buffer);
        if (paramAvailable)
        {
            std::size_t size = Serializable::deMarshal<std::size_t>(buffer);
            Serializable::deMarshal(buffer, lowerReference_, size);
        }
	}

	inline ResultArgsImpl *ResultArgs::deserializeRa( MessageBuffer const& buffer, RegTypes valueType)	{
		typedef ResultArgsImpl* (*ResultFactory)(MessageBuffer const& buffer);
		// Achtung! Initialiseirungreihenfolge muss mit Typ-Registrierung (geo.h) uebereinstimmen!
		static ResultFactory resultFactoryList[NumRegTypes] = {
			TResultArgsImpl<int>::create,				// deprectated, creates a vector now. Needed as placeholder, because new type RegDoubleArray must not replace an existing number.
			TResultArgsImpl<double>::create,			// deprectated, creates a vector now. Needed as placeholder, because new type RegDoubleArray must not replace an existing number.
			TResultArgsImpl<geo2d::Range>::create,		// deprectated, creates a vector now. Needed as placeholder, because new type RegDoubleArray must not replace an existing number.
			TResultArgsImpl<geo2d::Range1d>::create,	// deprectated, creates a vector now. Needed as placeholder, because new type RegDoubleArray must not replace an existing number.
			TResultArgsImpl<geo2d::Point>::create,		// deprectated, creates a vector now. Needed as placeholder, because new type RegDoubleArray must not replace an existing number.
			TResultArgsImpl<double>::create				// matches 'RegDoubleArray', creates a result containing a double vector
		};
		return resultFactoryList[valueType](buffer);
	}

	inline void ResultArgs::serialize ( MessageBuffer &buffer ) const {
		marshal(buffer, filterID_);
		marshal(buffer, resultType_);
		marshal(buffer, nioType_);
		marshal(buffer, isNio_);
		marshal(buffer, context_);
		marshal(buffer, isValid_);
		marshal(buffer, valueType_);
		if (!ra_) { // null-ptr treatment
			marshal(buffer, false);
		}else	{
			marshal(buffer, true);
			ra_->serialize(buffer);
		}
        if (!signalQuality_.empty())
        {
            marshal(buffer, true);
            marshal(buffer, signalQuality_.size());
            marshal(buffer, signalQuality_, signalQuality_.size());
        }
        else
        {
            marshal(buffer, false);
        }
        if (!nioPercentage_.empty())
        {
            marshal(buffer, true);
            marshal(buffer, nioPercentage_.size());
            marshal(buffer, nioPercentage_, nioPercentage_.size());
        }
        else
        {
            marshal(buffer, false);
        }
        if (!upperReference_.empty())
        {
            marshal(buffer, true);
            marshal(buffer, upperReference_.size());
            marshal(buffer, upperReference_, upperReference_.size());
        }
        else
        {
            marshal(buffer, false);
        }
        if (!lowerReference_.empty())
        {
            marshal(buffer, true);
            marshal(buffer, lowerReference_.size());
            marshal(buffer, lowerReference_, lowerReference_.size());
        }
        else
        {
            marshal(buffer, false);
        }
        
        
        
	}

	typedef Poco::SharedPtr<ResultArgs> SmpResultPtr;
	typedef std::vector<SmpResultPtr> SmpResultVector;
	typedef std::queue<SmpResultPtr> SmpResultQueue;

} // namespace interface
} // namespace precitec

#endif /*RESULTS_H_*/
