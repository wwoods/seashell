//Walt Woods
//April 6th, 2008

#ifndef SEASHELL_FILE_ENUM_H_
#define SEASHELL_FILE_ENUM_H_

namespace seashell
{

class FileEnumerator
{
public:
    /**Begins an enumeration of the specified path. 
      * @param searchPath Search path, using '*' for a wildcard. 
      * @throw Exception Thrown when the search path is invalid or does not 
      *exist.
      */
    FileEnumerator(const char* searchPath);

    /**Standard destructor.  */
    ~FileEnumerator();

    /** @return Returns true if there is another file in the search path. 
      *Otherwise, returns 0. */
    char hasNext();

    /** @return Returns the next file name.  Returns 0 if there are none. */
    const char* getNext();

    /** @return Returns true if the last retrieved file name corresponds to a
      *directory. */
    char isDirectory();

private:
#ifdef _WINDOWS
    //Handle to the search
    HANDLE hSearch_;

    //Data currently viewed by user
    WIN32_FIND_DATA data_;

    //The pending file's data
    WIN32_FIND_DATA pending_;
#else
#error "File Enumerator implemented only for Windows!"
#endif //OS Defines

    //Is there a pending file?
    char hasPending_;

    //The path of the search; used to prepend to returned file names
    char path_[MAX_PATH];

    //Buffer used for return values
    char buffer_[MAX_PATH];

    /**Fetch the next file. */
    void fetchNext_();
};

} //seashell

#endif//SEASHELL_FILE_ENUM_H_
