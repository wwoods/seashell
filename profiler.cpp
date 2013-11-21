
//This file should ALWAYS be compiled with full compiler optimizations (that's
//not to say that FAST need be defined.

#include <assert.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "seashell.h"
#include "disablemmgrmacros.h"
#include "threadprivate.h"
#include "profiler_timinginfo.h"

#if PROFILE

//Should the estimated runtime of profilers be subtracted from reported 
//execution times?
#define PROFILER_PROCESS_OVERHEAD 0
const int PROFILER_OVERHEAD_LOOPS = 2000000;

#define TIMING_METHOD_SAMPLING 1

//Sampling method 1: System times (Not thread execution times)
//Sampling method 2: Thread Execution times - Double-pass timer flag (Once on 
//profiler entry, another on exit).  Done because, in linux, getrusage() does
//not work for any thread except the caller.
#define METHOD_SAMPLING_METHOD 1

#define PROFILER_TIMING_METHOD TIMING_METHOD_SAMPLING

namespace profiler
{
    //Output file
    static char output[] = "profile.txt";

    //Top level timing node name
    static char topLevelName[] = "Application";
    
    //Master ProfilerManager.
    class ProfilerManager;
    extern ProfilerManager* master;
    char masterInitialized = 0;

    //Other ProfilerManagers.  Implemented as a member of a function because
    //static initialization order is unknown.
    class ManagerCapsule
    {
    public:
        seashell::ThreadPrivate<ProfilerManager> profileManagers;
        static char alive;

        ManagerCapsule() {}
        ~ManagerCapsule()
        {
            //kill sampling thread
        }
    }; 
    char ManagerCapsule::alive = 1;
    seashell::ThreadPrivate<ProfilerManager>& profileManagers()
    {
        static ManagerCapsule capsule;
        //if (ManagerCapsule::alive)
            return capsule.profileManagers;
        //return 0;
    }

    //Thread termination function
    void terminateThread()
    {
        profileManagers().destroy();
    }

    //Result printing function
    void printProfilerResults(TimingInfo* current);



    /**Crops a TimingInfo's name.
      */
    void cropProfilerName(TimingInfo* record)
    {
        sint similar = 0;
        TimingInfo* up = record->up;
        if (up && up->name) {
            sint upNameLength = (sint)strlen(up->name);
            if (strncmp(up->name, record->name, upNameLength) == 0) {
                record->name[0] = '*';
                record->name[1] = '*';
                sint i;
                for (i = 2; record->name[upNameLength] != 0; i++) {
                    record->name[i] = record->name[upNameLength++];
                }
                record->name[i] = 0;
            }
            else {
                for (sint i = upNameLength - 1; i > 0; i--)
                    if (up->name[i] == ':' && up->name[i - 1] == ':') {
                        if (strncmp(up->name, record->name, i) == 0) {
                            record->name[0] = '*';
                            record->name[1] = ':';
                            sint j;
                            for (j = 2; record->name[i] != 0; j++) {
                                record->name[j] = record->name[i++];
                            }
                            record->name[j] = 0;
                        }

                        break;
                    }
            }
        }
    }



    //The primary profiler manager class.  Each instance represents a separate
    //thraed.  On deletion, they are all merged into a single manager which 
    //represents the entirety of the application.
    class ProfilerManager
    {
    public:
        /**Initialize the ProfilerManager and its stack of TimingInfos.
          */
        ProfilerManager()
        {
            eassert(master, Exception, "Profilers are being created before "
              "static initialization has finished.");

#if PROFILER_TIMING_METHOD == TIMING_METHOD_SAMPLING
#if METHOD_SAMPLING_METHOD == 1
#else
            lastSampleTime = timing::getThreadExecutionMs();
            time = 0;
#endif
#endif

            current_ = (TimingInfo*)malloc(sizeof(TimingInfo));
            memset(current_, 0, sizeof(TimingInfo));

            numProfilerManager++;
        }



        /**Initialize the master ProfilerManager.
          */
        ProfilerManager(char isMaster)
        {
            current_ = (TimingInfo*)malloc(sizeof(TimingInfo));
            memset(current_, 0, sizeof(TimingInfo));
            //Timing set in initialize()
        }



