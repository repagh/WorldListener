/* ------------------------------------------------------------------   */
/*      item            : PortAudioMultiObject.hxx
        made by         : Rene van Paassen
        date            : 171112
        category        : header file
        description     :
        changes         : 171112 first version
        language        : C++
        copyright       : (c) 2017 TUDelft-AE-C&S
*/

#ifndef PortAudioMultiObject_hxx
#define PortAudioMultiObject_hxx

#include "../WorldListener/WorldObjectBase.hxx"
#include <string>
#include "PortAudioBufferManager.hxx"
#include "PortAudioObjectFixed.hxx"
#include <portaudio.h>
#include <dueca.h>
#include "comm-objects.h"

OPEN_NS_WORLDLISTENER;
class PortAudioListener;

/** PortAudioMulti source objects that play over multiple channels.

*/
class PortAudioMultiObject: public PortAudioObjectFixed
{
protected:

  // channels to write
  std::vector<float> base_volumes;

public:
  /** Constructor. By itself this class creates a static sound

      @param spec  Specification for the object, using spec members:
                   * name: for identification
                   * type: looping to indicate looping sound
                   * filename: only 1st element, file for sound
                   * coordinates: xyz (3), uvw (3), gain, pitch,
                     dirxyz(3), inner/outer cone angle, outer cone gain
  */
  PortAudioMultiObject(const WorldDataSpec& spec);

  /** Destructor */
  ~PortAudioMultiObject();

  /** Initialize the sound

      Initialize and start the sound playing, if applicable. May be
      overridden to provide specific behaviour.

      @param master Listener with context, etc. */
  virtual bool initSound(PortAudioListener* master);

  /** Play, update, recalculate, etc. */
  virtual void iterate(const TimeSpec& ts, const BaseObjectMotion& base);

  /** Pass data for playing on the card. */
  virtual void addData(float* out, unsigned frameCount);
};

CLOSE_NS_WORLDLISTENER;

#endif
