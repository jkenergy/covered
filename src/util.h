#ifndef __UTIL_H__
#define __UTIL_H__

/*!
 \file     util.h
 \author   Trevor Williams  (trevorw@charter.net)
 \date     11/27/2001
 \brief    Contains miscellaneous global functions used by many functions.
*/

#include <stdio.h>

#include "defines.h"


/*! \brief Sets error suppression to specified value */
void set_output_suppression( bool value );

/*! \brief Sets global debug flag to specified value */
void set_debug( bool value );

/*! \brief Displays error message to standard output. */
void print_output( char* msg, int type );

/*! \brief Returns TRUE if the specified string is a legal variable name. */
bool is_variable( char* token );

/*! \brief Returns TRUE if the specified string is a legal directory string. */
bool is_directory( char* token );

/*! \brief Returns TRUE if the specified directory exists. */
bool directory_exists( char* dir );

/*! \brief Loads contents of specified directory to file list if extension is part of list. */
void directory_load( char* dir, str_link* ext_head, str_link** file_head, str_link** file_tail );

/*! \brief Returns TRUE if the specified file exists. */
bool file_exists( char* file );

/*! \brief Reads line from file and returns it in string form. */
bool readline( FILE* file, char** line );

/*! \brief Extracts highest level of hierarchy from specified scope. */
void scope_extract_front( char* scope, char* front, char* rest );

/*! \brief Extracts lowest level of hierarchy from specified scope. */
void scope_extract_back( char* scope, char* back, char* rest );

/*! \brief Returns TRUE if specified scope is local (contains no periods). */
bool scope_local( char* scope );

/*! \brief Returns next Verilog file to parse. */
str_link* get_next_vfile( str_link* curr, char* mod );

/*! \brief Performs safe malloc call. */
void* malloc_safe(size_t size );

/*! \brief Performs safe deallocation of heap memory. */
void free_safe( void* ptr );

/*! \brief Creates a string containing space characters. */
void gen_space( char* spaces, int num_spaces );


/*
 $Log$
 Revision 1.10  2002/11/05 00:20:08  phase1geo
 Adding development documentation.  Fixing problem with combinational logic
 output in report command and updating full regression.

 Revision 1.9  2002/11/02 16:16:20  phase1geo
 Cleaned up all compiler warnings in source and header files.

 Revision 1.8  2002/10/31 23:14:31  phase1geo
 Fixing C compatibility problems with cc and gcc.  Found a few possible problems
 with 64-bit vs. 32-bit compilation of the tool.  Fixed bug in parser that
 lead to bus errors.  Ran full regression in 64-bit mode without error.

 Revision 1.7  2002/10/29 19:57:51  phase1geo
 Fixing problems with beginning block comments within comments which are
 produced automatically by CVS.  Should fix warning messages from compiler.

 Revision 1.6  2002/10/29 13:33:21  phase1geo
 Adding patches for 64-bit compatibility.  Reformatted parser.y for easier
 viewing (removed tabs).  Full regression passes.

 Revision 1.5  2002/07/18 22:02:35  phase1geo
 In the middle of making improvements/fixes to the expression/signal
 binding phase.

 Revision 1.4  2002/07/08 12:35:31  phase1geo
 Added initial support for library searching.  Code seems to be broken at the
 moment.

 Revision 1.3  2002/07/03 03:31:11  phase1geo
 Adding RCS Log strings in files that were missing them so that file version
 information is contained in every source and header file.  Reordering src
 Makefile to be alphabetical.  Adding mult1.v diagnostic to regression suite.
*/

#endif

