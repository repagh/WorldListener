/* ------------------------------------------------------------------   */
/*      item            : PortAudioListener.cxx
        made by         : repa
        from template   : DuecaModuleTemplate.cxx
        template made by: Rene van Paassen
        date            : Sat Nov 11 22:13:10 2017
        category        : body file
        description     :
        changes         : Sat Nov 11 22:13:10 2017 first version
        template changes: 030401 RvP Added template creation comment
        language        : C++
        copyright       : (c) 2016 TUDelft-AE-C&S
*/

#define PortAudioListener_cxx
// include the definition of the helper class
#include "PortAudioListener.hxx"
#include "PortAudioObject.hxx"
#include "PortAudioObjectFactory.hxx"
#include "PortAudioBufferManager.hxx"
#include <Eigen/Dense>

// include the debug writing header. Warning and error messages
// are on by default, debug and info can be selected by
// uncommenting the respective defines
//#define D_MOD
#define I_MOD
#include <debug.h>

// include additional files needed for your calculation here
#include "comm-objects.h"

#define DO_INSTANTIATE
USING_DUECA_NS;

OPEN_NS_WORLDLISTENER;

// constructor
PortAudioListener::PortAudioListener() :
  devicename(),
  idevice(-1),
  num_channels(0),
  samplerate(44100),
  suggested_output_latency(0.01),
  allow_unknown(false),
  buffermanager(new PortAudioBufferManager()),
  sound_off(true)
{
  //
}

int PortAudioListener::cfun(const void *input, void *output,
                            unsigned long frameCount,
                            const PaStreamCallbackTimeInfo* timeinfo,
                            PaStreamCallbackFlags statusFlags, void *userdata)
{
  auto self = reinterpret_cast<PortAudioListener*>(userdata);

  // convert to float
  auto out = reinterpret_cast<float*>(output);

  // initially zero the output
  std::fill(out, out+frameCount*self->num_channels, 0.0f);

  // run over all sound objects, let them add data to the buffer
  for (auto &o: self->controlled_sources) {
    o.second->addData(out, frameCount*self->num_channels);
  }

  // optionally also for static sound objects
  for (auto &o: self->other_sources) {
    o->addData(out, frameCount*self->num_channels);
  }

  // always continue
  return paContinue;
}


bool PortAudioListener::init()
{
  auto err = Pa_Initialize();
  if (err != paNoError) {
    E_MOD("Error initializing PortAudio: " << Pa_GetErrorText(err));
    return false;
  }

  // list devices
  int ndev = Pa_GetDeviceCount();
  if (ndev < 0) {
    E_MOD("Error in getting device count: " << Pa_GetErrorText(ndev));
    return false;
  }

  const PaDeviceInfo *devinfo = NULL;
  for (auto i = 0; i < ndev; i++) {
    devinfo = Pa_GetDeviceInfo(i);
    I_MOD("Dev " << i << " \"" << devinfo->name <<
          "\" hostapi: " << Pa_GetHostApiInfo(devinfo->hostApi)->name <<
          " in: " << devinfo->maxInputChannels <<
          " out: " << devinfo->maxOutputChannels);
    if (std::string(devinfo->name) == devicename) {
      I_MOD("Selecting device " << i);
      idevice = i;
      num_channels = devinfo->maxOutputChannels;
    }
  }

  PaStreamParameters out_param {
    .device = idevice,
      .channelCount = num_channels,
      .sampleFormat = paFloat32,
      .suggestedLatency = min(devinfo->defaultHighOutputLatency,
                              max(devinfo->defaultLowOutputLatency,
                                  PaTime(suggested_output_latency)))
  };

  // create a stream
  err = Pa_OpenStream
    (&stream, NULL, &out_param,
     samplerate, paFramesPerBufferUnspecified,
     paNoFlag, cfun, this);

  if (err != paNoError) {
    E_MOD("Error opening stream: " << Pa_GetErrorText(err));
    return false;
  }

  // initialisation done
  return true;
}



void PortAudioListener::setBase(const BaseObjectMotion& listener)
{
  // not relevant here
}

// destructor
PortAudioListener::~PortAudioListener()
{
  if (idevice >= 0) {
    auto err = Pa_Terminate();
    if (err != paNoError) {
      E_MOD("Error closing down PortAudio: " << Pa_GetErrorText(err));
    }
  }
}

