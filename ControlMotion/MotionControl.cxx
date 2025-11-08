/* ------------------------------------------------------------------   */
/*      item            : MotionControl.cxx
        made by         : repa
        from template   : DuecaModuleTemplate.cxx
        template made by: Rene van Paassen
        date            : Fri Dec 15 16:17:58 2017
        category        : body file
        description     :
        changes         : Fri Dec 15 16:17:58 2017 first version
        template changes: 030401 RvP Added template creation comment
                          060512 RvP Modified token checking code
                          160511 RvP Some comments updated
        language        : C++
        copyright       : (c) 2016 TUDelft-AE-C&S
*/

#define MotionControl_cxx

// include the definition of the module class
#include "MotionControl.hxx"

// include the debug writing header, by default, write warning and
// error messages
#define W_MOD
#define E_MOD
#include <debug.h>

// include additional files needed for your calculation here

// the standard package for DUSIME, including template source
#define DO_INSTANTIATE
#include <dueca.h>

// class/module name
const char *const MotionControl::classname = "motion-control";

// Parameters to be inserted
const ParameterTable *MotionControl::getMyParameterTable()
{
  static const ParameterTable parameter_table[] = {
    { "set-timing",
      new MemberCall<_ThisModule_, TimeSpec>(&_ThisModule_::setTimeSpec),
      set_timing_description },

    { "check-timing",
      new MemberCall<_ThisModule_, vector<int>>(&_ThisModule_::checkTiming),
      check_timing_description },

    { "add-moving-sound",
      new MemberCall<_ThisModule_, std::string>(&_ThisModule_::addMovingSound),
      "add a moving sound, specify its name as channel label" },

    { "add-fixed-sound",
      new MemberCall<_ThisModule_, std::string>(&_ThisModule_::addStaticSound),
      "add a static sound, specify its name as channel label" },

    { "event-interval",
      new VarProbe<_ThisModule_, int>(&_ThisModule_::next_eventinterval),
      "specify event type of next sound and interval of event sending" },

    { "set-coordinates",
      new VarProbe<_ThisModule_, std::vector<double>>(
        &_ThisModule_::next_coordinates),
      "sound coordinates, x, y, z, phi, tht, psi, u, v, w, p, q, r" },

    { "set-dt", new VarProbe<_ThisModule_, double>(&_ThisModule_::next_dt),
      "time step for motion, positive, a/c like integration, (uvw in body\n"
      "constant) negative, satellite-like motion (uvw in world constant)" },

      /* You can extend this table with labels and MemberCall or
     VarProbe pointers to perform calls or insert values into your
     class objects. Please also add a description (c-style string).

     Note that for efficiency, set_timing_description and
     check_timing_description are pointers to pre-defined strings,
     you can simply enter the descriptive strings in the table. */

      /* The table is closed off with NULL pointers for the variable
     name and MemberCall/VarProbe object. The description is used to
     give an overall description of the module. */
    { NULL, NULL, "listener position control" }
  };

  return parameter_table;
}

// constructor
MotionControl::MotionControl(Entity *e, const char *part,
                             const PrioritySpec &ps) :
  /* The following line initialises the SimulationModule base class.
     You always pass the pointer to the entity, give the classname and
     the part arguments. */
  SimulationModule(e, classname, part),

  // initialize the data you need in your simulation or process
  w_motion(getId(), NameSet(getEntity(), BaseObjectMotion::classname, ""),
           BaseObjectMotion::classname, "self", Channel::Continuous),

  // create a clock, if you need time based triggering
  // instead of triggering on the incoming channels
  myclock(),

  // a callback object, pointing to the main calculation function
  cb1(this, &_ThisModule_::doCalculation),
  // the module's main activity
  do_calc(getId(), "observer control", &cb1, ps)
{
  // do the actions you need for the simulation
  memset(current, 0, sizeof(current));

  // connect the triggers for simulation
  do_calc.setTrigger(myclock);
}

