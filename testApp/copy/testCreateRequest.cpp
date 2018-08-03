/* testCreateRequest.cpp */
/*
 * Copyright information and license terms for this software can be
 * found in the file LICENSE that is included with the distribution
 */
/* Author:  Matej Sekoranja Date: 2010.12.27 */

#include <stddef.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

#include <pv/pvUnitTest.h>
#include <testMain.h>

#include <pv/current_function.h>
#include <pv/createRequest.h>
#include <pv/bitSet.h>

namespace {

using namespace epics::pvData;
using std::string;
using std::cout;
using std::endl;

static void testCreateRequestInternal() {
    testDiag("===== testCreateRequest =====");

    CreateRequest::shared_pointer createRequest = CreateRequest::create();
    PVStructurePtr pvRequest;
    PVStringPtr pvString;
    string sval;
    string out;

    string request = "";
    pvRequest = createRequest->createRequest(request);
    if (testOk(!!pvRequest, "createRequest('%s')", request.c_str())) {
        testOk1(pvRequest->getStructure()->getNumberFields()==0);
    }
    else testSkip(1, createRequest->getMessage().c_str());

    request = "record[]field()getField()putField()";
    pvRequest = createRequest->createRequest(request);
    if (testOk(!!pvRequest, "createRequest('%s')", request.c_str())) {
        testOk1(!!pvRequest->getSubField<PVStructure>("field"));
        testOk1(!!pvRequest->getSubField<PVStructure>("putField"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField"));
    }
    else testSkip(3, createRequest->getMessage().c_str());

    request = "record[a=b,x=y]field(a) putField(a),getField(a)";
    pvRequest = createRequest->createRequest(request);
    if (testOk(!!pvRequest, "createRequest('%s')", request.c_str())) {
        pvString = pvRequest->getSubField<PVString>("record._options.a");
        sval = pvString->get();
        testOk(sval.compare("b")==0,"record.a = b");
        pvString = pvRequest->getSubField<PVString>("record._options.x");
        sval = pvString->get();
        testOk(sval.compare("y")==0,"record.x = y");
        testOk1(!!pvRequest->getSubField<PVStructure>("field.a"));
        testOk1(!!pvRequest->getSubField<PVStructure>("putField.a"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.a"));
    }
    else testSkip(5, createRequest->getMessage().c_str());

    request = "field(a.b[x=y])";
    pvRequest = createRequest->createRequest(request);
    if (testOk(!!pvRequest, "createRequest('%s')", request.c_str())) {
        pvString = pvRequest->getSubField<PVString>("field.a.b._options.x");
        sval = pvString->get();
        testOk(sval.compare("y")==0,"field.a.b._options.x = y");
    }
    else testSkip(1, createRequest->getMessage().c_str());

    request = "field(a.b{c.d})";
    pvRequest = createRequest->createRequest(request);
    if (testOk(!!pvRequest, "createRequest('%s')", request.c_str())) {
        testOk1(!!pvRequest->getSubField("field.a.b.c.d"));
    }
    else testSkip(1, createRequest->getMessage().c_str());

    request = "field(a.b[x=y]{c.d})";
    pvRequest = createRequest->createRequest(request);
    if (testOk(!!pvRequest, "createRequest('%s')", request.c_str())) {
        pvString = pvRequest->getSubField<PVString>("field.a.b._options.x");
        sval = pvString->get();
        testOk(sval.compare("y")==0,"field.a.b._options.x = y");
        testOk1(!!pvRequest->getSubField<PVStructure>("field.a.b.c.d"));
    }
    else testSkip(2, createRequest->getMessage().c_str());

    request = "field(a.b[x=y]{c.d[x=y]})";
    pvRequest = createRequest->createRequest(request);
    if (testOk(!!pvRequest, "createRequest('%s')", request.c_str())) {
        pvString = pvRequest->getSubField<PVString>("field.a.b._options.x");
        sval = pvString->get();
        testOk(sval.compare("y")==0,"field.a.b._options.x = y");
        pvString = pvRequest->getSubField<PVString>("field.a.b.c.d._options.x");
        sval = pvString->get();
        testOk(sval.compare("y")==0,"field.a.b.c.d._options.x = y");
    }
    else testSkip(2, createRequest->getMessage().c_str());

    request = "record[a=b,c=d] field(a.a[a=b]{a.a[a=b]},b.a[a=b]{a,b})";
    pvRequest = createRequest->createRequest(request);
    if (testOk(!!pvRequest, "createRequest('%s')", request.c_str())) {
        pvString = pvRequest->getSubField<PVString>("field.a.a._options.a");
        sval = pvString->get();
        testOk(sval.compare("b")==0,"field.a.a._options.a = b");
        pvString = pvRequest->getSubField<PVString>("field.a.a.a.a._options.a");
        sval = pvString->get();
        testOk(sval.compare("b")==0,"field.a.a.a.a._options.a = b");
        pvString = pvRequest->getSubField<PVString>("field.b.a._options.a");
        sval = pvString->get();
        testOk(sval.compare("b")==0,"field.b.a._options.a = b");
        testOk1(!!pvRequest->getSubField<PVStructure>("field.b.a.a"));
        testOk1(!!pvRequest->getSubField<PVStructure>("field.b.a.b"));
    }
    else testSkip(5, createRequest->getMessage().c_str());

    request = "alarm,timeStamp,power.value";
    pvRequest = createRequest->createRequest(request);
    if (testOk(!!pvRequest, "createRequest('%s')", request.c_str())) {
        testOk1(!!pvRequest->getSubField<PVStructure>("field.alarm"));
        testOk1(!!pvRequest->getSubField<PVStructure>("field.timeStamp"));
        testOk1(!!pvRequest->getSubField<PVStructure>("field.power.value"));
    }
    else testSkip(3, createRequest->getMessage().c_str());

    request = "record[process=true]"
        "field(alarm,timeStamp,power.value)";
    pvRequest = createRequest->createRequest(request);
    if (testOk(!!pvRequest, "createRequest('%s')", request.c_str())) {
        pvString = pvRequest->getSubField<PVString>("record._options.process");
        sval = pvString->get();
        testOk(sval.compare("true")==0,"record._options.process = true");
        testOk1(!!pvRequest->getSubField<PVStructure>("field.alarm"));
        testOk1(!!pvRequest->getSubField<PVStructure>("field.timeStamp"));
        testOk1(!!pvRequest->getSubField<PVStructure>("field.power.value"));
    }
    else testSkip(4, createRequest->getMessage().c_str());

    request = "record[process=true]"
        "field(alarm,timeStamp[algorithm=onChange,causeMonitor=false],"
            "power{value,alarm})";
    pvRequest = createRequest->createRequest(request);
    if (testOk(!!pvRequest, "createRequest('%s')", request.c_str())) {
        pvString = pvRequest->getSubField<PVString>("record._options.process");
        sval = pvString->get();
        testOk(sval.compare("true")==0,"record._options.process = true");
        testOk1(!!pvRequest->getSubField<PVStructure>("field.alarm"));
        testOk1(!!pvRequest->getSubField<PVStructure>("field.timeStamp"));
        pvString = pvRequest->getSubField<PVString>("field.timeStamp._options.algorithm");
        sval = pvString->get();
        testOk(sval.compare("onChange")==0,"field.timeStamp._options.algorithm = onChange");
        pvString = pvRequest->getSubField<PVString>("field.timeStamp._options.causeMonitor");
        sval = pvString->get();
        testOk(sval.compare("false")==0,"field.timeStamp._options.causeMonitor = false");
        testOk1(!!pvRequest->getSubField<PVStructure>("field.power.value"));
        testOk1(!!pvRequest->getSubField<PVStructure>("field.power.alarm"));
    }
    else testSkip(7, createRequest->getMessage().c_str());

    request = "record[int=2,float=3.14159]"
        "field(alarm,timeStamp[shareData=true],power.value)";
    pvRequest = createRequest->createRequest(request);
    if (testOk(!!pvRequest, "createRequest('%s')", request.c_str())) {
        pvString = pvRequest->getSubField<PVString>("record._options.int");
        sval = pvString->get();
        testOk(sval.compare("2")==0,"record._options.int = 2");
        pvString = pvRequest->getSubField<PVString>("record._options.float");
        sval = pvString->get();
        testOk(sval.compare("3.14159")==0,"record._options.float = 3.14159");
        testOk1(!!pvRequest->getSubField<PVStructure>("field.alarm"));
        pvString = pvRequest->getSubField<PVString>("field.timeStamp._options.shareData");
        sval = pvString->get();
        testOk(sval.compare("true")==0,"field.timeStamp._options.shareData = true");
        testOk1(!!pvRequest->getSubField<PVStructure>("field.power.value"));
    }
    else testSkip(5, createRequest->getMessage().c_str());

    request = "record[process=true,xxx=yyy]"
        "putField(power.value)"
        "getField(alarm,timeStamp,power{value,alarm},"
            "current{value,alarm},voltage{value,alarm})";
    pvRequest = createRequest->createRequest(request);
    if (testOk(!!pvRequest, "createRequest('%s')", request.c_str())) {
        testOk1(!!pvRequest->getSubField<PVStructure>("putField.power.value"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.alarm"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.timeStamp"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.power.value"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.power.alarm"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.current.value"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.current.alarm"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.voltage.value"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.voltage.alarm"));
    }
    else testSkip(9, createRequest->getMessage().c_str());

    request = "field(alarm,timeStamp,supply{"
        "zero{voltage.value,current.value,power.value},"
        "one{voltage.value,current.value,power.value}"
        "})";
    pvRequest = createRequest->createRequest(request);
    if (testOk(!!pvRequest, "createRequest('%s')", request.c_str())) {
        testOk1(!!pvRequest->getSubField<PVStructure>("field.alarm"));
        testOk1(!!pvRequest->getSubField<PVStructure>("field.timeStamp"));
        testOk1(!!pvRequest->getSubField<PVStructure>("field.supply.zero.voltage.value"));
        testOk1(!!pvRequest->getSubField<PVStructure>("field.supply.zero.current.value"));
        testOk1(!!pvRequest->getSubField<PVStructure>("field.supply.zero.power.value"));
        testOk1(!!pvRequest->getSubField<PVStructure>("field.supply.one.voltage.value"));
        testOk1(!!pvRequest->getSubField<PVStructure>("field.supply.one.current.value"));
        testOk1(!!pvRequest->getSubField<PVStructure>("field.supply.one.power.value"));
    }
    else testSkip(8, createRequest->getMessage().c_str());

    request = "record[process=true,xxx=yyy]"
        "putField(power.value)"
        "getField(alarm,timeStamp,power{value,alarm},"
            "current{value,alarm},voltage{value,alarm},"
            "ps0{alarm,timeStamp,power{value,alarm},current{value,alarm},voltage{value,alarm}},"
            "ps1{alarm,timeStamp,power{value,alarm},current{value,alarm},voltage{value,alarm}}"
            ")";
    pvRequest = createRequest->createRequest(request);
    if (testOk(!!pvRequest, "createRequest('%s')", request.c_str())) {
        testOk1(!!pvRequest->getSubField<PVStructure>("putField.power.value"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.alarm"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.timeStamp"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.power.value"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.power.alarm"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.current.value"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.current.alarm"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.voltage.value"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.voltage.alarm"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.ps0.alarm"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.ps0.timeStamp"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.ps0.power.value"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.ps0.power.alarm"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.ps0.current.value"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.ps0.current.alarm"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.ps0.voltage.value"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.ps0.voltage.alarm"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.ps1.alarm"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.ps1.timeStamp"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.ps1.power.value"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.ps1.power.alarm"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.ps1.current.value"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.ps1.current.alarm"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.ps1.voltage.value"));
        testOk1(!!pvRequest->getSubField<PVStructure>("getField.ps1.voltage.alarm"));
    }
    else testSkip(25, createRequest->getMessage().c_str());

    request = "a{b{c{d}}}";
    pvRequest = createRequest->createRequest(request);
    if (testOk(!!pvRequest, "createRequest('%s')", request.c_str())) {
        testOk1(!!pvRequest->getSubField<PVStructure>("field.a.b.c.d"));
    }
    else testSkip(1, createRequest->getMessage().c_str());
}

static void testBadRequests()
{
    testDiag("===== testBadRequests =====");

    // Basic syntax issues
    testThrows(std::runtime_error, createRequest("field("));
    testThrows(std::runtime_error, createRequest("field)"));
    testThrows(std::runtime_error, createRequest("record[field()"));
    testThrows(std::runtime_error, createRequest("record[[field()]"));
    testThrows(std::runtime_error, createRequest("record[field(]"));

    // Duplicate fieldName
    // Correct request would be: "field(A,C{D,E.F})"
    testThrows(std::invalid_argument, createRequest("field(A,C.D,C.E.F)"));

    // Use the non-throwing version for further tests
    CreateRequest::shared_pointer C(CreateRequest::create());

    testOk1(!!C->createRequest("field(value)"));
    testOk1(C->getMessage().empty());

    testOk1(!C->createRequest("field("));
    testOk(C->getMessage().find("mismatched ()") != string::npos,
        "'mismatched ()' error detected");

    string request = "record[process=true,power.value";
    testDiag("request = '%s'",request.c_str());
    testOk1(!C->createRequest(request));
    testOk(C->getMessage().find("mismatched []") != string::npos,
        "'mismatched []' error detected");

    // Buried in complex structure
    request = "record[process=true,xxx=yyy]"
        "putField(power.value)"
        "getField(alarm,timeStamp,power{value,alarm},"
            "current{value,alarm},voltage{value,alarm},"
            "ps0{alarm,timeStamp,power{value,alarm},"
                "current{value,alarm},voltage{value,alarm}},"
            "ps1{alarm,timeStamp,power{value,alarm},"
                "current{value,alarm},voltage{value,alarm}" // Missing }
            ")";
    testDiag("request = '%s'",request.c_str());
    testOk1(!C->createRequest(request));
    testOk(C->getMessage().find("mismatched {}") != string::npos,
        "'mismatched {}' error detected");

    request = "field(alarm.status,alarm.severity)";
    testDiag("request = '%s'",request.c_str());
    testOk1(!C->createRequest(request));
    testOk(C->getMessage().find("duplicate fieldName") != string::npos,
        "'duplicate fieldName' error detected");

    request = ":field(record[process=false]power.value)";
    testDiag("request = '%s'",request.c_str());
    testOk1(!C->createRequest(request));
    testOk(C->getMessage().find("bad request") != string::npos,
        "'bad request' error detected");
}

static
StructureConstPtr maskingType = getFieldCreate()->createFieldBuilder()
        ->add("A", pvInt)
        ->add("B", pvInt)
        ->addNestedStructure("C")
            ->add("D", pvInt)
            ->addNestedStructure("E")
                ->add("F", pvInt)
            ->endNested()
        ->endNested()
        ->createStructure();

static
void checkMask(bool expand,
               const std::string& request,
               const BitSet& expected)
{
    PVStructurePtr pvRequest(createRequest(request));
    PVStructurePtr value(getPVDataCreate()->createPVStructure(maskingType));

    BitSet actual(extractRequestMask(value, pvRequest->getSubField<PVStructure>("field"), expand));

    testEqual(actual, expected)<<" request=\""<<request<<"\"";
}

static void testMask()
{
    testDiag("===== testMask =====");

    PVStructurePtr V(getPVDataCreate()->createPVStructure(maskingType));
    testShow()<<V;

    checkMask(false, "", BitSet().set(0));
    checkMask(true, "", BitSet().set(0)
              .set(V->getSubField("A")->getFieldOffset())
              .set(V->getSubField("B")->getFieldOffset())
              .set(V->getSubField("C")->getFieldOffset())
              .set(V->getSubField("C.D")->getFieldOffset())
              .set(V->getSubField("C.E")->getFieldOffset())
              .set(V->getSubField("C.E.F")->getFieldOffset()));

    checkMask(false, "field()", BitSet().set(0));
    checkMask(true, "field()", BitSet().set(0)
              .set(V->getSubField("A")->getFieldOffset())
              .set(V->getSubField("B")->getFieldOffset())
              .set(V->getSubField("C")->getFieldOffset())
              .set(V->getSubField("C.D")->getFieldOffset())
              .set(V->getSubField("C.E")->getFieldOffset())
              .set(V->getSubField("C.E.F")->getFieldOffset()));

    checkMask(false, "field(A)", BitSet()
              .set(V->getSubField("A")->getFieldOffset()));
    checkMask(true, "field(A)", BitSet()
              .set(V->getSubField("A")->getFieldOffset()));

    checkMask(false, "field(A,B)", BitSet()
              .set(V->getSubField("A")->getFieldOffset())
              .set(V->getSubField("B")->getFieldOffset()));

    checkMask(false, "field(A,C)", BitSet()
              .set(V->getSubField("A")->getFieldOffset())
              .set(V->getSubField("C")->getFieldOffset()));

    checkMask(true, "field(A,C)", BitSet()
              .set(V->getSubField("A")->getFieldOffset())
              .set(V->getSubField("C")->getFieldOffset())
              .set(V->getSubField("C.D")->getFieldOffset())
              .set(V->getSubField("C.E")->getFieldOffset())
              .set(V->getSubField("C.E.F")->getFieldOffset()));

    checkMask(false, "field(C.D)", BitSet()
              .set(V->getSubField("C.D")->getFieldOffset()));

    checkMask(true, "field(C.D)", BitSet()
              .set(V->getSubField("C.D")->getFieldOffset()));

    checkMask(false, "field(A,C{D,E.F})", BitSet()
              .set(V->getSubField("A")->getFieldOffset())
              .set(V->getSubField("C.D")->getFieldOffset())
              .set(V->getSubField("C.E.F")->getFieldOffset()));

    checkMask(true, "field(A,C{D,E.F})", BitSet()
              .set(V->getSubField("A")->getFieldOffset())
              .set(V->getSubField("C.D")->getFieldOffset())
              .set(V->getSubField("C.E.F")->getFieldOffset()));

    // request for non-existant field is silently ignored
    checkMask(false, "field(A,foo)", BitSet()
              .set(V->getSubField("A")->getFieldOffset()));
}

} // namespace

MAIN(testCreateRequest)
{
    testPlan(130);
    testCreateRequestInternal();
    testBadRequests();
    testMask();
    return testDone();
}


