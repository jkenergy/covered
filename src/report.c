/*!
 \file     report.c
 \author   Trevor Williams  (trevorw@charter.net)
 \date     11/29/2001
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <assert.h>
#include <stdlib.h>

#include "defines.h"
#include "report.h"
#include "util.h"
#include "line.h"
#include "toggle.h"
#include "comb.h"
#include "fsm.h"
#include "instance.h"
#include "stat.h"
#include "db.h"
#include "fsm.h"


extern char      user_msg[USER_MSG_LENGTH];
extern mod_inst* instance_root;
extern mod_link* mod_head;

/*!
 If set to a boolean value of TRUE, reports the line coverage for the specified database
 file; otherwise, omits line coverage from the report output.
*/
bool report_line        = TRUE;

/*!
 If set to a boolean value of TRUE, reports the toggle coverage for the specified database
 file; otherwise, omits toggle coverage from the report output.
*/
bool report_toggle      = TRUE;

/*!
 If set to a boolean value of TRUE, reports the combinational logic coverage for the specified
 database file; otherwise, omits combinational logic coverage from the report output.
*/
bool report_combination = TRUE;

/*!
 If set to a boolean value of TRUE, reports the finite state machine coverage for the
 specified database file; otherwise, omits finite state machine coverage from the
 report output.
*/
bool report_fsm         = TRUE;

/*!
 If set to a boolean value of TRUE, provides a coverage information for individual
 module instances.  If set to a value of FALSE, reports coverage information on a
 module basis, merging results from all instances of same module.
*/
bool report_instance    = FALSE;

/*!
 If set to a boolean value of TRUE, displays covered logic for a particular CDD file.
 By default, Covered will display uncovered logic.  Must be used in conjunction with
 the -v (verbose output) option.
*/
bool report_covered     = FALSE;

/*!
 If set to a boolean value of TRUE, displays GUI report viewer instead of generating text
 report files.
*/
bool report_gui         = FALSE;

/*!
 If set to a non-zero value, causes Covered to only generate combinational logic
 report information for depths up to the number specified.
*/
unsigned int report_comb_depth = REPORT_SUMMARY;

/*!
 Name of output file to write report contents to.  If output_file is NULL, the report
 will be written to standard output.
*/
char* output_file      = NULL;

/*!
 Name of input CDD file to read for generating coverage report.  This value must be
 specified to a value other than NULL for the report phase to complete properly.
*/
char* input_db         = NULL;


/*!
 Displays usage information about the report command.
*/
void report_usage() {

  printf( "\n" );
  printf( "Usage:  covered report (-h | -view | [<options>] <database_file>)\n" );
  printf( "\n" );
  printf( "   -view                      Uses the graphical report viewer for viewing reports.  If this\n" );
  printf( "                               option is not specified, the text report will be generated.\n" );
  printf( "                               (This option is currently not available).\n" );
  printf( "   -h                         Displays this help information.\n" );
  printf( "\n" );
  printf( "   Options:\n" );
  printf( "      -m [l][t][c][f]         Type(s) of metrics to report.  Default is ltc.\n" );
  printf( "      -d (s|d|v)              Level of report detail (s=summary, d=detailed, v=verbose).\n" );
  printf( "                               Default is to display summary coverage information.\n" );
  printf( "      -i                      Provides coverage information for instances instead of module.\n" );
  printf( "      -c                      If '-d d' or '-d v' is specified, displays covered line, toggle\n" );
  printf( "                               and combinational cases.  Default is to display uncovered results.\n" );
  printf( "      -o <filename>           File to output report information to.  Default is standard output.\n" );
  printf( "\n" );

}

/*!
 \param metrics  Specified metrics to calculate coverage for.

 Parses the specified string containing the metrics to test.  If
 a legal metric character is found, its corresponding flag is set
 to TRUE.  If a character is found that does not correspond to a
 metric, an error message is flagged to the user (a warning).
*/
void report_parse_metrics( char* metrics ) {

  char* ptr;  /* Pointer to current character being evaluated */

  /* Set all flags to FALSE */
  report_line        = FALSE;
  report_toggle      = FALSE;
  report_combination = FALSE;
  report_fsm         = FALSE;

  for( ptr=metrics; ptr<(metrics + strlen( metrics )); ptr++ ) {

    switch( *ptr ) {
      case 'l' :
      case 'L' :  report_line        = TRUE;  break;
      case 't' :
      case 'T' :  report_toggle      = TRUE;  break;
      case 'c' :
      case 'C' :  report_combination = TRUE;  break;
      case 'f' :
      case 'F' :  report_fsm         = TRUE;  break;
      default  :
        snprintf( user_msg, USER_MSG_LENGTH, "Unknown metric specified '%c'...  Ignoring.", *ptr );
        print_output( user_msg, WARNING );
        break;
    }

  }

}

