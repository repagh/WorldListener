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

    By itself this implements a basic sound object, with a position and speed, 
    sound amplitude and pitch somewhere in the world.
*/
class PortAudioObject: public WorldObjectBase
{
protected:
  
  // source id
  PaStream *stream;

  // audio buffer
  PortAudioBufferManager::buffer_ptr_t buffer;
  
  // data specification
  WorldDataSpec spec;

  // base volume
  float base_volume;

  // internal error report & check
  bool haveError(int err, const char* stage="");

  // has the sound been played
  bool needstart;

  // looping property
  bool looping;
  
public:
  /** Constructor. By itself this class creates a static sound

      @param spec  Specification for the object, using spec members:
                   * name: for identification
                   * type: looping to indicate looping sound
                   * filename: only 1st element, file for sound
                   * coordinates: xyz (3), uvw (3), gain, pitch,
                     dirxyz(3), inner/outer cone angle, outer cone gain
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

  /** Name */
  inline const std::string& getName() { return spec.name; }

  /** Class of channel, if applicable */
  virtual const std::string& getChannelClass();
};

CLOSE_NS_WORLDLISTENER;

#endif
