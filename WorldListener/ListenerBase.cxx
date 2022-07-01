/* ------------------------------------------------------------------   */
/*      item            : ListenerBase.cxx
        made by         : repa
	from template   : DuecaModuleTemplate.cxx
        template made by: Rene van Paassen
        date            : Sat Nov 11 23:12:08 2017
	category        : body file 
        description     : 
	changes         : Sat Nov 11 23:12:08 2017 first version
	template changes: 030401 RvP Added template creation comment
        language        : C++
        copyright       : (c) 2016 TUDelft-AE-C&S
*/


#define ListenerBase_cxx
// include the definition of the helper class
#include "ListenerBase.hxx"
#include "WorldObjectBase.hxx"
OPEN_NS_WORLDLISTENER;

// constructor
ListenerBase::ListenerBase()
{
  //
}

// destructor
ListenerBase::~ListenerBase()
{
  //
}

void ListenerBase::setBase(const BaseObjectMotion& b)
{
  listener = b;
}

void ListenerBase::addControllable(const GlobalId& master_id,
                                   const NameSet& cname,
                                   entryid_type entry_id,
                                   uint32_t creation_id,
                                   const std::string data_class,
                                   const std::string entry_label,
                                   Channel::EntryTimeAspect time_aspect)
{
  // the actual object is created by the descendant
  bool res =
    this->createControllable(master_id, cname, entry_id, creation_id,
			     data_class, entry_label, time_aspect);
  
  if (!res) {
    uncreatables.insert(data_class);
  }
}

CLOSE_NS_WORLDLISTENER;
