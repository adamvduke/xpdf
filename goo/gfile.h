//========================================================================
//
// gfile.h
//
// Miscellaneous file and directory name manipulation.
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef GFILE_H
#define GFILE_H

#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>
#ifdef VMS
#include "vms_dirent.h"
#else
#include <dirent.h>
#endif
#include <gtypes.h>

class GString;

//------------------------------------------------------------------------

// Get home directory path.
extern GString *getHomeDir();

// Get current directory.
extern GString *getCurrentDir();

// Append a file name to a path string.  <path> may be an empty
// string, denoting the current directory).  Returns <path>.
extern GString *appendToPath(GString *path, char *fileName);

// Grab the path from the front of the file name.  If there is no
// directory component in <fileName>, returns an empty string.
extern GString *grabPath(char *fileName);

// Is this an absolute path or file name?
extern GBool isAbsolutePath(char *path);

// Make this path absolute by prepending current directory (if path is
// relative) or prepending user's directory (if path starts with '~').
GString *makePathAbsolute(GString *path);

//------------------------------------------------------------------------
// GDir and GDirEntry
//------------------------------------------------------------------------

class GDirEntry {
public:

  GDirEntry(char *dirPath, char *name1, GBool doStat);
  ~GDirEntry();
  GString *getName() { return name; }
  GBool isDir() { return dir; }

private:

  GString *name;		// dir/file name
  GBool dir;			// is it a directory?
};

class GDir {
public:

  GDir(char *name, GBool doStat1 = gTrue);
  ~GDir();
  GDirEntry *getNextEntry();
  void rewind();

private:

  GString *path;		// directory path
  GBool doStat;			// call stat() for each entry?
#ifdef VMS
  GBool needParent;		// need to return an entry for [-]
#endif
  DIR *dir;			// the DIR structure from opendir()
};

#endif
