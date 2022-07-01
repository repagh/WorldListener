# WorldListener, A DUECA Project

## Introduction

This is a very simple simulation project that uses [DUECA
middleware](https://github.com/dueca/dueca).It provides a general
interface to 3D audio play, through the "world-listener" module, and a
back-end to play audio:

- OpenALListener, interacting with OpenAL

## Application

The module and back-end are typically borrowed from simulation
projects. The interface is compatible with the 3D scene graph viewer
WorldView. Using `dueca-gproject`, borrow the following:

	WorldListener/WorldListener
	WorldListener/OpenALListener

For recording audio, use the `SoundRecorder`.

## Author(s)

René van Paassen

## LICENSE

EUPL-1.2

