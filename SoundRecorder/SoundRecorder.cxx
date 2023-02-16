/* ------------------------------------------------------------------   */
/*      item            : SoundRecorder.cxx
        made by         : repa
        from template   : DusimeModuleTemplate.cxx
        template made by: Rene van Paassen
        date            : Mon May 16 14:56:01 2022
        category        : body file
        description     :
        changes         : Mon May 16 14:56:01 2022 first version
        template changes: 030401 RvP Added template creation comment
                          060512 RvP Modified token checking code
                          131224 RvP convert snap.data_size to
                                 snap.getDataSize()
        language        : C++
        copyright       : (c) 2016 TUDelft-AE-C&S
*/


#define SoundRecorder_cxx
// include the definition of the module class
#include "SoundRecorder.hxx"

// include the debug writing header. Warning and error messages
// are on by default, debug and info can be selected by
// uncommenting the respective defines
//#define D_MOD
#define I_MOD
#include <debug.h>

// include additional files needed for your calculation here
#include <boost/date_time/posix_time/posix_time.hpp>
#include <cstring>
#include <debprint.h>

// the standard package for DUSIME, including template source
#define DO_INSTANTIATE
#include <dusime.h>

// class/module name
const char* const SoundRecorder::classname = "sound-recorder";

// parameters to be inserted
const ParameterTable* SoundRecorder::getMyParameterTable()
{
  static const ParameterTable parameter_table[] = {
    { "set-timing",
      new MemberCall<_ThisModule_,TimeSpec>
        (&_ThisModule_::setTimeSpec), set_timing_description },

    { "check-timing",
      new MemberCall<_ThisModule_,std::vector<int> >
      (&_ThisModule_::checkTiming), check_timing_description },

    { "capture-device",
      new VarProbe<_ThisModule_,std::string>
      (&_ThisModule_::capture_device_name),
      "SDL Capture device name" },

    { "sample-rate",
      new VarProbe<_ThisModule_,unsigned>
      (&_ThisModule_::sample_rate),
      "Sample rate (Hz) for the capture" },

    { "sample-channels",
      new VarProbe<_ThisModule_,unsigned>
      (&_ThisModule_::sample_channels),
      "Number of channels (usually 1 or 2)" },

    { "sample-format",
      new VarProbe<_ThisModule_,std::string>
      (&_ThisModule_::sample_format),
      "Sample format, currently S8, S16, S32" },

    { "filename",
      new VarProbe<_ThisModule_,std::string>
      (&_ThisModule_::filename),
      "Name/pattern for the resulting file, e.g. recording-%Y%m%d_%H%M%S.ogg" },

    { "record-in-hold",
      new VarProbe<_ThisModule_,bool>
      (&_ThisModule_::record_hold),
      "Also record when in hold mode" },

    { "single-file",
      new VarProbe<_ThisModule_,bool>
      (&_ThisModule_::single_file),
      "Use a single recording file for whole session, or one for each run" },

    /* You can extend this table with labels and MemberCall or
       VarProbe pointers to perform calls or insert values into your
       class objects. Please also add a description (c-style string).

       Note that for efficiency, set_timing_description and
       check_timing_description are pointers to pre-defined strings,
       you can simply enter the descriptive strings in the table. */

    /* The table is closed off with NULL pointers for the variable
       name and MemberCall/VarProbe object. The description is used to
       give an overall description of the module. */
    { NULL, NULL,
      "Record audio."} };

  return parameter_table;
}

// constructor
SoundRecorder::SoundRecorder(Entity* e, const char* part, const
                       PrioritySpec& ps) :
  /* The following line initialises the SimulationModule base class.
     You always pass the pointer to the entity, give the classname and the
     part arguments.
     If you give a NULL pointer instead of the inco table, you will not be
     called for trim condition calculations, which is normal if you for
     example implement logging or a display.
     If you give 0 for the snapshot state, you will not be called to
     fill a snapshot, or to restore your state from a snapshot. Only
     applicable if you have no state. */
  SimulationModule(e, classname, part, NULL, 0),

  // initialize the data you need in your simulation
  capture_dev(0),
  capture_device_name(),
  sample_rate(22050),
  sample_channels(2),
  sample_format("S16"),
  filename("recording-%Y%m%d_%H%M%S.ogg"),
  sndfile(NULL),
  record_hold(false),
  sample_count(0U),
  written_sample_count(0U),
  bytes_per_point(0),
  bytes_per_sample(1U),
  recording(false),
  single_file(true),
  prev_SimulationState(SimulationState::Undefined),

  // initialize the channel access tokens
  w_progress(getId(), NameSet(getEntity(), RecordingProgress::classname, part),
             RecordingProgress::classname, "", Channel::Events),

  // activity initialization
  myclock(),
  cb1(this, &_ThisModule_::doCalculation),
  do_calc(getId(), "record audio", &cb1, ps)
{
  // connect the triggers for simulation
  do_calc.setTrigger(myclock);
}

