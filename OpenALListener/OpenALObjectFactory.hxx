/* ------------------------------------------------------------------   */
/*      item            : OpenALObjectFactory.hxx
        made by         : Rene van Paassen
        date            : 171112
	category        : header file 
        description     : 
	changes         : 171112 first version
        language        : C++
	copyright       : (c) 2017 TUDelft-AE-C&S
*/

#ifndef OpenALObjectFactory_hxx
#define OpenALObjectFactory_hxx

#include "OpenALObject.hxx"
#include <ConglomerateFactory.hxx>
#include <boost/shared_ptr.hpp>
#include <boost/thread/detail/singleton.hpp>
#include "comm-objects.h"

OPEN_NS_WORLDLISTENER;

/** \file OpenALObjectFactory.hxx

    This file defines the specialization of an ObjectFactory singleton
    for sound objects to be shown with the OpenALViewer */

/** Index for base types in the factory */
struct OpenALObjectTypeKey
{
  /** map key, text defines type of object */
  typedef std::string Key;

  /** Returned object is simply a pointer */
  typedef OpenALObject* ProductBase;

  /** This is a generic strings + doubles object, flexible enough to
      define most stuff */
  typedef WorldDataSpec SpecBase;
};

/** Define the OpenALObjectFactory as an instantiation of the default
    ConglomerateFactory, combined with the singleton template from
    boost.
    
    The OpenALObjectFactory can create objects for audialisation with
    OpenAL. 
*/
typedef boost::detail::thread::singleton
<ConglomerateFactory
 <OpenALObjectTypeKey, 
  boost::shared_ptr<SubcontractorBase<OpenALObjectTypeKey> > > > 
OpenALObjectFactory;

/** Template for a simple (single case) subcontractor.

    If you want to make your object available for the OpenALViewer
    object factory, create a static SubContractor, as follows:

    \code
    class MySoundObject: public OpenALObject
    {
      // need a constructor that uses a WorldDataSpec object
      MySoundObject(const WorldDataSpec& spec);

      // for the rest, see what you need to redefine from OpenALObject!
    };

    // and in your cxx file:
    static SubContractor<OpenALObjectTypeKey, MySoundObject>
    *MySoundObject_maker = new 
    SubContractor<OpenALObjectTypeKey, MySoundObject>("my-sound-object");
    \endcode

    With this, the OpenALListener can figure out how to create objects of
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
    OpenALObjectFactory::instance().addSubcontractor
      (this->key, boost::shared_ptr<SubcontractorBase<Xbase > >(this));
  }
  
  /** create a new object */
  typename Xbase::ProductBase create(const typename Xbase::Key& key, 
				     const typename Xbase::SpecBase& spec)
  {
    return new Derived(spec);
  }
};

/** The base subcontractor pointer in OpenAL */
typedef boost::shared_ptr<SubcontractorBase<OpenALObjectTypeKey> > SubconPtr;

CLOSE_NS_WORLDLISTENER;

#endif
