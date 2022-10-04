/* ------------------------------------------------------------------   */
/*      item            : PortAudioObject.cxx
        made by         : Rene' van Paassen
        date            : 171112
        category        : body file
        description     :
        changes         : 171112 first version
        language        : C++
        copyright       : (c) 2017 TUDelft-AE-C&S
*/

//https://stackoverflow.com/questions/61326170/im-trying-to-open-a-stream-in-portaudio


#define PortAudioObject_cxx
#include "PortAudioObject.hxx"
#include "PortAudioListener.hxx"
#include "../WorldListener/AudioExceptions.hxx"
#include <debug.h>

OPEN_NS_WORLDLISTENER;

PortAudioObject::PortAudioObject(const WorldDataSpec& spec) :
  channel(0),
  buffer(NULL),
  spec(spec),
  base_volume(0.999999f),
  looping(spec.type.find("looping") != string::npos)
{
  // looping is determined here, since it may be overridden
  // in derived classes "connect"
}


PortAudioObject::~PortAudioObject()
{

}

bool PortAudioObject::haveError(int err, const char* stage)
{
  if (err != paNoError) {
    W_MOD("PortAudio error " << Pa_GetErrorText(err) <<
	  " for " << spec.name << " when " << stage);
    return true;
  }
  return false;
}


void PortAudioObject::addData(float* out, unsigned dataCount)
{
  unsigned ii = channel;
  while (ii < dataCount && ridx < buffer->info.frames) {
    out[ii] = base_volume * buffer->data[ridx];
    ii += num_channels; ridx++;
  }
}  

bool PortAudioObject::initSound(PortAudioListener* master)
{
  if (spec.filename.size() >= 1) {

    // create or get buffer
    try {
      buffer = master->getBufferManager().getBuffer(spec.filename[0]);
    }
    catch (const SoundFileReadError& e) {
      W_MOD("cannot init '" << spec.name << "' " << e.what());
      return false;
    }
  }
  
  channel = unsigned(spec.coordinates[0]);
  num_channels = master->getNumChannels();

  // not specified, cannot create
  return false;
}

// this version is for static, constant sounds. overridden in derived
// classes for controlled sounds
void PortAudioObject::iterate(const TimeSpec& ts, const BaseObjectMotion& base)
{
  //
}

void PortAudioObject::silence()
{
  //
}

const std::string& PortAudioObject::getChannelClass()
{
  const static std::string empty("");
  return empty;
}

CLOSE_NS_WORLDLISTENER;
