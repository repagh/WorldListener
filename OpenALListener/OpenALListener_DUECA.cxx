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


// include the definition of the helper class
#include "OpenALListener_DUECA.hxx"
#include "OpenALObjectFixed.hxx"
#include "OpenALObjectMoving.hxx"
#include <limits>

// include the debug writing header. Warning and error messages
// are on by default, debug and info can be selected by
// uncommenting the respective defines
//#define D_MOD
//#define I_MOD
#include <debug.h>

// include additional files needed for your calculation here

#define DO_INSTANTIATE
#include <VarProbe.hxx>
#include <MemberCall.hxx>
#include <MemberCall2Way.hxx>
#include <CoreCreator.hxx>
USING_DUECA_NS;

OPEN_NS_WORLDLISTENER;

// Parameters to be inserted
const ParameterTable* OpenALListener_DUECA::getParameterTable()
{
  static const ParameterTable parameter_table[] = {

    { "set-listener-gain",
      new VarProbe<_ThisObject_,float>(&_ThisObject_::listener_gain),
      "set listener gain (0-1). Note: 0.0=no sound, 1.0=no attenuation" },

    { "set-distance-model",
      new MemberCall<_ThisObject_,std::string>
      (&_ThisObject_::setDistanceModel),
      "select the distance model for the listener (\"none\", \"linear\",\n"
      " \"inverse\", \"exponential\"), the latter three can be also clamped,\n"
      "specified like \"inverse clamped\", etc." },

    { "set-devicename",
      new VarProbe<_ThisObject_,std::string>
      (&_ThisObject_::devicename),
      "OpenAL device name" },

    { "add-static-sound",
      new MemberCall<_ThisObject_,std::vector<std::string> >
      (&_ThisObject_::addStaticSound),
      "Add a sound fixed in the world, specify name and filename. After\n"
      "adding, you can set the sound properties (position, etc.)." },

    { "add-controlled-static-sound",
      new MemberCall<_ThisObject_,std::vector<std::string> >
      (&_ThisObject_::addControlledStaticSound),
      "Add a sound of which volume and pitch are controlled. Specify\n"
      "name and filename" },

    { "add-controlled-moving-sound",
      new MemberCall<_ThisObject_,std::vector<std::string> >
      (&_ThisObject_::addControlledMovingSound),
      "add a sound of which volume, pitch and movement are controlled.\n"
      "Specify a name for the sound and filename" },

    { "add-object-class-data",
      new MemberCall<_ThisObject_,std::vector<string> >
      (&_ThisObject_::addObjectClassData),
      "Create a new class (type) of simple objects, i.e. those that can be\n"
      "represented by one or more sound files. Specify the following\n"
      "- <DCO classname>[:objectname]. This is used to match to either\n"
      "  DCO classname (all objects look like this) or DCO classname+label\n"
      "- Object name, if not empty, overrides the name given in the label\n"
      "- Sound object class name, must be defined in the factory\n"
      "- Sound file name[s]\n" },

    { "add-object-class-coordinates",
      new MemberCall<_ThisObject_,std::vector<double> >
      (&_ThisObject_::addCoordinates),
      "optionally add coordinates to the latest created objectclass with\n"
      "\"add-object-class-data\". For default sound classes, you can define\n"
      "the following groups. Note that these are optional.\n"
      "Defaults given in parentheses:\n"
      "* base coordinates: x, y, z (0,0,0)\n"
      "* velocity:         u, v, w (0,0,0)\n"
      "* sound properties: volume (0, range 0-1), relative pitch (1.0)\n"
      "* distance effects: ref dist (1), max dist (flt_max), rollof factor(1)\n"
      "* cone definition: phi, theta, psi direction, \n"
      "                   cone inner, cone outer, cone gain\n"
    },

    { "set-coordinates",
      new MemberCall<_ThisObject_,std::vector<double> >
      (&_ThisObject_::setCoordinates),
      "set position (x,y,z), speed (u,v,w)" },

    { "set-direction",
      new MemberCall<_ThisObject_,std::vector<double> >
      (&_ThisObject_::setDirection),
      "set sound orientation (phi, tht, psi), inner+outer cone angles [deg],\n"
      "and cone outer gain" },

    { "set-distance-params",
      new MemberCall<_ThisObject_,std::vector<double> >
      (&_ThisObject_::setDistanceParams),
      "set distance parameters; reference distance (100 % gain), max (clamp)\n"
      "distance, and rolloff factor for exponential model (1..3 floats)" },

    { "set-looping",
      new MemberCall<_ThisObject_,bool>
      (&_ThisObject_::setLooping),
      "Set a sound to be looping (#t) or not (#f). Note that for controlled\n"
      "sounds driven by a continuous (stream) channel entry, looping is\n"
      "forced to on." },

    { "set-relative",
      new MemberCall<_ThisObject_,bool>(&_ThisObject_::setRelative),
      "specify that the created sound object position & speed are relative\n"
      "to the observer, e.g., carried in the same vehicle" },

    { "set-pitch",
      new MemberCall<_ThisObject_,double>
      (&_ThisObject_::setPitch),
      "set pitch multiplier" },

    { "set-gain",
      new MemberCall<_ThisObject_,double>
      (&_ThisObject_::setGain),
      "set sound gain, range of 0.0 to 1.0" },

    { "allow-unknown",
      new VarProbe<_ThisObject_,bool>(&_ThisObject_::allow_unknown),
      "ignore unknown or unconnected objects in world information channels" },


    /* You can extend this table with labels and MemberCall or
       VarProbe pointers to perform calls or insert values into your
       class objects. Please also add a description (c-style string). */

    /* The table is closed off with NULL pointers for the variable
       name and MemberCall/VarProbe object. The description is used to
       give an overall description of the module. */
    { NULL, NULL,
      "This helper class provides a sound interface to OpenAL"} };

  return parameter_table;
}

