/* ------------------------------------------------------------------   */
/*      item            : PortAudioObjectFactory.hxx
        made by         : Rene van Paassen
        date            : 171112
	category        : header file 
        description     : 
	changes         : 171112 first version
        language        : C++
	copyright       : (c) 2017 TUDelft-AE-C&S
*/

#ifndef PortAudioObjectFactory_hxx
#define PortAudioObjectFactory_hxx

#include "PortAudioObject.hxx"
#include <ConglomerateFactory.hxx>
#include <boost/shared_ptr.hpp>
#include <boost/thread/detail/singleton.hpp>
#include "comm-objects.h"

OPEN_NS_WORLDLISTENER;

/** \file PortAudioObjectFactory.hxx

    This file defines the specialization of an ObjectFactory singleton
    for sound objects to be shown with the PortAudioViewer */

/** Index for base types in the factory */
struct PortAudioObjectTypeKey
{
  /** map key, text defines type of object */
  typedef std::string Key;

  /** Returned object is simply a pointer */
  typedef PortAudioObject* ProductBase;

  /** This is a generic strings + doubles object, flexible enough to
      define most stuff */
  typedef WorldDataSpec SpecBase;
};

/** Define the PortAudioObjectFactory as an instantiation of the default
    ConglomerateFactory, combined with the singleton template from
    boost.
    
    The PortAudioObjectFactory can create objects for audialisation with
    PortAudio. 
*/
typedef boost::detail::thread::singleton
<ConglomerateFactory
 <PortAudioObjectTypeKey, 
  boost::shared_ptr<SubcontractorBase<PortAudioObjectTypeKey> > > > 
PortAudioObjectFactory;

/** Template for a simple (single case) subcontractor.

    If you want to make your object available for the PortAudioViewer
    object factory, create a static SubContractor, as follows:

    \code
    class MySoundObject: public PortAudioObject
    {
      // need a constructor that uses a WorldDataSpec object
      MySoundObject(const WorldDataSpec& spec);

      // for the rest, see what you need to redefine from PortAudioObject!
    };

    // and in your cxx file:
    static SubContractor<PortAudioObjectTypeKey, MySoundObject>
    *MySoundObject_maker = new 
    SubContractor<PortAudioObjectTypeKey, MySoundObject>("my-sound-object");
    \endcode

    With this, the PortAudioListener can figure out how to create objects of
    type "my-sound-object". 
*/
template<typename Xbase, typename Derived>
class SubContractor: public SubcontractorBase<Xbase>
{
  /** Basically the class name of the type of objects created here. */
  typename Xbase::Key key;
public:
  /** Constructor */
  SubContractor(const char* key) :
    key(key)
  {
    PortAudioObjectFactory::instance().addSubcontractor
      (this->key, boost::shared_ptr<SubcontractorBase<Xbase > >(this));
  }
  
  /** create a new object */
  typename Xbase::ProductBase create(const typename Xbase::Key& key, 
				     const typename Xbase::SpecBase& spec)
  {
    return new Derived(spec);
  }
};

/** The base subcontractor pointer in PortAudio */
typedef boost::shared_ptr<SubcontractorBase<PortAudioObjectTypeKey> > SubconPtr;

CLOSE_NS_WORLDLISTENER;

#endif
