/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* $Id: unassmbl.c,v 1.3 2001/01/31 04:38:59 pete Exp $ */
/*
** Copyright (C) 1993 DJ Delorie, 24 Kirsten Ave, Rochester NH 03867-2954
**
** This file is distributed under the terms listed in the document
** "copying.dj", available from DJ Delorie at the address above.
** A copy of "copying.dj" should accompany this file; if not, a copy
** should be available from where this file was obtained.  This file
** may not be distributed without a verbatim copy of "copying.dj".
**
** This file is distributed WITHOUT ANY WARRANTY; without even the implied
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/* Modified by Morten Welinder, terra@diku.dk, for use with full screen
   debugger.  These changes are copyright 1994 by Morten Welinder.  */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "ed.h"
#include "unassmbl.h"
#include <debug/syms.h>

#include "nasm/disasm.h"

#define SOURCE_LIST

static char ubuf[4000], *ubufp;

/*------------------------------------------------------------------------*/

void
uputchar(char c)
{
  if (c == '\t')
  {
    do {
      *ubufp++ = ' ';
    } while ((ubufp-ubuf) % 8);
  }
  else
    *ubufp++ = c;
  *ubufp = 0;
}

#ifdef SOURCE_LIST
/*
** A little brute force hacking and hey presto! A source debugger!
** Courtesy of Kent Williams williams@herky.cs.uiowa.edu
**
** KNOWN BUGS:
** The program will summarily terminate if you run out
** of memory while you're looking for all the line offsets.  Since
** a two thousand line source file only creats an 8K array, and the
** symbol table goes into virtual memory, this shouldn't happen too
** often.
**
** One file is left open for reading indefinitely.
*/
#include <stdlib.h>
#include <string.h>
/*
** keep the source line offsets in virtual memory, so you can
** debug big programs
*/
extern word32 salloc(word32 size);
#define symsput(where,ptr,size)     memput(where,ptr,size)
#define symsget(where,ptr,size)     memget(where,ptr,size)

/*
** for each file encountered, keep an array of line start offsets
** so you can seek into the file to display the current line.
*/
typedef struct {
        char *filename;
        long *offsets;
	int line_count;
} line_info;

static line_info *files;
static int last_file = 0;

/*
** cache_fopen -- cache the most recently accessed source file
** so you aren't constantly reopening a new file
*/
static FILE *
myfopen1 (char *name)
{
  static char fname[80] = "";
  static FILE *current = NULL;
  char temp[80];

  _fixpath (name, temp);

  if (current != NULL && strcmp (fname,temp) == 0)
    return current;
  if (current != NULL)
    fclose(current);
  strcpy (fname,temp);
  current = fopen (temp,"rb");
  return current;
}

FILE *
cache_fopen(char *name)
{
  FILE *f;
  char *s, *t, u[80];
  int l;

  f = myfopen1 (name);
#ifdef FULLSCR
  s = source_path;

  while (f == NULL && s && *s)
    {
      t = index (s, ';');
      l = t ? t - s : strlen (s);
      strncpy (u, s, l);
      if (l > 0 && u[l - 1] != '/' && u[l - 1] != '\\')
	u[l++] = '/';
      u[l] = 0;
      strcat (u, name);
      f = myfopen1 (u);
      s = t;
      if (s) s++;
    }
#endif
  return f;
}

/*
** add_file -- add a file to the source line database
*/
static int
add_file(char *name) {
        FILE *f = cache_fopen(name);
        char c;
        long *lines,curpos;
        unsigned curline = 0;

        if(!f)
                return -1;

        if (files == 0)
          files = (line_info *)malloc(sizeof(line_info));
        else
          files = realloc(files, (last_file+1) * sizeof(line_info));

        files[last_file].filename = strdup(name);

        /*
        ** build an array of line offsets in real memory.
        */
        lines = malloc(sizeof(long));
        lines[curline++] = curpos = 0L;

        while((c = getc(f)) != EOF) {
                curpos++;
                if(c == '\n') {
                        lines = realloc(lines,sizeof(long)*(curline+1));
                        lines[curline++] = curpos;
                }
        }
        /*
        ** now move the whole array into virtual memory
        */
        files[last_file].offsets = lines;
        files[last_file].line_count = curline;

        last_file++;
        return 0;
}

