/* ------------------------------------------------------------------   */
/*      item            : OpenALObjectFixed.cxx
        made by         : Rene' van Paassen
        date            : 171219
        category        : body file
        description     :
        changes         : 171219 first version
        language        : C++
        copyright       : (c) 17 TUDelft-AE-C&S
*/

#define OpenALObjectFixed_cxx
#include "OpenALObjectFixed.hxx"
#include "OpenALObjectFactory.hxx"

#include <debug.h>

OPEN_NS_WORLDLISTENER;

OpenALObjectFixed::OpenALObjectFixed(const WorldDataSpec& spec) :
  OpenALObject(spec)
{
  //
}


OpenALObjectFixed::~OpenALObjectFixed()
{

}

void OpenALObjectFixed::connect(const GlobalId& master_id,
                                const NameSet& cname,
                                entryid_type entry_id,
                                Channel::EntryTimeAspect time_aspect)
{
  looping = looping || (time_aspect == Channel::Continuous);

  r_audio.reset(new ChannelReadToken
                (master_id, cname, AudioObjectFixed::classname, entry_id,
                 time_aspect, Channel::OneOrMoreEntries,
                 looping ? Channel::JumpToMatchTime : Channel::ReadAllData));
  if (source) {
    alSourcei(source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
  }
}

void OpenALObjectFixed::iterate(const TimeSpec& ts,
                                const BaseObjectMotion& base)
{
  if (r_audio->isValid() && source) {
    if (looping || r_audio->getNumVisibleSets(ts.getValidityStart()) ) {
      try {
        // access the data, read latest matching, but accept older data
#if DUECA_VERSION_NUM >= DUECA_VERSION(2,5,0)
        DataReader<AudioObjectFixed,MatchIntervalStartOrEarlier>
#else
        DataReader<AudioObjectFixed,
                   MatchIntervalStartOrEarlier<AudioObjectFixed> >
#endif
          r(*r_audio, ts);


        if (!looping) {

          // event-based triggering, non-looping sounds
          // the arrival of data (re-)starts the sound

          // find current status, switch off if the sound is still
          // playing
          ALint source_state;
          alGetSourcei(source, AL_SOURCE_STATE, &source_state);
          if (source_state == AL_PLAYING) {
            if (r.data().volume != 0.0f) {
              I_MOD("Truncating and restarting " << spec.name);
            }
            alSourceStop(source);
            haveALError(r.data().volume == 0.0f ? "stopping" : "restarting");
          }

          if (r.data().volume > 0.0f) {

            // set parameters and start sound
            alSourcef(source, AL_GAIN, r.data().volume * base_volume);
            alSourcef(source, AL_PITCH, r.data().pitch);
            alSourcePlay(source);
            haveALError("single play");
          }
        }
        else {

          // continuous play of data, simply adjust sound parameters

          // set volume and pitch with update
          alSourcef(source, AL_GAIN, r.data().volume * base_volume);
          alSourcef(source, AL_PITCH, r.data().pitch);

          if (needstart) {
            alSourcePlay(source);
            haveALError("starting");
            needstart = false;
          }
        }
      }
      catch (const NoDataAvailable& e) {
        I_MOD("no audio data for " << spec.name);
      }
    }
  }
}

const std::string& OpenALObjectFixed::getChannelClass()
{
  const static std::string cname(AudioObjectFixed::classname);
  return cname;
}

static auto *OpenALObjectFixed_maker = new
  SubContractor<OpenALObjectTypeKey, OpenALObjectFixed>
  ("OpenALObjectFixed");
static auto *OpenALObjectFixed_maker2 = new
  SubContractor<OpenALObjectTypeKey, OpenALObjectFixed>
  ("OpenALObjectFixed_relative");

CLOSE_NS_WORLDLISTENER;
