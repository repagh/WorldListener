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

#ifndef OpenALListener_hxx
#define OpenALListener_hxx

// include the dueca header
#include <dueca.h>

#include "comm-objects.h"
#include "OpenALObject.hxx"
#include "../WorldListener/worldlistenerns.h"
#include "../WorldListener/ListenerBase.hxx"
#include <boost/scoped_ptr.hpp>
#include <AL/al.h>
#include <AL/alc.h>
#include <string>
#include <list>
#include <map>

OPEN_NS_WORLDLISTENER;
class OpenALBufferManager;

/** This is a wrapper that can load and display soundscapes in OpenAL.

    This class is derived from the WorldListenerBase class, which
    determines the base listening interaction with the WorldListener class.

    A derived class OpenALListener_DUECA, defines the interaction possible
    from the dueca.mod script. 

    \verbinclude open-al-listener.scm
 */
class OpenALListener: public ListenerBase
{
private: // simulation data
  /** Audio device */
  ALCdevice *device;

  /** Audio context */
  ALCcontext *context;

protected:
  /** String defining the default device */
  std::string devicename;

  /** Listener gain */
  float listener_gain;

  /** Distance model */
  ALenum distance_model;

  /** Accept unknown/unconfigured objects */
  bool allow_unknown;
  
private:
  /** position of observer */
  ALfloat xyz[3];

  /** speed of observer */
  ALfloat uvw[3];

  /** orientation of observer, look & up vectors */
  ALfloat ori[6];

  /** Buffer manager */
  boost::scoped_ptr<OpenALBufferManager> buffermanager;
  
protected:
  /** type for map with named sound objects; these are created through the
      interface, and may later be attached to channel entries, 
      (class, label) as key, and boost ptr as value */
  typedef std::map<std::pair<const std::string,const std::string>,
		   boost::intrusive_ptr<OpenALObject> > named_objects_t;

  /** Map with named but not yet connected/controlled objects. */
  named_objects_t named_objects;

  /** Type for list with anonymous and connected controllables */
  typedef std::map<uint32_t,
                   boost::intrusive_ptr<OpenALObject> > controllables_t;

  /** List with connected controlled objects */
  controllables_t controlled_sources;

  /** static source type */
  typedef std::list<boost::intrusive_ptr<OpenALObject> > statics_t;

  statics_t other_sources;

public: // construction and further specification
  /** Constructor. Is normally called from scheme/the creation script. */
  OpenALListener();

  /** Continued construction. This is called after all script
      parameters have been read and filled in, according to the
      parameter table. */
  virtual bool init();

  /** Destructor. */
  ~OpenALListener();

  /** Create a (normally simple) object based on the class name. This
      is used to implement automatic creation of objects that are
      specified in the klass member of the ObjectMotion channel. 
      \param master_id Dueca ID to allow creation of read token
      \param cname     Name of the DUECA channel with data for this object
      \param entry_id  Entry id in the channel
      \param data_class Datatype, determining class of the new object,
      \param entry_label Additional data
      \returns      Pointer to the newly created object. */
  bool createControllable
  (const GlobalId& master_id, const NameSet& cname, entryid_type entry_id,
   uint32_t creation_id,
   const std::string& data_class, const std::string& entry_label,
   Channel::EntryTimeAspect time_aspect);

  /** Remove a controllable */
  void removeControllable(uint32_t creation_id);
  
  /** read/process all sources. */
  void iterate(const TimeSpec& ts);

  /** reset/process all sources. */
  void silence(const TimeSpec& ts);

  /** Add a constant sound source */
  void addConstantSource(boost::intrusive_ptr<OpenALObject> op);
  
  /** Add a constant sound source */
  void addControlledSource(boost::intrusive_ptr<OpenALObject> op);
  
  /** Update the position of the listener. */
  void setBase(const BaseObjectMotion& listener);

  /** Access the buffer manager */
  inline OpenALBufferManager& getBufferManager() {return *buffermanager;}
};

CLOSE_NS_WORLDLISTENER;

#endif