/*!
 \param argc      Number of arguments in argument list argv.
 \param last_arg  Index of last parsed argument from list.
 \param argv      Argument list passed to this program.

 \return Returns TRUE if parsing was successful; otherwise, returns FALSE.

 Parses the argument list for options.  If a legal option is
 found for the report command, the appropriate action is taken for
 that option.  If an option is found that is not allowed, an error
 message is reported to the user and the program terminates immediately.
*/
bool report_parse_args( int argc, int last_arg, char** argv ) {

  bool retval = TRUE;  /* Return value for this function */
  int  i;              /* Loop iterator                  */

  i = last_arg + 1;

  while( (i < argc) && retval ) {

    if( strncmp( "-h", argv[i], 2 ) == 0 ) {
 
      report_usage();
      retval = FALSE;

    } else if( strncmp( "-m", argv[i], 2 ) == 0 ) {
    
      i++;
      report_parse_metrics( argv[i] );

    } else if( strncmp( "-view", argv[i], 5 ) == 0 ) {

      report_gui = TRUE;

    } else if( strncmp( "-i", argv[i], 2 ) == 0 ) {

      report_instance = TRUE;

    } else if( strncmp( "-c", argv[i], 2 ) == 0 ) {

      report_covered = TRUE;

    } else if( strncmp( "-d", argv[i], 2 ) == 0 ) {

      i++;
     
      if( argv[i][0] == 's' ) {
        report_comb_depth = REPORT_SUMMARY;
      } else if( argv[i][0] == 'd' ) {
        report_comb_depth = REPORT_DETAILED;
      } else if( argv[i][0] == 'v' ) {
        report_comb_depth = REPORT_VERBOSE;
      } else {
        snprintf( user_msg, USER_MSG_LENGTH, "Unrecognized detail type: -d %s\n", argv[i] );
        print_output( user_msg, FATAL );
        retval = FALSE;
      }

    } else if( strncmp( "-o", argv[i], 2 ) == 0 ) {

      i++;
      if( is_directory( argv[i] ) ) {
        output_file = strdup( argv[i] );
      } else {
  	snprintf( user_msg, USER_MSG_LENGTH, "Illegal output directory specified \"%s\"", argv[i] );
        print_output( user_msg, FATAL );
        retval = FALSE;
      }

    } else if( (i + 1) == argc ) {

      if( file_exists( argv[i] ) ) {
     
        input_db = strdup( argv[i] );
 
      } else {

        snprintf( user_msg, USER_MSG_LENGTH, "Cannot find %s database file for opening", argv[i] );
        print_output( user_msg, FATAL );
        exit( 1 );

      }

    } else {

      snprintf( user_msg, USER_MSG_LENGTH, "Unknown report command option \"%s\".  See \"covered -h\" for more information.", argv[i] );
      print_output( user_msg, FATAL );
      retval = FALSE;

    }

    i++;

  }

  return( retval );

}

/*!
 \param root   Pointer to root of instance tree to search.
 
 Recursively parses instance tree, creating statistic structures for each
 of the instances in the tree.  Calculates summary coverage information for
 children nodes first and parent nodes after.  In this way, the parent nodes
 will have the accumulated information from themselves and all of their
 children.
*/
void report_gather_instance_stats( mod_inst* root ) {

  mod_inst* curr;    /* Pointer to current instance being evaluated */

  /* Create a statistics structure for this instance */
  assert( root->stat == NULL );

  root->stat = statistic_create();

  /* Get coverage results for all children first */
  curr = root->child_head;
  while( curr != NULL ) {
    report_gather_instance_stats( curr );
    assert( curr->stat != NULL );
    statistic_merge( root->stat, curr->stat );
    curr = curr->next;
  }

  /* Get coverage results for this instance */
  if( report_line ) {
    line_get_stats( root->mod->stmt_head, &(root->stat->line_total), &(root->stat->line_hit) );
  }

  if( report_toggle ) {
    toggle_get_stats( root->mod->exp_head, 
                      root->mod->sig_head, 
                      &(root->stat->tog_total), 
                      &(root->stat->tog01_hit), 
                      &(root->stat->tog10_hit) );
  }

  if( report_combination ) {
    combination_get_stats( root->mod->exp_head,
                           &(root->stat->comb_total),
                           &(root->stat->comb_hit) );
  }

  if( report_fsm ) {
    fsm_get_stats( root->mod->fsm_head,
                   &(root->stat->state_total),
                   &(root->stat->state_hit),
                   &(root->stat->arc_total),
                   &(root->stat->arc_hit) );
  }

}

