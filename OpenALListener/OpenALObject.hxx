/* ------------------------------------------------------------------   */
/*      item            : OpenALObject.hxx
        made by         : Rene van Paassen
        date            : 171112
	category        : header file 
        description     : 
	changes         : 171112 first version
        language        : C++
	copyright       : (c) 2017 TUDelft-AE-C&S
*/

#ifndef OpenALObject_hxx
#define OpenALObject_hxx

#include "../WorldListener/WorldObjectBase.hxx"
#include <string>
#include <AL/al.h>
#include <AL/alc.h>
#include "OpenALBufferManager.hxx"

#include <dueca.h>
#include "comm-objects.h"

OPEN_NS_WORLDLISTENER;
class OpenALListener;

/** Base class for OpenAL source objects that are controlled from the
    simulation. 

    By itself this implements a basic sound object, with a position and speed, 
    sound amplitude and pitch somewhere in the world.
*/
class OpenALObject: public WorldObjectBase
{
protected:
  
  // source id
  ALuint source;

  // buffer
  ALuint buffer;

  // data specification
  WorldDataSpec spec;

  // base volume
  float base_volume;

  // internal error report & check
  bool haveALError(const char* stage="");

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
  OpenALObject(const WorldDataSpec& spec);

  /** Destructor */
  ~OpenALObject();

  /** Initialize the sound
      
      Initialize and start the sound playing, if applicable. May be
      overridden to provide specific behaviour. 

      @param master Listener with context, etc. */
  virtual bool initSound(OpenALListener* master);
  
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
