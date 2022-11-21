/* ------------------------------------------------------------------   */
/*      item            : PortAudioObject.hxx
        made by         : Rene van Paassen
        date            : 171112
        category        : header file
        description     :
        changes         : 171112 first version
        language        : C++
        copyright       : (c) 2017 TUDelft-AE-C&S
*/

#ifndef PortAudioObject_hxx
#define PortAudioObject_hxx

#include "../WorldListener/WorldObjectBase.hxx"
#include <string>
#include "PortAudioBufferManager.hxx"
#include <portaudio.h>
#include <dueca.h>
#include "comm-objects.h"

OPEN_NS_WORLDLISTENER;
class PortAudioListener;

/** Base class for PortAudio source objects that are controlled from the
    simulation.

    This implements a sound that is being played on one speaker.
*/
class PortAudioObject: public WorldObjectBase
{
protected:

  // channel to write
  unsigned channel;

  // number of channels
  unsigned num_channels;

  // buffer with audio play data
  PortAudioBufferManager::buffer_ptr_t buffer;

  // data specification
  WorldDataSpec spec;

  // base volume
  float base_volume;

  // internal error report & check
  bool haveError(int err, const char* stage="");

  // looping property
  bool looping;

  // read index
  unsigned ridx;

public:
  /** Constructor. By itself this class creates a static sound

      @param spec  Specification for the object, using spec members:
                   * name: for identification
                   * type: looping to indicate looping sound
                   * filename: only 1st element, file for sound
                   * coordinates: 
  */
  PortAudioObject(const WorldDataSpec& spec);

  /** Destructor */
  ~PortAudioObject();

  /** Initialize the sound

      Initialize and start the sound playing, if applicable. May be
      overridden to provide specific behaviour.

      @param master Listener with context, etc. */
  virtual bool initSound(PortAudioListener* master);

  /** Play, update, recalculate, etc. */
  virtual void iterate(const TimeSpec& ts, const BaseObjectMotion& base);

  /** Stop, reset, etc. */
  virtual void silence();

  /** Pass data for playing on the card. */
  virtual void addData(float* out, unsigned frameCount);

  /** Name */
  inline const std::string& getName() { return spec.name; }

  /** Class of channel, if applicable */
  virtual const std::string& getChannelClass();
};

CLOSE_NS_WORLDLISTENER;

#endif
