/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			HS
 *  @date			2014
 *  @brief			Provides and holds reference curves.
 */

#ifndef REFERENCECURVES_H_INCLUDED_20140612
#define REFERENCECURVES_H_INCLUDED_20140612

#include "message/db.interface.h"


namespace precitec {
namespace analyzer {

/**
  *	@brief		'ReferenceCurves' measures image- and sample-processing time, frame-to-frame time, overlay time.
  *	@ingroup Analyzer
  */
class ReferenceCurves {
public:

	typedef interface::TDb<interface::AbstractInterface>			if_tdb_t;

	static const std::string	g_oNameRefCurveFilterWithNs;	///< name
	static const std::string	g_oNameRefCurveIdParameter;		///< name

	/**
	  *	@brief	CTOR initialized with seam number and hw parameter set id.
	  * @param	p_rDbProxy			DB proxy for db queries
	  */
	ReferenceCurves(if_tdb_t* p_pDbProxy);

	/**
	  * @brief requests and stores product reference curves
	  * @param	p_rParameterList	parameter list
	  */
	void requestAndStoreRefCurves(const interface::ParameterList& p_rParameterList);

	/**
	  * @brief getter reference curves map
	  * @return						reference curves map
	  */
	const interface::id_refcurve_map_t* getRefCurveMap() const;

	/**
	  * @brief setter db proxy
	  * @return						void
	  */
	void setDbProxy(if_tdb_t* p_pDbProxy) 
	{ 
		std::cout << "Reference curves set proxy" << std::endl; 
		m_pDbProxy = p_pDbProxy; 
	};

private:

	/**
	  * @brief requests and stores product reference curves
	  * @param	p_rParameterList	parameter list
	  * @return						all ref curve ids found in parameter list
	  */
	std::vector<Poco::UUID> getRefCurveIds(const interface::ParameterList& p_rParameterList) const;

	/**
	  * @brief requests and stores product reference curves
	  * @param	p_rRefCurveIds		 ref curve ids
	  */
	void requestAndStoreRefCurves(const std::vector<Poco::UUID>& p_rRefCurveIds);

	if_tdb_t* 						m_pDbProxy;				///< db proxy
	interface::id_refcurve_map_t	m_oIdRefCurveMap;		///< stores reference curves
}; // class ReferenceCurves


} // namespace analyzer
} // namespace precitec

#endif /* REFERENCECURVES_H_INCLUDED_20140612 */
