/* ------------------------------------------------------------------   */
/*      item            : WorldListener.cxx
        made by         : repa
        from template   : DuecaModuleTemplate.cxx
        template made by: Rene van Paassen
        date            : Sat Nov 11 22:04:39 2017
        category        : body file
        description     :
        changes         : Sat Nov 11 22:04:39 2017 first version
        template changes: 030401 RvP Added template creation comment
                          060512 RvP Modified token checking code
                          160511 RvP Some comments updated
        language        : C++
        copyright       : (c) 2016 TUDelft-AE-C&S
*/


#define WorldListener_cxx

// include the definition of the module class
#include "WorldListener.hxx"

// include the debug writing header, by default, write warning and
// error messages
#define W_MOD
#define E_MOD
#include <debug.h>

// include additional files needed for your calculation here
#include <sstream>
#include <iomanip>

// the standard package for DUSIME, including template source
#define DO_INSTANTIATE
#include <dueca.h>
#include <MemberCall2Way.hxx>
#include <Entity.hxx>

// class/module name
const char* const WorldListener::classname = "world-listener";

// Parameters to be inserted
const ParameterTable* WorldListener::getMyParameterTable()
{
  static const ParameterTable parameter_table[] = {
    { "set-timing",
      new MemberCall<_ThisModule_,TimeSpec>
        (&_ThisModule_::setTimeSpec), set_timing_description },

    { "check-timing",
      new MemberCall<_ThisModule_,vector<int> >
      (&_ThisModule_::checkTiming), check_timing_description },

    { "predict-dt",
      new VarProbe<_ThisModule_,double> ( &_ThisModule_::t_predict ),
      "prediction time span for the data extrapolation" },

    { "predict-dt-max",
      new VarProbe<_ThisModule_,double> ( &_ThisModule_::max_predict ),
      "maximum prediction to be attempted on old data" },

    { "initial-ears" ,
      new MemberCall<_ThisModule_,vector<double> >
      ( &_ThisModule_::initialEar ),
      "Set initial listener position, orientation and speed" },

    { "config-per-cycle",
      new VarProbe<_ThisModule_,int> ( &_ThisModule_::num_config_per_round ),
      "For online configuration, number of configuration commands per\n"
      "draw cycle" },

    { "set-listener",
      new MemberCall2Way<_ThisModule_,ScriptCreatable>
      (&_ThisModule_::setListener),
      "Configure the listening back-end" },

    { "ego-motion-channel",
      new MemberCall<_ThisModule_,std::string>
      (&_ThisModule_::setEgoMotionChannel),
      "Override channel name for listening observer's motion" },

    { "add-world-information-channel",
      new MemberCall<_ThisModule_,std::vector<std::string> >
      (&_ThisModule_::addWorldInformationChannel),
      "Add one or more world info channels. If not used, only the default\n"
      "AnyAudioClass://audio channel is monitored, if used, and you need this\n"
      "channel, explicitly add it" },

#if defined(DUECA_CONFIG_HDF5)
    { "control-logger",
      new MemberCall<_ThisModule_,std::string>
      (&_ThisModule_::controlLogger),
      "Specify a channel for control of an HDF logger" },
#endif

    { "keep-running",
      new VarProbe<_ThisModule_,bool>(&_ThisModule_::keep_running),
      "Do not stop the sound module with DUECA's state machine." },

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
      "3D listening experience."} };

  return parameter_table;
}

// constructor
WorldListener::WorldListener(Entity* e, const char* part, const
                   PrioritySpec& ps) :
  /* The following line initialises the SimulationModule base class.
     You always pass the pointer to the entity, give the classname and the
     part arguments. */
  SimulationModule(e, classname, part),

  // initialize the data you need in your simulation or process
  listener(NULL),
  max_predict(1.0),
  t_predict(0.01),
  num_config_per_round(1),
  r_own(),
  m_others(),
  no_explicit_entity_watch(true),
#if defined(DUECA_CONFIG_HDF5)
  runnumber(0),
  sendlogcontrol(true),
  w_logconfig(),
#endif
  keep_running(false),
  // a callback object, pointing to the main calculation function
  cb1(this, &_ThisModule_::doCalculation),
  // the module's main activity
  do_calc(getId(), "run audio feed", &cb1, ps)
{
  //
}

bool WorldListener::complete()
{
  /* All your parameters have been set. You may do extended
     initialisation here. Return false if something is wrong. */
  if (!listener) {
    E_MOD("need to specify listener");
    return false;
  }

  // default case, for compatibility
  if (!r_own) {
    r_own.reset(new ChannelReadToken
                (getId(), NameSet(getEntity(), "BaseObjectMotion", ""),
                 "BaseObjectMotion", 0, Channel::Continuous,
                 Channel::OnlyOneEntry, Channel::JumpToMatchTime, 0.2));
  }

  // connect the triggers for simulation
  do_calc.setTrigger(*r_own);

  if (no_explicit_entity_watch) {
    m_others.push_back
      (boost::shared_ptr<ChannelWatcher>
       (new ChannelWatcher(NameSet( "audio", "AnyAudioClass", ""), true)));
  }

  return listener->init();
}

// destructor
WorldListener::~WorldListener()
{
  //
}

// as an example, the setTimeSpec function
bool WorldListener::setTimeSpec(const TimeSpec& ts)
{
  // a time span of 0 is not acceptable
  if (ts.getValiditySpan() == 0) return false;

  // specify the timespec to the activity
  do_calc.setTimeSpec(ts);

  // return true if everything is acceptable
  return true;
}

