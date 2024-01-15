/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR, HS
 * 	@date		2010
 *	@brief		Defines overlay text font.
 */

#ifndef FONT_H_20131210_INCLUDED
#define FONT_H_20131210_INCLUDED

#include <string>

#include "system/types.h"
#include "message/messageBuffer.h"
#include "message/serializer.h"
#include "InterfacesManifest.h"

namespace precitec {
namespace image {

	class INTERFACES_API Font : public precitec::system::message::Serializable
	{
	public:
		Font() : size(10), bold(false), italic(false), name("Arial") {}
		explicit Font(int s, bool b = false, bool i = false, std::string n = "Arial") : size(s), bold(b), italic(i), name(n) {}
		friend  std::ostream &operator <<(std::ostream &os, Font const& f) {
			os << "Font: " << f.name ; return os;
		}
		int size;
		bool bold;
		bool italic;
		std::string name;
	public:
		virtual void serialize ( system::message::MessageBuffer &buffer ) const
		{
			marshal(buffer, size);
			marshal(buffer, bold);
			marshal(buffer, italic);
			marshal(buffer, name);
		}

		virtual void deserialize( system::message::MessageBuffer const&buffer )
		{
			deMarshal(buffer, size);
			deMarshal(buffer, bold);
			deMarshal(buffer, italic);
			deMarshal(buffer, name);
		}
	};



} // namespace image
} // namespace precitec

#endif /*FONT_H_20131210_INCLUDED*/