bool MotionControl::complete()
{
  static GladeCallbackTable controls[] = {
    { "position1", "value_changed",
      gtk_callback(&_ThisModule_::changePositionOrientation), gpointer(0) },
    { "position2", "value_changed",
      gtk_callback(&_ThisModule_::changePositionOrientation), gpointer(1) },
    { "position3", "value_changed",
      gtk_callback(&_ThisModule_::changePositionOrientation), gpointer(2) },
    { "orientation1", "value_changed",
      gtk_callback(&_ThisModule_::changePositionOrientation), gpointer(3) },
    { "orientation2", "value_changed",
      gtk_callback(&_ThisModule_::changePositionOrientation), gpointer(4) },
    { "orientation3", "value_changed",
      gtk_callback(&_ThisModule_::changePositionOrientation), gpointer(5) },
    { NULL, NULL, NULL, NULL }
  };
#if GTK_MAJOR_VERSION == 3
  viewcontrol.readGladeFile(
    "../../../../WorldListener/ControlMotion/motioncontrol.ui", "viewcontrol",
    this, controls);
#elif GTK_MAJOR_VERSION == 2
  viewcontrol.readGladeFile(
    "../../../../WorldListener/ControlMotion/motioncontrol.glade",
    "viewcontrol", this, controls);
#else
#error "MotionControl not adapted to this gtk version"
#endif
  /* All your parameters have been set. You may do extended
     initialisation here. Return false if something is wrong. */
  return completeSound();
}

// destructor
MotionControl::~MotionControl()
{
  //
}

// as an example, the setTimeSpec function
bool MotionControl::setTimeSpec(const TimeSpec &ts)
{
  // a time span of 0 is not acceptable
  if (ts.getValiditySpan() == 0)
    return false;

  // specify the timespec to the activity
  myclock.changePeriodAndOffset(ts);

  // do whatever else you need to process this in your model
  // hint: ts.getDtInSeconds()

  // return true if everything is acceptable
  return true;
}

// the checkTiming function installs a check on the activity/activities
// of the module
bool MotionControl::checkTiming(const vector<int> &i)
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

bool MotionControl::completeSound()
{
  if (next_motion_name.size()) {
    if (next_coordinates.size() == 12) {
      AudioObjectMotion next;
      next.xyz[0] = next_coordinates[0];
      next.xyz[1] = next_coordinates[1];
      next.xyz[2] = next_coordinates[2];
      next.setquat(next_coordinates[3], next_coordinates[4],
                   next_coordinates[5]);
      next.uvw[0] = next_coordinates[6];
      next.uvw[1] = next_coordinates[7];
      next.uvw[2] = next_coordinates[8];
      next.omega[0] = next_coordinates[9];
      next.omega[1] = next_coordinates[10];
      next.omega[2] = next_coordinates[11];
      next.dt = next_dt;
      next.volume = 1.0;
      motion_sets.push_back(boost::shared_ptr<MotionSet>(
        new MotionSet(next, getId(), next_motion_name, next_eventinterval)));
      next_motion_name = "";
    }
    else {
      W_MOD("missing coordinates for sound");
      return false;
    }
  }
  if (next_fixed_name.size()) {
    if (next_coordinates.size() == 0) {
      AudioObjectFixed next;
      next.volume = 1.0;
      fixed_sets.push_back(boost::shared_ptr<FixedSet>(
        new FixedSet(next, getId(), next_fixed_name, next_eventinterval)));
      next_fixed_name = "";
    }
    else {
      W_MOD("missing coordinates for sound");
      return false;
    }
  }
  return true;
}

bool MotionControl::addMovingSound(const std::string &name)
{
  if (!completeSound())
    return false;

  // reset
  next_motion_name = name;
  next_eventinterval = 0;
  next_dt = 0.0;
  next_coordinates.resize(0);
  return true;
}

bool MotionControl::addStaticSound(const std::string &name)
{
  if (!completeSound())
    return false;

  // reset
  next_fixed_name = name;
  next_eventinterval = 0;
  next_dt = 0.0;
  next_coordinates.resize(0);
  return true;
}

MotionControl::MotionSet::MotionSet(const AudioObjectMotion &i,
                                    const GlobalId &master_id,
                                    const std::string name, int eventinterval) :
  name(name),
  eventinterval(eventinterval),
  eventcounter(eventinterval),
  initial(i),
  moving(i)
{
  w_entity.reset(new ChannelWriteToken(
    master_id, NameSet("audio", "AnyAudioClass", ""), "AudioObjectMotion", name,
    eventinterval ? Channel::Events : Channel::Continuous,
    Channel::OneOrMoreEntries));
}

bool MotionControl::MotionSet::isValid()
{
  bool res = w_entity->isValid();
  if (!res) {
    W_MOD("token for object " << name << " not valid");
  }
  return res;
}

