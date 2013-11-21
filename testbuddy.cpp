
#include <stdarg.h>
#include <stdio.h>

#include "seashell.h"

#if TESTING

namespace testbuddy
{

//Output file
const char outputFile[] = "TestResults";

//Succes variable
char testInstance::success = 0;

//Are we initialized?
char initialized = 0;

//Test suites are implemented via embedded buddies.  As such, track the 
//current nesting level
sint testLevel;

//Initialize the testbuddy system.
void initialize()
{
    eassert(!initialized, Exception, "Test buddies are being executed twice.");

    initialized = 1;
    testLevel = 0;
    FILE* f = fopen(outputFile, "wt");
    fclose(f);

    srand(1250);

#ifdef _WINDOWS
    freopen("TestConsole", "wb", stdout);
#endif
}



void cleanup()
{
    eassert(initialized, Exception, "Test buddies not being executed.");
}



const char* getIndent()
{
    static char* indent = 0;
    static class temp { public:
        ~temp() { delete[] indent; }
    } indentFree;
    static sint lastLevel = -1;
    if (lastLevel != testLevel - 1) {
        lastLevel = testLevel - 1;
        delete[] indent;
        indent = new char [lastLevel * 2 + 1];
        memset(indent, ' ', lastLevel * 2);
        indent[lastLevel * 2] = 0;
    }

    return indent;
}



void testInstance::testErrorMessage(const char* message)
{
    FILE* f = fopen(outputFile, "at");

    if (success)
        fprintf(f, "--------------------\n");
    success = 0;

    fprintf(f, "%s\n", message);
    printf("** Test Error: %s **\n", message);
    fclose(f);
}



void testInstance::testError(const char* message, ...)
{
    char errorMessage[4096];
    va_list vargs;
    va_start(vargs, message);
    vsnprintf(errorMessage, sizeof(errorMessage), message, vargs);
    testErrorMessage(errorMessage);
}



void testInstance::testAssert(bool condition, const char* message, ...)
{
    if (!condition) {
        char errorMessage[4096];
        va_list vargs;
        va_start(vargs, message);
        vsnprintf(errorMessage, sizeof(errorMessage), message, vargs);
        testErrorMessage(errorMessage);
    }
}



struct TestInstance
{
    TestInstance* next;
    testInstance* test;
    const char* name;
};
TestInstance* testList;



void runTests()
{
    try {
        initialize();

        while (testList) {
            TestInstance* t = testList->next;
            eassert(testLevel == 0, Exception, "Expected for new test: Nested "
              "level == 0; but nestings = %i", testLevel);
            testList->test->runTest(testList->name);
            free(testList);
            testList = t;
        }
        
        cleanup();
    }
    catch (const Exception& e) {
        elog(e);
    }
}



void registerTest(testInstance* test, const char* name)
{
    if (!initialized) {
        TestInstance* t = (TestInstance*)malloc(sizeof(TestInstance));
        t->next = testList;
        t->test = test;
        t->name = name;
        testList = t;
    }
    else {
        test->runTest(name);
    }
}



void testInstance::registerTest(const char* name)
{
    testbuddy::registerTest(this, name);
}



void testInstance::runTest(const char* name)
{
    char originalSuccess = success;
    success = 1;
    testLevel++;

    try {
        printf("-- BEGIN TEST %s\n", name);
        //Make sure the test that crashes is known, if nothing else.
        fflush(stdout);
        testFunction();
    }
    catch (Exception& e) {
        e.printStack(stdout);
        testErrorMessage(e.what());
        testErrorMessage("Unable to recover from exception.  Stack trace "
          "printed in console.");
    }
    catch (...) {
        testErrorMessage("Unhandled exception thrown.");
        testErrorMessage("Unable to recover from exception.");
    }
    printf("-- END TEST %s\n", name);
    FILE* f = fopen(outputFile, "at");
    if (success)
        fprintf(f, "%sPassed %s\n", getIndent(), name);
    else {
        fprintf(f, "%sFailed %s\n", getIndent(), name);
        if (testLevel == 1) {
            fprintf(f, "--------------------\n");
        }
    }
    fclose(f);

    if (success) {
        success = originalSuccess;
    }

    testLevel--;
    if (testLevel == 0)
        printf("\n");
}

} //testbuddy

#endif
