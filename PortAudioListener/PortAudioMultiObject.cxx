/* ------------------------------------------------------------------   */
/*      item            : PortAudioMultiObject.cxx
        made by         : Rene' van Paassen
        date            : 171112
        category        : body file
        description     :
        changes         : 171112 first version
        language        : C++
        copyright       : (c) 2017 TUDelft-AE-C&S
*/


#define PortAudioMultiObject_cxx
#include "PortAudioMultiObject.hxx"
#include "PortAudioObjectFactory.hxx"
#include "PortAudioListener.hxx"
#include "../WorldListener/AudioExceptions.hxx"
#include <debug.h>

OPEN_NS_WORLDLISTENER;

PortAudioMultiObject::PortAudioMultiObject(const WorldDataSpec& spec) :
  PortAudioObjectFixed(spec),
  base_volumes()
{
  //
}


PortAudioMultiObject::~PortAudioMultiObject()
{

}

void PortAudioMultiObject::addData(float* out, unsigned dataCount)
{
  unsigned ii = 0;
  while (ii < dataCount && ridx < buffer->info.frames) {
    for (const auto vol: base_volumes) {
      out[ii++] += volume * vol * buffer->data[ridx];
    }
    ridx++;
    if (looping && ridx == buffer->info.frames) { ridx = 0; }
  }
}

bool PortAudioMultiObject::initSound(PortAudioListener* master)
{
  // basic initialization first
  if (this->PortAudioObject::initSound(master)) {

    base_volumes.resize(master->getNumChannels(), 0.0f);
    for (unsigned ii = min(master->getNumChannels(),
			   unsigned(spec.coordinates.size())); ii--; ) {
      base_volumes[ii] = spec.coordinates[ii];
    }
    return true;
  }

  // not specified, cannot create
  return false;
}

void PortAudioMultiObject::iterate(const TimeSpec& ts,
				   const BaseObjectMotion& base)
{
  if (r_audio->isValid()) {
    if (r_audio->getNumVisibleSets(ts.getValidityStart()) ) {
      try {
        // access the data, read latest matching, but accept older data
        DataReader<AudioObjectFixed,MatchIntervalStartOrEarlier>
          r(*r_audio, ts);

	// event based, replay this sound from start
        if (!looping) { ridx = 0; }

	// continuous play of data, simply adjust sound parameters
	volume = r.data().volume;
      }
      catch (const NoDataAvailable& e) {
        I_MOD("no audio data for " << spec.name);
      }
    }
  }
}

static auto *PortAudioMultiObject_maker = new
  SubContractor<PortAudioObjectTypeKey, PortAudioMultiObject>
  ("PortAudioMultiObject");

CLOSE_NS_WORLDLISTENER;
