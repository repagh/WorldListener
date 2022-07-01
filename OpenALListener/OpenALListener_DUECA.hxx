/* ------------------------------------------------------------------   */
/*      item            : OpenALListener.hxx
        made by         : repa
	from template   : DuecaHelperTemplate.hxx
        template made by: Rene van Paassen
        date            : Sat Nov 11 22:13:10 2017
	category        : header file 
        description     : 
	changes         : Sat Nov 11 22:13:10 2017 first version
	template changes: 050825 RvP Added template creation comment
        language        : C++
        copyright       : (c) 2016 TUDelft-AE-C&S
*/

#ifndef OpenALListener_DUECA_hxx
#define OpenALListener_DUECA_hxx

// include the dueca header
#include "../WorldListener/worldlistenerns.h"
#include "OpenALListener.hxx"
#include <ScriptCreatable.hxx>
#include <stringoptions.h>
#include <ParameterTable.hxx>
#include <dueca_ns.h>

#include <AL/al.h>
#include <AL/alc.h>

OPEN_NS_WORLDLISTENER;

/** Create an OpenAL backend for sound generation
    
    This class has been derived from the ScriptCreatable base class,
    and has a (scheme) script command to create it and optionally add
    parameters.

    The instructions to create an object of this class from the Scheme
    script are:

    \verbinclude openal-listener.scm

    First create one of the sounds, then specify coordinates, etc.

    In this backend, specify which sounds are to be played. A static sound
    ('add-static-sound) will not change location and not need interaction 
    from the simulation. These background sounds will always be playing
    following the prescribed pitch and volume. 
    
    If you create a controlled static sound ('add-controlled-static-sound)
    the name has to match an "AudioObjectFixed" entry in the 
    AnyAudioClass://audio channel before it is activated. The entry 
    controls volume and pitch, the location is determined by the coordinates
    specified in the interface. 

    If you create a controlled moving sound ('add-controlled-moving-sound)
    the name has to match an "AudioObjectMotion" entry in the channel. 
    Coordinates only specify the initial position, the actual position will
    be updated with the information from the channel. 

    Looping is overridden by the entry type, continuous (stream)
    entries will produce looping sound, event entries will produce a
    single play of the sound.

    Specification of directionality, giving a sound cone, is
    optional. Only in that case the orientation of the sound source has a 
    significant effect.
 */
class OpenALListener_DUECA: public dueca::ScriptCreatable,
                            public OpenALListener
{
  /** self-define the module type, to ease writing the parameter table */
  typedef OpenALListener_DUECA _ThisObject_;

  /** Next sound object to create */
  enum SoundType {
    None,         /**< No type, cleared .. */
    StaticSound,  /**< A static sound */
    ControlledStaticSound, /**< static in the world, or attached to vehicle */
    ControlledMovingSound  /**< position controlled, or moves wrt vehicle */
  };

  /** Next type to create */
  SoundType nexttype;

  /** Parameters for creation */
  WorldDataSpec spec;
  
public: // construction and further specification
  /** Constructor. Is normally called from scheme/the creation script. */
  OpenALListener_DUECA();

  /** Continued construction. This is called after all script
      parameters have been read and filled in, according to the
      parameter table. */
  bool complete();

  /** Destructor. */
  ~OpenALListener_DUECA();

  /** Obtain a pointer to the parameter table. */
  static const ParameterTable* getParameterTable();

public:
  /** Default script linkage. */
  SCM_FEATURES_DEF;

private:
  /** Complete specified new sound */
  bool completeSound();
  
  /** Create new static sound */
  bool addStaticSound(const std::vector<std::string>& names);

  /** Create new controlled static sound */
  bool addControlledStaticSound(const std::vector<std::string>& names);

  /** Add the data for a new type */
  bool addObjectClassData(const std::vector<std::string>& names);

  /** Create new controlled moving sound */
  bool addControlledMovingSound(const std::vector<std::string>& names);

  /** Specify sound coordinates */
  bool setCoordinates(const std::vector<double>& coords);

  /** Specify distance model */
  bool setDistanceModel(const std::string& model);

  /** Specify distance parameters */
  bool setDistanceParams(const std::vector<double>& params);
  
  /** Specify sound looping or not */
  bool setLooping(const bool& l);

  /** Specify pitch */
  bool setPitch(const double& p);

  /** Specify gain */
  bool setGain(const double& g);

  /** Directional sound specification */
  bool setDirection(const std::vector<double> &dir);

  /** specify object to be relative to listener */
  bool setRelative(const bool &rel);
};

CLOSE_NS_WORLDLISTENER;

#endif
