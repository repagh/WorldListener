/* ------------------------------------------------------------------   */
/*      item            : WorldObjectBase.cxx
        made by         : Rene' van Paassen
        date            : 171119
	category        : body file 
        description     : 
	changes         : 171119 first version
        language        : C++
*/

#define WorldObjectBase_cxx
#include "WorldObjectBase.hxx"

OPEN_NS_WORLDLISTENER;

WorldObjectBase::WorldObjectBase() :
  intrusive_refcount(0)
{

}


WorldObjectBase::~WorldObjectBase()
{

}

void WorldObjectBase::connect(const GlobalId& master_id, const NameSet& cname,
                              entryid_type entry_id,
                              Channel::EntryTimeAspect time_aspect)
{
  // no action
}

void intrusive_ptr_add_ref(WorldObjectBase* t)
{
  t->intrusive_refcount++;
}

void intrusive_ptr_release(WorldObjectBase* t)
{
  if (--(t->intrusive_refcount) == 0) {
    delete t;
  }
}

CLOSE_NS_WORLDLISTENER;