        /**Since static initialization does not have the profileManagers class
          *initialized before the master, the first non-master profiler must 
          *call this function.
          */
        void initialize();



        /**Merge the ProfilerManager into the master tree.  If we are the
          *final ProfilerManager, print out the final tree.
          */
        ~ProfilerManager();



        /** @return Returns the current TimingInfo, which is also unique to 
          *the current stack trace.
          */
        TimingInfo* getCurrent()
        {
            return current_;
        }



#if PROFILER_TIMING_METHOD == TIMING_METHOD_SAMPLING
        /**Increments the current profile's running time.
          */
        void incrementCurrent(void* param)
        {
            //This is implemented in parallel to other functions that update
            //the current profiler.  As such, grab the currently valid tag,
            //and update that (Note that without temporary, += requires two 
            //operations and can thus overwrite valid values).
#if METHOD_SAMPLING_METHOD == 1
            TimingInfo* current = current_;
            current->result.runtime += *(big_suint*)param;
#else
            time = 1;
#endif
        }
        big_suint lastSampleTime;
#endif
        //Macro used to add execution time to current profiler
        
#if PROFILER_TIMING_METHOD == TIMING_METHOD_SAMPLING && \
  METHOD_SAMPLING_METHOD == 2
    #define Sample_Exec_Time \
      if (time) { \
        time = 0; \
        big_suint temp = timing::getThreadExecutionMs(); \
        current_->result.runtime += temp - lastSampleTime; \
        lastSampleTime = temp; \
      }
#else
    #define Sample_Exec_Time 
#endif
        



        /**Based on a fingerprint, return the applicable TimingInfo*.
          */
        TimingInfo* locateFingerprint(void* fingerprint)
        {
            if (!masterInitialized) {
                masterInitialized = 1;
                master->initialize();
            }
            
            Sample_Exec_Time

            TimingInfo* t = current_->down;
            while (t) {
                if (t->fingerprint == fingerprint) {
                    current_ = t;
                    return current_;
                }
                t = t->next;
            }

            t = (TimingInfo*)malloc(sizeof(TimingInfo));
            memset(t, 0, sizeof(TimingInfo));
            t->fingerprint = fingerprint;
            t->up = current_;
            t->next = current_->down;
            current_->down = t;
            current_ = t;
            return current_;
        }



        /**When a profile has finished, calling this function will set the 
          *current profile to its parent.
          */
        void invalidateFingerprint()
        {
            Sample_Exec_Time

            current_ = current_->up;
            eassert(current_, Exception, "The Profiler has for some reason "
              "allowed its current node to become null.");
        }
#undef Sample_Exec_Time

#if MMGR
    public:
        /**DO NOT use mmgr's new operator!!
          */
        void* operator new(size_t size)
        {
            return malloc(size);
        }

        /**DO NOT use mmgr's delete operator!
          */
        void operator delete(void* v)
        {
            free(v);
        }
#endif

    private:
        /**Merge another manager's tree with this tree.  Keeps the other tree
          *valid.
          */
        void mergeWithAndSteal_(TimingInfo* mine, TimingInfo* other)
        {
            mine->result.calls += other->result.calls;
            mine->result.nestedcalls += other->result.nestedcalls;
            mine->result.runtime += other->result.runtime;
#if MMGR
            mine->result.allocations += other->result.allocations;
            mine->result.deallocations += other->result.deallocations;
#endif

            TimingInfo* last = 0;
            TimingInfo* child = other->down;
            while (child) {
                TimingInfo* myChild = mine->down;
                while (myChild && myChild->fingerprint != child->fingerprint) {
                    myChild = myChild->next;
                }
                if (myChild) { //match
                    mergeWithAndSteal_(myChild, child);

                    if (last) {
                        last->next = child->next;
                        child->next = 0;
                        freeTimingGroup_(child);
                        child = last->next;
                        continue;
                    }
                    else {
                        last = child->next;
                        child->next = 0;
                        freeTimingGroup_(child);
                        child = last;
                        last = 0;
                        other->down = child;
                        continue;
                    }
                }
                else { //we must add it!
                    if (last) {
                        last->next = child->next;
                        child->next = mine->down;
                        mine->down = child;
                        child->up = mine;
                        child = last->next;
                        continue;
                    }
                    else {
                        last = child->next;
                        child->next = mine->down;
                        mine->down = child;
                        child->up = mine;
                        child = last;
                        last = 0;
                        other->down = child;
                        continue;
                    }
                }

                last = child;
                child = child->next;
            }
        }



