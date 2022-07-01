/* ------------------------------------------------------------------   */
/*      item            : WorldObjectBase.hxx
        made by         : Rene van Paassen
        date            : 171119
	category        : header file 
        description     : 
	changes         : 171119 first version
        language        : C++
*/

#ifndef WorldObjectBase_hxx
#define WorldObjectBase_hxx

#include "worldlistenerns.h"
#include <dueca.h>
#include "comm-objects.h"

OPEN_NS_WORLDLISTENER;

// advance declaration for ref pointers. 
class WorldObjectBase;
void intrusive_ptr_add_ref(WorldObjectBase* t);
void intrusive_ptr_release(WorldObjectBase* t);

/** Base object for handling by viewers/listeners */
class WorldObjectBase
{
  
  /** ref counter */
  unsigned intrusive_refcount;

  friend void intrusive_ptr_add_ref(WorldObjectBase*);
  friend void intrusive_ptr_release(WorldObjectBase*);

public:
  /** Constructor */
  WorldObjectBase();

  /** Destructor */
  virtual ~WorldObjectBase();

  /** Read new input or otherwise
      @param ts    Time for this update
      @param base  Position and orientation of the base listener */
  virtual void iterate(const TimeSpec& ts, const BaseObjectMotion& base) = 0;

  /** Connect to a channel entry 
      @param master_id ID for opening a channel reader
      @param cname     Channel with object data
      @param entry_id  Entry in the channel */
  virtual void connect(const GlobalId& master_id, const NameSet& cname,
		       entryid_type entry_id,
                       Channel::EntryTimeAspect time_aspect);
};

CLOSE_NS_WORLDLISTENER;

#endif
