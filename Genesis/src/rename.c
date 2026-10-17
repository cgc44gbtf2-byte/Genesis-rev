/******************************************************************
 *                     RENAME.C Version 1.1                       *
 *        Written by Voltec (Voltec@cyberdude.com) 1998           *
 *      For Empire Of The Night (eotn.oaktree.co.uk 4100)         *
 *       All the usual DIKU/MERC/ROM/EOTN Licences apply          *
 *  If you use this code, drop me a line so I can see if people   *
 *                 Actually find this usefull.                    *
 *                         REMEMBER                               *
 *      Share and Enjoy!                      -Voltec 98          *
 ******************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include <unistd.h> 

/* Voltecs attempt at re-naming players in game... 
 * I don't know how effective this is, or wether it
 * causes any memory fragmentation problems or anything
 * But hey, it's worth a shot.  It works by changing
 * the players name, saving the p-file and then
 * using unlink to delete the old one....
 * You might want to put in a check to stop Imms
 * re-naming their piers or superiors...
 *
 *
 * Syntax to be...
 *
 * rename <char> <newname>
 */

bool valid_name(char *name);
bool check_parse_name(char *name);

void do_pcrename(CHAR_DATA *ch, char *argument)
{
   CHAR_DATA *victim;
   OBJ_DATA  *obj;
   OBJ_DATA  *obj_next;
   char       buf[MAX_STRING_LENGTH];
	char      arg1[MAX_STRING_LENGTH];
	char      arg2[MAX_STRING_LENGTH];
   char  old_name[MAX_STRING_LENGTH];
   char   strsave[MAX_STRING_LENGTH];

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);

   if ((arg1[0] =='\0') || (arg2[0]=='\0'))
      {
      send_to_char("Syntax : rename <char> <newname>\n\r", ch);
      return;
      }

   victim = get_char_world(ch, arg1);

   if (victim == NULL)
      {
      send_to_char("They must be playing.\n\r", ch);
      return;
      }

   if (IS_NPC(victim))
      {
      send_to_char("Not on NPC's\n\r", ch);
      return;
      }
        if ( get_trust( victim ) >= get_trust( ch ) )
        {
            send_to_char( "You failed.\n\r", ch );
            return;
        }

   if (valid_name(arg2))
      {
		sprintf(old_name, victim->name);
		sprintf(buf, "%s renamed to %s\n\r", old_name, arg2);
		send_to_char(buf,ch);
		sprintf(buf, "%s renamed %s to %s", ch->name, old_name, arg2);
		log_string( buf );
		send_to_char("You feel like a new person!\n\r", victim);
		/* Report this on wiznet aswell if you like... */

		for ( obj = victim->carrying; obj != NULL; obj = obj_next )
        	{
            	 obj_next = obj->next_content;
		 if (!str_cmp(obj->questowner, victim->name))
		 {
		  free_string(obj->questowner);
		  obj->questowner = strdup(capitalize(arg2));
    		 }
    	}
		free_string(victim->name);
		victim->name = strdup(capitalize(arg2));
	//	sprintf(victim->name, arg2); /* Save the new p-file */
		save_char_obj(victim);

		sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( old_name ) );
		unlink(strsave);             /* And delete the old one */
		return;
		}
	else
		{
		send_to_char("They cannot be known as that.\n\r", ch);
		return;
		}
}


bool valid_name(char *name)
{
	bool valid;
	char path[MAX_STRING_LENGTH];
	FILE *renamefp;

	valid = FALSE;

	fclose(fpReserve);
	sprintf(path, "%s%s", PLAYER_DIR, capitalize( name ) );

	if ( (renamefp = fopen(path, "r")) == NULL) /* Cant open the p-file, so player doesn't exist yet*/
		{
		valid = check_parse_name(name);
		}
	else
		{
		valid = FALSE;
		fclose(renamefp);
		}
   
   fpReserve = fopen( NULL_FILE, "r" );
   return valid;
}
