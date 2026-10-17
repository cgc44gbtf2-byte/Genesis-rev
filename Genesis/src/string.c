/***************************************************************************
 *  File: string.c                                                         *
 *  Usage: various functions for new NOTE system                           *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  Much of this code came from CircleMUD.                                 *
 *  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
 *                                                                         *
 *  Revised for Merc 2.1 by Jason Dinkel.                                  *
 *  Revised for Envy 1.0 by Jason Dinkel.				   *
 ***************************************************************************/

/***************************************************************************
 *  God Wars Mud improvements copyright (C) 1994, 1995, 1996 by Richard    *
 *  Woolcock.  This mud is NOT to be copied in whole or in part, or to be  *
 *  run without the permission of Richard Woolcock.  Nobody else has       *
 *  permission to authorise the use of this code.                          *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"



void string_edit( CHAR_DATA *ch, char **pString )
{
    send_to_char( "-=======================================-\n\r", ch );
    send_to_char( " Entering line editing mode.\n\r", ch );
    send_to_char( " Terminate with a ~ or @ on a blank line.\n\r", ch );
    send_to_char( "-=======================================-\n\r", ch );

    if ( *pString == NULL )
    {
        *pString = str_dup( "" );
    }
    else
    {
        **pString = '\0';
    }

    ch->desc->pString = pString;

    return;
}



void string_append( CHAR_DATA *ch, char **pString )
{
    send_to_char( "-=======- Entering APPEND Mode -========-\n\r", ch );
    send_to_char( "    Type .h on a new line for help\n\r", ch );
    send_to_char( " Terminate with a ~ or @ on a blank line.\n\r", ch );
    send_to_char( "-=======================================-\n\r", ch );

    if ( *pString == NULL )
    {
        *pString = str_dup( "" );
    }
    send_to_char( *pString, ch );
    
    if ( **pString == '\0' || (*pString)[strlen( *pString ) - 1] != '\r' )
    send_to_char( "\n\r", ch );

    ch->desc->pString = pString;

    return;
}



char * string_replace( char * orig, char * old, char * new )
{
    char xbuf[MAX_STRING_LENGTH];
    char *match = strstr( orig, old );
    char *result;
    size_t prefix, replacement, suffix;

    if ( match == NULL || *old == '\0' )
        return orig;

    prefix = (size_t)(match - orig);
    replacement = strlen( new );
    suffix = strlen( match + strlen(old) );
    if ( prefix >= sizeof(xbuf)
      || replacement >= sizeof(xbuf) - prefix
      || suffix >= sizeof(xbuf) - prefix - replacement )
    {
        bug( "String_replace: result too long.", 0 );
        return orig;
    }

    memcpy( xbuf, orig, prefix );
    memcpy( xbuf + prefix, new, replacement );
    memcpy( xbuf + prefix + replacement, match + strlen(old), suffix + 1 );
    result = str_dup( xbuf );
    free_string( orig );
    return result;
}



void string_add( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( *argument == '.' )
    {
        char arg1 [MAX_INPUT_LENGTH];
        char arg2 [MAX_INPUT_LENGTH];
        char arg3 [MAX_INPUT_LENGTH];

        argument = one_argument( argument, arg1 );
        argument = first_arg( argument, arg2, FALSE );
        argument = first_arg( argument, arg3, FALSE );

        if ( !str_cmp( arg1, ".c" ) )
        {
            send_to_char( "String cleared.\n\r", ch );
            **ch->desc->pString = '\0';
            return;
        }

        if ( !str_cmp( arg1, ".s" ) )
        {
            send_to_char( "String so far:\n\r", ch );
            send_to_char( *ch->desc->pString, ch );
            return;
        }

        if ( !str_cmp( arg1, ".r" ) )
        {
            if ( arg2[0] == '\0' )
            {
                send_to_char( "usage:  .r \"old string\" \"new string\"\n\r", ch );
                return;
            }

            char *original = *ch->desc->pString;
            *ch->desc->pString = string_replace( original, arg2, arg3 );
            if ( *ch->desc->pString == original )
            {
                send_to_char( "Replacement not applied: no match or result too long.\n\r", ch );
                return;
            }
            snprintf( buf, sizeof(buf), "'%s' replaced with '%s'.\n\r", arg2, arg3 );
            send_to_char( buf, ch );
            return;
        }

        if ( !str_cmp( arg1, ".f" ) )
        {
            char *original = *ch->desc->pString;
            *ch->desc->pString = format_string( *ch->desc->pString );
            send_to_char( *ch->desc->pString == original
                ? "String too long to format.\n\r" : "String formatted.\n\r", ch );
            return;
        }
        
        if ( !str_cmp( arg1, ".h" ) )
        {
            send_to_char( 
"Sedit help (commands available from the beginning of a new line only):  \n\r", ch );
            send_to_char( 
".r 'old string' 'new string'   - replace a substring with a new string  \n\r", ch );
            send_to_char( 
"                                (requires '', \"\" around the arguments)\n\r", ch );
            send_to_char( 
".h                             - get help (this info)                   \n\r", ch );
            send_to_char( 
".s                             - show string so far                     \n\r", ch );
            send_to_char( 
".f                             - format (word wrap) string              \n\r", ch );
            send_to_char( 
".c                             - clear string so far                    \n\r", ch );
            send_to_char( 
"@  or  ~                       - end string editor (end string)         \n\r", ch );
            return;
        }
            

        send_to_char( "SEdit:  Invalid dot command.\n\r", ch );
        return;
    }

    if ( *argument == '~' || *argument == '@' )
    {
        ch->desc->pString = NULL;
        return;
    }

    if ( strlen( *ch->desc->pString ) >= sizeof(buf) )
    {
        send_to_char( "Existing string too long to edit; line not added.\n\r", ch );
        return;
    }
    strcpy( buf, *ch->desc->pString );

    if ( strlen( argument ) >= ( MAX_STRING_LENGTH - 4 )
      || strlen( buf ) >= ( MAX_STRING_LENGTH - 4 ) - strlen( argument ) )
    {
        send_to_char( "String too long, truncating.\n\r", ch );
        strncat( buf, argument, sizeof(buf) - strlen(buf) - 1 );
        free_string( *ch->desc->pString );
        *ch->desc->pString = str_dup( buf );
        ch->desc->pString = NULL;
        return;
    }

    strcat( buf, argument );
    strcat( buf, "\n\r" );
    free_string( *ch->desc->pString );
    *ch->desc->pString = str_dup( buf );
    return;
}



/*
 * Thanks to Kalgen for the new procedure (no more bug!)
 * Original wordwrap() written by Surreality.
 */