        /**This function is responsible for compounding recursive TimingInfos 
          *(detectable via identical fingerprints).  Recursion detection is 
          *delayed until this step for both speed and reliable stack traces.
          *
          *Merges recursive trees.  Used in destructor to compact findings.
          * @return Returns non-zero if node was a recursive call.  Zero if it
          *was not.
          */
        char cascadeRecursive_(TimingInfo* node)
        {
            TimingInfo* last = 0;
            TimingInfo* child = node->down;
            while (child) {
                TimingInfo* next = child->next;
                if (cascadeRecursive_(child)) {
                    child->next = 0;
                    freeTimingGroup_(child);
                    if (last) {
                        last->next = next;
                    }
                    else {
                        //If last is unset, it does NOT mean that node->down
                        //refers to this child anymore.  That's because when
                        //during the merge a new profile is added to this 
                        //node, last would not have been set.
                        if (node->down == child)
                            node->down = next;
                        else {
                            last = node->down;
                            while (last->next != child)
                                last = last->next;
                            last->next = next;
                        }
                    }
                }
                else
                    last = child;
                child = next;
            }

            TimingInfo* parent = node->up;
            while (parent) {
                if (parent->fingerprint == node->fingerprint) {
                    node->result.nestedcalls += node->result.calls;
                    node->result.calls = 0;
                    mergeWithAndSteal_(parent, node);
                    return 1;
                }

                parent = parent->up;
            }

            return 0;
        }



        /**Frees the timing group specified by current_.  This function, 
          *unfortunately, should never be called if the memory manager is 
          *running because it hooks into these
          */
        void freeTimingGroup_(TimingInfo* node)
        {
#if !MMGR
            while (node) {
                freeTimingGroup_(node->down);
                TimingInfo* remove = node;
                node = node->next;
                free(remove);
            }
#endif//!MMGR
        }


        //This ProfilerManager's associated thread.
        ThreadId myThreadId;

        //The current timing entry
        TimingInfo* current_;

        //Mutex used to lock producing the master ProfilerManager
        seashell::Mutex masterMutex;
        
#if METHOD_SAMPLING_METHOD == 2
        //Flag; if 1, add execution time to active profile.
        char time;
#endif

    public:
        //The number of running ProfilerManagers.
        static sint numProfilerManager;
    
        //Entry whose children are overhead timers.
        static TimingInfo* initializationTimings;

        //The overhead timing entry for a profiler's overhead with its own
        //reported timings
        static TimingInfo* overheadSelfTimings;

        //Overhead timing entry for a profiler's overhead to its parent.
        static TimingInfo* overheadContainedTimings;
    
        //Is it time when statics are being destroyed?  Do not delete the master
        //ProfilerManager until this is non-zero.
        static char staticDestruction;
    };
    sint ProfilerManager::numProfilerManager = 0;
    char ProfilerManager::staticDestruction = 0;
    TimingInfo* ProfilerManager::initializationTimings = 0;
    TimingInfo* ProfilerManager::overheadSelfTimings = 0;
    TimingInfo* ProfilerManager::overheadContainedTimings = 0;
    ProfilerManager* master = new ProfilerManager(1);
    
    
    
    static class ProfilerManagerDestroy {
    public:
        ~ProfilerManagerDestroy()
        {
            ProfilerManager::staticDestruction = 1;
            if (ProfilerManager::numProfilerManager == 0) {
                delete master;
            }
        }
    } staticDestructionInformant;



    void* getStackFingerprint()
    {
        if (!master)
            return 0;
        ProfilerManager* pm = profileManagers().get();
        if (!pm) //destructing
            return 0;
        return (void*)pm->getCurrent();
    }