// anon namespace for local stuff
namespace {

  // organise different formats
  struct format_table_struct {
    const char*     name;
    SDL_AudioFormat sdl_format;
    int             sf_format;
    unsigned        nbytes;
  };

  // conversion table
  static format_table_struct format_table[] = {
    { "S16", AUDIO_S16, SF_FORMAT_PCM_16, 2 },
    { "S32", AUDIO_S32, SF_FORMAT_PCM_32, 4 },
    { NULL, 0, 0, 0 }
  };

  // error exception
  struct audio_format_missing: public std::exception
  {
    const char* what() const throw()
    { return "invalid audio format supplied"; }
  };

  // get the SDL format from the given string
  SDL_AudioFormat table_SDL_Format(const std::string& fmt)
  {
    for (auto &tf: format_table) {
      if (!strcmp(tf.name, fmt.c_str())) {
	return tf.sdl_format;
      }
    }
    throw(audio_format_missing());
  }

  // get the soundfile format from the given string
  int table_SF_Format(const std::string& fmt)
  {
    for (auto &tf: format_table) {
      if (!strcmp(tf.name, fmt.c_str())) {
	return tf.sf_format;
      }
    }
    throw(audio_format_missing());
  }

  // get the soundfile format from the given string
  int table_bytes_Format(const std::string& fmt)
  {
    for (auto &tf: format_table) {
      if (!strcmp(tf.name, fmt.c_str())) {
	return tf.nbytes;
      }
    }
    throw(audio_format_missing());
  }

  // get the string from an SDL format
  const char* table_SDL_to_format(SDL_AudioFormat fmt)
  {
    for (auto &tf: format_table) {
      if (fmt == tf.sdl_format) {
	return tf.name;
      }
    }
    throw(audio_format_missing());
  }

  // adapt a filename
  std::string formatTime(const boost::posix_time::ptime& now,
                         const std::string& lft)
  {
    using namespace boost::posix_time;
    std::locale loc(std::cout.getloc(),
                    new time_facet(lft.c_str()));
    std::basic_stringstream<char> wss;
    wss.imbue(loc);
    wss << now;
    return wss.str();
  }
}


static void audiocallback(void* mod, Uint8* data, int len)
{
  reinterpret_cast<SoundRecorder*>(mod)->cbAudio(data, len);
}


void SoundRecorder::cbAudio(Uint8* data, int len)
{
  switch(bytes_per_sample) {
  case 2: {
    sf_count_t count = sf_write_short
      (sndfile, reinterpret_cast<short int*>(data), len/bytes_per_sample);
    sample_count += len/bytes_per_point;
  }
    break;
  case 4: {
    sf_count_t count = sf_write_int
      (sndfile, reinterpret_cast<int*>(data), len/bytes_per_sample);
    sample_count += len/bytes_per_point;
  }
    break;
  }
}


bool SoundRecorder::complete()
{
  /* All your parameters have been set. You may do extended
     initialisation here. Return false if something is wrong. */

  // initialize the audio
  int res = SDL_Init(SDL_INIT_AUDIO);
  if (res != 0) {
    E_MOD("Cannot initialize SDL audio, result=" << SDL_GetError());
    return false;
  }

  // check available devices
  int ndev = SDL_GetNumAudioDevices(SDL_TRUE);
  for (int ii = 0; ii < ndev; ii++) {
    I_MOD("SDL audio capture device " << ii << ": "
	  << SDL_GetAudioDeviceName(ii, 1));
    if (capture_device_name.size() == 0) {
      // take the first device as default
      capture_device_name = SDL_GetAudioDeviceName(ii, 1);
    }
  }

  // specify the recording format
  SDL_AudioSpec want, have;
  SDL_zero(want);
  want.freq = sample_rate;
  want.format = table_SDL_Format(sample_format);
  want.channels = sample_channels;
  want.samples = 4096;
  want.callback = audiocallback;
  want.userdata = this;

  capture_dev = SDL_OpenAudioDevice
    (capture_device_name.c_str(), SDL_TRUE, &want, &have, 0);
  if (capture_dev == 0) {
    E_MOD("Error opening capture device " << SDL_GetError());
    return false;
  }
  DEB("recording " << int(have.channels) << " channels at " << have.freq <<
      " format " << table_SDL_to_format(have.format));

  bytes_per_sample = table_bytes_Format(table_SDL_to_format(have.format));
  bytes_per_point = int(have.channels) * bytes_per_sample;
  memset(&sfinfo, 0, sizeof(sfinfo));
  sfinfo.samplerate = have.freq;
  sfinfo.channels = int(have.channels);
  sfinfo.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS;

  if(single_file) {
    std::string fname = formatTime
      (boost::posix_time::second_clock::universal_time(), filename);
    sndfile = sf_open(fname.c_str(), SFM_WRITE, &sfinfo);

    if (sndfile == NULL) {
      E_MOD("Cannot open capture file " << fname);
      return false;
    }
    else {
      I_MOD("Storing recording in single file " << fname);
    }
  }

  return true;
}