static bool append_checked( char *dest, size_t capacity, const char *src )
{
    size_t used = strlen( dest );
    size_t added = strlen( src );

    if ( used >= capacity || added >= capacity - used )
        return FALSE;
    memcpy( dest + used, src, added + 1 );
    return TRUE;
}

char *format_string( char *oldstring /*, bool fSpace */)
{
  char xbuf[MAX_STRING_LENGTH];
  char xbuf2[MAX_STRING_LENGTH];
  char *rdesc;
  char *result;
  long i=0;
  bool cap=TRUE;
  
  xbuf[0]=xbuf2[0]=0;
  
  i=0;
  
  for (rdesc = oldstring; *rdesc; rdesc++)
  {
    /* One input character may expand to punctuation, a quote and two spaces. */
    if (i > (long)sizeof(xbuf) - 5)
      goto too_long;
    if (*rdesc=='\n')
    {
      if (i > 0 && xbuf[i-1] != ' ')
      {
        xbuf[i]=' ';
        i++;
      }
    }
    else if (*rdesc=='\r') ;
    else if (*rdesc==' ')
    {
      if (i > 0 && xbuf[i-1] != ' ')
      {
        xbuf[i]=' ';
        i++;
      }
    }
    else if (*rdesc==')')
    {
      if (i >= 3 && xbuf[i-1]==' ' && xbuf[i-2]==' ' &&
          (xbuf[i-3]=='.' || xbuf[i-3]=='?' || xbuf[i-3]=='!'))
      {
        xbuf[i-2]=*rdesc;
        xbuf[i-1]=' ';
        xbuf[i]=' ';
        i++;
      }
      else
      {
        xbuf[i]=*rdesc;
        i++;
      }
    }
    else if (*rdesc=='.' || *rdesc=='?' || *rdesc=='!') {
      if (i >= 3 && xbuf[i-1]==' ' && xbuf[i-2]==' ' &&
          (xbuf[i-3]=='.' || xbuf[i-3]=='?' || xbuf[i-3]=='!')) {
        xbuf[i-2]=*rdesc;
        if (*(rdesc+1) != '\"')
        {
          xbuf[i-1]=' ';
          xbuf[i]=' ';
          i++;
        }
        else
        {
          xbuf[i-1]='\"';
          xbuf[i]=' ';
          xbuf[i+1]=' ';
          i+=2;
          rdesc++;
        }
      }
      else
      {
        xbuf[i]=*rdesc;
        if (*(rdesc+1) != '\"')
        {
          xbuf[i+1]=' ';
          xbuf[i+2]=' ';
          i += 3;
        }
        else
        {
          xbuf[i+1]='\"';
          xbuf[i+2]=' ';
          xbuf[i+3]=' ';
          i += 4;
          rdesc++;
        }
      }
      cap = TRUE;
    }
    else
    {
      xbuf[i]=*rdesc;
      if ( cap )
        {
          cap = FALSE;
          xbuf[i] = UPPER( xbuf[i] );
        }
      i++;
    }
  }
  xbuf[i]=0;
  strcpy(xbuf2,xbuf);
  
  rdesc=xbuf2;
  
  xbuf[0]=0;
  
  for ( ; ; )
  {
    for (i=0; i<77; i++)
    {
      if (!*(rdesc+i)) break;
    }
    if (i<77)
    {
      break;
    }
    for (i=(xbuf[0]?76:73) ; i ; i--)
    {
      if (*(rdesc+i)==' ') break;
    }
    if (i)
    {
      *(rdesc+i)=0;
      if (!append_checked(xbuf, sizeof(xbuf), rdesc)
       || !append_checked(xbuf, sizeof(xbuf), "\n\r"))
        goto too_long;
      rdesc += i+1;
      while (*rdesc == ' ') rdesc++;
    }
    else
    {
      char split = rdesc[75];
      bug ("No spaces", 0);
      *(rdesc+75)=0;
      if (!append_checked(xbuf, sizeof(xbuf), rdesc)
       || !append_checked(xbuf, sizeof(xbuf), "-\n\r"))
        goto too_long;
      /* Restore the split character; the old code silently dropped it. */
      rdesc[75] = split;
      rdesc += 75;
    }
  }
   i = strlen(rdesc);
   while (i > 0 && (rdesc[i-1]==' ' || rdesc[i-1]=='\n' || rdesc[i-1]=='\r'))
     i--;
   rdesc[i]=0;
   if (!append_checked(xbuf, sizeof(xbuf), rdesc))
     goto too_long;
   if (strlen(xbuf) < 2 || xbuf[strlen(xbuf)-2] != '\n')
     if (!append_checked(xbuf, sizeof(xbuf), "\n\r"))
       goto too_long;

  result = str_dup(xbuf);
  free_string(oldstring);
  return result;

too_long:
  bug("Format_string: result too long.", 0);
  return oldstring;
}



