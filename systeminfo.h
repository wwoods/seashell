//Walt Woods
//September 2nd, 2007

#ifndef SYSTEMINFO_H_
#define SYSTEMINFO_H_

namespace seashell
{

//System information retrieval functions
namespace systeminfo
{

/** @return Returns the number of active processors on the current system.
  *If the information cannot be retrieved, returns 0.
  */
sint getActiveProcessors();

} //systeminfo

} //seashell

#endif//SYSTEMINFO_H_
