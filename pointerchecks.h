
#if TESTING >= TESTLEVEL_CORE

class pspeed : public seashell::Thread
{
public:
    pspeed()
    {
        startThread();
    }

    ~pspeed()
    {
        stopThread();
    }

    void run()
    {
        for (suint i = 0; i < 1000000; i++) {
            handle_to<int> hInt;
            (*hInt) = 6;
        }
    }
};

TEST_BUDDY(pointerSpeed)
{
    pspeed p[2];
}
END_TEST_BUDDY();

TEST_BUDDY(pointerChecks)
{
    //int* unfreed = new int [2]; //8 bytes unfreed
    handle_to<int> test2;
    (*test2) = 5;

    handle_to<int> test3 = test2;

    handle_to<int> int1; (*int1) = 5;
    handle_to<int> int15 = int1;
    handle_to<int> int2 = int1;

    *int2.getUniqueCopy() = 6; //int1 and int15 should be unchanged
    testAssert(*int1 == 5 && *int15 == 5, "unique() Modified other values");
    *int2.getUniqueCopy() = 7;
    testAssert(*int1 == 5 && *int15 == 5, "unique() Modified other values");
    *int15.getUniqueCopy() = 10;
    testAssert(*int1 == 5 && *int15 == 10, "unique() Modified other values");
}
END_TEST_BUDDY()

#endif //TESTING
