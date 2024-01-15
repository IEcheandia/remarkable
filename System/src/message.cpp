#include "message/message.h"
#include "message/messageBuffer.h"

namespace precitec
{
namespace system
{
namespace message
{
/*
	void SYSTEM_API	MessageBuffer::setData(int size) {
		if (rawData_) delete [] rawData_;
		rawData_ = new char [size+sizeof(Header)];
		//header_  = new(rawData_) Header; // in-place Allokation
		data_		 = rawData_ + sizeof(Header);
		cursor_  = data_;
		m_oMaxBufferSize = size;
	}

	// fuer Null-Protocol -> \todo sollte ueber Copy-Ctor erledigbar sein, oder??
	void SYSTEM_API	MessageBuffer::copyFrom(MessageBuffer &from) {
		// Buffer-overflow vermeiden
		int xferSize = std::min(m_oMaxBufferSize, from.dataSize() );
		//std::cout << "copyFrom: " << from.msgSize() << " to: " << m_oMaxBufferSize << std::endl;
		std::memcpy(rawData_, from.rawData_, xferSize);
		// \todo muss das hier rein????
		cursor_ = data_;
	}
*/

} // message
} // system
} // precitec
