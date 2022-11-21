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
#include "comm-objects.h"
#include <debug.h>

OPEN_NS_WORLDLISTENER;

PortAudioMultiObject::PortAudioMultiObject(const WorldDataSpec& spec) :
  PortAudioObjectFixed(spec),
  master(NULL),
  base_volumes(),
  r_newfile()
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

bool PortAudioMultiObject::initSound(PortAudioListener* _master)
{
  // remember the PA master
  master = _master;
  
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

void PortAudioMultiObject::connect(const GlobalId& master_id,
                                   const NameSet& cname,
                                   entryid_type entry_id,
                                   Channel::EntryTimeAspect time_aspect)
{
  this->PortAudioObjectFixed::connect(master_id, cname, entry_id, time_aspect);

  // when so configured, read an additional file for resetting file name
  if (spec.filename.size() > 1) {
    r_newfile.reset
      (new ChannelReadToken
       (master_id, NameSet(spec.filename[1]),
	getclassname<AudioFileSelection>(), r_audio->getEntryLabel(),
	Channel::Events, Channel::OnlyOneEntry, Channel::ReadAllData));
  }
}

void PortAudioMultiObject::iterate(const TimeSpec& ts,
				   const BaseObjectMotion& base)
{
  if (r_newfile && r_newfile->isValid() && r_newfile->haveVisibleSets(ts)) {
    try {
      DataReader<AudioFileSelection> nf(*r_newfile, ts);
      try {
	assert(master != NULL);
	buffer = master->getBufferManager().getBuffer(nf.data().filename);
	ridx = 0U;
      }
      catch (const SoundFileReadError& e) {
	W_MOD("cannot load '" << nf.data().filename << "' " << e.what());
      }
    }
    catch (const NoDataAvailable& e) {
      W_MOD("Error reading '" << r_newfile->getName() << "' " << e.what());
    }
  }
  
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