    void _printStackTraceBackwards(FILE* file, TimingInfo* record)
    {
        if (!record)
            return;
        _printStackTraceBackwards(file, record->up);
        if (record->line > 0) {
            fprintf(file, "%s(%i)\n  %s\n",
              record->file, record->line, record->function);
        }
        else {
            fprintf(file, "%s\n", topLevelName);
        }
    }



    void printStackTrace(FILE* file, void* fingerprint)
    {
        TimingInfo* record = (TimingInfo*)fingerprint;
        _printStackTraceBackwards(file, record);
    }



    class ColumnData
    {
    public:
        /**Prints the name of the column.
          */
        virtual void printName(FILE* f) = 0;
        /**Prints the value of the TimingInfo struct.
          */
        virtual void printValue(FILE* f, TimingInfo* t) = 0;
        /**Returns the char width of this column (not including dividers)
          */
        virtual sint getSize() { return 8; }
    };



    class a : public ColumnData { public:
        void printName(FILE* f) { fprintf(f, "|Total ms|   Calls"); }
        void printValue(FILE* f, TimingInfo* t) 
        {
            sint runtime = (sint)t->result.runtime;
            fprintf(f, "|%8i|%8i", runtime, t->result.calls); 
        }
        sint getSize() { return 17; }
    } ColumnRuntimeAndCalls;

    class c : public ColumnData { public:
        void printName(FILE* f) { fprintf(f, "|  Nested"); }
        void printValue(FILE* f, TimingInfo* t) { fprintf(f, "|%8i", t->result.nestedcalls); }
    } ColumnNested;

#if MMGR
    class d : public ColumnData { public:
        void printName(FILE* f) { fprintf(f, "|Allocated"); }
        void printValue(FILE* f, TimingInfo* t)
        {
            big_suint allocations = t->result.allocations;
            if (allocations < 1000)
                fprintf(f, "|%7i b", (suint)allocations);
            else if ((allocations >> 10) < 1000)
                fprintf(f, "|%7.3fkb", (double)allocations / 1024.0);
            else if ((allocations >> 20) < 1000)
                fprintf(f, "|%7.3fmb", (double)allocations / 1048576.0);
            else
                fprintf(f, "|%7.3fgb", (double)allocations / (1048576.0 * 1024.0));
        }
        sint getSize() { return 9; }
    } ColumnAllocations;

    class e : public ColumnData { public:
        void printName(FILE* f) { fprintf(f, "|    Freed"); }
        void printValue(FILE* f, TimingInfo* t)
        { 
            big_suint deallocations = t->result.deallocations;
            if (deallocations < 1000)
                fprintf(f, "|%7i b", (suint)deallocations);
            else if ((deallocations >> 10) < 1000)
                fprintf(f, "|%7.3fkb", (double)deallocations / 1024.0);
            else if ((deallocations >> 20) < 1000)
                fprintf(f, "|%7.3fmb", (double)deallocations / 1048576.0);
            else
                fprintf(f, "|%7.3fgb", (double)deallocations / (1048576.0 * 1024.0));
        }
        sint getSize() { return 9; }
    } ColumnFrees;
#endif



#if MMGR
    static const sint numColumns = 4;
#else
    static const sint numColumns = 2;
#endif
    static ColumnData* columns[numColumns] = { 
        &ColumnRuntimeAndCalls,
        &ColumnNested
#if MMGR
        ,
        &ColumnAllocations,
        &ColumnFrees 
#endif
    };
    


