/* ------------------------------------------------------------------   */
/*      item            : AudioTestGui.cxx
        made by         : repa
        from template   : DuecaModuleTemplate.cxx (2022.06)
        date            : Mon Nov 28 14:26:11 2022
        category        : body file
        description     :
        changes         : Mon Nov 28 14:26:11 2022 first version
        language        : C++
        copyright       : (c)
*/


#define AudioTestGui_cxx

// include the definition of the module class
#include "AudioTestGui.hxx"

// include additional files needed for your calculation here

// the standard package for DUSIME, including template source
#define DO_INSTANTIATE
#include <dueca.h>

// include the debug writing header, by default, write warning and
// error messages
#define W_MOD
#define E_MOD
#include <debug.h>

// class/module name
const char* const AudioTestGui::classname = "audio-test-gui";

// Parameters to be inserted
const ParameterTable* AudioTestGui::getMyParameterTable()
{
  static const ParameterTable parameter_table[] = {
    { "set-timing",
      new MemberCall<_ThisModule_,TimeSpec>
        (&_ThisModule_::setTimeSpec), set_timing_description },

    { "check-timing",
      new MemberCall<_ThisModule_,std::vector<int> >
      (&_ThisModule_::checkTiming), check_timing_description },

    { "add-control",
      new MemberCall<_ThisModule_,std::vector<std::string> >
      (&_ThisModule_::addControl),
      "Add a control window for sending volume/pitch updates, and optionally\n"
      "select new files to play. Arguments:\n"
      "- Window name\n"
      "- Name of the AudioObjectFixed channel\n"
      "- Label for the AudioObjectFixed channel\n"
      "- Name for the AudioFileSelection channel (optional)\n"
    },
    /* You can extend this table with labels and MemberCall or
       VarProbe pointers to perform calls or insert values into your
       class objects. Please also add a description (c-style string).

       Note that for efficiency, set_timing_description and
       check_timing_description are pointers to pre-defined strings,
       you can simply enter the descriptive strings in the table. */

    /* The table is closed off with NULL pointers for the variable
       name and MemberCall/VarProbe object. The description is used to
       give an overall description of the module. */
    { NULL, NULL,
      "Test module for generating inputs to a PortAudio based WorldListener"
    }
  };

  return parameter_table;
}

// constructor
AudioTestGui::AudioTestGui(Entity* e, const char* part, const
                   PrioritySpec& ps) :
  /* The following line initialises the SimulationModule base class.
     You always pass the pointer to the entity, give the classname and the
     part arguments. */
  Module(e, classname, part),

  // initialize the data you need in your simulation or process

  // initialize the channel access tokens, check the documentation for the
  // various parameters. Some examples:
  // r_mytoken(getId(), NameSet(getEntity(), MyData::classname, part),
  //           MyData::classname, 0, Channel::Events),
  // w_mytoken(getId(), NameSet(getEntity(), MyData2::classname, part),
  //           MyData2::classname, "label", Channel::Continuous),

  // create a clock, if you need time based triggering
  // instead of triggering on the incoming channels
  // myclock(),

  // a callback object, pointing to the main calculation function
  cb1(this, &_ThisModule_::doCalculation),
  // the module's main activity
  do_calc(getId(), "none", &cb1, ps)
{
  // connect the triggers for simulation
  //do_calc.setTrigger(/* fill in your triggering channels,
  // or enter the clock here */);
}

bool AudioTestGui::complete()
{
  bool res = true;

  /* All your parameters have been set. You may do extended
     initialisation here. Return false if something is wrong. */
  for (auto &u: testunits) {
    res = res && u.complete();
  }

  return res;
}

// destructor
AudioTestGui::~AudioTestGui()
{
  //
}

// as an example, the setTimeSpec function
bool AudioTestGui::setTimeSpec(const TimeSpec& ts)
{
  // a time span of 0 is not acceptable
  if (ts.getValiditySpan() == 0) return false;

  // specify the timespec to the activity
  do_calc.setTimeSpec(ts);
  // or do this with the clock if you have it (don't do both!)
  // myclock.changePeriodAndOffset(ts);

  // do whatever else you need to process this in your model
  // hint: ts.getDtInSeconds()

  // return true if everything is acceptable
  return true;
}

// the checkTiming function installs a check on the activity/activities
// of the module
bool AudioTestGui::checkTiming(const std::vector<int>& i)
{
  if (i.size() == 3) {
    new TimingCheck(do_calc, i[0], i[1], i[2]);
  }
  else if (i.size() == 2) {
    new TimingCheck(do_calc, i[0], i[1]);
  }
  else {
    return false;
  }
  return true;
}

bool AudioTestGui::addControl(const std::vector<std::string>& args)
{
  if (args.size() != 3 && args.size() != 4) {
    W_MOD("AudioTestGui::addControl needs three or four string arguments");
    return false;
  }
  try {
    if (args.size() == 3) {
      testunits.emplace_back(this, args[0], args[1], args[2]);
    }
    else {
      testunits.emplace_back(this, args[0], args[1], args[2], args[3]);
    }
    return true;
  }
  catch (const std::exception &e) {
    W_MOD("AudioTestGui::addControl, error in creating test unit: " <<
          e.what());
  }
  return false;
}

