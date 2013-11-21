/**Walt Woods
  *Converted for Hoppers on Nov. 24th, 2006
  *Random class that uses Marsenne twister generator.
  */

#ifndef RANDOM_H_
#define RANDOM_H_

//modified Walt Woods 2005 for Push/Pop and classification to avert desynchronization
namespace seashell
{

namespace marsenne
{

const int N = 624;					/* number of random words generated at once */

//Modified 05/07/2005 Walt Woods for simple push/restore operations on randomizer.
const int BACKUP_STACK_SIZE = 20;

class RandHandler
{
#define TRACK_NUM_RANDS 0
#if TRACK_NUM_RANDS
	int NumRandSinceLastSRandLongName;
#endif //TRACK_NUM_RANDS

	char RandString[256];

	int BackupSize;
	int BackupMti[BACKUP_STACK_SIZE];
	unsigned long Backup[BACKUP_STACK_SIZE][N];

	unsigned long mt[N]; /* the array for the state vector  */
	int mti; /* mti==N+1 means mt[N] is not initialized */

	unsigned long genrand_int32(void); //all numbers included.
	double genrand_real1(void); //[0, 1]
	double genrand_real2(void); //[0, 1)
	double genrand_real3(void); //(0, 1)
	double genrand_res53(void); //[0, 1), 53 bit res version

public:
	long genrand_int31(void); //positive only
	RandHandler()
	{
		mti=N+1;
		BackupSize = 0;
#if TRACK_NUM_RANDS
		NumRandSinceLastSRandLongName = 0; 
#endif //TRACK_NUM_RANDS

		init_genrand(0);
	}

    /**Seeds the randomizer.  Called in constructor with s == 0. 
      * @param s Seed for randomizer.  If 0, then the current system time is
      *used. */
	void init_genrand(unsigned long s);
	void init_by_array(unsigned long init_key[], int key_length);

	void PushRandState();
	void PopRandState();
	void ResetRandMatrix(); //sets depth to 0, useful to staving off errors.

	inline real rand(const real Min, const real Max) 
	{
#if TRACK_NUM_RANDS
		NumRandSinceLastSRandLongName++; 
#endif //TRACK_NUM_RANDS
		return Min + (Max - Min) * (real)genrand_real2(); 
	}

	inline sint irand(const sint Min, const sint Max) 
	{
		if (Min == Max) 
			return Min;
#if TRACK_NUM_RANDS
		NumRandSinceLastSRandLongName++; 
#endif //TRACK_NUM_RANDS
		return (sint)((genrand_int31() % (Max - Min)) + Min); 
	}

    inline suint uirand(const suint Min, const suint Max)
    {
        if (Min == Max)
            return Min;
#if TRACK_NUM_RANDS
		NumRandSinceLastSRandLongName++; 
#endif //TRACK_NUM_RANDS
        return (((suint)genrand_int31() % (Max - Min)) + Min);
    }
};

} //marsenne

static marsenne::RandHandler randGenerator;
inline real rand(const real min, const real max) 
{
    return randGenerator.rand(min, max);
}

inline sint irand(const sint min, const sint max)
{
    return randGenerator.irand(min, max);
}

inline suint uirand(const suint min, const suint max)
{
    return randGenerator.uirand(min, max);
}

} //seashell

#endif //RANDOM_H_