## -*-python-*-
## this is an example dueca_mod.py file, for you to start out with and adapt
## according to your needs. Note that you need a dueca_mod.py file only for the
## node with number 0

## in general, it is a good idea to clearly document your set up
## this is an excellent place.

## node set-up
ecs_node = 0   # dutmms1, send order 3
#aux_node = 1   # dutmms3, send order 1
#pfd_node = 2   # dutmms5, send order 2
#cl_node = 3    # dutmms4, send order 0

## priority set-up
# normal nodes: 0 administration
#               1 simulation, unpackers
#               2 communication
#               3 ticker

# administration priority. Run the interface and logging here
admin_priority = dueca.PrioritySpec(0, 0)

# priority of simulation, just above adiminstration
sim_priority = dueca.PrioritySpec(1, 0)

# nodes with a different priority scheme
# control loading node has 0, 1 and 2 as above and furthermore
#               3 stick priority
#               4 ticker priority
# priority of the stick. Higher than prio of communication
# stick_priority = dueca.PrioritySpec(3, 0)

# timing set-up
# timing of the stick calculations. Assuming 100 usec ticks, this gives 2500 Hz
# stick_timing = dueca.TimeSpec(0, 4)

# this is normally 100, giving 100 Hz timing
sim_timing = dueca.TimeSpec(0, 100)

## for now, display on 50 Hz
display_timing = dueca.TimeSpec(0, 200)

## log a bit more economical, 25 Hz
log_timing = dueca.TimeSpec(0, 400)

## try to find the openal devices, requires python-openal (pyal)

try:
    from openal import alc
    res = alc.alcGetString(None, alc.ALC_ALL_DEVICES_SPECIFIER)

    # initialize
    dev = b''
    aldevices = []
    i = 0

    # run until stopped by 2 nulls
    while not (ord(res[i]) == 0 and not(dev)):
        if ord(res[i]) == 0:
            aldevices.append(dev.decode('ascii'))
            dev = b''
        else:
            dev += res[i]
        i += 1
    print(aldevices)

except ImportError:
    #aldevices = [ 'Built-in Audio Analog Stereo' ]
    aldevices = [
        'soundcore Q20i',
        'Built-in Audio Analog Stereo',
        'Navi 21/23 HDMI/DP Audio Controller Digital Stereo (HDMI 4)'
    ]


## ---------------------------------------------------------------------
### the modules needed for dueca itself
if this_node_id == ecs_node:

    # create a list of modules:
    DUECA_mods = []
    DUECA_mods.append(dueca.Module("dusime", "", admin_priority))
    DUECA_mods.append(dueca.Module("dueca-view", "", admin_priority))
    DUECA_mods.append(dueca.Module("activity-view", "", admin_priority))
    DUECA_mods.append(dueca.Module("timing-view", "", admin_priority))
    DUECA_mods.append(dueca.Module("log-view", "", admin_priority))
    DUECA_mods.append(dueca.Module("channel-view", "", admin_priority))

    # create the entity with that list
    DUECA_entity = dueca.Entity("dueca", DUECA_mods)

## ---------------------------------------------------------------------
# modules for your project (example)
mymods = []

