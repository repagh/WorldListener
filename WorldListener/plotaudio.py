#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import h5py
import numpy as np
from matplotlib import pyplot as plt
import sys

# read the file

try:
    fname = sys.argv[1]
    sd = h5py.File(fname, 'r')
    nruns = len(sd.keys())
    print("Datafile contains %i runs" % nruns)
    selrun = int(input("Which run to analyse? "))
    
    rdata = sd["run%04i" % selrun]
    sounds = rdata['data/audio'].keys()
    f = plt.figure()
    
    nmax = max([len(rdata['data/audio/%s/tick' % s]) for s in sounds])
    toff = min(rdata['data/audio/%s/tick' % s][0]
               for s in sounds if len(rdata['data/audio/%s/tick' % s]))
    off = 0
    for s in sounds:
        snd = rdata['data/audio/' + s]
        print("sound %s" % snd.attrs['label'])
        pitch = snd['data/pitch']
        volume = snd['data/volume']
        times = 0.0001 * (np.array(snd['tick']) - toff)
        if len(times) == nmax:
            plt.plot(times, np.array(volume)+off, label=snd.attrs['label'])
        else:
            plt.plot(times, np.array(volume)+off, '.', label=snd.attrs['label'])
        off += 1
    plt.gca().legend()
    plt.show()

except IndexError:
    print("please supply a file name with sound data")

# read all runs