/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes, parenthesis (barring ) ('s) and percentages.
 * arg_first must hold MAX_INPUT_LENGTH bytes.
 */
char *first_arg( char *argument, char *arg_first, bool fCase )
{
    char cEnd;
    size_t length = 0;

    while ( *argument == ' ' )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"'
      || *argument == '%'  || *argument == '(' )
    {
        if ( *argument == '(' )
        {
            cEnd = ')';
            argument++;
        }
        else cEnd = *argument++;
    }

    while ( *argument != '\0' )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
        if ( length < MAX_INPUT_LENGTH - 1 )
        {
            arg_first[length++] = fCase ? LOWER(*argument) : *argument;
        }
	argument++;
    }
    arg_first[length] = '\0';

    while ( *argument == ' ' )
	argument++;

    return argument;
}



char * string_unpad( char * argument )
{
    char *s;
    char *result;
    size_t length;

    s = argument;

    while ( *s == ' ' )
        s++;

    length = strlen( s );
    while ( length > 0 && s[length-1] == ' ' )
        length--;
    /* Do not modify s: it may be a shared, read-only database string. */
    if ( length == 0 )
        result = str_dup( "" );
    else
    {
        result = alloc_mem( length + 1 );
        memcpy( result, s, length );
        result[length] = '\0';
    }
    free_string( argument );
    return result;
}



char * string_proper( char * argument )
{
    char *s;

    s = argument;

    while ( *s != '\0' )
    {
        if ( *s != ' ' )
        {
            *s = UPPER(*s);
            while ( *s != ' ' && *s != '\0' )
                s++;
        }
        else
        {
            s++;
        }
    }

    return argument;
}



long arg_count( char *argument )
{
    long total;
    char *s;

    total = 0;
    s = argument;

    while ( *s != '\0' )
    {
        if ( *s != ' ' )
        {
            total++;
            while ( *s != ' ' && *s != '\0' )
                s++;
        }
        else
        {
            s++;
        }
    }

    return total;
}
