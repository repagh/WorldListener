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
#include "../WorldListener/AudioExceptions.hxx"

OPEN_NS_WORLDLISTENER;

PortAudioBufferManager::PortAudioBufferManager()
{

}


PortAudioBufferManager::~PortAudioBufferManager()
{

}

PortAudioBufferManager::Buffer::Buffer(const std::string& fname)
{
  SNDFILE* file = sf_open(fname.c_str(), SFM_READ, &info);
  if (!file) {
    throw(SoundFileReadError(fname, "cannot read file"));
  }
  data.resize(info.frames*info.channels);
  sf_readf_float(file, data.data(), info.frames);
  sf_close(file);
}

PortAudioBufferManager::Buffer::~Buffer()
{
  //
}

const PortAudioBufferManager::buffer_ptr_t
PortAudioBufferManager::getBuffer(const std::string& fname)
{
  buffermap_t::iterator ib = buffers.find(fname);

  if (ib != buffers.end()) {
    return &(ib->second);
  }

  // length of file name
  size_t lenfn = fname.size();
  SF_INFO info;

  auto newbuf = buffers.emplace
    (std::piecewise_construct,
     std::forward_as_tuple(fname),
     std::forward_as_tuple(fname));

  if (!newbuf.second) {
    throw(SoundFileReadError(fname, "cannot store buffer in map"));
  }
  return &(newbuf.first->second);

  //throw(SoundFileReadError(fname, "cannot read file type"));
}

CLOSE_NS_WORLDLISTENER;
