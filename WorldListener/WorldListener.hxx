/* ------------------------------------------------------------------   */
/*      item            : WorldListener.hxx
        made by         : repa
        from template   : DuecaModuleTemplate.hxx
        template made by: Rene van Paassen
        date            : Sat Nov 11 22:04:39 2017
        category        : header file
        description     :
        changes         : Sat Nov 11 22:04:39 2017 first version
        template changes: 030401 RvP Added template creation comment
        language        : C++
        copyright       : (c) 2016 TUDelft-AE-C&S
*/

#ifndef WorldListener_hxx
#define WorldListener_hxx

// include the dusime header
#include <dusime.h>
#include <ChannelWatcher.hxx>

// This includes headers for the objects that are sent over the channels
#include "comm-objects.h"
#include "ListenerBase.hxx"
#include <boost/scoped_ptr.hpp>

// include headers for functions/classes you need in the module
#if defined(DUECA_CONFIG_HDF5)
#include <hdf5utils/HDFLogConfig.hxx>
#endif

USING_DUECA_NS;

/** Stick your ear in the world module.

    This module creates a sound scene. This scene can be populated with
    different sound sources. The basic flavours of sound sources are:

    * A static source, with a fixed volume and location

    * A source for which the location is fixed relative to the observer
      (such as a vehicle-carried sound) or it can be fixed in the world.
      Only the amplitude and pitch of the source can be controlled.

    * A source that can move. Either the movement is specified relative
      to the observer (again, a vehicle-carried sound, now the sound
      moves in the vehicle, for example a bottle rolling in the aisle of
      a bus) or the movement is specified as absolute in the world.
      Amplitude and pitch can be controlled.

    The current back-end is OpenAL (in the OpenAL-soft version). The
    module has been set-up to handle multiple back-end types, similar to
    the WorldView module.

    This module controls the back end, and communicates with the rest of
    the simulation. You need two channels, one to control the observer
    motion, and another to control the motion and sound characteristics
    of all sounds.

    The observer motion is controlled with a continuous channel with only
    one entry, to create it use something like:

    \code
    w_observer(getId(), NameSet(getEntity(), "BaseObjectMotion", ""),
               "BaseObjectMotion", "any label", Channel::Continuous,
               Channel::OnlyOneEntry),
    \endcode

    You can share the observer motion with the ego-motion of the WorldView
    module.

    The control of all sound sources is in a multi-entry channel, with
    either event-type or stream-type (continuous) entries. An event-type
    entry will play once for every event coming in, a stream-type entry
    will be played continuously looping.

    As a special exception, an event with volume 0.0f will result in a
    playing sound being stopped.

    To create an entry, use something like:

    \code
    w_audio(getId(), NameSet("audio", "AnyAudioClass", ""),
            "AudioObjectMotion", "name of the sound",
            eventtype ? Channel::Events : Channel::Continuous,
            Channel::OneOrMoreEntries, Channel::MixedPacking)
    \endcode

    or

    \code
    w_audio(getId(), NameSet("audio", "AnyAudioClass", ""),
            "AudioObjectFixed", "name of the sound",
            eventtype ? Channel::Events : Channel::Continuous,
            Channel::OneOrMoreEntries, Channel::MixedPacking)
    \endcode

    By using an AudioObjectMotion object, this entry controls both
    volume, pitch and movement. With an AudioObjectFixed object, only
    volume and pitch are to be specified, and the location is fixed.

    Note that AudioObjectMotion objects are derived from BaseObjectMotion
    objects. This makes them compatible with WorldView, if you add the
    object. Note that the name of the channel to read for sound objects in
    the (in the example NameSet("audio", "AnyAudioClass", ""), which
    gives the AnyAudioClass://audio channel), is configurable. You need to
    configure a new name if you want to share the audio information with
    the WorldView movement information.

    Each audio object in the channel needs to be further specified
    in the backend, in this case for now there is only the OpenALListener
    backend. There, you must specify:

    * the audio file to be played
    * the location (or initial location) of the sound
    * possibly orientation and cone direction
    * whether the sound location is fixed in the world or relative to
      the observer

    Based on a match using the
    ( {AudioObjectMotion|AudioObjectFixed};<sound name>) combination
    this sound is attached to the right entry in the channel. See the
    information in the SpecificationBase class (from WorldView) for
    further details on the matching process. The match results in a type
    or class of audio object that handles the connection to the channel
    entry and the playing. These types are available through a factory,
    and it is possible to extend that factory, and use custom types.

    The WorldListener module follows the DUSIME state machine, and
    silences the sounds in HoldCurrent mode.

    As an additional (and totally gratuitous) service, the module can
    also send messages to an HDF5 logger module, which will ensure new
    log files when transitioning from HoldCurrent to Advance state.

    To create the module in dueca.mod, follow further instructions:

    \verbinclude world-listener.scm
 */