// constructor
OpenALListener_DUECA::OpenALListener_DUECA() :
  ScriptCreatable(),
  OpenALListener(),
  nexttype(None),
  spec()
{
  //
}

bool OpenALListener_DUECA::complete()
{
  /* All your parameters have been set. You may do extended
     initialisation here. Return false if something is wrong. */
  return completeSound();
}

// destructor
OpenALListener_DUECA::~OpenALListener_DUECA()
{
  //
}

bool OpenALListener_DUECA::completeSound()
{
  switch (nexttype) {
  case None:
    return true;
  case StaticSound: {
    boost::intrusive_ptr<OpenALObject> nob(new OpenALObject(spec));
    this->addConstantSource(nob);
  }
    return true;
  case ControlledStaticSound: {
    boost::intrusive_ptr<OpenALObject> nob(new OpenALObjectFixed(spec));
    this->addControlledSource(nob);
  }
    return true;
  case ControlledMovingSound: {
    boost::intrusive_ptr<OpenALObject> nob(new OpenALObjectMoving(spec));
    this->addControlledSource(nob);
  }
    return true;
  }
  return false;
}

bool OpenALListener_DUECA::addStaticSound(const std::vector<std::string>&
                                          names)
{
  if (!completeSound()) return false;
  nexttype = StaticSound;
  if (names.size() != 2) {
    E_MOD("Need a name and filename for static sound");
    return false;
  }
  spec.name = names[0];
  spec.type = "static";
  spec.filename.resize(0);
  spec.filename.push_back(names[1]);
  spec.coordinates.resize(0);
  spec.coordinates.resize(8, 0.0);
  spec.coordinates[6] = 0.999999f;
  spec.coordinates[7] = 1.0f;
  return true;
}

bool OpenALListener_DUECA::addControlledStaticSound
(const std::vector<std::string>& names)
{
  if (!addStaticSound(names)) return false;
  spec.type = "static controlled";
  nexttype = ControlledStaticSound;
  return true;
}

bool OpenALListener_DUECA::addControlledMovingSound
(const std::vector<std::string>& names)
{
  if (!addStaticSound(names)) return false;
  spec.type = "moving controlled";
  nexttype = ControlledMovingSound;
  return true;
}

bool OpenALListener_DUECA::setRelative(const bool& rel)
{
  if (rel && spec.type.find("relative") == string::npos) {
    spec.type += " relative";
  }
  else if (!rel && spec.type.find("relative") != string::npos) {
    spec.type = spec.type.erase(spec.type.find("relative"), 8);
  }
  return true;
}

bool OpenALListener_DUECA::setCoordinates(const std::vector<double>& coords)
{
  if (coords.size() != 6) {
    E_MOD("Need 6 coordinate values");
    return false;
  }
  std::copy(coords.begin(), coords.end(), spec.coordinates.begin());
  return true;
}

