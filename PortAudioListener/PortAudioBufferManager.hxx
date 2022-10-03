/* ------------------------------------------------------------------   */
/*      item            : PortAudioBufferManager.hxx
        made by         : Rene van Paassen
        date            : 171210
	category        : header file 
        description     : 
	changes         : 171210 first version
        language        : C++
	copyright       : (c) 2017 TUDelft-AE-C&S
*/

#ifndef PortAudioBufferManager_hxx
#define PortAudioBufferManager_hxx
#include "../WorldListener/WorldObjectBase.hxx"
#include <map>
#include <string>
#include <exception>
#include <sstream>

OPEN_NS_WORLDLISTENER;

class PortAudioBufferManager
{
  /** connect the sound file name to a buffer id */
  typedef std::map<std::string,int> buffermap_t;

  /** connect the sound file to a buffer id */
  buffermap_t buffers;
  
public:
  /** Constructor */
  PortAudioBufferManager();

  /** Destructor */
  ~PortAudioBufferManager();

  /** Return an openal buffer, given a file name. Re-uses buffers when 
      the file name is recognized

      @param fname Name of the file with sound data; currently only 
                   wav is supported
      @returns     A buffer id
      @throws      SoundFileReadError, in case the file cannot be read
  */
  int getBuffer(const std::string& fname);
};


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