static line_info *
find_file(char *name) {
        int i;
        for(i = 0; i < last_file; i++)
                if(strcmp(name,files[i].filename) == 0)
                        return &files[i];
        if(add_file(name) == -1)
                        return NULL;
        return find_file(name);
}


int
file_line_count (char *file)
{
  line_info *li = find_file (file);

  if (li)
    return li->line_count;
  else
    return 0;
}

/*
** put_source_line -- print the current source line, along with
** the line # and file name, if necessary.
*/
#ifdef FULLSCR
void put_source_line(int fmt, char *name, int line, char *result)
#else
void put_source_line(int fmt,char *name,int line)
#endif
{
        line_info *current = find_file(name);
        FILE *cur;
#ifdef FULLSCR
	/* See below */
	char *sbuf, *chp;
#endif
        if(current == NULL) {
        regular:
#ifdef FULLSCR
	  sbuf = alloca (strlen (name) + 10);
	  switch (fmt)
	    {
	    case 0:
	      sprintf (sbuf, " (%s#%d):", name, line);
	      break;
	    case 1:
	      sprintf (sbuf, "#%d:", line);
	      break;
	    case 2:
	      sprintf (sbuf, "%4d: ", line);
	      break;
	    }
	  strcat (result, sbuf);
#else
          if(fmt == 0)
                  printf(" (%s#%d):\n", name, line);
          else
                  printf("#%d:\n", line);
#endif
        } else {
#ifdef FULLSCR
	        /* I really hate set a maximum like this, but setting it
		   this high will hopefully not feel like a maximum.  */
                char buf[1000];
#else
                char buf[70];
#endif
                long offset;
                if((cur = cache_fopen(name)) == NULL)
                        goto regular;
                /*
                ** get the symbol out of virtual memory
                */
                offset = current->offsets[line-1];
                fseek(cur,offset,0);
                /*
                ** truncate line so it fits on screen.
                */
                fgets(buf,sizeof(buf)-2,cur);
#ifdef FULLSCR
                if ((chp = strchr (buf, '\n')))
		{
		  if (chp != buf && chp[-1] == '\r')
		    chp [-1] = '\0';
		  else
		    *chp = '\0';
		}
		sbuf = alloca (sizeof (buf) + 100);
		switch (fmt)
		  {
		  case 0:
		    sprintf (sbuf, " (%s#%d): %s", name, line, buf);
		    break;
		  case 1:
		    sprintf (sbuf, "#%d: %s", line, buf);
		    break;
		  case 2:
		    sprintf (sbuf, "%4d: %s", line, buf);
		    break;
		  }
		strcat (result, sbuf);
#else
                if(strchr(buf,'\n') == NULL)
                        strcat(buf,"\n");
                if(fmt == 0)
                        printf(" (%s#%d): %s", name, line,buf);
                else
                        printf("#%d: %s",line,buf);
#endif
        }
}

#endif

int last_unassemble_unconditional;
int last_unassemble_jump;
int last_unassemble_extra_lines;
/* Please read the above regarding this max.  */
char last_unassemble_text[1100];

char *
unassemble_proper (word32 v, int *len)
{
  char unasm_buf[1000];
  word8 buf[20];
  char *p;

  read_child(v, buf, 20);
  *len = disasm(buf, unasm_buf, 32, v, 0, 0);
  ubufp = ubuf;
  p = unasm_buf;
  while(*p) {
    uputchar(*p); p++;
  }
  return ubuf;
}

char *
unassemble_source (word32 v)
{
  word32 delta;
  char *name, *lname, *result = last_unassemble_text;
  int linenum;

  result[0] = '\0';

  name = syms_val2name (v, &delta);
  if (!delta && (name[0] != '0'))
    {
      strcat (result, name);
      lname = syms_val2line (v, &linenum, 1);
      if (lname)
	put_source_line (0, lname, linenum, result);
      else
	strcat (result, ":");
    }
  else
    {
      lname = syms_val2line (v, &linenum, 1);
      if (lname)
	put_source_line (1, lname, linenum, result);
      else
	return NULL;
    }
  return result;
}

