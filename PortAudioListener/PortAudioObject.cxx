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

#define PortAudioObject_cxx
#include "PortAudioObject.hxx"
#include "PortAudioListener.hxx"
#include <debug.h>

OPEN_NS_WORLDLISTENER;

PortAudioObject::PortAudioObject(const WorldDataSpec& spec) :
  stream(NULL),
  spec(spec),
  base_volume(0.999999f),
  needstart(true),
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

    // create the source
    alGenSources((ALuint)1, &source);
    if (haveALError("obtaining source")) return false;

    // bind them together
    alSourcei(source, AL_BUFFER, buffer);
    if (haveALError("binding source to buffer")) return false;

    // data in the spec;
    // * coordinates used for xyz, uvw, pitch, gain
    // * type used for "looping", otherwise single
    // * name not used
    // openal coord sys x=right, y=up, z=towards you

    if (spec.coordinates.size() >= 3) {
      alSource3f(source, AL_POSITION, spec.coordinates[1],
                 -spec.coordinates[2], -spec.coordinates[0]);
      if (haveALError("setting position")) return false;
    }
    if (spec.coordinates.size() >= 6) {
      alSource3f(source, AL_VELOCITY, spec.coordinates[4],
                 -spec.coordinates[5], -spec.coordinates[3]);
      if (haveALError("setting velocity")) return false;
    }
    if (spec.coordinates.size() >= 7) {
      base_volume = spec.coordinates[6];
      alSourcef(source, AL_GAIN, spec.coordinates[6]);
      if (haveALError("setting gain")) return false;
    }
    if (spec.coordinates.size() >= 8) {
      alSourcef(source, AL_PITCH, spec.coordinates[7]);
      if (haveALError("setting pitch")) return false;
    }
    alSourcei(source, AL_LOOPING,
              looping ? AL_TRUE : AL_FALSE);
    if (haveALError("setting looping")) return false;

    // absolute source
    alSourcei(source, AL_SOURCE_RELATIVE,
              (spec.type.find("relative") != string::npos) ?
              AL_TRUE : AL_FALSE);
    if (haveALError("specifying relative")) return false;

    if (spec.coordinates.size() >= 11) {
      // distance model adjustments
      alSourcef(source, AL_REFERENCE_DISTANCE, spec.coordinates[8]);
      alSourcef(source, AL_MAX_DISTANCE, spec.coordinates[9]);
      alSourcef(source, AL_ROLLOFF_FACTOR, spec.coordinates[10]);
    }

    if (spec.coordinates.size() == 17) {
      // directional sound
      alSource3f(source, AL_DIRECTION, spec.coordinates[12],
                 -spec.coordinates[13], -spec.coordinates[11]);
      alSourcef(source, AL_CONE_INNER_ANGLE, spec.coordinates[14]);
      alSourcef(source, AL_CONE_OUTER_ANGLE, spec.coordinates[15]);
      alSourcef(source, AL_CONE_OUTER_GAIN, spec.coordinates[16]);
      if (haveALError("setting cone")) return false;
    }
    return true;
  }

  // not specified, cannot create
  return false;
}

// this version is for static, constant sounds. overridden in derived
// classes for controlled sounds
void PortAudioObject::iterate(const TimeSpec& ts, const BaseObjectMotion& base)
{
  if (source) {
    if (needstart) {
      //std::cerr << "starting play " << spec.name << std::endl;
      alSourcePlay(source);
      haveALError("playing");
      needstart = false;
    }
    else {
      ALint source_state;
      alGetSourcei(source, AL_SOURCE_STATE, &source_state);
      if (source_state == AL_PLAYING) {
        std::cerr << "playing " << spec.name << std::endl;
      }
    }
  }
}

void PortAudioObject::silence()
{
  if (source) {
    ALint state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING) {
      alSourceStop(source);
      //std::cerr << "stopping " << spec.name << std::endl;
      haveALError("stopping");
    }
    needstart = true;
  }
}

const std::string& PortAudioObject::getChannelClass()
{
  const static std::string empty("");
  return empty;
}

CLOSE_NS_WORLDLISTENER;
