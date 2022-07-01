/* ------------------------------------------------------------------   */
/*      item            : OpenALBufferManager.cxx
        made by         : Rene' van Paassen
        date            : 171210
	category        : body file 
        description     : 
	changes         : 171210 first version
        language        : C++
	copyright       : (c) 2017 TUDelft-AE-C&S
*/

#define OpenALBufferManager_cxx
#include "OpenALBufferManager.hxx"

#include <SDL_audio.h>
#include <sstream>
#define I_MOD
#include <debug.h>

OPEN_NS_WORLDLISTENER;

OpenALBufferManager::OpenALBufferManager()
{

}


OpenALBufferManager::~OpenALBufferManager()
{

}

static inline ALenum to_al_format(uint8_t channels, SDL_AudioFormat format)
{
  bool stereo = (channels > 1);
  
  switch (SDL_AUDIO_BITSIZE(format)) {
  case 16:
    if (stereo) {
      I_MOD("16 bit stereo format");
      return AL_FORMAT_STEREO16;
    }
    else {
      I_MOD("16 bit mono format")
      return AL_FORMAT_MONO16;
    }
  case 8:
    if (stereo) {
      I_MOD("8 bit stereo format");
      return AL_FORMAT_STEREO8;
    }
    else {
      I_MOD("8 bit mono format");
      return AL_FORMAT_MONO8;
    }
  default:
    return -1;
  }
}

ALuint OpenALBufferManager::getBuffer(const std::string& fname)
{
  buffermap_t::iterator ib = buffers.find(fname);

  if (ib != buffers.end()) {
    return ib->second;
  }

  // new buffer
  ALuint buffer;
  
  // length of file name
  size_t lenfn = fname.size();

  if (lenfn > 4 && fname.substr(lenfn - 4) == std::string(".wav")) {

#if 1
    SDL_AudioSpec wav_spec;
    uint32_t wav_len;
    uint8_t *bufferData;
    I_MOD("Loading file " << fname);
    if (SDL_LoadWAV(fname.c_str(), &wav_spec, &bufferData, &wav_len) == NULL) {
      throw(SoundFileReadError(fname, "cannot open"));
    }
    alGenBuffers((ALuint)1, &buffer);
    alBufferData(buffer, to_al_format(wav_spec.channels, wav_spec.format),
                 bufferData, wav_len, wav_spec.freq);
    SDL_FreeWAV(bufferData);
    buffers[fname] = buffer;
    return buffer;
#else
    WaveInfo *wave = WaveOpenFileForReading(fname.c_str());
    if (!wave) {
      throw(SoundFileReadError(fname, "cannot open"));
    }
    int ret = WaveSeekFile(0, wave);
    if (ret) {
      throw(SoundFileReadError(fname, "cannot seek"));
    }
    bufferData = new char[wave->dataSize];
    size_t nread = WaveReadFile(bufferData, wave->dataSize, wave);
    if (nread != wave->dataSize) {
      throw(SoundFileReadError(fname, "incomplete read"));
    }
    alGenBuffers((ALuint)1, &buffer);
    alBufferData(buffer, to_al_format(wave->channels, wave->bitsPerSample),
                 bufferData, wave->dataSize, wave->sampleRate);
    buffers[fname] = buffer;
    return buffer;
#endif
  }
  
  throw(SoundFileReadError(fname, "cannot read file type"));
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
