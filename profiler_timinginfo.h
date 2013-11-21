//Walt Woods
//August 24th, 2007
//TimingInfo structure for the profiler and MMGR to share.

#ifndef PROFILER_TIMINGINFO_H_
#define PROFILER_TIMINGINFO_H_

namespace profiler
{

	struct TimingInfo
	{
        //Children
        TimingInfo* down;

        //Siblings
        TimingInfo* next;

        //Parent
        TimingInfo* up;

        //Fingerprint
        void* fingerprint;

        struct
        {//Runtime information
            //The number of active calls to this profiler
            sint callDepth;
        } runtime;

        struct 
        {//Result information
		    //Number of calls.
		    suint calls;
    		
		    //Number of calls when nesting > 0.
		    suint nestedcalls;
    		
		    //Total running time.
		    big_suint runtime;
#if MMGR
		    //Total allocated bytes.
		    big_suint allocations;

		    //Total deallocated bytes.
		    big_suint deallocations;
#endif
        } result;

        //Name of profiler.
        char* name;

        //File of this Timing Info.
        const char* file;

        //Line
        suint line;

        //Function
        const char* function;
	};

} //profiler

#endif//PROFILER_TIMINGINFO_H_
