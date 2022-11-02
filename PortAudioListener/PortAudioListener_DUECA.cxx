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


// include the definition of the helper class
#include "PortAudioListener_DUECA.hxx"
#include "PortAudioObjectFixed.hxx"
#include <limits>


// include additional files needed for your calculation here

#define DO_INSTANTIATE
#include <VarProbe.hxx>
#include <MemberCall.hxx>
#include <MemberCall2Way.hxx>
#include <CoreCreator.hxx>

// include the debug writing header. Warning and error messages
// are on by default, debug and info can be selected by
// uncommenting the respective defines
//#define D_MOD
//#define I_MOD
#include <debug.h>


USING_DUECA_NS;

OPEN_NS_WORLDLISTENER;

// Parameters to be inserted
const ParameterTable* PortAudioListener_DUECA::getParameterTable()
{
  static const ParameterTable parameter_table[] = {

    { "set-devicename",
      new VarProbe<_ThisObject_,std::string>
      (&_ThisObject_::devicename),
      "PortAudio device name" },

    { "set-samplerate",
      new VarProbe<_ThisObject_,unsigned>
      (&_ThisObject_::samplerate),
      "Sample rate for playback" },

    { "suggested-latency",
      new VarProbe<_ThisObject_,double>
      (&_ThisObject_::suggested_output_latency),
      "Output latency setting for the stream, affects buffer size" },

    { "add-static-sound",
      new MemberCall<_ThisObject_,std::vector<std::string> >
      (&_ThisObject_::addStaticSound),
      "Add a sound fixed in the world, specify name and filename. After\n"
      "adding, you can set the sound properties (position, etc.)." },

    { "add-controlled-static-sound",
      new MemberCall<_ThisObject_,std::vector<std::string> >
      (&_ThisObject_::addControlledStaticSound),
      "Add a sound of which volume and pitch are controlled. Specify\n"
      "name and filename. The label of an entry in the controlling channel\n"
      "should match the name"
    },

    { "add-object-class-data",
      new MemberCall<_ThisObject_,std::vector<string> >
      (&_ThisObject_::addObjectClassData),
      "Create a new class (type) of simple objects, i.e. those that can be\n"
      "represented by one or more sound files. Specify the following\n"
      "- A key to link this type of sound to a channel entry. This may take\n"
      "  one of the following forms:\n"
      "  * <DCO classname>:<label>. Match the class of the DCO object, with"
      "    the label. If no match is found, parent class names are tried.\n"
      "  * <DCO classname>. Only match on the class of the DCO object.\n"
      "- Object name, if not empty, overrides the name given in the label\n"
      "- Sound object class name, must match a type defined in the factory\n"
      "- Sound file name[s]\n" },

    { "add-object-class-coordinates",
      new MemberCall<_ThisObject_,std::vector<double> >
      (&_ThisObject_::addCoordinates),
      "optionally add coordinates to the latest created objectclass with\n"
      "\"add-object-class-data\". For default sound classes, you can define\n"
      "the following groups.\n"
      "* channel: c (channel number)\n"
      "* volume: between 0.0 and 1.0"
    },

    { "set-coordinates",
      new MemberCall<_ThisObject_,std::vector<double> >
      (&_ThisObject_::setCoordinates),
      "set channel c" },

    { "set-looping",
      new MemberCall<_ThisObject_,bool>
      (&_ThisObject_::setLooping),
      "Set a sound to be looping (#t) or not (#f). Note that for controlled\n"
      "sounds driven by a continuous (stream) channel entry, looping is\n"
      "forced to on." },

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
      "This helper class provides a sound interface to PortAudio\n"
      "Instead of using spatial audio, like the OpenAL interface, this\n"
      "interface uses assignment to (single) speakers. Only mono files\n"
      "can be played, speaker selection and volume can be made."
    }
  };

  return parameter_table;
}

// constructor
PortAudioListener_DUECA::PortAudioListener_DUECA() :
  ScriptCreatable(),
  PortAudioListener(),
  nexttype(None),
  spec()
{
  //
}

bool PortAudioListener_DUECA::complete()
{
  /* All your parameters have been set. You may do extended
     initialisation here. Return false if something is wrong. */
  return completeSound();
}

// destructor
PortAudioListener_DUECA::~PortAudioListener_DUECA()
{
  //
}

bool PortAudioListener_DUECA::completeSound()
{
  switch (nexttype) {
  case None:
    return true;
  case StaticSound: {
    boost::intrusive_ptr<PortAudioObject> nob(new PortAudioObject(spec));
    this->addConstantSource(nob);
  }
    return true;
  case ControlledStaticSound: {
    boost::intrusive_ptr<PortAudioObject> nob(new PortAudioObjectFixed(spec));
    this->addControlledSource(nob);
  }
    return true;
  }
  return false;
}

bool PortAudioListener_DUECA::addStaticSound(const std::vector<std::string>&
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
  spec.coordinates.resize(2, 0.0);
  spec.coordinates[1] = 0.999999f;
  return true;
}

bool PortAudioListener_DUECA::addControlledStaticSound
(const std::vector<std::string>& names)
{
  if (!addStaticSound(names)) return false;
  spec.type = "static controlled";
  nexttype = ControlledStaticSound;
  return true;
}

bool PortAudioListener_DUECA::setCoordinates(const std::vector<double>& coords)
{
  if (coords.size() < 2) {
    E_MOD("Need at least 2 coordinate values (channel number, volume)");
    return false;
  }
  spec.coordinates.resize(coords.size());
  for (unsigned ii = coords.size(); ii--; ) {
    spec.coordinates[ii] = coords[ii];
  }
  return true;
}

bool PortAudioListener_DUECA::addObjectClassData
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

bool PortAudioListener_DUECA::setLooping(const bool& l)
{
  spec.type += " looping";
  return true;
}

inline double limit(double dmin, double dval, double dmax)
{
  return (dval < dmin) ? dmin : ((dval > dmax) ? dmax : dval);
}

bool PortAudioListener_DUECA::setGain(const double& g)
{
  spec.coordinates[1] = limit(0.0, g, 1.0);
  return true;
}

CLOSE_NS_WORLDLISTENER;

// script access macro
SCM_FEATURES_NOIMPINH(worldlistener::PortAudioListener_DUECA,
                      ScriptCreatable, "portaudio-listener");

// Make a CoreCreator object for this module, the CoreCreator
// will check in with the scheme-interpreting code, and enable the
// creation of objects of this type
static CoreCreator<worldlistener::PortAudioListener_DUECA>
a(worldlistener::PortAudioListener_DUECA::getParameterTable(),
  "PortAudioListener");