// tell DUECA you are prepared
bool AudioTestGui::isPrepared()
{
  bool res = true;

  for (auto &u: testunits) {
    CHECK_TOKEN(u.w_test);
    if (u.w_newfile) CHECK_TOKEN(*u.w_newfile);
  }

  // return result of checks
  return res;
}

// start the module
void AudioTestGui::startModule(const TimeSpec &time)
{
  do_calc.switchOn(time);
}

// stop the module
void AudioTestGui::stopModule(const TimeSpec &time)
{
  do_calc.switchOff(time);
}

// this routine contains the main simulation process of your module. You
// should read the input channels here, and calculate and write the
// appropriate output
void AudioTestGui::doCalculation(const TimeSpec& ts)
{
  // access the input
  // example:
  // try {
  //   DataReader<MyData> u(r_mytoken, ts);
  //   throttle = u.data().throttle;
  //   de = u.data().de; ....
  // }
  // catch(Exception& e) {
  //   // strange, there is no input. Should I try to continue or not?
  // }
  /* The above piece of code shows a block in which you try to catch
     error conditions (exceptions) to handle the case in which the input
     data is lost. This is not always necessary, if you normally do not
     foresee such a condition, and you don t mind being stopped when
     it happens, forget about the try/catch blocks. */

  // do the simulation or other calculations, one step

  // DUECA applications are data-driven. From the time a module is switched
  // on, it should produce data, so that modules "downstream" are
  // activated
  // access your output channel(s)
  // example
  // DataWriter<MyData2> y(w_mytoken, ts);

  // write the output into the output channel, using the data writer
  // y.data().var1 = something; ...
}

// Make a TypeCreator object for this module, the TypeCreator
// will check in with the script code, and enable the
// creation of modules of this type
static TypeCreator<AudioTestGui> a(AudioTestGui::getMyParameterTable());

AudioTestGui::PATestUnit::PATestUnit(const AudioTestGui *master,
                                      const std::string& name,
                                      const std::string& channel,
                                      const std::string& label,
                                      const std::string& file_channel) :
  AssociateObject<AudioTestGui>(*master),
  name(name),
  uifile("../../../AudioTestGui/testgui-full.ui"),
  window(),
  volume(0.0),
  pitch(1.0),
  w_test(getId(), NameSet(channel), getclassname<AudioObjectFixed>(),
         label, Channel::Events, Channel::OneOrMoreEntries),
  w_newfile(new ChannelWriteToken
            (getId(), NameSet(file_channel), getclassname<AudioFileSelection>(),
             label, Channel::Events, Channel::OneOrMoreEntries))
{
  //
}

AudioTestGui::PATestUnit::PATestUnit(const AudioTestGui *master,
                                      const std::string& name,
                                      const std::string& channel,
                                      const std::string& label) :
  AssociateObject<AudioTestGui>(*master),
  name(name),
  uifile("../../../AudioTestGui/testgui-small.ui"),
  window(),
  volume(0.0),
  pitch(1.0),
  w_test(getId(), NameSet(channel), getclassname<AudioObjectFixed>(),
         label, Channel::Events, Channel::OneOrMoreEntries),
  w_newfile()
{
  //
}

bool AudioTestGui::PATestUnit::complete()
{
  static GladeCallbackTable cb_table[] = {
    { "ft_volume", "value-changed",
      gtk_callback(&AudioTestGui::PATestUnit::changeVolume) },

    { "ft_pitch", "value-changed",
      gtk_callback(&AudioTestGui::PATestUnit::changePitch) },

    { "ft_filechoice", "selection-changed",
      gtk_callback(&AudioTestGui::PATestUnit::selectFile) },
    { NULL, NULL, NULL }
  };

  window.readGladeFile
    (uifile.c_str(), "ft_window", reinterpret_cast<gpointer>(this),
     cb_table);
  gtk_window_set_title(GTK_WINDOW(window["ft_window"]), name.c_str());
  if (window.getObject("file_dialog")) {
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(window["file_dialog"]), ".");
  }
  window.show();
  return true;
}

void AudioTestGui::PATestUnit::changeVolume(GtkRange* w, gpointer p)
{
  volume = gtk_range_get_value(w);
  DataWriter<AudioObjectFixed> v(w_test);
  v.data().volume = volume;
  v.data().pitch = pitch;
}

void AudioTestGui::PATestUnit::changePitch(GtkRange* w, gpointer p)
{
  pitch = gtk_range_get_value(w);
  DataWriter<AudioObjectFixed> v(w_test);
  v.data().volume = volume;
  v.data().pitch = pitch;
}

void AudioTestGui::PATestUnit::selectFile(GtkFileChooser* w, gpointer p)
{
  gchar *fname = gtk_file_chooser_get_filename(w);
  if (fname) {
    DataWriter<AudioFileSelection> s(*w_newfile);
    s.data().filename = fname;
    g_free(fname);
  }
}
