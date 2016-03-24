/*
 * bsed - binary stream editor.  Written Feb 1987 by David W. Dykstra
 * Copyright (C) 1987-2002 by Dave Dykstra and Lucent Technologies
 *
 */

/* If given a search=replace string, both input and output file names 	*/
/*   must be present; else only the input file name must be present.	*/
/* Both input and output file names can be "-" to signify standard 	*/
/*   input or standard output respectively.				*/

char* Version = "@(#) bsed 1.0, November 16, 1989";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>
#include <limits.h>

char* arg0;		/* program name argv[0] */

FILE* ifile,*ofile;		/* input and output files */
char* ifilenm,*ofilenm; /* input and output file names */
#define REPLACESIZE	128	/* maximum replacement string size */
unsigned char* search;		/* search string */
unsigned char sbuf[REPLACESIZE+1+1]; /* search string buffer */
int slen;			/* search string length */
unsigned char* replace;		/* replace string */
unsigned char rbuf[REPLACESIZE+1+1]; /* replace string buffer */
int rlen;			/* replace string length */

int silent = 0;			/* silent flag */
int nowarn = 0;			/* nowarn flag */
int verbose = 0;		/* verbose flag */
int inplace = 0;    /* inplace flag */
int minmatch = 1;		/* minimum match count */
int maxmatch = -1;		/* maxmatch count */
int match = 0;			/* match count */
int zeropad = 0;    /* zero-pad replacement string if it is smaller */
int nulterm = 0;   /* zero-terminate both strings */

#define CTXTSIZE	5	/* size of left and right context  */
unsigned char ltxt[CTXTSIZE+1],rtxt[CTXTSIZE+1];
int ltlen = 0;
int rtlen = 0;

int stack[REPLACESIZE+CTXTSIZE+1]; /* saved character stack */
int topstack = 0;

static int convert(unsigned char* s,unsigned char* o);
static unsigned char* dump(unsigned char* str,int len);

#define mygetc()	((topstack == 0) ? getc(ifile) : stack[--topstack])
#define myungetc(c)	(stack[topstack++] = c)
/* myputc saves the character in the ltxt arrray for context printing */
#ifdef lint
#define myputc(c) putc(c,ofile)
#else
#define myputc(c)	do { \
    register unsigned char *p; \
    for(p = &ltxt[0]; p < &ltxt[CTXTSIZE-1]; p++)\
      *p = *(p+1); \
    *p = c; \
    if (ltlen < CTXTSIZE) \
      ltlen++; \
    if (ofile != NULL) \
      putc(c,ofile); \
  } while(0);
#endif

void usage()
{
  fprintf(stderr,
          "Usage:\t%s [-0ivswz] [-m [minmatch-]maxmatch] search=replace infile outfile\n",
          arg0);
  fprintf(stderr,
          "\t%s [-0ivswz] [-m [minmatch-]maxmatch] search infile\n",arg0);
  fprintf(stderr, "%s\n", Version);
  exit(2);
}

