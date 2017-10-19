# GLESRadarPC

Implementation of the Kodan RadarPC controller in C++ with OpenGLES rendering.

## Why this project
Almost all commercially available Radar interfaces are only available for x86 architectures.  The Kodan RadarPC system particularly only has a very small number of interfaces with terrible features.

Mark Pitman wanted to be able to operate his radar through his low-powered ARM-based computer (Cubieboard) and also operate the radar on a fixed schedule to reduce power consumption.

Allows operation of the Kodan RadarPC on a low-powered ARM processor (RaspPI, Cubieboard, etc.) with greater feature-set and flexibility than is available on commercial interfaces.

## How was this done
All the calls to the RadarPC controller were reverse-engineered by intercepting signals in the UDP stream from the RadarPC unit.

## Installation

* Run the makefile in the src directory to build the "cRadar" executable.  
* Launch this from the command line 
