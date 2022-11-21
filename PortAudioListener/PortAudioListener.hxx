/* ------------------------------------------------------------------   */
/*      item            : PortAudioListener.hxx
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

#ifndef PortAudioListener_hxx
#define PortAudioListener_hxx

// include the dueca header
#include <dueca.h>

#include "comm-objects.h"
#include "PortAudioObject.hxx"
#include "../WorldListener/worldlistenerns.h"
#include "../WorldListener/ListenerBase.hxx"
#include <boost/scoped_ptr.hpp>
#include <portaudiocpp/PortAudioCpp.hxx>
#include <string>
#include <list>
#include <map>

OPEN_NS_WORLDLISTENER;
class PortAudioBufferManager;

/** This is a wrapper that can load and display soundscapes in PortAudio.

    This class is derived from the WorldListenerBase class, which
    determines the base listening interaction with the WorldListener class.

    A derived class PortAudioListener_DUECA, defines the interaction possible
    from the dueca.mod script.

    \verbinclude open-al-listener.scm
 */
class PortAudioListener: public ListenerBase
{
private: // simulation data
  // source id
  PaStream *stream;

protected:
  /** String defining the default device */
  std::string devicename;

  /** Device enumerator */
  int idevice;

  /** Number of channels */
  int num_channels;

  /** Sample rate, same for all sources */
  unsigned samplerate;

  /** Output latency suggestion */
  double suggested_output_latency;

  /** Accept unknown/unconfigured objects */
  bool allow_unknown;

  /** Buffer manager */
  boost::scoped_ptr<PortAudioBufferManager> buffermanager;

  /** Status of sound */
  bool sound_off;

protected:
  /** type for map with named sound objects; these are created through the
      interface, and may later be attached to channel entries,
      (class, label) as key, and boost ptr as value */
  typedef std::map<std::pair<const std::string,const std::string>,
                   boost::intrusive_ptr<PortAudioObject> > named_objects_t;

  /** Map with named but not yet connected/controlled objects. */
  named_objects_t named_objects;

  /** Type for list with anonymous and connected controllables */
  typedef std::map<uint32_t,
                   boost::intrusive_ptr<PortAudioObject> > controllables_t;

  /** List with connected controlled objects */
  controllables_t controlled_sources;

  /** static source type */
  typedef std::list<boost::intrusive_ptr<PortAudioObject> > statics_t;

  statics_t other_sources;

public: // construction and further specification
  /** Constructor. Is normally called from scheme/the creation script. */
  PortAudioListener();

  /** Continued construction. This is called after all script
      parameters have been read and filled in, according to the
      parameter table. */
  virtual bool init();

  /** Destructor. */
  ~PortAudioListener();

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
  void addConstantSource(boost::intrusive_ptr<PortAudioObject> op);

  /** Add a controlled sound source */
  void addControlledSource(boost::intrusive_ptr<PortAudioObject> op);

  /** Update the position of the listener. */
  void setBase(const BaseObjectMotion& listener);

  /** Access the buffer manager */
  inline PortAudioBufferManager& getBufferManager() {return *buffermanager;}

  /** Callback Function */
  static int cfun(const void *input, void *output,
		  unsigned long frameCount,
		  const PaStreamCallbackTimeInfo* timeinfo,
		  PaStreamCallbackFlags statusFlags, void *userdata);

  /** Number of channels on the device */
  inline unsigned getNumChannels() const { return num_channels; }

};

CLOSE_NS_WORLDLISTENER;

#endif