/*!
 \param head  Pointer to head of module list to search.
 
 Traverses module list, creating statistic structures for each
 of the modules in the tree, and calculates summary coverage information.
*/
void report_gather_module_stats( mod_link* head ) {

  while( head != NULL ) {

    head->mod->stat = statistic_create();

    /* Get coverage results for this instance */
    if( report_line ) {
      line_get_stats( head->mod->stmt_head, &(head->mod->stat->line_total), &(head->mod->stat->line_hit) );
    }

    if( report_toggle ) {
      toggle_get_stats( head->mod->exp_head, 
                        head->mod->sig_head, 
                        &(head->mod->stat->tog_total), 
                        &(head->mod->stat->tog01_hit), 
                        &(head->mod->stat->tog10_hit) );
    }

    if( report_combination ) {
      combination_get_stats( head->mod->exp_head,
                             &(head->mod->stat->comb_total),
                             &(head->mod->stat->comb_hit) );
    }

    if( report_fsm ) {
      fsm_get_stats( head->mod->fsm_head,
                     &(head->mod->stat->state_total),
                     &(head->mod->stat->state_hit),
                     &(head->mod->stat->arc_total),
                     &(head->mod->stat->arc_hit) );
    }

    head = head->next;

  }

}

/*!
 \param ofile    Pointer to output stream to display report information to.

 Generates generic report header for all reports.
*/
void report_print_header( FILE* ofile ) {

  switch( report_comb_depth ) {
    case REPORT_SUMMARY  :
      fprintf( ofile, "Covered -- Verilog Coverage Summarized Report\n" );
      fprintf( ofile, "=============================================\n" );
      break;
    case REPORT_DETAILED :
      fprintf( ofile, "Covered -- Verilog Coverage Detailed Report\n" );
      fprintf( ofile, "===========================================\n" );
      break;
    case REPORT_VERBOSE  :
      fprintf( ofile, "Covered -- Verilog Coverage Verbose Report\n" );
      fprintf( ofile, "==========================================\n" );
      break;
    default:
      break;
  }

  fprintf( ofile, "\n" );

}

/*!
 \param ofile  Pointer to output stream to display report information to.

 Generates a coverage report based on the options specified on the command line
 to the specified output stream.
*/
void report_generate( FILE* ofile ) {

  report_print_header( ofile );

  /* Gather statistics first */
  if( report_instance ) {
    report_gather_instance_stats( instance_root );
  } else {
    report_gather_module_stats( mod_head );
  }

  /* Call out the proper reports for the specified metrics to report */
  if( report_line ) {
    line_report( ofile, (report_comb_depth != REPORT_SUMMARY) );
  }

  if( report_toggle ) {
    toggle_report( ofile, (report_comb_depth != REPORT_SUMMARY) );
  }

  if( report_combination ) {
    combination_report( ofile, (report_comb_depth != REPORT_SUMMARY) );
  }

  if( report_fsm ) {
    fsm_report( ofile, (report_comb_depth != REPORT_SUMMARY) );
  }

}

/*!
 \param argc      Number of arguments in report command-line.
 \param last_arg  Index of last parsed argument from list.
 \param argv      Arguments passed to report command to parse.

 \return Returns 0 is report is successful; otherwise, returns -1.

 Performs report command functionality.
*/
int command_report( int argc, int last_arg, char** argv ) {

  int   retval = 0;  /* Return value of this function */
  FILE* ofile;       /* Pointer to output stream      */

  /* Parse score command-line */
  if( report_parse_args( argc, last_arg, argv ) ) {

    snprintf( user_msg, USER_MSG_LENGTH, COVERED_HEADER );
    print_output( user_msg, NORMAL );

    if( !report_gui ) {

      /* Open output stream */
      if( output_file != NULL ) {

        if( (ofile = fopen( output_file, "w" )) == NULL ) {

          snprintf( user_msg, USER_MSG_LENGTH, "Unable to open %s for writing", output_file );
          print_output( user_msg, FATAL );
          exit( 1 );

        } else {

          /* Free up memory for holding output_file */
          free_safe( output_file );

        }
    
      } else {
 
        ofile = stdout;

      }

      /* Open database file for reading */
      if( input_db == NULL ) {

        snprintf( user_msg, USER_MSG_LENGTH, "Database file not specified in command line" );
        print_output( user_msg, FATAL );
        exit( 1 );

      } else {

        if( report_instance ) {
          if( db_read( input_db, READ_MODE_REPORT_NO_MERGE ) ) {
            report_generate( ofile );
          }
        } else {
          if( db_read( input_db, READ_MODE_REPORT_MOD_MERGE ) ) {
            report_generate( ofile );
          }
        }

      }

      fclose( ofile );

    } else {

      /* Call GUI here */
      print_output( "The -view option is currently not available\n", FATAL );

    }

  }

  return( retval );

}


