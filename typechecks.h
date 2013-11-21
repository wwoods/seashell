
#if TESTING >= TESTLEVEL_CORE
TEST_BUDDY(seashellTypes){
    EMBED_TEST_BUDDY(OsInfo)
        //Machine information
        const char* platformString;
#ifdef _WINDOWS
        platformString = "Windows";
#elif defined(_LINUX)
        platformString = "Linux";
#endif
        printf("%i-Bit application on %s.\n", BITS, platformString);
    END_EMBED_TEST_BUDDY()

#if TESTING >= TESTLEVEL_THOROUGH
    EMBED_TEST_BUDDY(CpuSpeed)
        //World's most inaccurate CPU time approximator
        //Addition takes four clock cycles.  1.0 Ghz = 1024*1024*1024 clock cycles
        //per second
        const sint ghz = (sint)1000*1000*1000;
        const sint max = ghz / 10;
        volatile sint i = 0;
        big_suint tempTime = timing::getThreadExecutionMs();
        while (++i != max);
        tempTime = timing::getThreadExecutionMs() - tempTime;
        if (tempTime == 0)
            printf("Unable to estimate processor speed.\n");
        else
            printf("Estimated %f ghz processor.\n", 
              (double)i * (7.0) / ((double)ghz * tempTime / 1000.0));
        if (seashell::systeminfo::getActiveProcessors() == 0)
            printf("Unable to determine number of processors.\n");
        else
            printf("Active processors: %i\n", 
              seashell::systeminfo::getActiveProcessors());
    END_EMBED_TEST_BUDDY()
#endif //TESTING >= TESTLEVEL_THOROUGH

    //Type size checking
    testAssert(sizeof(sint) == BITS / 8, "Expected sint to match OS bits.");
    testAssert(sizeof(real) == BITS / 8, "Expected real to match OS bits.");
    testAssert(sizeof(voidptr) == sizeof(void*), "voidptr is not size of void "
      "pointer");
    testAssert(sizeof(sint64)  == 8, "Expected sint64 8 byte integer");
    testAssert(sizeof(sint32)  == 4, "Expected sint32 4 byte integer");
    testAssert(sizeof(sint16)  == 2, "Expected sint16 2 byte integer");
    testAssert(sizeof(sint8)   == 1, "Expected sint8 1 byte integer");
    testAssert(sizeof(suint64) == 8, "Expected suint64 8 byte integer");
    testAssert(sizeof(suint32) == 4, "Expected suint32 4 byte integer");
    testAssert(sizeof(suint16) == 2, "Expected suint16 2 byte integer");
    testAssert(sizeof(suint8)  == 1, "Expected suint8 1 byte integer");
    testAssert(sizeof(real32)  == 4, "Expected real32 4 byte floating point");
    testAssert(sizeof(real64)  == 8, "Expected real64 8 byte floating point");}
END_TEST_BUDDY()
#endif //TESTING >= TESTLEVEL_CORE
