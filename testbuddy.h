//Walt Woods
//July 31st, 2007
//A miniature system for running test code in a project.
//Test code should throw exceptions to indicate critical failure of a test 
//(for instance, if object construction fails so testing cannot occur).  
//
//For a non-aborting test failure, explicitly call testError("reason", ...) or call 
//testAssert(condition, "reason", ...).
//To abort a test, such as a construction failure, use ethrow().
//
//Note that when linking a library with static tests like these, the test 
//project must link with dependency inputs; otherwise, the static objects are
//lost.

//Example:
//TEST_BUDDY(exampleTest) //Parameter MUST be valid identifier.
//  int i = 5;
//  testAssert(i == 5, Exception, "Value of i incorrectly set to %i", i);
//  printf("Data: %i\n", i); //prints to TestConsole file (all test buddies do
//                           //this with printf()).
//END_TEST_BUDDY()

#ifndef TESTBUDDY_H_
#define TESTBUDDY_H_

#if TESTING

namespace testbuddy
{

/**Executes all tests.  Should be called at the top of main().
  */
void runTests();

class testInstance
{
	static char success;

public:
	/**The test must be run after this base class' instantiation.
	  *The solution is to have derived constructors call runTest.
	  */
	void runTest(const char* name);

    /**Registers the test for execution on testbuddy::runTests().
      * @param name Name of test.
      */
    void registerTest(const char* name);

    /**Used when the error message is already creeated (such as when catching 
      *a thrown exception).
      * @param message Plain text message reason for failure.
      */
	static void testErrorMessage(const char* message);

    /**Manual test failure function.  Allows test to resume processing before 
      *termination.
      * @param message printf() formatted reason for failure.
      * @param ... Additional information used in message.
      */
    static void testError(const char* message, ...);

    /**Manual non-aborting test condition.  Allows test to resume processing 
      *before termination if the test fails.
      * @param condition If false, the test is pronounced a failure.
      */
    static void testAssert(bool condition, const char* message, ...);

	virtual void testFunction() = 0;
};

} //testbuddy

//A note about why the function is also called name():
//Linux __FUNCTION__ only outputs the immediate function name.  Therefore,
//profiler presents more readable information if it is done this way (see 
//testFunction()).
#define TEST_BUDDY_HEADER(name) \
    static class TEST : public ::testbuddy::testInstance \
    { \
    public: \
        TEST() \
        { \
            static char initialized = 0; \
            if (!initialized) { \
                initialized = 1; \
                registerTest(#name); \
            } \
        } \
        void testFunction() { name(1); } \
        void name(char notConstructor) { PROFILER(0);

#define END_TEST_BUDDY_HEADER() } } test_instance;

#define TEST_BUDDY(name) \
    namespace name { TEST_BUDDY_HEADER(name)
#define END_TEST_BUDDY() \
    END_TEST_BUDDY_HEADER() }

#define EMBED_TEST_BUDDY(name) \
    { TEST_BUDDY_HEADER(name)
#define END_EMBED_TEST_BUDDY() END_TEST_BUDDY_HEADER() }

#endif //TESTING

#endif//TESTBUDDY_H_