/*
 $Log$
 Revision 1.22  2003/08/25 13:02:04  phase1geo
 Initial stab at adding FSM support.  Contains summary reporting capability
 at this point and roughly works.  Updated regress suite as a result of these
 changes.

 Revision 1.21  2003/08/10 03:50:10  phase1geo
 More development documentation updates.  All global variables are now
 documented correctly.  Also fixed some generated documentation warnings.
 Removed some unnecessary global variables.

 Revision 1.20  2003/02/18 20:17:03  phase1geo
 Making use of scored flag in CDD file.  Causing report command to exit early
 if it is working on a CDD file which has not been scored.  Updated testsuite
 for these changes.

 Revision 1.19  2002/12/29 06:09:32  phase1geo
 Fixing bug where output was not squelched in report command when -Q option
 is specified.  Fixed bug in preprocessor where spaces where added in when newlines
 found in C-style comment blocks.  Modified regression run to check CDD file and
 generated module and instance reports.  Started to add code to handle signals that
 are specified in design but unused in Covered.

 Revision 1.18  2002/11/02 16:16:20  phase1geo
 Cleaned up all compiler warnings in source and header files.

 Revision 1.17  2002/10/29 19:57:51  phase1geo
 Fixing problems with beginning block comments within comments which are
 produced automatically by CVS.  Should fix warning messages from compiler.

 Revision 1.16  2002/10/11 05:23:21  phase1geo
 Removing local user message allocation and replacing with global to help
 with memory efficiency.

 Revision 1.15  2002/09/13 05:12:25  phase1geo
 Adding final touches to -d option to report.  Adding documentation and
 updating development documentation to stay in sync.

 Revision 1.14  2002/09/12 05:16:25  phase1geo
 Updating all CDD files in regression suite due to change in vector handling.
 Modified vectors to assign a default value of 0xaa to unassigned registers
 to eliminate bugs where values never assigned and VCD file doesn't contain
 information for these.  Added initial working version of depth feature in
 report generation.  Updates to man page and parameter documentation.

 Revision 1.13  2002/08/20 05:55:25  phase1geo
 Starting to add combination depth option to report command.  Currently, the
 option is not implemented.

 Revision 1.12  2002/08/20 04:48:18  phase1geo
 Adding option to report command that allows the user to display logic that is
 being covered (-c option).  This overrides the default behavior of displaying
 uncovered logic.  This is useful for debugging purposes and understanding what
 logic the tool is capable of handling.

 Revision 1.11  2002/08/19 04:34:07  phase1geo
 Fixing bug in database reading code that dealt with merging modules.  Module
 merging is now performed in a more optimal way.  Full regression passes and
 own examples pass as well.

 Revision 1.10  2002/07/20 18:46:38  phase1geo
 Causing fully covered modules to not be output in reports.  Adding
 instance3.v diagnostic to verify this works correctly.

 Revision 1.9  2002/07/09 04:46:26  phase1geo
 Adding -D and -Q options to covered for outputting debug information or
 suppressing normal output entirely.  Updated generated documentation and
 modified Verilog diagnostic Makefile to use these new options.

 Revision 1.8  2002/07/08 16:06:33  phase1geo
 Updating help information.

 Revision 1.7  2002/07/02 22:37:35  phase1geo
 Changing on-line help command calling.  Regenerated documentation.

 Revision 1.6  2002/06/28 03:04:59  phase1geo
 Fixing more errors found by diagnostics.  Things are running pretty well at
 this point with current diagnostics.  Still some report output problems.

 Revision 1.5  2002/06/25 21:46:10  phase1geo
 Fixes to simulator and reporting.  Still some bugs here.

 Revision 1.4  2002/05/13 03:02:58  phase1geo
 Adding lines back to expressions and removing them from statements (since the line
 number range of an expression can be calculated by looking at the expression line
 numbers).

 Revision 1.3  2002/05/03 03:39:36  phase1geo
 Removing all syntax errors due to addition of statements.  Added more statement
 support code.  Still have a ways to go before we can try anything.  Removed lines
 from expressions though we may want to consider putting these back for reporting
 purposes.
*/

