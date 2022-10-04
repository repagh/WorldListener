/* ------------------------------------------------------------------   */
/*      item            : AudioExceptions.hxx
        made by         : Rene van Paassen
        date            : 221004
        category        : header file
        description     :
        changes         : 221004 first version
        language        : C++
        copyright       : (c) 2022 RvP
*/

#ifndef AudioExceptions_hxx
#define AudioExceptions_hxx

#include "worldlistenerns.h"
#include <exception>
#include <string>

OPEN_NS_WORLDLISTENER;

#ifndef _NOEXCEPT
#define _NOEXCEPT throw()
#endif

/** Exception for sound file processing */
class SoundFileReadError: public std::exception
{
  std::string msg;
public:
  SoundFileReadError(const std::string& file, const char* msg);
  SoundFileReadError(const SoundFileReadError& o);
  ~SoundFileReadError() _NOEXCEPT;
  const char* what() const throw();
};

CLOSE_NS_WORLDLISTENER;

#endif
