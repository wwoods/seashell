//July 31, 2008

#if TESTING >= TESTLEVEL_CORE

TEST_BUDDY(bitfieldTests)
{
    suint testData[5] = { 0xAAAAAAAA, 0x12345678, 0x87654321, 0x55555555, 
      0x10100101 
    };

    seashell::Bitfield bf;
    bf.setReadfield(testData, sizeof(testData) * 8);
    for (int i = 0; i < 32; i++) {
        bf.writeBits(bf.readBits(5), 5);
    }

    testAssert(memcmp(bf.getWritefield(0), testData, sizeof(testData)) == 0,
      "Memory comparison did not pan out correctly for bitfield");
    testAssert(bf.readBits(5) == (suint)-1, "EOF not detected");
}
END_TEST_BUDDY()

#endif //TESTING