if this_node_id == ecs_node:
    mymods.append(dueca.Module(
        "motion-control", "", admin_priority).param(
            ('set_timing', display_timing),
            ('check_timing', (10000, 20000)),
            # ('add-moving-sound', "mosquito"),
            # ('set-coordinates', (0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0.5)),
            ('set-dt', 0.1),
            ('add-fixed-sound', "rpm1left"),
            # ('add-fixed-sound', "rpm2left"),
            # ('add-fixed-sound', "rpm1right"),
            # ('add-fixed-sound', "rpm2right"),
            ('add-fixed-sound', "gearup"),
            ('event-interval', 500),
            # ('add-fixed-sound', "geardown"),
            # ('event-interval', 53),
            # ('add-fixed-sound', "shutdown_left"),
            # ('event-interval', 59),
            # ('add-fixed-sound', "shutdown_right"),
            # ('event-interval', 43),
            # ('add-fixed-sound', "damage"),
            # ('event-interval', 19),
            # ('add-fixed-sound', "mass"),
            # ('event-interval', 61),
            # ('add-fixed-sound', "flaps"),
            # ('event-interval', 64),
            # ('add-fixed-sound', "wind"),
            # ('add-fixed-sound', "wind_gear"),
            # ('add-fixed-sound', "stall"),
            # ('event-interval', 66),
            # ('add-fixed-sound', "overspeed"),
            # ('event-interval', 69),
            # ('add-fixed-sound', "wheels"),
            # ('event-interval', 71),
            # ('add-fixed-sound', "touchdown"),
        ))

    mymods.append(dueca.Module(
        'world-listener', "", sim_priority).param(
            ('set-timing', display_timing),
            ('check-timing', (10000, 20000)),
            ('control-logger', "HDFLogConfig://ph-sound"),
            ('set-listener',
             dueca.OpenALListener().param(
                 ('set-devicename', aldevices[0]),
      #           ('add-object-class-data', (
       #              "AudioObjectMotion:mosquito", "mosquito #",
        #             "OpenALObjectMoving", "mosquito.wav")),
                 # coordinates are x/y/z, u/v/w, volume, pitch
         #        ('add-object-class-coordinates', (
          #           0, 0, 0, 0, 0, 0, 0.99, 1.0)),

                 # old style hacks, link to the label
                 ('add-controlled-static-sound', (
                     "rpm1left", "PA34_rpm1_left.wav")),
                 ( 'set-coordinates', ( 0, 0, 0, 0, 0, 0)),

                 ('add-controlled-static-sound', (
                     "rpm1right", "PA34_rpm1_right.wav")),
                 ('set-coordinates', ( 0, 2, 0, 0, 0, 0)),

                 ('add-controlled-static-sound', (
                     "rpm2left", "PA34_rpm2_left.wav")),
                 ('set-coordinates', ( 0, -2, 0, 0, 0, 0)),

                 ('add-controlled-static-sound', (
                     "rpm2right", "PA34_rpm2_right.wav")),
                 ('set-coordinates', ( 0, 2, 0, 0, 0, 0)),

                 ('add-controlled-static-sound', (
                     "gearup", "PA34_gear_up.wav")),
                 ('set-coordinates', ( 3, 4, 5, 0, 0, 0)),

                 ('add-controlled-static-sound', (
                     "geardown", "PA34_gear_down.wav")),
                 ('set-coordinates', ( 0, 0, -2, 0, 0, 0)),

                 ('add-controlled-static-sound', (
                     "shutdown_left", "PA34_shutdown_left.wav")),
                 ('set-coordinates', ( 0, -2, 0, 0, 0, 0)),

                 ('add-controlled-static-sound', (
                     "shutdown_right", "PA34_shutdown_right.wav")),
                 ('set-coordinates', ( 0, 2, 0, 0, 0, 0)),

                 ('add-controlled-static-sound', (
                     "damage", "damage.wav")),
                 ('set-coordinates', ( 0, 0, 0, 0, 0, 0)),

                 ('add-controlled-static-sound', (
                     "mass", "PA34_mass.wav")),
                 ('set-coordinates', ( -2, 0, 0, 0, 0, 0)),

                 # flaps sound, is defective file
                 ('add-controlled-static-sound', (
                     "flaps", "PA34_flaps.wav")),
                 ('set-coordinates', ( 0, 0, 0, 0, 0, 0)),

                 ('add-controlled-static-sound', (
                     "wind", "PA34_wind.wav")),
                 ('set-coordinates', ( 0, 0, 0, 0, 0, 0)),
                 ('set-looping', True),

                 ('add-controlled-static-sound', (
                     "wind_gear", "PA34_wind_gear.wav")),
                 ('set-coordinates', ( 0, 0, -2, 0, 0, 0)),

                 ('add-controlled-static-sound', (
                     "stall", "PA34_stall.wav")),
                 ('set-coordinates', ( 0, 0, 0, 0, 0, 0)),

                 ('add-controlled-static-sound', (
                     "overspeed", "PA34_overspeed.wav")),
                 ('set-coordinates', ( 0, 0, 0, 0, 0, 0)),

                 ('add-controlled-static-sound', (
                     "wheels", "PA34_groundroll.wav")),
                 ('set-coordinates', ( 0, 0, 0, 0, 0, 0)),

                 ('add-controlled-static-sound', (
                     "touchdown", "PA34_touchdown.wav")),
                 ('set-coordinates', ( 0, 0, -2, 0, 0, 0)),

#                 ('read-xml-definitions', 'openalworld.xml')
             ).complete()),
            ('initial-ears', (100, 0, 0, 0, 0, 0, 0, 0, 0)),
            ('keep-running', False),
        ))



# etc, each node can have modules in its mymods list

# then combine in an entity (one "copy" per node)
if mymods:
    myentity = dueca.Entity("ph-sound", mymods)
