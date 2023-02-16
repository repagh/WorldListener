/* ------------------------------------------------------------------   */
/*      item            : SoundRecorder.hxx
        made by         : repa
        from template   : DusimeModuleTemplate.hxx
        template made by: Rene van Paassen
        date            : Mon May 16 14:56:01 2022
        category        : header file
        description     :
        changes         : Mon May 16 14:56:01 2022 first version
        template changes: 030401 RvP Added template creation comment
        language        : C++
        copyright       : (c) 2016 TUDelft-AE-C&S
*/

#ifndef SoundRecorder_hxx
#define SoundRecorder_hxx

// include the dusime header
#include <dusime.h>
USING_DUECA_NS;

// This includes headers for the objects that are sent over the channels
#include "comm-objects.h"

// include headers for functions/classes you need in the module
#include <AL/al.h>
#include <AL/alc.h>

#include <SDL2/SDL.h>
#include <sndfile.h>

/** A module.

    The instructions to create an module of this class from the Scheme
    script are:

    \verbinclude open-al-recorder.scm
*/
class SoundRecorder: public SimulationModule
{
  /** self-define the module type, to ease writing the parameter table */
  typedef SoundRecorder _ThisModule_;

private: // simulation data

  /** Audio device */
  SDL_AudioDeviceID    capture_dev;
  
  /** Device for capture */
  std::string  capture_device_name;

  /** Capture frequency */
  unsigned sample_rate;

  /** Number of channels */
  unsigned sample_channels;
  
  /** Type of capture/data format */
  std::string  sample_format;

  /** Result file name */
  std::string  filename;

  /** Result file */
  SNDFILE*      sndfile;

  /** Record in hold mode or only advance */
  bool record_hold;

  /** Count of samples */
  volatile unsigned sample_count;

  /** Count of samples */
  unsigned written_sample_count;

  /** Bytes per recording (n-channel) point */
  unsigned bytes_per_point;

  /** Bytes per single channel sample */
  unsigned bytes_per_sample;

  /** Recording status */
  bool recording;

  /** Specification of recording file */
  SF_INFO sfinfo;

  /** Use a single file for recording (or open a new one in Advance) */
  bool single_file;

  /** Previous SimState for detecting state changes */
  SimulationState prev_SimulationState;
  
private: // channel access
  /** Information on recording progress */
  ChannelWriteToken   w_progress;

private: // activity allocation
  /** Check-up on the state of recording */
  PeriodicAlarm        myclock;

  /** Callback object for simulation calculation. */
  Callback<SoundRecorder>  cb1;

  /** Activity for simulation calculation. */
  ActivityCallback      do_calc;

public: // class name and trim/parameter tables
  /** Name of the module. */
  static const char* const           classname;

  /** Return the parameter table. */
  static const ParameterTable*       getMyParameterTable();

public: // construction and further specification
  /** Constructor. Is normally called from scheme/the creation script. */
  SoundRecorder(Entity* e, const char* part, const PrioritySpec& ts);

  /** Continued construction. */
  bool complete();

  /** Destructor. */
  ~SoundRecorder();

  // add here the member functions you want to be called with further
  // parameters. These are then also added in the parameter table
  // The most common one (addition of time spec) is given here.
  // Delete if not needed!

  /** Specify a time specification for the simulation activity. */
  bool setTimeSpec(const TimeSpec& ts);

  /** Request check on the timing. */
  bool checkTiming(const std::vector<int>& i);

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

public: // callback with new audio data

  /** callback with audio buffer */
  void cbAudio(Uint8* data, int len);
};

#endif
