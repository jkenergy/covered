#ifndef __EXPR_H__
#define __EXPR_H__

/*!
 \file     expr.h
 \author   Trevor Williams  (trevorw@charter.net)
 \date     12/1/2001
 \brief    Contains functions for handling expressions.
*/

#include <stdio.h>

#include "defines.h"


/*! \brief Creates an expression value and initializes it. */
void expression_create_value( expression* exp, int width, int lsb, bool data );

/*! \brief Creates new expression. */
expression* expression_create( expression* right, expression* left, int op, int id, int line, bool data );

/*! \brief Sets the specified expression value to the specified vector value. */
void expression_set_value( expression* exp, vector* vec );

/*! \brief Recursively resizes specified expression tree leaf node. */
void expression_resize( expression* expr, bool recursive );

/*! \brief Returns expression ID of this expression. */
int expression_get_id( expression* expr );

/*! \brief Writes this expression to the specified database file. */
void expression_db_write( expression* expr, FILE* file, char* scope );

/*! \brief Reads current line of specified file and parses for expression information. */
bool expression_db_read( char** line, module* curr_mod, bool eval );

/*! \brief Reads and merges two expressions and stores result in base expression. */
bool expression_db_merge( expression* base, char** line, bool same );

/*! \brief Displays the specified expression information. */
void expression_display( expression* expr );

/*! \brief Performs operation specified by parameter expression. */
void expression_operate( expression* expr );

/*! \brief Performs recursive expression operation (parse mode only). */
void expression_operate_recursively( expression* expr );

/*! \brief Returns a compressed, 1-bit representation of the value after a unary OR. */
int expression_bit_value( expression* expr );

/*! \brief Returns TRUE if specified expression is found to contain all static leaf expressions. */
bool expression_is_static_only( expression* expr );

/*! \brief Deallocates memory used for expression. */
void expression_dealloc( expression* expr, bool exp_only );


/*
 $Log$
 Revision 1.18  2002/11/27 03:49:20  phase1geo
 Fixing bugs in score and report commands for regression.  Finally fixed
 static expression calculation to yield proper coverage results for constant
 expressions.  Updated regression suite and development documentation for
 changes.

 Revision 1.17  2002/11/05 00:20:07  phase1geo
 Adding development documentation.  Fixing problem with combinational logic
 output in report command and updating full regression.

 Revision 1.16  2002/10/31 23:13:47  phase1geo
 Fixing C compatibility problems with cc and gcc.  Found a few possible problems
 with 64-bit vs. 32-bit compilation of the tool.  Fixed bug in parser that
 lead to bus errors.  Ran full regression in 64-bit mode without error.

 Revision 1.15  2002/10/29 19:57:50  phase1geo
 Fixing problems with beginning block comments within comments which are
 produced automatically by CVS.  Should fix warning messages from compiler.

 Revision 1.14  2002/10/23 03:39:07  phase1geo
 Fixing bug in MBIT_SEL expressions to calculate the expression widths
 correctly.  Updated diagnostic testsuite and added diagnostic that
 found the original bug.  A few documentation updates.

 Revision 1.13  2002/10/11 04:24:02  phase1geo
 This checkin represents some major code renovation in the score command to
 fully accommodate parameter support.  All parameter support is in at this
 point and the most commonly used parameter usages have been verified.  Some
 bugs were fixed in handling default values of constants and expression tree
 resizing has been optimized to its fullest.  Full regression has been
 updated and passes.  Adding new diagnostics to test suite.  Fixed a few
 problems in report outputting.

 Revision 1.12  2002/09/29 02:16:51  phase1geo
 Updates to parameter CDD files for changes affecting these.  Added support
 for bit-selecting parameters.  param4.v diagnostic added to verify proper
 support for this bit-selecting.  Full regression still passes.

 Revision 1.11  2002/09/25 02:51:44  phase1geo
 Removing need of vector nibble array allocation and deallocation during
 expression resizing for efficiency and bug reduction.  Other enhancements
 for parameter support.  Parameter stuff still not quite complete.

 Revision 1.10  2002/09/19 05:25:19  phase1geo
 Fixing incorrect simulation of static values and fixing reports generated
 from these static expressions.  Also includes some modifications for parameters
 though these changes are not useful at this point.

 Revision 1.9  2002/08/19 04:34:07  phase1geo
 Fixing bug in database reading code that dealt with merging modules.  Module
 merging is now performed in a more optimal way.  Full regression passes and
 own examples pass as well.

 Revision 1.8  2002/07/10 03:01:50  phase1geo
 Added define1.v and define2.v diagnostics to regression suite.  Both diagnostics
 now pass.  Fixed cases where constants were not causing proper TRUE/FALSE values
 to be calculated.

 Revision 1.7  2002/06/28 03:04:59  phase1geo
 Fixing more errors found by diagnostics.  Things are running pretty well at
 this point with current diagnostics.  Still some report output problems.

 Revision 1.6  2002/06/25 03:39:03  phase1geo
 Fixed initial scoring bugs.  We now generate a legal CDD file for reporting.
 Fixed some report bugs though there are still some remaining.

 Revision 1.5  2002/06/21 05:55:05  phase1geo
 Getting some codes ready for writing simulation engine.  We should be set
 now.

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

#endif

