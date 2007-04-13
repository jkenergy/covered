#ifdef DEBUG_MODE

/*
 Copyright (c) 2006 Trevor Williams

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by the Free Software
 Foundation; either version 2 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program;
 if not, write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*!
 \file     cli.c
 \author   Trevor Williams  (trevorw@sgi.com)
 \date     4/11/2007
*/


#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "cli.h"
#include "codegen.h"
#include "scope.h"
#include "sim.h"
#include "util.h"
#include "vsignal.h"


extern char                 user_msg[USER_MSG_LENGTH];
extern bool                 flag_use_command_line_debug;
extern uint64               curr_sim_time;
extern /*@null@*/inst_link* inst_head;


/*!
 Specifies the number of statements left to execute before returning to the CLI prompt.
*/
static int    stmts_left = 0;

/*!
 Specifies the number of timesteps left to execute before returning to the CLI prompt.
*/
static int    timesteps_left = 0;

/*!
 Records the last timestep that was seen by the CLI.
*/
static uint64 last_timestep;

/*!
 Specifies if we should run without stopping (ignore stmts_left value)
*/
static bool   dont_stop = FALSE;

/*!
 Specifies if the history buffer needs to be replayed prior to CLI prompting.
*/
static int    cli_replay_index = 0;

/*!
 Specifies if simulator debug information should be output during CLI operation.
*/
bool          cli_debug_mode = FALSE;

/*!
 Array of user-specified command strings.
*/
static char** history = NULL;

/*!
 Index of current command string in history array to store.
*/
static int    history_index = 0;

/*!
 The currently allocated size of the history array.
*/
static int    history_size = 0;


/*!
 Displays CLI usage information to standard output.
*/
void cli_usage() {

  printf( "\n" );
  printf( "Covered score command CLI usage:\n" );
  printf( "\n" );
  printf( "  step [<num>]            Advances to the next statement if <num> is not\n" );
  printf( "                          specified; otherwise, advances <num> statements\n" );
  printf( "                          before returning to the CLI prompt.\n" );
  printf( "\n" );
  printf( "  next [<num>]            Advances to the next timestep if <num> is not\n" );
  printf( "                          specified; otherwise, advances <num> timesteps\n" );
  printf( "                          before returning to the CLI prompt.\n" );
  printf( "\n" );
  printf( "  run                     Runs the simulation.\n" );
  printf( "\n" );
  printf( "  continue                Continues running the simulation.\n" );
  printf( "\n" );
  printf( "  display <type>          Displays the current state of the given type.\n" );
  printf( "      Valid types:\n" );
  printf( "        active_queue      Displays the current state of the active\n" );
  printf( "                          simulation queue.\n" );
  printf( "        delayed_queue     Displays the current state of the delayed\n" );
  printf( "                          simulation queue.\n" );
  printf( "        all_list          Displays the list of all threads.\n" );
  printf( "        current           Displays the current scope, block, filename\n" );
  printf( "                          and line number.\n" );
  printf( "        time              Displays the current simulation time.\n" );
  printf( "\n" );
  printf( "  show <signal>           Displays the current value of the given net/variable.\n" );
  printf( "\n" );
  printf( "  debug [on | off]        Turns verbose debug output from simulator on\n" );
  printf( "                          or off.  If 'on' or 'off' is not specified,\n" );
  printf( "                          displays the current debug mode.\n" );
  printf( "\n" );
  printf( "  list [<num>]            Lists the contents of the file where the\n" );
  printf( "                          current statement is to be executed.  If\n" );
  printf( "                          <num> is specified, outputs the given number\n" );
  printf( "                          of lines; otherwise, outputs 10 lines.\n" );
  printf( "\n" );
  printf( "  savehist <file>         Saves the current history to the specified\n" );
  printf( "                          file.\n" );
  printf( "\n" );
  printf( "  history [(<num> | all)] Displays the last 10 lines of command-line\n" );
  printf( "                          history.  If 'all' is specified, the entire\n" );
  printf( "                          history contents will be displayed.  If <num>\n" );
  printf( "                          is specified, the last <num> commands will be\n" );
  printf( "                          displayed.\n" );
  printf( "\n" );
  printf( "  !<num>                  Executes the command at the <num> position\n" );
  printf( "                          in history.\n" );
  printf( "\n" );
  printf( "  !!                      Executes the last valid command.\n" );
  printf( "\n" );
  printf( "  help                    Displays this usage message.\n" );
  printf( "\n" );
  printf( "  quit                    Ends simulation.\n" );
  printf( "\n" );

}

