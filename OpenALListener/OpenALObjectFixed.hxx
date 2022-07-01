/* ------------------------------------------------------------------   */
/*      item            : OpenALObjectFixed.hxx
        made by         : Rene van Paassen
        date            : 171219
	category        : header file 
        description     : 
	changes         : 171219 first version
        language        : C++
        copyright       : (c) 2017 TUDelft-AE-C&S
*/

#ifndef OpenALObjectFixed_hxx
#define OpenALObjectFixed_hxx

#include "OpenALObject.hxx"
#include <boost/scoped_ptr.hpp>

OPEN_NS_WORLDLISTENER;

class OpenALObjectFixed: public OpenALObject
{
  /** Channel read token for audio control */
  boost::scoped_ptr<ChannelReadToken>      r_audio;

public:
  /** Constructor
      
      @param spec  Specification for the object, using spec members:
                   * name: for identification
                   * type: looping to indicate looping sound
                           relative to indicate move with observer
                   * filename: only 1st element, file for sound
                   * coordinates: xyz (3), uvw (3), unused, unused, 
                     dirxyz(3), inner/outer cone angle, outer cone gain
                   */
  OpenALObjectFixed(const WorldDataSpec& spec);
  
  /** Destructor */
  ~OpenALObjectFixed();

  /** Connect to a channel entry 
      @param master_id ID for opening a channel reader
      @param cname     Channel with object data
      @param entry_id  Entry in the channel */
  void connect(const GlobalId& master_id, const NameSet& cname,
               entryid_type entry_id,
               Channel::EntryTimeAspect time_aspect);
  
  /** Play, update, recalculate, etc. */
  virtual void iterate(const TimeSpec& ts, const BaseObjectMotion& base);

  /** Class of channel, if applicable */
  const std::string& getChannelClass();
};

CLOSE_NS_WORLDLISTENER;


#endif
