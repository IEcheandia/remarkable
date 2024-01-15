#pragma once

#include <vector>
#include <cstdint>
#include <cmath>

#include "module/moduleLogger.h"

namespace precitec
{

namespace ethercat
{

template <typename T>
class RingBuffer
{
public:
    RingBuffer(void) = delete;
    RingBuffer(unsigned p_oBufferSize);
    virtual ~RingBuffer();

    bool IsBufferFull();

    void Write(T p_oInput);
    T Read(void);

    void StartReadOut(void);
    void SetReadIsActive(bool p_oValue);

    void SetValuesPerECATCycle(unsigned int p_oValue);
    unsigned int GetValuesPerECATCycle(void);

    void SetTriggerDist_ns(unsigned int p_oValue);
    unsigned int GetTriggerDist_ns(void);

    unsigned int GetValuesPerImageCycle(void);

    unsigned int GetReadIndex(void);
    unsigned int GetWriteIndex(void);
    unsigned int GetBufferSize(void);

private:
    void CalcValuesPerImageCycle(void);

    std::vector<T> m_oBuffer;
    bool m_oBufferFull;
    unsigned int m_oReadIndex;
    unsigned int m_oWriteIndex;
    bool m_oReadIsActive;

    unsigned int m_oValuesPerECATCycle;
    unsigned int m_oTriggerDist_ns;
    unsigned int m_oValuesPerImageCycle;
};
 
template <typename T>
RingBuffer<T>::RingBuffer(unsigned p_oBufferSize): 
        m_oBuffer(p_oBufferSize),
        m_oBufferFull(false),
        m_oReadIndex(0),
        m_oWriteIndex(p_oBufferSize - 1),
        m_oReadIsActive(false),
        m_oValuesPerECATCycle(50),
        m_oTriggerDist_ns(100000000), // 100ms
        m_oValuesPerImageCycle(5000)
{
}
 
template <typename T>
RingBuffer<T>::~RingBuffer()
{
}
 
template <typename T>
bool RingBuffer<T>::IsBufferFull()
{
    return m_oBufferFull;
}

template <typename T>
void RingBuffer<T>::Write(T p_oInput)
{
    m_oBuffer[m_oWriteIndex++] = p_oInput;
    if(m_oWriteIndex >= m_oBuffer.size())
    {
        m_oWriteIndex = 0;
        m_oBufferFull =  true;
    }
    if (m_oReadIsActive)
    {
        if (m_oWriteIndex == m_oReadIndex)
        {
            wmLog(eError, "XXXXXXXXXX Write: Indizes sind ident ! (WR:%d RD:%d)\n", m_oWriteIndex, m_oReadIndex);
        }
    }
}
 
template <typename T>
T RingBuffer<T>::Read()
{
    T val = m_oBuffer[m_oReadIndex++];
    if(m_oReadIndex >= m_oBuffer.size())
    {
        m_oReadIndex = 0;
    }
    if (m_oReadIsActive)
    {
        if (m_oWriteIndex == m_oReadIndex)
        {
            wmLog(eDebug, "XXXXXXXXXX Read: Indizes sind ident ! (RD:%d WR:%d)\n", m_oReadIndex, m_oWriteIndex);
        }
    }
    return val;
}

template <typename T>
void RingBuffer<T>::StartReadOut(void)
{
    if (m_oWriteIndex >= (m_oValuesPerImageCycle + 1))
    {
        m_oReadIndex = m_oWriteIndex - (m_oValuesPerImageCycle + 1);
    }
    else
    {
        unsigned int oDiff = (m_oValuesPerImageCycle + 1) - m_oWriteIndex;
        m_oReadIndex = m_oBuffer.size() - oDiff;
    }
//wmLog(eDebug, "WR: %d, RD: %d, ImgCyc: %d, ECATCyc: %d, trgDist: %d\n", m_oWriteIndex, m_oReadIndex, m_oValuesPerImageCycle, m_oValuesPerECATCycle, m_oTriggerDist_ns);
}

template <typename T>
void RingBuffer<T>::SetReadIsActive(bool p_oValue)
{
    m_oReadIsActive = p_oValue;
}

template <typename T>
void RingBuffer<T>::SetValuesPerECATCycle(unsigned int p_oValue)
{
    m_oValuesPerECATCycle = p_oValue;
    CalcValuesPerImageCycle();
}

template <typename T>
unsigned int RingBuffer<T>::GetValuesPerECATCycle(void)
{
    return m_oValuesPerECATCycle;
}

template <typename T>
void RingBuffer<T>::SetTriggerDist_ns(unsigned int p_oValue)
{
    m_oTriggerDist_ns = p_oValue;
    CalcValuesPerImageCycle();
}

template <typename T>
unsigned int RingBuffer<T>::GetTriggerDist_ns(void)
{
    return m_oTriggerDist_ns;
}

template <typename T>
unsigned int RingBuffer<T>::GetValuesPerImageCycle(void)
{
    return m_oValuesPerImageCycle;
}

template <typename T>
void RingBuffer<T>::CalcValuesPerImageCycle(void)
{
    float oHelpFloat1 = static_cast<float>(m_oValuesPerECATCycle);
    float oHelpFloat2 = static_cast<float>(m_oTriggerDist_ns) / 1000000.0; // now in ms
    float oHelpFloat3 = oHelpFloat1 * oHelpFloat2;
    m_oValuesPerImageCycle = static_cast<unsigned int>(std::lround(oHelpFloat3));
}

template <typename T>
unsigned int RingBuffer<T>::GetReadIndex(void)
{
    return m_oReadIndex;
}

template <typename T>
unsigned int RingBuffer<T>::GetWriteIndex(void)
{
    return m_oWriteIndex;
}

template <typename T>
unsigned int RingBuffer<T>::GetBufferSize(void)
{
    return m_oBuffer.size();
}

} // namespace ethercat
} // namespace precitec

