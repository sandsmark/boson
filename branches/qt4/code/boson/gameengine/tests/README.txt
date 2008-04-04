This directory contains the "unittests" directory, where unit tests (which have
nothing to do with the Unit class are "units" in the game in general) for the game
engine are implemented. These tests make sure that certain aspects of the
gameengine work and that they keep working.

In addition this directory also contains random test applications meant to test
various aspects. In contrast to the unit tests, these can usually not easily be
automated or are not meant to be automated, such as performance tests.

The unit tests are meant to be maintained for as long as possible (i.e. as long
as the code they test still exists), the other tests are probably required for a
short time only and probably can be removed when they don't compile anymore.


Note that all tests here are gameengine tests, there should be no GUI/gameview
dependency.
