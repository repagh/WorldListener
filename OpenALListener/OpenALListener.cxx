/* ------------------------------------------------------------------   */
/*      item            : OpenALListener.cxx
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

#define OpenALListener_cxx
// include the definition of the helper class
#include "OpenALListener.hxx"
#include "OpenALObject.hxx"
#include "OpenALObjectFactory.hxx"
#include "OpenALBufferManager.hxx"
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
OpenALListener::OpenALListener() :
  device(NULL),
  context(NULL),
  devicename(),
  listener_gain(0.999),
  distance_model(AL_INVERSE_DISTANCE_CLAMPED),
  allow_unknown(false),
  buffermanager(new OpenALBufferManager())
{

}

bool OpenALListener::init()
{
  // enumerate possible devices
  ALboolean enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
  if (enumeration == AL_TRUE) {
    cerr << "OpenAL devices:" << endl;
    const ALCchar *device = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
    while (device && device[0] != '\0') {
      cerr << device << endl;
      size_t len = strlen(device);
      device += len + 1;
    }
  }

  // open the device
  device = alcOpenDevice(devicename.size() ? devicename.c_str() : NULL);
  if (device == NULL) {
    E_MOD("Cannot open audio device " << devicename);
#if 0
    // enumerate possible devices
    ALboolean enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
    if (enumeration == AL_TRUE) {
      cerr << "OpenAL devices:" << endl;
      const ALCchar *device = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
      while (device && device[0] != '\0') {
        cerr << device << endl;
        size_t len = strlen(device);
        device += len + 1;
      }
    }
#endif
    return false;
  }
  else {
    W_MOD("opened device " << alcGetString(device, ALC_DEVICE_SPECIFIER));
  }

  // create audio context
  context = alcCreateContext(device, NULL);
  if (!alcMakeContextCurrent(context)) {
    ALCenum error = alGetError();
    E_MOD("Cannot make audio context current, error " << error);
    return false;
  }

  // distance model setting
  alDistanceModel(distance_model);

  this->setBase(BaseObjectMotion());

  // set listener initial position
  alListenerfv(AL_POSITION, xyz);
  alListenerfv(AL_VELOCITY, uvw);
  alListenerfv(AL_ORIENTATION, ori);
  alListenerf(AL_GAIN, listener_gain);

  for (controllables_t::iterator ii = controlled_sources.begin();
       ii != controlled_sources.end(); ii++) {
    ii->second->initSound(this);
  }
  for (statics_t::iterator ii = other_sources.begin();
       ii != other_sources.end(); ii++) {
    (*ii)->initSound(this);
  }

  return true;
}

void OpenALListener::setBase(const BaseObjectMotion& listener)
{
  Eigen::Matrix<double,3,3> orimat;
  listener.orientationToR(orimat);

  for (unsigned ii = 3; ii--; ) {
    xyz[ii] = listener.xyz[ii];
    uvw[ii] = listener.uvw[ii];
    ori[ii] = orimat(ii,0);
    ori[ii+3] = orimat(ii,2);
  }
  if (device) {
    // update position, vel and orientation
    alListenerfv(AL_POSITION, xyz);
    alListenerfv(AL_VELOCITY, uvw);
    alListenerfv(AL_ORIENTATION, ori);
  }
}

// destructor
OpenALListener::~OpenALListener()
{
  alcDestroyContext(context);
  alcCloseDevice(device);
}

bool OpenALListener::createControllable
  (const GlobalId& master_id, const NameSet& cname, entryid_type entry_id,
   uint32_t creation_id, const std::string& data_class,
   const std::string& entry_label,
   Channel::EntryTimeAspect time_aspect)
{
  try {
    // check for a "pre-cooked" entry
    named_objects_t::key_type key(data_class, entry_label);
    named_objects_t::iterator oidx = named_objects.find(key);

    // if found, connect entry, transfer the controlled list and return
    if (oidx != named_objects.end()) {
      I_MOD("connecting sound class " << entry_label << " entry " << entry_id);
      oidx->second->connect(master_id, cname, entry_id, time_aspect);
      if (context) {
        alcMakeContextCurrent(context);
        oidx->second->initSound(this);
      }
      controlled_sources[creation_id] = oidx->second;
      named_objects.erase(oidx);
      return true;
    }

    // not found, create entry on the basis of data class and entry label
    I_MOD("creating for sound class " << entry_label << " entry " << entry_id);
    boost::intrusive_ptr<OpenALObject> op;
    WorldDataSpec obj = retrieveFactorySpec
      (data_class, entry_label, creation_id);
    I_MOD("creating with data " << obj);
    op = OpenALObjectFactory::instance().create(obj.type, obj);
    op->connect(master_id, cname, entry_id, time_aspect);
    if (context) {
      alcMakeContextCurrent(context);
      op->initSound(this);
    }
    controlled_sources[creation_id] = op;
    return true;
  }
  catch (const CFCannotMake& problem) {
    if (!allow_unknown) {
      W_MOD("OpenALListener: factory cannot create for " << data_class <<
            " encountered: " <<  problem.what());
      throw(problem);
    }
    W_MOD("OpenALListener: factory cannot create for " << data_class <<
          ", ignoring this entry");
  }
  catch (const MapSpecificationError& problem) {
    if (!allow_unknown) {
      W_MOD("OpenALListener: not configured for " << data_class <<
            " encountered: " <<  problem.what());
      throw(problem);
    }
    W_MOD("OpenALListener: not configured for " << data_class <<
          ", ignoring this entry");
  }
  catch (const std::exception& problem) {
    W_MOD("OpenALListener: when trying to create for " << data_class <<
          " encountered: " <<  problem.what());
    throw(problem);
  }
  return false;
}

void OpenALListener::addConstantSource(boost::intrusive_ptr<OpenALObject> op)
{
  if (context) {
    alcMakeContextCurrent(context);
    op->initSound(this);
  }
  other_sources.push_back(op);
}

void OpenALListener::
addControlledSource(boost::intrusive_ptr<OpenALObject> op)
{
  named_objects_t::key_type key(op->getChannelClass(), op->getName());
  if (named_objects.find(key) == named_objects.end()) {
    named_objects[key] = op;
  }
  else {
    W_MOD("Already have source (" << key.first << "," << key.second << ")");
  }
}

void OpenALListener::removeControllable(uint32_t creation_id)
{
  controlled_sources.erase(creation_id);
}

void OpenALListener::iterate(const TimeSpec& ts)
{
  alcMakeContextCurrent(context);
  for (controllables_t::iterator ii = controlled_sources.begin();
       ii != controlled_sources.end(); ii++) {
    ii->second->iterate(ts, listener);
  }
  for (statics_t::iterator ii = other_sources.begin();
       ii != other_sources.end(); ii++) {
    (*ii)->iterate(ts, listener);
  }
}

void OpenALListener::silence(const TimeSpec& ts)
{
  alcMakeContextCurrent(context);
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
