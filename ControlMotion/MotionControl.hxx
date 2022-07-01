/* ------------------------------------------------------------------   */
/*      item            : MotionControl.hxx
        made by         : repa
	from template   : DuecaModuleTemplate.hxx
        template made by: Rene van Paassen
        date            : Fri Dec 15 16:17:58 2017
	category        : header file 
        description     : 
	changes         : Fri Dec 15 16:17:58 2017 first version
	template changes: 030401 RvP Added template creation comment
        language        : C++
        copyright       : (c) 2016 TUDelft-AE-C&S
*/

#ifndef MotionControl_hxx
#define MotionControl_hxx

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

// include the dusime header
#include <dusime.h>
USING_DUECA_NS;

// This includes headers for the objects that are sent over the channels
#include "comm-objects.h"

// include headers for functions/classes you need in the module
#include <GtkGladeWindow.hxx>


/** A test module for the audio class. It generates an
    "own" aircraft controllable by a GTK interface, as well as a
    variable number of other devices driven by their initial speed. 

    The instructions to create an module of this class from the Scheme
    script are:

    \verbinclude motion-control.scm
 */
class MotionControl: public SimulationModule
{
  /** self-define the module type, to ease writing the parameter table */
  typedef MotionControl _ThisModule_;

private: // simulation data
  /** Position and orientation */
  double                            current[6];
  
  /** Object sent over channels */
  BaseObjectMotion                  object;

  /** Window with controls */
  GtkGladeWindow                    viewcontrol;

  /** Combination of an initial and current motion description */
  struct MotionSet
  {
    /** name */
    std::string                            name;

    /** eventtype */
    int                                    eventinterval;

    /** event counter */
    int                                    eventcounter;
    
    /** Initial motion object */
    AudioObjectMotion                      initial;
    
    /** Current motion object, gets updated during simulation */
    AudioObjectMotion                      moving;
    
    /** channel token pointer for writing the results */
    boost::scoped_ptr<ChannelWriteToken>   w_entity;

    /** check validity of the token */
    bool isValid();
    
    /** make a simulation step & write data if needed */
    void step(const TimeSpec &ts);

    /** repeat known output */
    void hold(const TimeSpec& ts);
    
    /** Constructor, with initial motion object and token pointer */
    MotionSet(const AudioObjectMotion& i, const GlobalId &master_id,
              const std::string name, int eventinterval);

    /** Empty constructor, placeholder */
    MotionSet() { };
  };

  struct FixedSet
  {
    /** name */
    std::string                            name;

    /** eventtype */
    int                                    eventinterval;

    /** event counter */
    int                                    eventcounter;
    
    /** Initial motion object */
    AudioObjectFixed                       initial;
    
    /** channel token pointer for writing the results */
    boost::scoped_ptr<ChannelWriteToken>   w_entity;

    /** check validity of the token */
    bool isValid();
    
    /** make a simulation step & write data if needed */
    void step(const TimeSpec &ts);
    
    /** repeat known output */
    void hold(const TimeSpec& ts);
    
    /** Constructor, with initial motion object and token pointer */
    FixedSet(const AudioObjectFixed& i, const GlobalId &master_id,
              const std::string name, int eventinterval);

    /** Empty constructor, placeholder */
    FixedSet() { };
  };
 
  
  /** next sound */
  std::string next_motion_name;

  /** next sound */
  std::string next_fixed_name;
  
  /** Next set event driven */
  int next_eventinterval;

  /** Time step next motionset */
  double next_dt;

  /** Coordinates next motionset */
  std::vector<double>   next_coordinates;
  
  /** List of motion sets. */
  typedef std::list<boost::shared_ptr<MotionSet> >  motion_sets_t;

  /** List of motion sets. */
  motion_sets_t  motion_sets;

  /** List of motion sets. */
  typedef std::list<boost::shared_ptr<FixedSet> >  fixed_sets_t;

  /** List of motion sets. */
  fixed_sets_t  fixed_sets;

private: // channel access

  /** write token for own motion */
  ChannelWriteToken                 w_motion;  
  
private: // activity allocation
  /** You might also need a clock. Don't mis-use this, because it is
      generally better to trigger on the incoming channels */
  PeriodicAlarm        myclock;

  /** Callback object for simulation calculation. */
  Callback<MotionControl>  cb1;

  /** Activity for simulation calculation. */
  ActivityCallback      do_calc;
  
public: // class name and trim/parameter tables
  /** Name of the module. */
  static const char* const           classname;

  /** Return the parameter table. */
  static const ParameterTable*       getMyParameterTable();
  
public: // construction and further specification
  /** Constructor. Is normally called from scheme/the creation script. */
  MotionControl(Entity* e, const char* part, const PrioritySpec& ts);

  /** Continued construction. This is called after all script
      parameters have been read and filled in, according to the
      parameter table. Your running environment, e.g. for OpenGL
      drawing, is also prepared. Any lengty initialisations (like
      reading the 4 GB of wind tables) should be done here.
      Return false if something in the parameters is wrong (by
      the way, it would help if you printed what!) May be deleted. */
  bool complete();

  /** Destructor. */
  ~MotionControl();

  // add here the member functions you want to be called with further 
  // parameters. These are then also added in the parameter table
  // The most common one (addition of time spec) is given here. 
  // Delete if not needed!

  /** Specify a time specification for the simulation activity. */
  bool setTimeSpec(const TimeSpec& ts);

  /** Request check on the timing. */
  bool checkTiming(const vector<int>& i);

  /** Add a driver of moving sound */
  bool addMovingSound(const std::string& name);
  
  /** Add a driver of static sound */
  bool addStaticSound(const std::string& name);

  /** Complete currently specified sound */
  bool completeSound();
  
public: // member functions for cooperation with DUECA
  /** indicate that everything is ready. */
  bool isPrepared();

  /** start responsiveness to input data. */
  void startModule(const TimeSpec &time);
  
  /** stop responsiveness to input data. */
  void stopModule(const TimeSpec &time);

  /** position change */
  void changePositionOrientation(GtkSpinButton *widget, gpointer udata);

public: // the member functions that are called for activities
  /** the method that implements the main calculation. */
  void doCalculation(const TimeSpec& ts);
};

#endif
