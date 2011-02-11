// popen.h
//
// Mark Johnson, 14th Febuary 2005
//
//! An ipstream is an istream that reads from a popen command.
//! A izstream is an istream that reads from a (possibly) compressed file

#ifndef POPEN_H
#define POPEN_H

#include <cstdio>
#include <cstring>
#include <ext/stdio_filebuf.h>
#include <iostream>
#include <string>

//! ipstream_helper{} exists so that the various file buffers get created before the istream
//! gets created.
//
struct ipstream_helper {
  FILE* stdio_fp;
  __gnu_cxx::stdio_filebuf<char> stdio_fb;

  ipstream_helper(const char* command) 
    : stdio_fp(popen(command, "r")), stdio_fb(stdio_fp, std::ios_base::in) { }

  ~ipstream_helper() { pclose(stdio_fp); }  // close the popen'd stream
}; // ipstream_helper{}


//! An ipstream inherits from an istream.  
//! I really can't believe that this works, but it does.
//
struct ipstream : public ipstream_helper, public std::istream {
  ipstream(const char* command)       //!< shell command whose output is sent to the istream
    : ipstream_helper(command), std::istream(&stdio_fb) { }

  ipstream(const std::string command) //!< shell command whose output is sent to the istream
    : ipstream_helper(command.c_str()), std::istream(&stdio_fb) { }
}; // ipstream{}


//! izstream_helper{} exists to compute the command that ipstream has to execute
//! while izstream is being constructed.
//
struct izstream_helper {
  std::string popen_command;

  izstream_helper(const char* filename) 
  {
    const char* filesuffix = strrchr(filename, '.');

    if (filesuffix == NULL)
        filesuffix = "";

    popen_command = (strcasecmp(filesuffix, ".bz2")
		     ? (strcasecmp(filesuffix, ".gz") ? "cat " : "zcat ")
		     : "bzcat ");
    popen_command += filename;
  }
};  // izstream_helper{}

//! An izstream popen's a set of files, running them through zcat, bzcat or cat
//! depending on the suffix of the last file.
//
struct izstream : public izstream_helper, public ipstream {
  izstream(const char* filename) 
    : izstream_helper(filename), ipstream(popen_command.c_str()) { }
}; // izstream{}

#endif // POPEN_H