void MotionControl::MotionSet::step(const TimeSpec &ts)
{
  moving.extrapolate(ts.getDtInSeconds());

  if (!eventinterval || --eventcounter == 0) {
    DataWriter<AudioObjectMotion> w(*w_entity, ts);
    w.data() = moving;
    eventcounter = eventinterval;
  }
}

void MotionControl::MotionSet::hold(const TimeSpec &ts)
{
  if (!eventinterval) {
    DataWriter<AudioObjectMotion> w(*w_entity, ts);
    w.data() = moving;
    eventcounter = eventinterval;
  }
}

MotionControl::FixedSet::FixedSet(const AudioObjectFixed &i,
                                  const GlobalId &master_id,
                                  const std::string name, int eventinterval) :
  name(name),
  eventinterval(eventinterval),
  eventcounter(eventinterval),
  initial(i)
{
  w_entity.reset(new ChannelWriteToken(
    master_id, NameSet("audio", "AnyAudioClass", ""), "AudioObjectFixed", name,
    eventinterval ? Channel::Events : Channel::Continuous,
    Channel::OneOrMoreEntries));
}

bool MotionControl::FixedSet::isValid()
{
  bool res = w_entity->isValid();
  if (!res) {
    W_MOD("token for object " << name << " not valid");
  }
  return res;
}

void MotionControl::FixedSet::step(const TimeSpec &ts)
{
  if (!eventinterval || --eventcounter == 0) {
    DataWriter<AudioObjectFixed> w(*w_entity, ts);
    w.data() = initial;
    eventcounter = eventinterval;
  }
}

void MotionControl::FixedSet::hold(const TimeSpec &ts)
{
  if (!eventinterval) {
    DataWriter<AudioObjectFixed> w(*w_entity, ts);
    w.data() = initial;
    eventcounter = eventinterval;
  }
}

// tell DUECA you are prepared
bool MotionControl::isPrepared()
{
  bool res = true;

  CHECK_TOKEN(w_motion);

  for (motion_sets_t::iterator ii = motion_sets.begin();
       ii != motion_sets.end(); ii++) {
    res = res && (*ii)->isValid();
  }
  for (fixed_sets_t::iterator ii = fixed_sets.begin(); ii != fixed_sets.end();
       ii++) {
    res = res && (*ii)->isValid();
  }

  // return result of checks
  return res;
}

// start the module
void MotionControl::startModule(const TimeSpec &time)
{
  do_calc.switchOn(time);
}

// stop the module
void MotionControl::stopModule(const TimeSpec &time)
{
  do_calc.switchOff(time);
}

// this routine contains the main simulation process of your module. You
// should read the input channels here, and calculate and write the
// appropriate output
void MotionControl::doCalculation(const TimeSpec &ts)
{
  switch (getAndCheckState(ts)) {
  case SimulationState::HoldCurrent: {
    for (motion_sets_t::iterator ii = motion_sets.begin();
         ii != motion_sets.end(); ii++) {
      (*ii)->hold(ts);
    }
    for (fixed_sets_t::iterator ii = fixed_sets.begin(); ii != fixed_sets.end();
         ii++) {
      (*ii)->hold(ts);
    }
  } break;
  case SimulationState::Advance: {
    for (motion_sets_t::iterator ii = motion_sets.begin();
         ii != motion_sets.end(); ii++) {
      (*ii)->step(ts);
    }
    for (fixed_sets_t::iterator ii = fixed_sets.begin(); ii != fixed_sets.end();
         ii++) {
      (*ii)->step(ts);
    }
  } break;
  default:
    break;
  }

  DataWriter<BaseObjectMotion> w(w_motion, ts);
  w.data() = object;
}

#if GTK_MAJOR_VERSION >= 3
#define gtk_spin_button_get_value_as_float gtk_spin_button_get_value
#endif

void MotionControl::changePositionOrientation(GtkSpinButton *widget,
                                              gpointer udata)
{
  union {
    gpointer udata;
    long idx;
  } conv = { udata };
  current[conv.idx] = gtk_spin_button_get_value_as_float(widget);

  if (w_motion.isValid()) {
    object.setquat(current[3] * M_PI / 180.0, current[4] * M_PI / 180.0,
                   current[5] * M_PI / 180.0);
    object.xyz[0] = current[0];
    object.xyz[1] = current[1];
    object.xyz[2] = current[2];
  }
}

// Make a TypeCreator object for this module, the TypeCreator
// will check in with the script code, and enable the
// creation of modules of this type
static TypeCreator<MotionControl> a(MotionControl::getMyParameterTable());