int main(int argc,char* argv[])
{
  register int c;
  register long cnt;
  register unsigned char* s;
  unsigned char* dump();
  extern char* optarg;
  extern int optind;
  int errflg = 0;

  if ((arg0 = strrchr(*argv,'/')) == NULL)
    arg0 = *argv;
  else
    arg0++;
  *argv = arg0;

  ofilenm = NULL;
  ofile = NULL;

  while ((c = getopt(argc, (char**)argv, "isvwm:z0")) != EOF) {
    switch (c) {
      case 's':
        silent++;
        break;
      case 'i':
        inplace++;
        break;
      case 'w':
        nowarn++;
        break;
      case 'v':
        verbose++;
        break;
      case 'z':
        zeropad++;
        break;
      case '0':
        nulterm++;
        break;
      case 'm':
        if ((c = sscanf(optarg,"%d-%d",&minmatch,&maxmatch)) <= 0) {
          fprintf(stderr,"%s: bad max match count\n",arg0);
          errflg++;
        }
        if (c == 1) {
          maxmatch = minmatch;
          minmatch = 1;
        }
        if (minmatch < 1) {
          fprintf(stderr,"%s: minmatch must be greater than 0\n",arg0);
          errflg++;
        }
        if (maxmatch < minmatch) {
          fprintf(stderr,"%s: maxmatch must not be less than minmatch\n",
                  arg0);
          errflg++;
        }
        break;
      default:
        errflg++;
        break;
    }
  }
  if (errflg)
    usage();

  argc -= optind;
  argv +=optind;

  if (argc < 2)
    usage();

  search = (unsigned char*)*argv++;
  ifilenm = *argv++;

  if ((s = (unsigned char*) strchr((char*)search,'=')) != NULL) {
    static char outnm[PATH_MAX+1];
    if (argc != 3 && !inplace)
      usage();
    strncpy(outnm, ifilenm, PATH_MAX);
    strncat(outnm, ".bsed", PATH_MAX);
    ofilenm = (inplace ? outnm : *argv);
    *s = '\0';
    replace = s + 1;
  } else {
    if (argc != 2)
      usage();
    replace = NULL;
  }

  if ((slen = convert(search,sbuf)) == -1) {
    fprintf(stderr,"%s: search string too long\n",arg0);
    usage();
  }
  search = &sbuf[0];
  if (slen == 0) {
    fprintf(stderr,"%s: null search string\n",arg0);
    usage();
  }

  if (replace != NULL) {
    if ((rlen = convert(replace, rbuf)) == -1) {
      fprintf(stderr,"%s: replace string too long\n",arg0);
      usage();
    }
    replace = &rbuf[0];
    if (rlen == 0) {
      fprintf(stderr,"%s: null replace string\n",arg0);
      usage();
    }
    if (nulterm) {
      sbuf[slen++] = '\0';
      rbuf[rlen++] = '\0';
    }
    if ((slen != rlen)) {
      if (zeropad) {
        if (rlen < slen) {
          memset(&rbuf[rlen], 0, slen - rlen);
          rlen = slen;
        }
        if (rlen > slen) {
          memset(&sbuf[slen], 0, rlen - slen);
          slen = rlen;
          if (!nowarn)
            fprintf(stderr,
                    "%s: warning: zero-padding search string. this is probably not what you intended.\n",
                    arg0);
        }
      } else if (!nowarn)
        fprintf(stderr,
                "%s: warning: search and replace strings not same length\n",
                arg0);
    }
  }

  if (verbose) {
    fprintf(stderr,"search\t%s\n",dump(search,slen));
    if (replace != NULL)
      fprintf(stderr,"replace\t%s\n",dump(replace,rlen));
  }

  if (strcmp(ifilenm,"-") == 0)
    ifile = stdin;
  else {
    if ((ifile = fopen(ifilenm,"r")) == NULL) {
      fprintf(stderr,"%s: cannot open %s for input\n",arg0,ifilenm);
      exit(3);
    }
  }

  if (ofilenm != NULL) {
    if (strcmp(ofilenm,"-") == 0)
      ofile = stdout;
    else {
      if ((ofile = fopen(ofilenm,"w")) == NULL) {
        fprintf(stderr,"%s: cannot open %s for output\n",arg0,ofilenm);
        exit(3);
      }
    }
  }

  cnt = 0;
  s = search;
  do {
    cnt++;
    if (((c = mygetc()) == *s) && ((maxmatch < 0) || (match < maxmatch))) {
      register long savcnt;
      register unsigned char* end;
      static int savbuf[REPLACESIZE];
      int savlen;


      savcnt = cnt;
      savlen = 0;
      savbuf[savlen++] = c;
      end = s + slen;

      while (1) {
        if (++s == end) {
          register int i;

          match++;
          if (match < minmatch) {
            for (i = 0; i < savlen; i++)
              myputc(savbuf[i]);
          } else {
            if (verbose) {
              register unsigned char* p;
              register int c=0;

              /* read next few characters for right context */
              rtlen = 0;
              for (p = &rtxt[0]; p < &rtxt[CTXTSIZE]; p++) {
                c = mygetc();
                if (c == -1)
                  break;
                *p = c;
                rtlen++;
              }
              *p = '\0';
              for (p = &rtxt[rtlen-1]; p >= &rtxt[0]; p--)
                myungetc(*p);
              if (c == -1)
                myungetc(c);
              /* left context may have leading nulls */
              /* double printf call: dump uses static buffer */
              fprintf(stderr,"%-20s -- %8.8lx -- ",
                      dump(&ltxt[CTXTSIZE-ltlen],ltlen),savcnt-1);
              fprintf(stderr,"%s\n",dump(rtxt,rtlen));
              fflush(stderr);
            }
            if (replace == NULL) {
              /* for the sake of the context prints; ofile is */
              /*   NULL so nothing will actually be written   */
              for (i = 0; i < savlen; i++)
                myputc(savbuf[i]);
            } else {
              for (i = 0; i < rlen; i++)
                myputc(*(replace+i));
            }
          }
          break;
        } else {
          cnt++;
          c = mygetc();
          savbuf[savlen++] = c; /* save for possible later use */
          if (c != *s) {
            register int i;

            /* non-match, restore saved chars */
            for (i = savlen-1; i >= 1; i--)
              myungetc(savbuf[i]);
            /* and write out the first one */
            c = savbuf[0];
            myputc(c);
            cnt = savcnt;
            break;
          }
        }
      }
      s = search;
    } else if (c != EOF)
      myputc(c);

  } while (c != EOF);

  match -= (minmatch - 1);
  if (!silent)
    fprintf(stderr,"%d %s\n",match,(match == 1 ? "match" : "matches"));

  if (inplace) {
    struct stat st;
    fstat(fileno(ifile), &st);
    fclose(ifile);
    unlink(ifilenm);
    chmod(ofilenm, st.st_mode);
    if (geteuid() == 0)
      chown(ofilenm, st.st_uid, st.st_gid);
    rename(ofilenm, ifilenm);
  }
  return (match <= 0 ? 1 : 0);
}

