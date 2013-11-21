
#if defined(_WINDOWS)
#include <windows.h>
#elif defined(_LINUX)
#include <unistd.h>
#endif

#include "seashell.h"

namespace seashell
{

namespace clipboard
{

void setText(const std::string& str)
{
    if (OpenClipboard(0)) {
        EmptyClipboard();
        HGLOBAL setBuffer = GlobalAlloc(GMEM_MOVEABLE, str.length() + 1);
        StringCchCopy((char*)GlobalLock(setBuffer), 
          str.length() + 1, str.c_str());
        GlobalUnlock(setBuffer);
        SetClipboardData(CF_TEXT, setBuffer);
        CloseClipboard();
    }
}

std::string getText()
{
    std::string cboard = "";

#if defined(_WINDOWS)
    if (OpenClipboard(0)) {
        const char* c = (const char*)GetClipboardData(CF_TEXT);
        if (c)
            cboard = c;
        CloseClipboard();
    }
#elif defined(_LINUX)
    #pragma message("not implemented.")
#endif

    return cboard;
}

#if TESTING >= TESTLEVEL_THOROUGH
    TEST_BUDDY(clipboardTesting)
    {
		std::string testString = "nothing";
		setText(testString);
		std::string result = getText();
		testAssert(testString == result, "Basic string not properly stored in clipboard");

		testString = "";
		setText(testString);
		result = getText();
		testAssert(testString == result, "Empty string not properly stored in clipboard");

		testString = "stringwi7hnumb345";
		setText(testString);
		result = getText();
		testAssert(testString == result, "String with numbers not properly stored in clipboard");

		testString = "!@#$%^&*()_-+{}[]|\\:;\"'<,>.?/ ";
		setText(testString);
		result = getText();
		testAssert(testString == result, "String with symbols not properly stored in clipboard");

		testString = "areallyreallyreallyreallyreallyreallyreallyreallyreallyreallyreallyreallyreallyreallyreallyreallyreallyreallyreallylongstring";
		setText(testString);
		result = getText();
		testAssert(testString == result, "Annoyingly long string not properly stored in clipboard");
    }
    END_TEST_BUDDY()

#endif

} //clipboard

} //seashell