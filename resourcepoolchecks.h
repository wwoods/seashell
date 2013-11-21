//Walt Woods
//April 18th, 2008

//Resource pool tests

#if TESTING >= TESTLEVEL_CORE

static void printNumber(int* i, void*)
{
    printf("%i ", *i);
}

static void freeNumber(int* i, void*)
{
    delete i;
}

TEST_BUDDY(resourcePoolChecks)
{
    seashell::ResourcePool<int> intPool;
    int* a = new int(0);
    testAssert(intPool.assign(a) == 0, "Unique id not assigned correctly.");
    a = new int(1);
    testAssert(intPool.assign(a) == 1, "Unique id not assigned correctly.");
    a = new int(2);
    testAssert(intPool.assign(a) == 2, "Unique id not assigned correctly.");
    a = new int(3);
    testAssert(intPool.assign(a) == 3, "Unique id not assigned correctly.");

    printf("The numbers 0-3 should be printed out below, in order:\n");
    intPool.callForEach(printNumber, 0);
    printf("\n");

    freeNumber(intPool.get(2), 0);
    intPool.release(2);
    a = new int(4);
    testAssert(intPool.assign(a) == 2, "Unique id not replaced.");

    //Shouldn't see memory leaks
    intPool.callForEach(freeNumber, 0);
}
END_TEST_BUDDY()

#endif//TESTING