/*!
 \param msg       Message to display as error message.
 \param standard  If set to TRUE, output to standard output.

 Displays the specified error message.
*/
void cli_print_error( char* msg, bool standard ) {

  if( standard ) {
    printf( "%s.  Type 'help' for usage information.\n", msg );
  }

}

/*!
 Displays the current statement to standard output.
*/
void cli_display_current_stmt() {

  thread* curr;        /* Pointer to current thread in queue */
  char**  code;        /* Pointer to code string from code generator */
  int     code_depth;  /* Depth of code array */
  int     i;           /* Loop iterator */

  /* Get current thread from simulator */
  curr = sim_current_thread();

  assert( curr != NULL );
  assert( curr->funit != NULL );
  assert( curr->curr != NULL );

  /* Generate the logic */
  codegen_gen_expr( curr->curr->exp, curr->curr->exp->op, &code, &code_depth, curr->funit );

  /* Output the full expression */
  for( i=0; i<code_depth; i++ ) {
    printf( "    %7d:    %s\n", curr->curr->exp->line, code[i] );
    free_safe( code[i] );
  }

  if( code_depth > 0 ) {
    free_safe( code );
  }

}

/*!
 Outputs the scope, block name, filename and line number of the current thread in the active queue to standard output.
*/
void cli_display_current() {

  thread* curr;         /* Pointer to current thread */
  char    scope[4096];  /* String containing scope of given functional unit */
  int     ignore = 0;   /* Specifies that we should not ignore a matching functional unit */

  /* Get current thread from simulator */
  curr = sim_current_thread();

  assert( curr != NULL );
  assert( curr->funit != NULL );
  assert( curr->curr != NULL );

  /* Get the scope of the functional unit represented by the current thread */
  scope[0] = '\0';
  instance_gen_scope( scope, inst_link_find_by_funit( curr->funit, inst_head, &ignore ), TRUE );

  /* Output the given scope */
  printf( "    SCOPE: %s, BLOCK: %s, FILE: %s\n", scope, funit_flatten_name( curr->funit ), curr->funit->filename );

  /* Display current statement */
  cli_display_current_stmt();

}

/*!
 \param name  Name of signal to display

 Outputs the given signal value to standard output.
*/
bool cli_display_signal( char* name ) {

  bool       retval = TRUE;  /* Return value for this function */
  thread*    curr;           /* Pointer to current thread in simulator */
  vsignal*   sig;            /* Pointer to found signal */
  func_unit* funit;          /* Pointer to found functional unit */

  /* Get current thread from simulator */
  curr = sim_current_thread();

  assert( curr != NULL );
  assert( curr->funit != NULL );
  assert( curr->curr != NULL );

  /* Find the given signal name based on the current scope */
  if( scope_find_signal( name, curr->funit, &sig, &funit, 0 ) ) {

    /* Output the signal contents to standard output */
    printf( "  " );
    vsignal_display( sig );

  } else {

    cli_print_error( "Unable to find specified signal", TRUE );
    retval = FALSE;

  }

  return( retval );

}

/*!
 \param num  Maximum number of lines to display

 Starting at the current statement line, outputs the next num lines to standard output.
*/
void cli_display_lines( unsigned num ) {

  thread* curr;        /* Pointer to current thread in simulation */
  FILE*   vfile;       /* File pointer to Verilog file */
  char*   line;        /* Pointer to current line */
  int     lnum = 1;    /* Current line number */
  int     start_line;  /* Starting line */

  /* Get the current thread from the simulator */
  curr = sim_current_thread();

  assert( curr != NULL );
  assert( curr->funit != NULL );
  assert( curr->curr != NULL );
  assert( (vfile = fopen( curr->funit->filename, "r" )) != NULL );

  /* Get the starting line number */
  start_line = curr->curr->exp->line;

  /* Read the Verilog file and output lines when we are in range */
  while( util_readline( vfile, &line ) ) {
    if( (lnum >= start_line) && (lnum < (start_line + num)) ) {
      printf( "    %7d:  %s\n", lnum, line );
    }
    free_safe( line );
    lnum++;
  }

  fclose( vfile );

}

