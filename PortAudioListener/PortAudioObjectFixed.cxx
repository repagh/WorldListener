/* ------------------------------------------------------------------   */
/*      item            : PortAudioObjectFixed.cxx
        made by         : Rene' van Paassen
        date            : 171219
        category        : body file
        description     :
        changes         : 171219 first version
        language        : C++
        copyright       : (c) 17 TUDelft-AE-C&S
*/

#define PortAudioObjectFixed_cxx
#include "PortAudioObjectFixed.hxx"
#include "PortAudioObjectFactory.hxx"

#include <debug.h>

OPEN_NS_WORLDLISTENER;

PortAudioObjectFixed::PortAudioObjectFixed(const WorldDataSpec& spec) :
  PortAudioObject(spec)
{
  //
}


PortAudioObjectFixed::~PortAudioObjectFixed()
{

}

void PortAudioObjectFixed::connect(const GlobalId& master_id,
                                   const NameSet& cname,
                                   entryid_type entry_id,
                                   Channel::EntryTimeAspect time_aspect)
{
  looping = looping || (time_aspect == Channel::Continuous);

  r_audio.reset(new ChannelReadToken
                (master_id, cname, AudioObjectFixed::classname, entry_id,
                 time_aspect, Channel::OneOrMoreEntries,
                 looping ? Channel::JumpToMatchTime : Channel::ReadAllData));
}

void PortAudioObjectFixed::iterate(const TimeSpec& ts,
                                   const BaseObjectMotion& base)
{
  if (r_audio->isValid()) {
    if (looping || r_audio->getNumVisibleSets(ts.getValidityStart()) ) {
      try {
        // access the data, read latest matching, but accept older data
        DataReader<AudioObjectFixed,MatchIntervalStartOrEarlier>
          r(*r_audio, ts);

	// event based, replay this sound
        if (!looping) {

          ridx = 0;
          volume = base_volume * r.data().volume;
        }
        else {

          // continuous play of data, simply adjust sound parameters
          volume = base_volume * r.data().volume;
        }
      }
      catch (const NoDataAvailable& e) {
        I_MOD("no audio data for " << spec.name);
      }
    }
  }
}

const std::string& PortAudioObjectFixed::getChannelClass()
{
  const static std::string cname(AudioObjectFixed::classname);
  return cname;
}

void PortAudioObjectFixed::addData(float* out, unsigned dataCount)
{
  unsigned ii = channel;
  while (ii < dataCount && ridx < buffer->info.frames) {
    out[ii] = volume * buffer->data[ridx];
    ii += num_channels; ridx++;
    if (looping && ridx == buffer->info.frames) { ridx = 0; }
  }
}

static auto *PortAudioObjectFixed_maker = new
  SubContractor<PortAudioObjectTypeKey, PortAudioObjectFixed>
  ("PortAudioObjectFixed");

CLOSE_NS_WORLDLISTENER;
