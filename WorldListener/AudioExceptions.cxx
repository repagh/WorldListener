/* ------------------------------------------------------------------   */
/*      item            : AudioExceptions.cxx
        made by         : Rene' van Paassen
        date            : 221004
        category        : body file
        description     :
        changes         : 221004 first version
        language        : C++
        copyright       : (c) 22 TUDelft-AE-C&S
*/

#define AudioExceptions_cxx
#include "AudioExceptions.hxx"
#include <sstream>

OPEN_NS_WORLDLISTENER;

SoundFileReadError::SoundFileReadError(const std::string& file,
                                       const char* message)
{
  std::stringstream msgb;
  msgb << "Error '" << message << "' reading file '" << file << "'";
  msg = msgb.str();
}

SoundFileReadError::SoundFileReadError(const SoundFileReadError& o) :
  msg(o.msg)
{ }

SoundFileReadError::~SoundFileReadError() _NOEXCEPT
{
  //
}
const char* SoundFileReadError::what() const throw()
{ return msg.c_str(); }

CLOSE_NS_WORLDLISTENER;
