#ifndef NO_GAME_CODE_H
#define NO_GAME_CODE_H


/**
 * This file checks whether boson.h (or any other file from the actual game) is
 * included and produces an error if so.
 *
 *
 * The file converters should NEVER use methods/variables/.. from the current boson version:
 * file converters are supposed to be indepenedant of the current
 * implementation.
 *
 * A file converter converts from one format version to another - all
 * methods/defines/... of these versions are constant once they have been
 * released and thus can (and should) be hardcoded into the converter.
 **/


#ifdef BOSON_H
#error boson.h must not be included by this file! all methods and values must be hardcoded when writing file converters!
#endif

#endif

