/* ------------------------------------------------------------------   */
/*      item            : PortAudioBufferManager.cxx
        made by         : Rene' van Paassen
        date            : 171210
	category        : body file 
        description     : 
	changes         : 171210 first version
        language        : C++
	copyright       : (c) 2017 TUDelft-AE-C&S
*/

#define PortAudioBufferManager_cxx
#include "PortAudioBufferManager.hxx"

#include <sstream>
#define I_MOD
#include <debug.h>

OPEN_NS_WORLDLISTENER;

PortAudioBufferManager::PortAudioBufferManager()
{

}


PortAudioBufferManager::~PortAudioBufferManager()
{

}

PortAudioBufferManager::Buffer::Buffer(const std::string& fname)
{
  file = sf_open(fname.c_str(), SFM_READ, &info);
}

const PortAudioBufferManager::buffer_ptr_t
PortAudioBufferManager::getBuffer(const std::string& fname)
{
  buffermap_t::iterator ib = buffers.find(fname);

  if (ib != buffers.end()) {
    return ib->second;
  }
  
  // length of file name
  size_t lenfn = fname.size();
  SF_INFO info;
  
  auto newbuf = buffers.emplace
    (std::piecewise_construct,
     std::forward_as_tuple(fname),
     std::forward_as_tuple(fname));
    
  return &(newbuf->second);
  
  //throw(SoundFileReadError(fname, "cannot read file type"));
}

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
