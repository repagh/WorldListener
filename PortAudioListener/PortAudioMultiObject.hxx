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

    This plays a mono source file with adjustable strength, given a
    base volume level for each channel, over multiple channels. 

    The source file may be substituted by means of an event sent to
    a secondary configuration channel.

    Example configuration for a Python set-up

    ~~~{.py}
    mymods.append(dueca.Module(
        'world-listener', "", sim_priority).param(
	    ('set-timing', display_timing),
            ('check-timing', (10000, 20000)),
	    ('set-listener',
             dueca.PortAudioListener().param(
                 ('set-devicename',
                  "<< fill in your device"),
		 # portaudio object playing sound over multiple speakers
		 ('add-object-class-data',
		  (# data type, and label to match in audio input channel
		   "AudioObjectFixed:some label",
		   # override for the object name, not needed here, use label
		   "",
		   # type of object, as defined in the factory for PortAudio
		   # objects
		   "PortAudioMultiObject",
		   # file to play
		   "initial-soundfile.wav",
		   # the following, audio file modification channel, is
		   # optional. If given, the PortAudioMultiObject will
		   # open a read entry in the channel, with the "some label"
		   # label given above. Events on that entry, with new audio
		   # file names, lead to a loading of new audio
		   "AudioFileSelection://ph-sound") ),
		 # for this type of object, the "coordinates" represent
		 # volume levels on speakers
		 ('add-object-class-coordinates',
		  ( 0.1, 0.2, 0.1, 0.0) )
	     ).complete() )
	))
    ~~~
*/
class PortAudioMultiObject: public PortAudioObjectFixed
{ 
protected:
  /** Pointer to the master object */
  PortAudioListener* master;
  
  /** Volume levels for the audio outputs */
  std::vector<float> base_volumes;

  /** Optional channel for selection of new audio file */
  boost::scoped_ptr<ChannelReadToken>      r_newfile;
  
public:
  /** Constructor. By itself this class creates a static sound

      @param spec  Specification for the object, using spec members:
                   * name: for identification
                   * type: "looping" to indicate looping sound
                   * filename: 1st element, file for sound
		   *           2nd element, if present, channel name for
		               new file feed; selected with the label from
			       the connected channel entry
                   * coordinates: Array with volume levels for all speakers
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

  /** Connect to a channel entry
      @param master_id ID for opening a channel reader
      @param cname     Channel with object data
      @param entry_id  Entry in the channel */
  void connect(const GlobalId& master_id, const NameSet& cname,
               entryid_type entry_id,
               Channel::EntryTimeAspect time_aspect) override;
};

CLOSE_NS_WORLDLISTENER;

#endif
