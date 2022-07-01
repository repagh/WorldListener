/* ------------------------------------------------------------------   */
/*      item            : ListenerBase.hxx
        made by         : repa
	from template   : DuecaHelperTemplate.hxx
        template made by: Rene van Paassen
        date            : Sat Nov 11 23:12:08 2017
	category        : header file 
        description     : 
	changes         : Sat Nov 11 23:12:08 2017 first version
	template changes: 050825 RvP Added template creation comment
        language        : C++
        copyright       : (c) 2016 TUDelft-AE-C&S
*/

#ifndef ListenerBase_hxx
#define ListenerBase_hxx

#include <string>
#include <map>
#include <set>
#include "comm-objects.h"
#include "worldlistenerns.h"
#include <dueca.h>
#include "../../WorldView/SpecificationBase/SpecificationBase.hxx"

OPEN_NS_WORLDLISTENER;

class WorldObjectBase;

/** Set for keeping track of object classes I cannot make */
typedef std::set<std::string> NotCreatable;

/** This is a base class for sound producing classes based on audio
    scene generation toolkits. Currenly I know only OpenAL for this. 

    This base class defines how the WorldListener module will interact
    with the audio generation toolkit
 */
class ListenerBase: public SpecificationBase
{
protected:
  /** Have tried to, but could not create these */
  NotCreatable     uncreatables;

  /** Ears position, orientation and speed */
  BaseObjectMotion listener;

public: // construction and further specification
  /** Constructor. Is normally called from scheme/the creation script. */
  ListenerBase();

  /** Initialise the object */
  virtual bool init() = 0;

  /** Destructor. */
  virtual ~ListenerBase();

public: /* per-cycle interaction */
  
  /** set the base listener position 
      \param b    Object with position, orientatin, speed */
  virtual void setBase(const BaseObjectMotion& b);
  
public: /* management of objects in the world */

  /** Insert another controlled object */
  void addControllable
  (const GlobalId& master_id, const NameSet& cname, entryid_type entry_id,
   uint32_t creation_id, const std::string data_class,
   const std::string entry_label, Channel::EntryTimeAspect time_aspect);

  /** Create a controllable object. Object creation depends on class of 
      data supplied, further init may rely on fist data entering. */
  virtual bool createControllable
  (const GlobalId& master_id, const NameSet& cname, entryid_type entry_id,
   uint32_t creation_id, const std::string& data_class,
   const std::string& entry_label, Channel::EntryTimeAspect time_aspect) = 0;
  
  /** Remove a controllable */
  virtual void removeControllable(uint32_t creation_id) = 0;
  
  /** read all controllables. */
  virtual void iterate(const TimeSpec& ts) = 0;

  /** stop playing. */
  virtual void silence(const TimeSpec& ts) = 0;
};

CLOSE_NS_WORLDLISTENER;

#endif
