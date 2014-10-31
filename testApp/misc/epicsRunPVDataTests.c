/*
 * Run pvData tests as a batch.
 *
 * Do *not* include performance measurements here, they don't help to
 * prove functionality (which is the point of this convenience routine).
 */

#include <stdio.h>
#include <epicsThread.h>
#include <epicsUnitTest.h>

int testBaseException(void);
int testBitSet(void);
int testByteBuffer(void);
int testMessageQueue(void);
int testOverrunBitSet(void);
int testQueue(void);
int testSerialization(void);
int testSharedVector(void);
int testThread(void);
int testTimeStamp(void);
int testTimer(void);
int testTypeCast(void);

void epicsRunPVDataTests(void)
{
    testHarness();

    runTest(testBaseException);
    runTest(testBitSet);
    runTest(testByteBuffer);
    runTest(testMessageQueue);
    runTest(testOverrunBitSet);
    runTest(testQueue);
    runTest(testSerialization);
    runTest(testSharedVector);
    runTest(testThread);
    runTest(testTimeStamp);
    runTest(testTimer);
    runTest(testTypeCast);
}