    void outputProfilerGroup(FILE* f, sint columnStart, sint columnStop, 
      sint childLevel, TimingInfo* record, const suint maxNameLength)
    {
        while (record)
        {
            for (sint t = 0; t < childLevel - 1; t++)
                fprintf(f, "  ");
            if (record->down) {
                if (childLevel > 0)
                    fprintf(f, "+ ");
            }
            else if (childLevel > 0) {
                fprintf(f, "| ");
            }
            const char* name = record->name;
            if (!name)
                name = topLevelName;

            const sint nameLength = (sint)strlen(name);
            fwrite(name, 1, nameLength, f);

            sint spaces = maxNameLength - nameLength - childLevel * 2;
            while (spaces > 1) {
                if ((spaces & 1) == 0)
                    fprintf(f, " .");
                else
                    fprintf(f, ". ");
                spaces -= 2;
            }
            while (spaces > 0) {
                fprintf(f, ".");
                spaces--;
            }

            for (sint i = columnStart; i < columnStop; i++)
                columns[i]->printValue(f, record);
            fprintf(f, "\n");

            outputProfilerGroup(f, columnStart, columnStop, childLevel + 1, record->down, maxNameLength);
            record = record->next;
        }
    }



    /**Sorts the profiles in order of descending execution time.  Also 
      *processes record names to crop names that are similar to their 
      *parents'.
      * @return Returns the maximum name length.
      */
    suint sortTimingsGroup(TimingInfo* profile, suint additionalLength)
    {
        //Very poor sorting algorithm.
        if (!profile)
            return 0;

        //process the first record in the list outside of the loop.
        suint ret = sortTimingsGroup(profile->down, additionalLength + 2);

        //After children have cropped their names, crop ours.
        cropProfilerName(profile);

        suint returnTemp;
        if (profile->name)
            returnTemp = additionalLength + (suint)strlen(profile->name);
        else
            returnTemp = additionalLength + (suint)strlen(topLevelName);
        if (returnTemp > ret)
            ret = returnTemp;

        TimingInfo* t = profile->next;
        profile->next = 0;
        while (t) {
            big_suint runtime = t->result.runtime;
            TimingInfo* next = t->next;
            t->next = 0;

            TimingInfo* last = 0;
            TimingInfo* temp = profile;
            while (true) {
                if (temp->result.runtime <= runtime) {
                    if (last) {
                        last->next = t;
                        t->next = temp;
                    }
                    else {
                        t->up->down = t;
                        t->next = profile;
                        profile = t;
                    }
                    break;
                }

                if (!temp->next) {
                    temp->next = t;
                    break;
                }
                last = temp;
                temp = temp->next;
            }

            suint returnTemp = sortTimingsGroup(t->down, additionalLength + 2);
            if (returnTemp > ret)
                ret = returnTemp;
            //After r's children's names are cropped, crop r's
            cropProfilerName(t);
            returnTemp = additionalLength + (suint)strlen(t->name);
            if (returnTemp > ret)
                ret = returnTemp;

            t = next;
        }

        return ret;
    }



    void printProfilerResults(TimingInfo* current)
    {
        eassert(current->up == 0, Exception, "Non-top level print-out requested.");
        suint maxNameLength = sortTimingsGroup(current, 2);

        const sint spacesLeft = (sint)(maxNameLength - strlen("Function"));
#ifdef CONSOLE_OUTPUT
        printf("--------------------\nProfiling results:\n");

        //Output all results upside-down.  Since this is a console
        //print out, we want the most relevant information at the
        //bottom.
        std::vector<sint> columnTraverse;
        columnTraverse.push_back(0);
        sint columnStart = 0;
        while (columnStart < numColumns) {
            sint columnStop = columnStart + 1;
            sint columnsForData = 80 - maxNameLength - 1 - columns[columnStart]->getSize();
            while (columnStop < numColumns && columnsForData > 0) {
                columnsForData -= columns[columnStop]->getSize() + 1;
                if (columnsForData < 0)
                    break;
                columnStop++;
            }
            columnTraverse.push_back(columnStop);
            columnStart = columnStop;
        }

        const sint profilerBlocks = (sint)columnTraverse.size();
        sint columnStop = numColumns;
        for (sint j = profilerBlocks - 2; j >= 0; j--)
        {
            columnStart = columnTraverse[j];
            for (sint i = 0; i < spacesLeft; i++)
                printf(" ");
            printf("%s", "Function");
            for (sint i = columnStart; i < columnStop; i++)
                columns[i]->printName(stdout);
            printf("\n");
            outputProfilerGroup(stdout, columnStart, columnStop, 0, current, maxNameLength);
            printf("\n\n");
            columnStop = columnStart;
        }
        printf("--------------------\n");
#endif
        FILE* f = fopen(output, "wt");
        for (sint i = 0; i < spacesLeft; i++)
            fprintf(f, " ");
        fprintf(f, "%s", "Function");
        for (sint i = 0; i < numColumns; i++)
            columns[i]->printName(f);

        fprintf(f, "\n");
        outputProfilerGroup(f, 0, numColumns, 0, current, maxNameLength);
        fclose(f);
    }



#if PROFILER_TIMING_METHOD == TIMING_METHOD_SAMPLING
    //Thread for samples.
    class Sampler : public seashell::Thread
    {
    public:
        Sampler()
        {
            startThread();
        }

