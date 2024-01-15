#ifndef VISERVICE_H_
#define VISERVICE_H_

#include "message/messageBuffer.h"
#include "message/serializer.h"

#if defined __linux__
	#include "common/ethercat.h"
#else
	#include "AtEthercat.h"
#endif

namespace precitec {
using namespace system;
using namespace message;

namespace interface {

//typedef char*	ProcessData;

#define MAX_SIZE 10000
#define MAX_SLAVE_COUNT 50

class ProcessData: public Serializable {

protected:
	int m_size;
	int m_offset;

public:
	char m_data[MAX_SIZE]; //TODO: check size

	ProcessData() {
		m_size = 0;
		m_offset = 0;

		memset(m_data, 0, MAX_SIZE);
	}

    ProcessData(int size, const char *data)
        : m_size(size)
    {
        FillBuffer(data);
    }

	int GetSize() const {
		return m_size;
	}
	void SetSize(int size) {
		m_size = size;
	}
	void SetOffset(int offset) {
		m_offset = offset;
	}
	int GetOffset() const {
		return m_offset;
	}
	const char* GetData() const {
		return m_data;
	}

	void FillBuffer(const char* data) {
		if (m_size <= 0) {
			throw new Poco::Exception(
					"ProcessData::FillBuffer: Buffer size <= 0");
		}
		if (m_size > MAX_SIZE) {
			throw new Poco::Exception(
					"ProcessData::FillBuffer: Buffer overflow!");
		}

		memcpy(m_data, data, m_size);
	}

	virtual ~ProcessData() {

	}

	virtual void serialize(MessageBuffer & buffer) const {
		marshal(buffer, m_size);
		marshal(buffer, m_offset);
		marshal(buffer, m_data, m_size);

	}
	virtual void deserialize(MessageBuffer const& buffer) {
		deMarshal(buffer, m_size);
		deMarshal(buffer, m_offset);
		deMarshal(buffer, m_data, m_size);

	}

};

class SlaveInfo: public Serializable {
private:

	int m_count;

	EC_T_GET_SLAVE_INFO m_slaves[MAX_SLAVE_COUNT]; //TODO: check size
public:

	std::vector<EC_T_GET_SLAVE_INFO> GetSlaveInfoVector(){
		std::vector<EC_T_GET_SLAVE_INFO> ret;
		for (int i = 0; i < m_count; ++i) {
			ret.push_back(m_slaves[i]);
		}
		return ret;
	}

	int GetSize() {
		return m_count;
	}
	EC_T_GET_SLAVE_INFO* GetDataPtr() {
		return m_slaves;
	}
	EC_T_GET_SLAVE_INFO GetInfoAt(int i) {
		return m_slaves[i];
	}

	SlaveInfo(int count) {
		m_count = count;
		if (m_count > MAX_SLAVE_COUNT) {
			throw new Poco::Exception("SlaveInfo::FillBuffer: Buffer overflow!");
		}
		memset(m_slaves, 0, sizeof(EC_T_GET_SLAVE_INFO) * m_count);
	}

	void FillBuffer(EC_T_GET_SLAVE_INFO* info) {
		memcpy(m_slaves, info, m_count * sizeof(EC_T_GET_SLAVE_INFO));
	}

	virtual ~SlaveInfo() {

	}

	virtual void serialize(MessageBuffer & buffer) const {
		marshal(buffer, m_count);
		marshal(buffer, m_slaves, m_count);
	}

	virtual void deserialize(MessageBuffer const& buffer) {
		deMarshal(buffer, m_count);
		deMarshal(buffer, m_slaves, m_count);
	}

};
typedef std::vector<ProcessData> ProcessDataVector;

} // namespace interface
}

#endif /*VISERVICE_H_*/
