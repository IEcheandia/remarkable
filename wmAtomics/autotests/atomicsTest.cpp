#include "../../Mod_Grabber/autotests/testHelper.h"
#include "../wmAtomics.h"

#include <future>
#include <vector>
#include <iostream>

using namespace precitec::utils;

template <typename Function>
static int32_t testIncrement(Function f, int32_t startValue)
{
    // test the atomic increment/decrement
    // we start 100 threads, each thread (in|de)crements by one
    // in the end the printed result should be: startValue + 100
    // or in case of decrement: startValue - 100
    // if the increment is not atomic, we would lose operations
    volatile int32_t test = startValue;
    std::vector<std::future<void>> futures;
    for (int i = 0; i < 100; i++) {
        futures.push_back(std::async(std::launch::async, f, &test));
    }
    for (auto &f : futures) {
        f.wait();
    }
    return test;
}

class WmAtomicsTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(WmAtomicsTest);
CPPUNIT_TEST(increment);
CPPUNIT_TEST(decrement);
CPPUNIT_TEST(compareAndSwap);
CPPUNIT_TEST(exchange);
CPPUNIT_TEST_SUITE_END();
public:
    void increment();
    void decrement();
    void compareAndSwap();
    void exchange();
};

void WmAtomicsTest::increment()
{
    CPPUNIT_ASSERT_EQUAL(100, testIncrement(&atomicInc32, 0));
    CPPUNIT_ASSERT_EQUAL(500, testIncrement(&atomicInc32, 400));
    CPPUNIT_ASSERT_EQUAL(0, testIncrement(&atomicInc32, -100));
}

void WmAtomicsTest::decrement()
{
    CPPUNIT_ASSERT_EQUAL(0, testIncrement(&atomicDec32, 100));
    CPPUNIT_ASSERT_EQUAL(400, testIncrement(&atomicDec32, 500));
    CPPUNIT_ASSERT_EQUAL(-100, testIncrement(&atomicDec32, 0));
}

void WmAtomicsTest::compareAndSwap()
{
    // tests atomic compare and swap
    // each thread has an identifier number
    // if the lock variable has the content of the thread variable
    // the content gets incremented by 1 and the thread terminates
    // otherwise it continues to wait in a spin lock till the lock
    // has the expected number
    // if the method terminates all threads operated as expected
    volatile int32_t lock = 0;
    auto threadFunc = [] (volatile int32_t *lock, int threadNumber) {
        while (threadNumber != atomicCASInt32(lock, threadNumber, threadNumber + 1)) {
            // spin lock
        }
        std::cout << "Output from thread " << threadNumber << std::endl;
    };
    std::vector<std::future<void>> futures;
    for (int i = 0; i < 100; i++) {
        futures.push_back(std::async(std::launch::async, threadFunc, &lock, i));
    }
    for (auto &f : futures) {
        f.wait();
    }
}

void WmAtomicsTest::exchange()
{
    // just some tests to verify that atomicXchg does the correct tasks
    int32_t test = 0;
    CPPUNIT_ASSERT_EQUAL(0, atomicXchgInt32(&test, 1));
    CPPUNIT_ASSERT_EQUAL(1, test);
    CPPUNIT_ASSERT_EQUAL(1, atomicXchgInt32(&test, 0));
    CPPUNIT_ASSERT_EQUAL(0, test);
    int32_t *ptr = &test;
    int32_t value = 1;
    CPPUNIT_ASSERT_EQUAL((void*)&test, atomicXchgPtr32((volatile void**)&ptr, (void*)&value));
    CPPUNIT_ASSERT_EQUAL(&value, ptr);
    CPPUNIT_ASSERT_EQUAL((void*)&value, atomicXchgPtr32((volatile void**)&ptr, (void*)&value));
    CPPUNIT_ASSERT_EQUAL(&value, ptr);
}

TEST_MAIN(WmAtomicsTest)
