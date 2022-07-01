/* ------------------------------------------------------------------   */
/*      item            : OpenALObjectMoving.cxx
        made by         : Rene' van Paassen
        date            : 171219
        category        : body file
        description     :
        changes         : 171219 first version
        language        : C++
        copyright       : (c) 17 TUDelft-AE-C&S
*/

#define OpenALObjectMoving_cxx
#include "OpenALObjectMoving.hxx"
#include "OpenALObjectFactory.hxx"
#include <debug.h>

OPEN_NS_WORLDLISTENER;

OpenALObjectMoving::OpenALObjectMoving(const WorldDataSpec& spec) :
  OpenALObject(spec)
{
  //
}


OpenALObjectMoving::~OpenALObjectMoving()
{

}

void OpenALObjectMoving::connect(const GlobalId& master_id,
                                const NameSet& cname,
                                entryid_type entry_id,
                                Channel::EntryTimeAspect time_aspect)
{
  looping = looping || (time_aspect == Channel::Continuous);
  r_audio.reset(new ChannelReadToken
                (master_id, cname, AudioObjectMotion::classname, entry_id,
                 time_aspect, Channel::OneOrMoreEntries,
                 looping ? Channel::JumpToMatchTime : Channel::ReadAllData));
  if (source) {
    alSourcei(source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
  }
}

void OpenALObjectMoving::iterate(const TimeSpec& ts,
                                 const BaseObjectMotion& base)
{
  if (r_audio->isValid() && source) {
    if (looping || r_audio->getNumVisibleSets(ts.getValidityStart()) ) {
      try {

        // access the data, read latest matching, but accept older
        // data
#if DUECA_VERSION_NUM >= DUECA_VERSION(2,5,0)
        DataReader<AudioObjectMotion,MatchIntervalStartOrEarlier>
#else
        DataReader<AudioObjectMotion,
                   MatchIntervalStartOrEarlier<AudioObjectMotion> >
#endif
          r(*r_audio, ts);
        //cerr << r.data() << endl;

        // set source position, speed and direction vector
        alSource3f(source, AL_POSITION, r.data().xyz[1],
                   -r.data().xyz[2],-r.data().xyz[0]);
        alSource3f(source, AL_VELOCITY, r.data().uvw[1],
                   -r.data().uvw[2],-r.data().uvw[0]);
        float dir[] = { float(cos(r.data().getPsi()) * cos(r.data().getTht())),
                        float(sin(r.data().getPsi()) * cos(r.data().getTht())),
                        float(-sin(r.data().getTht())) };
        alSourcefv(source, AL_DIRECTION, dir);

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

const std::string& OpenALObjectMoving::getChannelClass()
{
  const static std::string cname(AudioObjectMotion::classname);
  return cname;
}

static auto *OpenALObjectMoving_maker = new
  SubContractor<OpenALObjectTypeKey, OpenALObjectMoving>
  ("OpenALObjectMoving");
static auto *OpenALObjectMoving_maker2 = new
  SubContractor<OpenALObjectTypeKey, OpenALObjectMoving>
  ("OpenALObjectMoving_relative");

CLOSE_NS_WORLDLISTENER;
