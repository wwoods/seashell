
#include "seashell.h"

namespace seashell
{

FileEnumerator::FileEnumerator(const char* searchPath)
{
    //Init: fill pending_ with first file data
#ifdef _WINDOWS
    hSearch_ = FindFirstFile(searchPath, &pending_);
    if (hSearch_ == INVALID_HANDLE_VALUE) {
        //Check if the error is path-related or null-search related
        sint i = GetLastError();
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            hasPending_ = 0;
            return;
        }

        //Path not found; throw exception
        ethrow(Exception, "File enumerator failed to enumerate path '%s'",
          searchPath);
    }
#endif //OS Defines

    //Initialize hasPending and get the next file
    hasPending_ = 1;

    //If the first file retrieved is "." or "..", get a new one
    if (pending_.cFileName[0] == '.') fetchNext_();

    //Initialize the path
        //Scan backwards for a directory marker
        sint endOfPath = (sint)strlen(searchPath) - 1;
        while (endOfPath > 0 && searchPath[endOfPath] != '/' && 
          searchPath[endOfPath] != '\\')
            endOfPath--;

        //Was directory found?
        if (endOfPath > 0) {
            //Copy it over
            strncpy(path_, searchPath, endOfPath + 1);
            path_[endOfPath + 1] = 0;
        }
        else {
            //No path
            path_[0] = 0;
        }
}



FileEnumerator::~FileEnumerator()
{
#ifdef _WINDOWS
    FindClose(hSearch_);
#endif //OS Defines
}



void FileEnumerator::fetchNext_()
{
    if (hasPending_) {
        //Set current data
        data_ = pending_;

        //Get the next file
#ifdef _WINDOWS
        BOOL hasResult = true;
        //reset search
        pending_.cFileName[0] = '.';
        while (pending_.cFileName[0] == '.' && hasResult) {
            hasResult = FindNextFile(hSearch_, &pending_);
        }

        if (hasResult)
            hasPending_ = 1;
        else
            hasPending_ = 0;
#endif //OS Defines
    }
}



char FileEnumerator::hasNext()
{
    return hasPending_;
}



const char* FileEnumerator::getNext()
{
    if (hasPending_) {
        //Get the next, return the file name of current
        fetchNext_();

        //Add the file name
#ifdef _WINDOWS
        StringCchPrintf(buffer_, sizeof(buffer_), "%s%s",
          path_, data_.cFileName);
#endif //OS Defines

        return buffer_;
    }

    //No pending, return
    return 0;
}



char FileEnumerator::isDirectory()
{
#ifdef _WINDOWS
    if (data_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        return 1;
#endif //OS Defines

    return 0;
}

#if TESTING >= TESTLEVEL_CORE
TEST_BUDDY(FileEnumeratorTests)
{
    FileEnumerator fe("./*");
    while (fe.hasNext()) {
        printf("%s\n", fe.getNext());
    }
}
END_TEST_BUDDY()
#endif //TESTING

} //seashell