bool WorldListener::initialEar(const std::vector<double>& epos)
{
  if (epos.size() != 9) {
    E_MOD("Need 9 parameters, position, attitude (phi, theta, psi) and speed");
    return false;
  }
  if (!listener) {
    E_MOD("Need to specify listener first");
    return false;
  }
  BaseObjectMotion ear;
  for (unsigned ii = 3; ii--; ) {
    ear.xyz[ii] = epos[ii];
    ear.uvw[ii] = epos[ii+6];
  }
  ear.setquat(epos[3], epos[4], epos[5]);
  listener->setBase(ear);
  return true;
}

#if defined(DUECA_CONFIG_HDF5)
bool WorldListener::controlLogger(const std::string& cname)
{
  w_logconfig.reset
    (new ChannelWriteToken
     (getId(), NameSet(cname), HDFLogConfig::classname,
      "from WorldListener", Channel::Events));
  return true;
}
#endif

bool WorldListener::setListener(ScriptCreatable& obj, bool in)
{
  // check direction
  if (!in) return false;

  // try a dynamic cast
  worldlistener::ListenerBase* l =
    dynamic_cast<worldlistener::ListenerBase*>(&obj);
  if (l == NULL) {
    E_MOD("Object is not derived from ListenerBase");
    return false;
  }
  if (listener) {
    E_MOD("Second listener specified");
    return false;
  }

#if DUECA_VERSION_NUM < DUECA_VERSION(2,3,0)
  // now make sure Scheme does not clean this helper from right
  // under our noses:
  getMyEntity()->scheme_id.addReferred(obj.scheme_id.getSCM());
#endif

  listener.reset(l);
  return true;
}

bool WorldListener::setEgoMotionChannel(const std::string& n)
{
  r_own.reset(new ChannelReadToken
              (getId(), NameSet(n),
               "BaseObjectMotion", 0, Channel::Continuous,
               Channel::OnlyOneEntry, Channel::JumpToMatchTime, 0.2));
  return true;
}

bool WorldListener::addWorldInformationChannel
(const std::vector<std::string>& ch)
{
  no_explicit_entity_watch = false;
  for (auto &chn: ch) {
    m_others.push_back
      (boost::shared_ptr<ChannelWatcher>
       (new ChannelWatcher(NameSet(chn), true)));
  }
  return true;
}

// the checkTiming function installs a check on the activity/activities
// of the module
bool WorldListener::checkTiming(const vector<int>& i)
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
bool WorldListener::isPrepared()
{
  bool res = true;

  // Example checking a token:
  CHECK_TOKEN(*r_own);

#if defined(DUECA_CONFIG_HDF5)
  if (bool(w_logconfig)) {
    CHECK_TOKEN(*w_logconfig);
  }
#endif

  // return result of checks
  return res;
}

// start the module
void WorldListener::startModule(const TimeSpec &time)
{
  do_calc.switchOn(time);
}

// stop the module
void WorldListener::stopModule(const TimeSpec &time)
{
  if (!keep_running) {
    do_calc.switchOff(time);
  }
}

// this routine contains the main simulation process of your module. You
// should read the input channels here, and calculate and write the
// appropriate output
void WorldListener::doCalculation(const TimeSpec& ts)
{
  // read the listener motion
  try {
    DataReader<BaseObjectMotion> o(*r_own, ts);
    listener->setBase(o.data());
  }
  catch (const exception& e) {
    W_MOD("caught exception " << e.what());
  }

  // check addition and removal of other entities
  ChannelEntryInfo ei;
  for (auto &wchannel: m_others) {
    while (wchannel->checkChange(ei)) {
      if (ei.created) {
        // new entry
  #if DUECA_VERSION_NUM < DUECA_VERSION(2,6,0)
        listener->addControllable
          (getId(), NameSet( "audio", "AnyAudioClass", ""), ei.entry_id,
           ei.creation_id, ei.data_class, ei.entry_label,
           ei.time_aspect);
  #else
          listener->addControllable
            (getId(), wchannel->getChannelName(), ei.entry_id,
             ei.creation_id, ei.data_class, ei.entry_label,
             ei.time_aspect);
  #endif
      }
      else {
        // remove an entry
        listener->removeControllable(ei.creation_id);
      }
    }
  }

  switch(getAndCheckState(ts)) {
  case SimulationState::HoldCurrent:
    if (keep_running) {
      listener->iterate(ts);
    }
    else {
      listener->silence(ts);
    }

#if defined(DUECA_CONFIG_HDF5)
    if (sendlogcontrol && bool(w_logconfig)) {
      DataWriter<HDFLogConfig> cnf(*w_logconfig, ts);
      std::stringstream prf;
      prf << "run" << std::setw(4) << std::setfill('0') << runnumber++;
      cnf.data().prefix = prf.str();
      sendlogcontrol = false;
    }
#endif
    break;
  case SimulationState::Advance:
    // now run over the other entities and update/read
    listener->iterate(ts);
#if defined(DUECA_CONFIG_HDF5)
    sendlogcontrol = true;
#endif
    break;
  default:
    break;
    // not handled
  }
}

// Make a TypeCreator object for this module, the TypeCreator
// will check in with the script code, and enable the
// creation of modules of this type
static TypeCreator<WorldListener> a(WorldListener::getMyParameterTable());
