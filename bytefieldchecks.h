//August 8th, 2008

#if TESTING >= TESTLEVEL_CORE

TEST_BUDDY(bytefieldTests)
{
    char string[5] = "abcd";

    seashell::Bytefield bf;

    //TODO
    //bf.setReadfield((suint8*)string, 5);
    //for (int i = 0; i < 32; i++) {
        //bf.writeBits(bf.readBits(5), 5);
    //}

    //testAssert(memcmp(bf.getWritefield(0), testData, sizeof(testData)) == 0,
//      "Memory comparison did not pan out correctly for bitfield");
    //testAssert(bf.readBits(5) == (suint)-1, "EOF not detected");
}
END_TEST_BUDDY()

#endif //TESTING