/*!
 \param line       User-specified command line to parse
 \param perform    Set to TRUE if we should perform the specified command
 \param replaying  Set to TRUE if we are calling this due to replaying the history

 \return Returns TRUE if the user specified a valid command; otherwise, returns FALSE

 Parses the given command from the user.
*/
bool cli_parse_input( char* line, bool perform, bool replaying ) {

  char     arg[4096];         /* Holder for user argument */
  bool     valid_cmd = TRUE;  /* Specifies if the given command was valid */
  int      chars_read;        /* Specifies the number of characters that was read from the string */
  int      i;                 /* Iterator */
  unsigned num;               /* Unsigned integer value from user */
  FILE*    hfile;             /* History file to read or to write */

  /* Resize the history if necessary */
  if( history_index == history_size ) {
    history_size = (history_size == 0) ? 50 : (history_size * 2);
    history      = (char**)realloc( history, (sizeof( char* ) * (history_size * 2)) );
  }

  /* Store this command in the history buffer if we are not in replay mode */
  if( !replaying ) {
    history[history_index] = line;
  }

  /* Parse first string */
  if( sscanf( line, "%s%n", arg, &chars_read ) == 1 ) {

    line += chars_read;

    if( arg[0] == '!' ) {

      line = arg;
      line++;
      if( arg[1] == '!' ) {
        free_safe( history[history_index] );
        cli_parse_input( strdup( history[history_index-1] ), perform, replaying );
        history_index--;
      } else if( sscanf( line, "%d", &num ) == 1 ) {
        if( num < (history_index + 1) ) {
          free_safe( history[history_index] );
          cli_parse_input( strdup( history[num-1] ), perform, replaying );
          history_index--;
        } else {
          cli_print_error( "Illegal history number", perform );
          valid_cmd = FALSE;
        }
      } else {
        cli_print_error( "Illegal value to the right of '!'", perform );
        valid_cmd = FALSE;
      }
            
    } else if( strncmp( "help", arg, 4 ) == 0 ) {

      if( perform ) {
        cli_usage();
      }

    } else if( strncmp( "step", arg, 4 ) == 0 ) {

      if( perform ) {
        if( sscanf( line, "%d", &stmts_left ) != 1 ) {
          stmts_left = 1;
        }
      }

    } else if( strncmp( "next", arg, 4 ) == 0 ) {

      if( perform ) {
        if( sscanf( line, "%d", &timesteps_left ) != 1 ) {
          timesteps_left = 1;
        }
      }

    } else if( strncmp( "run", arg, 3 ) == 0 ) {
  
      if( perform ) {
        /* TBD - We should probably allow a way to restart the simulation??? */
        dont_stop = TRUE;
      }

    } else if( strncmp( "continue", arg, 8 ) == 0 ) {

      if( perform ) {
        dont_stop = TRUE;
      }

    } else if( strncmp( "display", arg, 7 ) == 0 ) {

      if( sscanf( line, "%s", arg ) == 1 ) {
        if( strncmp( "active_queue", arg, 12 ) == 0 ) {
          if( perform ) {
            sim_display_active_queue();
          }
        } else if( strncmp( "delayed_queue", arg, 13 ) == 0 ) {
          if( perform ) {
            sim_display_delay_queue();
          }
        } else if( strncmp( "all_list", arg, 8 ) == 0 ) {
          if( perform ) {
            sim_display_all_list();
          }
        } else if( strncmp( "current", arg, 5 ) == 0 ) {
          if( perform ) {
            cli_display_current();
          }
        } else if( strncmp( "time", arg, 4 ) == 0 ) {
          if( perform ) {
            printf( "    TIME: %lld\n", curr_sim_time );
          }
        } else {
          cli_print_error( "Unknown display type", perform );
          valid_cmd = FALSE; 
        }
      } else {
        cli_print_error( "Type information missing from display command", perform );
        valid_cmd = FALSE; 
      }

    } else if( strncmp( "show", arg, 4 ) == 0 ) {

      if( sscanf( line, "%s", arg ) == 1 ) {
        if( perform ) {
          cli_display_signal( arg );
        }
      } else {
        cli_print_error( "No signal name specified", perform );
        valid_cmd = FALSE;
      }

    } else if( strncmp( "quit", arg, 4 ) == 0 ) {

      if( perform ) {
        exit( 0 );
      }
 
    } else if( strncmp( "debug", arg, 5 ) == 0 ) {

      if( sscanf( line, "%s", arg ) == 1 ) {
        if( strncmp( "on", arg, 2 ) == 0 ) {
          if( perform ) {
            cli_debug_mode = TRUE;
            set_debug( TRUE );
          }
        } else if( strncmp( "off", arg, 3 ) == 0 ) {
          if( perform ) {
            cli_debug_mode = FALSE;
          }
        } else {
          cli_print_error( "Unknown debug command parameter", perform );
          valid_cmd = FALSE; 
        }
      } else {
        if( perform ) {
          if( cli_debug_mode ) {
            printf( "Current debug mode is on.\n" );
          } else {
            printf( "Current debug mode is off.\n" );
          }
        }
      }

    } else if( strncmp( "history", arg, 7 ) == 0 ) {

      i = (history_index - 9);
      if( sscanf( line, "%d", &num ) == 1 ) {
        i = (history_index - (num - 1));
      } else if( sscanf( line, "%s", arg ) == 1 ) {
        if( strncmp( "all", arg, 3 ) == 0 ) {
          i = 0;
        }
      }
      if( perform ) {
        printf( "\n" );
        for( i=((i<0)?0:i); i<=history_index; i++ ) {
          printf( "%7d  %s\n", (i + 1), history[i] );
        }
      }

    } else if( strncmp( "savehist", arg, 8 ) == 0 ) {

      if( sscanf( line, "%s", arg ) == 1 ) {
        if( perform ) {
          if( (hfile = fopen( arg, "w" )) != NULL ) {
            for( i=0; i<history_index; i++ ) {
              fprintf( hfile, "%s\n", history[i] );
            }
            fclose( hfile );
            printf( "History saved to file '%s'\n", arg );
          } else {
            cli_print_error( "Unable to write history file", perform );
            valid_cmd = FALSE;
          }
        }
      } else {
        cli_print_error( "Filename not specified", perform );
        valid_cmd = FALSE;
      }

    } else if( strncmp( "list", arg, 4 ) == 0 ) {

      if( perform ) {
        if( sscanf( line, "%u", &num ) == 1 ) {
          cli_display_lines( num );
        } else {
          cli_display_lines( 10 );
        }
      }

    } else {

      cli_print_error( "Unknown command", perform );
      valid_cmd = FALSE; 

    }

  } else {

    valid_cmd = FALSE;

  }

  /* If the command was valid, increment the history index */
  if( !replaying ) {
    if( valid_cmd ) {
      history_index++;
 
    /* Otherwise, deallocate the command from the history buffer */
    } else {
      free_safe( history[history_index] );
    }
  }
  
  /* Always increment the replay index if we are performing */
  if( perform ) {
    cli_replay_index++;  
  }

  return( valid_cmd );

}

