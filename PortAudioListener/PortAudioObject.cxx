/* ------------------------------------------------------------------   */
/*      item            : PortAudioObject.cxx
        made by         : Rene' van Paassen
        date            : 171112
        category        : body file
        description     :
        changes         : 171112 first version
        language        : C++
        copyright       : (c) 2017 TUDelft-AE-C&S
*/

//https://stackoverflow.com/questions/61326170/im-trying-to-open-a-stream-in-portaudio


#define PortAudioObject_cxx
#include "PortAudioObject.hxx"
#include "PortAudioListener.hxx"
#include <debug.h>

OPEN_NS_WORLDLISTENER;

PortAudioObject::PortAudioObject(const WorldDataSpec& spec) :
  stream(NULL),
  spec(spec),
  base_volume(0.999999f),
  needstart(true),
  looping(spec.type.find("looping") != string::npos)
{
  // looping is determined here, since it may be overridden
  // in derived classes "connect"
}


PortAudioObject::~PortAudioObject()
{

}

bool PortAudioObject::haveError(int err, const char* stage)
{
  if (err != paNoError) {
    W_MOD("PortAudio error " << Pa_GetErrorText(err) <<
	  " for " << spec.name << " when " << stage);
    return true;
  }
  return false;
}

bool PortAudioObject::initSound(PortAudioListener* master)
{
  if (spec.filename.size() >= 1) {

    // create or get buffer
    try {
      buffer = master->getBufferManager().getBuffer(spec.filename[0]);
    }
    catch (const SoundFileReadError& e) {
      W_MOD("cannot init '" << spec.name << "' " << e.what());
      return false;
    }

    // loading function
    auto cfun =
      [this](const void *input, void *output, unsigned long frameCount,
             const PaStreamCallbackTimeInfo* timeinfo,
             PaStreamCallbackFlags statusFlags, void *userdata) {

        auto out = reinterpret_cast<float*>(output);
        auto lastout = out + frameCount;
        while (this->buffer->info.frames - this->ridx <= frameCount) {
          std::copy(&this->buffer->data[this->ridx],
                    &this->buffer->data[this->buffer->info.frames],
                    reinterpret_cast<float*>(output));

          out += this->buffer->info.frames - this->ridx;
          frameCount -= this->buffer->info.frames - this->ridx;
          if (!this->looping) {
            std::fill(reinterpret_cast<float*>(output), lastout, 0.0f);
            return paComplete;
          }
          else {
            this->ridx = 0;
          }
        }

        std::copy(&this->buffer->data[this->ridx],
                  &this->buffer->data[this->ridx+frameCount],
                  out);
        return paContinue;
      };

    PaStreamParameters out_param {
      .device = master->getDeviceIndex(),
      .channelCount = master->getNumChannels(),
      .sampleFormat = paFloat32,
      .suggestedLatency = 0};

    // create a stream
    stream = Pa_OpenStream
      (&stream, NULL, &out_param,
       buffer->info.samplerate, master->getBufferSize(),
       paNoFlag, cfun, NULL);

    return true;
  }

  // not specified, cannot create
  return false;
}

// this version is for static, constant sounds. overridden in derived
// classes for controlled sounds
void PortAudioObject::iterate(const TimeSpec& ts, const BaseObjectMotion& base)
{
  if (stream) {
    if (needstart) {
      //std::cerr << "starting play " << spec.name << std::endl;
      //alSourcePlay(source);
      //haveALError("playing");
      Pa_StartStream(stream);
      needstart = false;
    }
    else {
      if (Pa_IsStreamActive(stream)) {
        std::cerr << "playing " << spec.name << std::endl;
      }
    }
  }
}

void PortAudioObject::silence()
{
  if (stream && Pa_IsStreamActive(stream)) {
    Pa_StopStream(stream);
    needstart = true;
  }
}

const std::string& PortAudioObject::getChannelClass()
{
  const static std::string empty("");
  return empty;
}

CLOSE_NS_WORLDLISTENER;