class WorldListener: public SimulationModule
{
  /** self-define the module type, to ease writing the parameter table */
  typedef WorldListener _ThisModule_;

private: // simulation data
  /** Object creating the audio interface */
  boost::scoped_ptr<worldlistener::ListenerBase> listener;

  /** Own position and orientation, speed */
  BaseObjectMotion                     current_view;

  /** Maximum prediction time */
  double                               max_predict;

  /** Prediction correction */
  double                               t_predict;

  /** Number of configuration commands to accept per cycle. */
  int                                  num_config_per_round;

private: // channel access

  /** Channel with position and rates. Position as quaternion */
  boost::scoped_ptr<ChannelReadToken>  r_own;

  /** Channel watcher for the other entities */
  typedef std::list<boost::shared_ptr<ChannelWatcher> > watcher_list_t;

  /** Channel watchers for the other entities */
  watcher_list_t                       m_others;

  /** Flag to remember if the watcher list has been explicitly called */
  bool                                 no_explicit_entity_watch;

#if defined(DUECA_CONFIG_HDF5)
  /** Run counter, counts number of "Run" modes */
  int                                  runnumber;

  /** Flag to determine if logging control messages have been sent */
  bool                                 sendlogcontrol;

  /** Channel toke for control of a HDF logger, if applicable */
  boost::scoped_ptr<ChannelWriteToken> w_logconfig;
#endif

  /** "Keep running" flag, will not stop sound production when DUECA
      process is stopped */
  bool                                 keep_running;

private: // activity allocation

  /** Callback object for simulation calculation. */
  Callback<WorldListener>  cb1;

  /** Activity for simulation calculation. */
  ActivityCallback      do_calc;

public: // class name and trim/parameter tables
  /** Name of the module. */
  static const char* const           classname;

  /** Return the parameter table. */
  static const ParameterTable*       getMyParameterTable();

public: // construction and further specification
  /** Constructor. Is normally called from scheme/the creation script. */
  WorldListener(Entity* e, const char* part, const PrioritySpec& ts);

  /** Continued construction. This is called after all script
      parameters have been read and filled in, according to the
      parameter table. Your running environment, e.g. for OpenGL
      drawing, is also prepared. Any lengty initialisations (like
      reading the 4 GB of wind tables) should be done here.
      Return false if something in the parameters is wrong (by
      the way, it would help if you printed what!) May be deleted. */
  bool complete();

  /** Destructor. */
  ~WorldListener();

  // add here the member functions you want to be called with further
  // parameters. These are then also added in the parameter table
  // The most common one (addition of time spec) is given here.
  // Delete if not needed!

  /** Specify a time specification for the simulation activity. */
  bool setTimeSpec(const TimeSpec& ts);

  /** Request check on the timing. */
  bool checkTiming(const vector<int>& i);

  /** Function call that adds an object to the scene. */
  bool addObject(ScriptCreatable& ava, bool in);

  /** Function call that sets the listener object. */
  bool setListener(ScriptCreatable& ava, bool in);

  /** Set the initial listener position, orientation and speed */
  bool initialEar(const vector<double>& i);

  /** Specify a channel for control of an HDF logger */
  bool controlLogger(const std::string& cname);

  /** Specify which channels need to be monitored for entries */
  bool addWorldInformationChannel(const std::vector<std::string>& s);

  /** Specify the own motion channel name */
  bool setEgoMotionChannel(const std::string& n);

public: // member functions for cooperation with DUECA
  /** indicate that everything is ready. */
  bool isPrepared();

  /** start responsiveness to input data. */
  void startModule(const TimeSpec &time);

  /** stop responsiveness to input data. */
  void stopModule(const TimeSpec &time);

public: // the member functions that are called for activities
  /** the method that implements the main calculation. */
  void doCalculation(const TimeSpec& ts);
};

#endif
