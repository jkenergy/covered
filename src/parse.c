/*!
 \file     parse.c
 \author   Trevor Williams  (trevorw@charter.net)
 \date     11/27/2001
*/

#include <stdio.h>

#include "defines.h"
#include "parse.h"
#include "link.h"
#include "util.h"
#include "db.h"
#include "binding.h"

extern void reset_lexer( FILE* out, str_link* file_list_head );
extern int VLparse();
extern int VLdebug;

extern str_link* use_files_head;

extern str_link* modlist_head;
extern str_link* modlist_tail;

/*!
 \param file  Pointer to file to read
 \param line  Read line from specified file.
 \param size  Maximum number of characters that can be stored in line.
 \return Returns the number of characters read from this line.

 Reads from specified file, one character at a time, until either a newline
 or EOF is encountered.  Returns the read line and the number of characters
 stored in the line.  No newline character is added to the line.
*/
int parse_readline( FILE* file, char* line, int size ) {

  int  i;             /* Loop iterator */
  char error[1024];   /* Error line    */

  while( (i < (size - 1)) && !feof( file ) && ((line[i] = fgetc( file )) != '\n') ) {
    i++;
  }

  if( !feof( file ) ) {
    line[i] = '\0';
  }

  if( i == size ) {
    snprintf( error, 1024, "Line too long.  Must be less than %d characters.", size );
    print_output( error, FATAL );
  }

  return( !feof( file ) );

}

/*!
 \param top        Name of top-level module to score
 \param output_db  Name of output directory for generated scored files.

 \return Returns TRUE if parsing is successful; otherwise, returns FALSE.

 Resets the lexer and parses all Verilog files specified in use_files list.
 After all design files are parsed, their information will be appropriately
 stored in the associated lists.
*/
bool parse_design( char* top, char* output_db ) {

  bool retval = TRUE;   /* Return value of this function */
  char msg[4096];       /* Output message                */

  str_link_add( top, &modlist_head, &modlist_tail );

  if( use_files_head != NULL ) {

    /* Initialize lexer with first file */
    reset_lexer( stdout, use_files_head );

    /* Starting parser */
    if( VLparse() != 0 ) {
      print_output( "Error in parsing design", FATAL );
      exit( 1 );
    }

    print_output( "========  Completed design parsing  ========\n", DEBUG );

    bind();
  
  } else {

    print_output( "No Verilog input files specified", FATAL );
    retval = FALSE;

  }

  /* Write contents to baseline database file. */
  if( !db_write( output_db ) ) {
    print_output( "Unable to write database file", FATAL );
    exit( 1 );
  }

  snprintf( msg, 4096, "========  Design written to database %s successfully  ========\n\n", output_db );
  print_output( msg, DEBUG );
 
  return( retval );

}

/*!
 \param db   Name of output database file to generate.
 \param vcd  Name of VCD file to parse for scoring.
 \return Returns TRUE if VCD parsing and scoring is successful; otherwise,
         returns FALSE.

*/
bool parse_and_score_dumpfile( char* db, char* vcd ) {

  bool retval = TRUE;   /* Return value of this function */
  char msg[4096];       /* Output message                */

  snprintf( msg, 4096, "========  Reading in database %s  ========\n", db );
  print_output( msg, DEBUG );

  /* Read in contents of specified database file */
  if( !db_read( db, READ_MODE_MERGE_NO_MERGE ) ) {
    print_output( "Unable to read database file", FATAL );
    exit( 1 );
  }

  /* Read in contents of VCD file */
  if( vcd == NULL ) {
    print_output( "VCD file not specified on command line", FATAL );
    exit( 1 );
  }

  snprintf( msg, 4096, "========  Reading in VCD dumpfile %s  ========\n", vcd );
  print_output( msg, DEBUG );

  reset_vcd_lexer( vcd );
  
  if( VCDparse() != 0 ) {
    print_output( "Error parsing VCD file", FATAL );
    exit( 1 );
  } else {
    /* Flush any pending statement trees that are waiting for delay */
    db_do_timestep( -1 );
  }

  snprintf( msg, 4096, "========  Writing database %s  ========\n", db );
  print_output( msg, DEBUG );

  /* Write contents to database file */
  if( !db_write( db ) ) {
    print_output( "Unable to write database file", FATAL );
    exit( 1 );
  }

  return( retval );

}

/* $Log$
/* Revision 1.7  2002/07/18 22:02:35  phase1geo
/* In the middle of making improvements/fixes to the expression/signal
/* binding phase.
/*
/* Revision 1.6  2002/07/13 04:09:18  phase1geo
/* Adding support for correct implementation of `ifdef, `else, `endif
/* directives.  Full regression passes.
/*
/* Revision 1.5  2002/07/09 04:46:26  phase1geo
/* Adding -D and -Q options to covered for outputting debug information or
/* suppressing normal output entirely.  Updated generated documentation and
/* modified Verilog diagnostic Makefile to use these new options.
/*
/* Revision 1.4  2002/07/03 03:31:11  phase1geo
/* Adding RCS Log strings in files that were missing them so that file version
/* information is contained in every source and header file.  Reordering src
/* Makefile to be alphabetical.  Adding mult1.v diagnostic to regression suite.
/* */