bool PortAudioListener::createControllable
  (const GlobalId& master_id, const NameSet& cname, entryid_type entry_id,
   uint32_t creation_id, const std::string& data_class,
   const std::string& entry_label,
   Channel::EntryTimeAspect time_aspect)
{
  try {
    // check for a "pre-cooked" entry, with data class and entry label
    // as key
    named_objects_t::key_type key(data_class, entry_label);
    named_objects_t::iterator oidx = named_objects.find(key);

    // if found, connect entry, transfer the controlled list and return
    if (oidx != named_objects.end()) {

      // connect to the DUECA infrastructure/channels
      I_MOD("connecting sound class " << entry_label << " entry " << entry_id);
      oidx->second->connect(master_id, cname, entry_id, time_aspect);

      // initialize with the sound system
      if (idevice >= 0) {
        oidx->second->initSound(this);
      }

      // transfer to controlled sources
      controlled_sources[creation_id] = oidx->second;
      named_objects.erase(oidx);
      return true;
    }

    // not found, create entry on the basis of data class and entry label
    I_MOD("creating for sound class " << entry_label << " entry " << entry_id);
    boost::intrusive_ptr<PortAudioObject> op;

    // Use the factory to find the matching supplier
    WorldDataSpec obj = retrieveFactorySpec
      (data_class, entry_label, creation_id);
    I_MOD("creating with data " << obj);

    // run the factory
    op = PortAudioObjectFactory::instance().create(obj.type, obj);

    // connect to the DUECA infrastructure/channels
    op->connect(master_id, cname, entry_id, time_aspect);

    // initialize with the sound system
    if (idevice >= 0) {
      op->initSound(this);
    }

    // add to the controlled sources
    controlled_sources[creation_id] = op;
    return true;
  }
  catch (const CFCannotMake& problem) {
    if (!allow_unknown) {
      W_MOD("PortAudioListener: factory cannot create class \"" << data_class <<
            "\" encountered: " <<  problem.what());
      throw(problem);
    }
    W_MOD("PortAudioListener: factory cannot create class \"" << data_class <<
          "\", ignoring this entry");
  }
  catch (const MapSpecificationError& problem) {
    if (!allow_unknown) {
      W_MOD("PortAudioListener: not configured for " << data_class <<
            " encountered: " <<  problem.what());
      throw(problem);
    }
    W_MOD("PortAudioListener: not configured for " << data_class <<
          ", ignoring this entry");
  }
  catch (const std::exception& problem) {
    W_MOD("PortAudioListener: when trying to create for " << data_class <<
          " encountered: " <<  problem.what());
    throw(problem);
  }
  return false;
}

void PortAudioListener::addConstantSource(boost::intrusive_ptr<PortAudioObject> op)
{
  if (idevice >= 0) {
    op->initSound(this);
  }
  other_sources.push_back(op);
}

void PortAudioListener::
addControlledSource(boost::intrusive_ptr<PortAudioObject> op)
{
  named_objects_t::key_type key(op->getChannelClass(), op->getName());
  if (named_objects.find(key) == named_objects.end()) {
    named_objects[key] = op;
  }
  else {
    W_MOD("Already have source (" << key.first << "," << key.second << ")");
  }
}

void PortAudioListener::removeControllable(uint32_t creation_id)
{
  controlled_sources.erase(creation_id);
}

void PortAudioListener::iterate(const TimeSpec& ts)
{
  if (sound_off) {
    I_MOD("Starting stream, first iterate");
    auto err = Pa_StartStream(stream);
    if (err != paNoError) {
      E_MOD("Error starting PortAudio stream: " << Pa_GetErrorText(err));
    }
    sound_off = false;
  }
    
  for (controllables_t::iterator ii = controlled_sources.begin();
       ii != controlled_sources.end(); ii++) {
    ii->second->iterate(ts, listener);
  }
  for (statics_t::iterator ii = other_sources.begin();
       ii != other_sources.end(); ii++) {
    (*ii)->iterate(ts, listener);
  }
}

void PortAudioListener::silence(const TimeSpec& ts)
{
  if (!sound_off) {
    I_MOD("Stopping stream, first silence");
    auto err = Pa_StopStream(stream);
    if (err != paNoError) {
      E_MOD("Error starting PortAudio stream: " << Pa_GetErrorText(err));
    }
    sound_off = true;
  }
  
  for (controllables_t::iterator ii = controlled_sources.begin();
       ii != controlled_sources.end(); ii++) {
    ii->second->silence();
  }
  for (statics_t::iterator ii = other_sources.begin();
       ii != other_sources.end(); ii++) {
    (*ii)->silence();
  }
}

CLOSE_NS_WORLDLISTENER;