// destructor
SoundRecorder::~SoundRecorder()
{
  //
  sf_close(sndfile);
}

// as an example, the setTimeSpec function
bool SoundRecorder::setTimeSpec(const TimeSpec& ts)
{
  // set time spec on the clock
  myclock.changePeriodAndOffset(ts);

  // return true if everything is acceptable
  return true;
}

// and the checkTiming function
bool SoundRecorder::checkTiming(const std::vector<int>& i)
{
  if (i.size() == 3) {
    new TimingCheck(do_calc, i[0], i[1], i[2]);
  }
  else if (i.size() == 2) {
    new TimingCheck(do_calc, i[0], i[1]);
  }
  else {
    return false;
  }
  return true;
}

// tell DUECA you are prepared
bool SoundRecorder::isPrepared()
{
  bool res = true;

  // channel for writing progress of recording
  CHECK_TOKEN(w_progress);

  // return result of checks
  return res;
}

// start the module
void SoundRecorder::startModule(const TimeSpec &time)
{
  do_calc.switchOn(time);
}

// stop the module
void SoundRecorder::stopModule(const TimeSpec &time)
{
  do_calc.switchOff(time);
}

// should read the input channels here, and calculate and write the
// appropriate output
void SoundRecorder::doCalculation(const TimeSpec& ts)
{
  // check the state we are supposed to be in
  SimulationState::Type cur_SimulationState = getAndCheckState(ts);
  switch (cur_SimulationState) {
  case SimulationState::HoldCurrent: {

    // switch off at last cycle before stop
    if(do_calc.lastCycle(ts)) {
      SDL_PauseAudioDevice(capture_dev, SDL_TRUE);
      sf_close(sndfile);
      recording = false;
    }

    // switch when changing state
    if(prev_SimulationState != SimulationState::HoldCurrent) {

      // switch off for "advance-only" recording
      if (!record_hold) {
	SDL_PauseAudioDevice(capture_dev, SDL_TRUE);
	if(!single_file) {
	  sf_close(sndfile);
	}
	recording = false;
      }
      // only switch on for recording in hold
      else {
	if(!single_file) {
	  // open new file
	  std::string fname = formatTime
	    (boost::posix_time::second_clock::universal_time(), filename);
	  sndfile = sf_open(fname.c_str(), SFM_WRITE, &sfinfo);

	  if (sndfile == NULL) {
	    E_MOD("Cannot open new capture file " << fname);
	    recording = false;
	    break;
	  }
	  else {
	    I_MOD("Storing recording in new HC file " << fname);
	  }
	}

	SDL_PauseAudioDevice(capture_dev, SDL_FALSE);
	recording = true;
      }
    }
    break;
    }

  case SimulationState::Replay:
  case SimulationState::Advance: {
    // switch when changing state
    if(prev_SimulationState != SimulationState::Advance &&
       prev_SimulationState != SimulationState::Replay) {
      if(!single_file) {
	// open new file
	std::string fname = formatTime
	  (boost::posix_time::second_clock::universal_time(), filename);
	sndfile = sf_open(fname.c_str(), SFM_WRITE, &sfinfo);

	if (sndfile == NULL) {
	  E_MOD("Cannot open new capture file " << fname);
	  recording = false;
	  break;
	}
	else {
	  I_MOD("Storing recording in new Adv/Replay file " << fname);
	}
      }

      SDL_PauseAudioDevice(capture_dev, SDL_FALSE);
      recording = true;
    }
    break;
    }
  default:
    // other states should never be entered for a SimulationModule,
    // HardwareModules on the other hand have more states. Throw an
    // exception if we get here,
    throw CannotHandleState(getId(),GlobalId(), "state unhandled");
  }

  // remember for next time
  prev_SimulationState = cur_SimulationState;

  // write information on the number of samples if recording
  if (recording && written_sample_count < sample_count) {
    DataWriter<RecordingProgress> pr(w_progress, ts);
    pr.data().sample_count = sample_count;
    written_sample_count = sample_count;
  }
}

// Make a TypeCreator object for this module, the TypeCreator
// will check in with the scheme-interpreting code, and enable the
// creation of modules of this type
static TypeCreator<SoundRecorder> a(SoundRecorder::getMyParameterTable());