        ~Sampler()
        {
            stopThread();
        }

        void run()
        {
#if METHOD_SAMPLING_METHOD == 1 //perceived run time
            const big_suint interval = 1;
#else                           //processor run time
            const big_suint interval = 10;
#endif
            big_suint last = timing::getSystemMs();
            while (1) {
                queryExit();

                timing::sleepThread(interval);
                big_suint time = timing::getSystemMs();
                if (time - last >= interval) {
                    big_suint timeElapsed = time - last;
                    last = time;
                    profileManagers().foreach(
                      &ProfilerManager::incrementCurrent, &timeElapsed);
                }
            }
        }
    };
    Sampler* sampleThread = 0;



    /**Sums timings from all children, if necessary.
      * @return Returns the runtime of the queried node.
      */
    big_suint cascadeTimings(TimingInfo* node)
    {
        if (node == ProfilerManager::initializationTimings)
            return node->result.runtime;
        big_suint childSum = 0;

        TimingInfo* child = node->down;
        while (child) {
            childSum += cascadeTimings(child);

#if MMGR
            node->result.allocations += child->result.allocations;
            node->result.deallocations += child->result.deallocations;
#endif
            child = child->next;
        }
        node->result.runtime += childSum;

        if (PROFILER_PROCESS_OVERHEAD) {
            if (ProfilerManager::initializationTimings) {
                TimingInfo* t = ProfilerManager::overheadSelfTimings;
                big_suint tempRuntime = node->result.runtime;
                //Take off overhead caused by merely starting the profiler
                node->result.runtime -= 
                  (node->result.calls + node->result.nestedcalls) *
                  t->result.runtime / (t->result.calls + 
                  t->result.nestedcalls);

                //Since returns are processed by our parents, the return value
                //should represent contained overhead
                t = ProfilerManager::overheadContainedTimings;
                return tempRuntime - 
                  ((node->result.calls + node->result.nestedcalls) *
                  t->result.runtime / (t->result.calls + 
                  t->result.nestedcalls));
            }
        }
        return node->result.runtime;
    }
#endif //PROFILER_TIMING_METHOD



    void ProfilerManager::initialize()
    {PROFILER(0);
#if PROFILER_TIMING_METHOD == TIMING_METHOD_SAMPLING
        sampleThread = new Sampler();
#endif

        {//Initialize a profiler for the overhead of this mechanism.
            TimingInfo* temp = 0;
            const sint profilerLoops = PROFILER_OVERHEAD_LOOPS;
            for (sint i = 0; i < profilerLoops; i++)
            {PROFILER("Profiler Self-Overhead");
            }
            temp = profileManagers().get()->getCurrent()->down;
            temp->result.calls = profilerLoops;

            ProfilerManager::overheadSelfTimings = temp;
        }

        {//Another overhead mechanism; this one is the overhead for a parent
            TimingInfo* temp = 0;
            const sint profilerLoops = PROFILER_OVERHEAD_LOOPS;
            {PROFILER("Profiler Contained-Overhead");
                for (sint i = 0; i < profilerLoops; i++) {
                    PROFILER("temp");
                }
                temp = PROFBOMB192525.timingInfo_;
            }
            temp->result.calls = profilerLoops;
            cascadeTimings(temp);
            freeTimingGroup_(temp->down);
            temp->down = 0;

            ProfilerManager::overheadContainedTimings = temp;
        }

        {//Time a X ms segment to count ticks
            TimingInfo* temp = 0;
            {PROFILER("123 ms");
                big_suint start = timing::getThreadExecutionMs();
                while (timing::getThreadExecutionMs() - start < 123) {
                    PROFILER("temp");
                    {
                        {PROFILER("temp");}
                        PROFILER("temp");
                    }
                }

                temp = PROFBOMB192525.timingInfo_;
            }
            cascadeTimings(temp);
            freeTimingGroup_(temp->down);
            temp->down = 0;
        }

        //cascade the timings here; it won't happen later because we don't
        //want these time info's measurements altered.
        cascadeTimings(PROFBOMB192525.timingInfo_);

        ProfilerManager::initializationTimings = PROFBOMB192525.timingInfo_;
    }



