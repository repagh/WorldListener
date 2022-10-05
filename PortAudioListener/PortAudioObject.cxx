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
#include "PortAudioObjectFactory.hxx"
#include "PortAudioListener.hxx"
#include "../WorldListener/AudioExceptions.hxx"
#include <debug.h>

OPEN_NS_WORLDLISTENER;

PortAudioObject::PortAudioObject(const WorldDataSpec& spec) :
  channel(0),
  buffer(NULL),
  spec(spec),
  base_volume(0.999999f),
  looping(spec.type.find("looping") != string::npos),
  ridx(0U)
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
    out[ii] += base_volume * buffer->data[ridx];
    ii += num_channels; ridx++;
    if (looping && ridx == buffer->info.frames) { ridx = 0; }
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
    
    if (spec.coordinates.size() >= 1) {
      channel = unsigned(spec.coordinates[0]);
    }
    num_channels = master->getNumChannels();
    if (channel >= num_channels) {
      W_MOD("cannot create sound, channel " << channel << " not available");
      return false;
    }
    if (spec.coordinates.size() >= 2) {
      base_volume = spec.coordinates[1];
    }

    // should be OK
    return true;
  }

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
  if (looping) { ridx = 0; }
}

const std::string& PortAudioObject::getChannelClass()
{
  const static std::string empty("");
  return empty;
}

static auto *PortAudioObject_maker = new
  SubContractor<PortAudioObjectTypeKey, PortAudioObject>
  ("PortAudioObject");

CLOSE_NS_WORLDLISTENER;