/***********************************************************************
*
* convert: convert a hex/ascii string into a string of characters
*
* arguments: s - pointer to string to convert
*	     o - pointer to output string
*
* returns: number of bytes convert
*
* algorithm: If string begins with a digit (0-9), assume a hex number is
*	beginning.  Continue converting hex digits (0-f) into binary
*	until a non-hex digit begins, unless the string began with
*	"0x" or "0x": in that case just throw away the 0x.  Every
*	2 hex digits makes a byte; if there is an odd number then
*	the last digit is taken as the low nibble of a byte.  Anytime
*	a backslash (\) is encountered, consider it to be a break
*	in the conversion process and treat it as if starting a new
*	string.  If another backslash is immediately following it,
*	insert a backslash.  Once converting an ascii string, continue
*	converting it until a backslash or end of string.
*	NOTE: be sure to escape the backslash from the shell.
*
***********************************************************************/

#define isnum(c)	(((c) >= '0') && ((c) <= '9'))
#define ishex(c)	(isnum(c) || (((c) >= 'a') && ((c) <= 'f')) || \
                   (((c) >= 'A') && ((c) <= 'F')))
#define hexval(c)	(isnum(c) ? ((c) & 0xf) : (((c) & 0xf) + 9))

int convert(unsigned char* s,unsigned char* o)
{
  register unsigned char* p;
  register unsigned char c,t;
  register unsigned char* end;

  p = o;
  end = p + REPLACESIZE;
  c = *s++;
  while (c != '\0') {
    if (isnum(c)) {
      t = hexval(c);
      if (c == '0') {
        if (((c = *s++) == 'x') || (c == 'X')) {
          c = *s++;
          if (!ishex(c))
            continue;
          t = hexval(c);
          c = *s++;
        }
      } else
        c = *s++;
      while (ishex(c)) {
        t = (t << 4) + hexval(c);
        c = *s++;
        if (ishex(c)) {
          *p++ = t;
          if (p >= end)
            return (-1);
          t = hexval(c);
          c = *s++;
        }
      }
      *p++ = t;
    } else if (c == '\\') {
      /* backslash is special delimiter */
      if ((c = *s++) == '\\') {
        /* if double backslash put in backslash */
        *p++ = c;
        c = *s++;
      }
    } else {
      /* regular ascii character */
      do {
        *p++ = c;
        if (p >= end)
          return (-1);
        c = *s++;
      } while ((c != '\0') && (c != '\\'));
    }
    if (p >= end)
      return (-1);
  }
  *p = '\0';
  return (p - o);
}

/***********************************************************************
*
* dump: hex/ascii dump a string
*
* arguments: str - pointer to string to dump
*	     len - number of bytes to dump (string may contain nulls)
*
* returns: pointer to string to dump
*
***********************************************************************/

#define NUMDUMP REPLACESIZE
#define ascval(c)	(((c) < 10) ? ((c) + '0') : ((c) - 10 + 'a'))
#define isprint(c)	(((c) >= ' ') && ((c) <= '~'))

unsigned char*
dump(unsigned char* str,int len)
{
  static unsigned char buf[NUMDUMP*4+1];
  register unsigned char c,*p,*s,*end;

  if (len > NUMDUMP)
    len = NUMDUMP;
  end = str + len;
  p = &buf[0];

  for (s = str; s < end; s++) {
    c = (*s) >> 4;
    *p++ = ascval(c);
    c = (*s) & 0xf;
    *p++ = ascval(c);
    *p++ = ' ';
  }

  for (s = str; s < end; s++) {
    c = *s;
    if (isprint(c))
      *p++ = c;
    else
      *p++ = '.';
  }
  *p++ = '\0';
  return (&buf[0]);
}