    ProfilerManager::~ProfilerManager()
    {
#if PROFILER_TIMING_METHOD == TIMING_METHOD_SAMPLING
        if (master == this) {
            delete sampleThread;
        }
#endif

        //eassert(current_->up == 0, Exception, "Profiler is being "
        //  "terminated without all profilers being finished!");
        //Thread premature termination invalidates the above logic.
        while (current_->up != 0)
            current_ = current_->up;

        if (this != master) {
#if PROFILER_TIMING_METHOD == TIMING_METHOD_SAMPLING
            cascadeRecursive_(current_);
            cascadeTimings(current_);
#endif

            LockMutex(masterMutex);

            master->mergeWithAndSteal_(master->current_, current_);

            numProfilerManager--;
            if (numProfilerManager == 0 && staticDestruction) {
                delete master;
            }
        }
        else { //master
            //We are the master.  Print ourselves out.
            printProfilerResults(current_);
            master = 0;
        }

        //Freeing the timing group, while good programming practice, inhibits
        //the memory manager's ability to draw conclusions about memory leaks
        //in this thread's area.
        //freeTimingGroup_(current_);
    }



    Bomb::Bomb(void* fingerprint,
      const char* file, suint line, const char* function, const char* name)
    {
        profiler::ProfilerManager* pm = profiler::profileManagers().get();
        if (pm) {
            timingInfo_ = pm->locateFingerprint(fingerprint);
            if (timingInfo_->result.calls == 0) {
                //Initialize the name
                timingInfo_->file = file;
                timingInfo_->line = line;
                timingInfo_->function = function;
                
                {//generate display name string
                    char* temp;
                    if (!name)
                        temp = strdup(function);
                    else {
                        sint nameLength = (sint)strlen(function) + (sint)strlen(name) + 
                          4;
                        temp = (char*)malloc(nameLength);
                        StringCchCopy(temp, nameLength, function);
                        StringCchCat(temp, nameLength, " \"");
                        StringCchCat(temp, nameLength, name);
                        StringCchCat(temp, nameLength, "\"");
                    }

                    timingInfo_->name = temp;
                }
            }
            eassert(timingInfo_->runtime.callDepth >= 0, Exception, "Why does a "
              "profiler have a negative call depth?");
            timingInfo_->runtime.callDepth++;
            if (timingInfo_->runtime.callDepth == 1) {
                timingInfo_->result.calls++;
            }
            else {
                timingInfo_->result.nestedcalls++;
            }
        }
        else { //No profiler manager; in destruction sequence?
            timingInfo_ = 0;
        }
    }



    Bomb::~Bomb()
    {
        if (timingInfo_) {
            timingInfo_->runtime.callDepth--;
            eassert(timingInfo_->runtime.callDepth >= 0, Exception, "Why does a "
              "profiler have a negative call depth?");
            profiler::profileManagers().get()->invalidateFingerprint();
        }
    }
} //profiler

#if TESTING >= TESTLEVEL_THOROUGH
    void recurse(sint i)
    {PROFILER(0);
        if (!i)
            return;
        for (sint k = 0; k < i; k++) {
            PROFILER("internal");
            recurse(i - 1);
        }
    }
    TEST_BUDDY(profilerNestedTesting)
    {PROFILER(0);
        recurse(5);
    }
    END_TEST_BUDDY()
#endif

#endif