/*!
 Takes care of either replaying the history buffer or prompting the user for the next command
 to be issued.
*/
void cli_prompt_user() {

  char* line;        /* Read line from user */
  char  arg[4096];   /* Holder for user argument */
  bool  valid_cmd;   /* Specifies if the given command was valid */
  int   chars_read;  /* Specifies the number of characters that was read from the string */
  int   i;           /* Iterator */

  do {

    /* If the history buffer still needs to be replayed, do so instead of prompting the user */
    if( cli_replay_index < history_index ) {

      printf( "\ncli %d> %s\n", (cli_replay_index + 1), history[cli_replay_index] );

      (void)cli_parse_input( history[cli_replay_index], TRUE, TRUE );

    } else {

      /* Prompt the user for input */
      printf( "\ncli %d> ", (history_index + 1) ); 
      fflush( stdout );

      /* Read the user-specified command */
      (void)util_readline( stdin, &line );

      /* Parse the command line */
      (void)cli_parse_input( line, TRUE, FALSE );

    }

  } while( (stmts_left == 0) && (timesteps_left == 0) && !dont_stop );

}

/*!
 Performs CLI prompting if necessary.
*/
void cli_execute() {

  char args[3][4096];  /* User-specified command-line args */
  int  argc;           /* Number of arguments specified by the user */
  bool simulate;       /* Specifies if we should continue with simulation */
  int  i;              /* Loop iterator */

  if( flag_use_command_line_debug ) {

    /* Decrement stmts_left if it is set */
    if( stmts_left > 0 ) {
      stmts_left--;
    }

    /* Decrement timesteps_left it is set and the last timestep differs from the current simulation time */
    if( (timesteps_left > 0) && (last_timestep != curr_sim_time) ) {
      timesteps_left--;
    }

    /* Record the last timestep seen */
    last_timestep = curr_sim_time;

    /* If we have no more statements to execute and we are not supposed to continuely run, prompt the user */
    if( (stmts_left == 0) && (timesteps_left == 0) && !dont_stop ) {

      /* Display current line that will be executed if we are not replaying */
      if( cli_replay_index == history_index ) {
        cli_display_current_stmt();
      }

      /* Get the next instruction from the user */
      cli_prompt_user();

    }

  }

}