bool OpenALListener_DUECA::addObjectClassData
(const std::vector<std::string>& names)
{
  if (names.size() < 3) {
    E_CNF("Specify a match string for creation, class name, factory type and"
          << " sound file");
    return false;
  }

  WorldDataSpec obj;
  obj.type = names[2];
  obj.name = names[1];
  for (size_t ii = 3; ii < names.size(); ii++) {
    obj.filename.push_back(names[ii]);
  }

  addFactorySpec(names[0], obj);
  return true;
}

bool OpenALListener_DUECA::setLooping(const bool& l)
{
  spec.type += " looping";
  return true;
}

inline double limit(double dmin, double dval, double dmax)
{
  return (dval < dmin) ? dmin : ((dval > dmax) ? dmax : dval);
}

bool OpenALListener_DUECA::setPitch(const double& p)
{
  spec.coordinates[7] = limit(0.001, p, 1000.0);
  return true;
}

bool OpenALListener_DUECA::setGain(const double& g)
{
  spec.coordinates[6] = limit(0.0, g, 1.0);
  return true;
}

inline double d2r(double x)
{
  return M_PI/180.0*x;
}

bool OpenALListener_DUECA::setDirection(const std::vector<double> &dir)
{
  if (dir.size() != 6) {
    W_MOD("need 6 parameters for direction");
    return false;
  }

  // inner and outer cone check
  if (dir[3] < 0.0 || dir[4] < 0.0 || dir[3] > dir[4] || dir[4] > 360.0) {
    W_MOD("cone angles improperly specified");
    return false;
  }

  if (spec.coordinates.size() == 8) {
    spec.coordinates.resize(17, 0.0);
    spec.coordinates[8] = 1.0;
    spec.coordinates[9] = std::numeric_limits<float>::max();
    spec.coordinates[10] = 1.0;
  }
  else if (spec.coordinates.size() == 11) {
    spec.coordinates.resize(17, 0.0);
  }

  // x, y and z component direction vector
  const int b = 11;
  spec.coordinates[b+0] = cos(d2r(dir[2]))*cos(d2r(dir[1]));
  spec.coordinates[b+1] = sin(d2r(dir[2]))*cos(d2r(dir[1]));
  spec.coordinates[b+2]= -sin(d2r(dir[1]));

  // cone angles and gain outer cone
  spec.coordinates[b+3] = dir[3];
  spec.coordinates[b+4] = dir[4];
  spec.coordinates[b+5] = dir[5];

  return true;
}

bool OpenALListener_DUECA::setDistanceParams(const std::vector<double> &dir)
{
  if (dir.size() < 1 || dir.size() > 3) {
    W_MOD("need 1 to 3 distance parameters");
    return false;
  }
  if (spec.coordinates.size() == 8) {
    spec.coordinates.resize(11, 0.0);
    spec.coordinates[9] = std::numeric_limits<float>::max();
    spec.coordinates[10] = 1.0;
  }
  const int b = 8;
  spec.coordinates[b+0] = dir[0];
  if (dir.size() > 1) spec.coordinates[b+1] = dir[1];
  if (dir.size() > 2) spec.coordinates[b+2] = dir[2];
  return true;
}

bool OpenALListener_DUECA::setDistanceModel(const std::string& model)
{
  if (model == "none") {
    distance_model = AL_NONE;
  }
  else if (model == "inverse") {
    distance_model = AL_INVERSE_DISTANCE;
  }
  else if (model == "inverse clamped") {
    distance_model = AL_INVERSE_DISTANCE_CLAMPED;
  }
  else if (model == "linear") {
    distance_model = AL_LINEAR_DISTANCE;
  }
  else if (model == "linear clamped") {
    distance_model = AL_LINEAR_DISTANCE_CLAMPED;
  }
  else if (model == "exponent") {
    distance_model = AL_EXPONENT_DISTANCE;
  }
  else if (model == "exponent clamped") {
    distance_model = AL_EXPONENT_DISTANCE_CLAMPED;
  }
  else {
    E_MOD("Invalid distance model parameter '" << model << "'");
    return false;
  }
  return true;
}

CLOSE_NS_WORLDLISTENER;

// script access macro
SCM_FEATURES_NOIMPINH(worldlistener::OpenALListener_DUECA,
                      ScriptCreatable, "openal-listener");

// Make a CoreCreator object for this module, the CoreCreator
// will check in with the scheme-interpreting code, and enable the
// creation of objects of this type
static CoreCreator<worldlistener::OpenALListener_DUECA>
a(worldlistener::OpenALListener_DUECA::getParameterTable(),
  "OpenALListener");