/*!
 \param fname  Name of history file to read in

 \return Returns TRUE if history file was read without error; otherwise, returns FALSE.
*/
bool cli_read_hist_file( char* fname ) {

  bool  retval = TRUE;  /* Return value for this function */
  char* line;           /* Holds current line read from history file */
  FILE* hfile;          /* File containing history file */

  /* Make sure that this function was not called twice */
  assert( (cli_replay_index == 0) && !flag_use_command_line_debug );

  /* Read the contents of the history file, storing the read lines into the history array */
  if( (hfile = fopen( fname, "r" )) != NULL ) {

    while( util_readline( hfile, &line ) && retval ) {
      retval = cli_parse_input( line, FALSE, FALSE );
    }

    fclose( hfile );

    if( !retval ) {
      snprintf( user_msg, USER_MSG_LENGTH, "Specified -cli file \"%s\" is not a valid CLI history file", fname );
      print_output( user_msg, FATAL, __FILE__, __LINE__ );
    }

  } else {

    snprintf( user_msg, USER_MSG_LENGTH, "Unable to read history file \"%s\"", fname );
    print_output( user_msg, FATAL, __FILE__, __LINE__ );
    retval = FALSE;

  }

  return( retval );

}

/*
 $Log$
 Revision 1.5  2007/04/12 20:54:55  phase1geo
 Adding cli > output when replaying and adding back all of the functions (since
 the cli > prompt helps give it context.  Fixing bugs in simulation core.
 Checkpointing.

 Revision 1.4  2007/04/12 14:28:35  phase1geo
 Adding ability to display the last <num> lines of history to the history
 command.  Changed behavior of 'display active_queue' and 'display delayed_queue'
 commands to output when replaying history.  Also modified look of the help
 output to hopefully make it easier to read.

 Revision 1.3  2007/04/12 04:15:40  phase1geo
 Adding history all command, added list command and updated the display current
 command to include statement output.

 Revision 1.2  2007/04/12 03:46:30  phase1geo
 Fixing bugs with CLI.  History and history file saving/loading is implemented
 and working as desired.

 Revision 1.1  2007/04/11 22:29:48  phase1geo
 Adding support for CLI to score command.  Still some work to go to get history
 stuff right.  Otherwise, it seems to be working.

*/

#endif