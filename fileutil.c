 /* ---------------------------------------------------------------------- *
 * fileutil.c
 * This file is part of lincity.
 * Lincity is copyright (c) I J Peters 1995-1997, (c) Greg Sharp 1997-2001.
 * ---------------------------------------------------------------------- */
#include "lcconfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> /* XXX: GCS FIX: What does configure need to know? */
#include "lcintl.h"
#include "lcstring.h"
#include "ldsvgui.h"

/* XXX: Where are SVGA specific includes? */

/* this is for OS/2 - RVI */
#ifdef __EMX__
#include <sys/select.h>
#include <X11/Xlibint.h>      /* required for __XOS2RedirRoot */
#define chown(x,y,z)
#define OS2_DEFAULT_LIBDIR "/XFree86/lib/X11/lincity"
#endif

#ifdef ENABLE_BINRELOC
#include "prefix.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#if defined (TIME_WITH_SYS_TIME)
#include <time.h>
#include <sys/time.h>
#else
#if defined (HAVE_SYS_TIME_H)
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

#if defined (WIN32)
#include <winsock.h>
#if defined (__BORLANDC__)
#include <dir.h>
#include <dirent.h>
#include <dos.h>
#endif
#include <io.h>
#include <direct.h>
#include <process.h>
#include "lcwin32.h"
#endif

#if defined (HAVE_DIRENT_H)
#include <dirent.h>
#define NAMLEN(dirent) strlen((dirent)->d_name)
#else
#define dirent direct
#define NAMLEN(dirent) (dirent)->d_namlen
#if defined (HAVE_SYS_NDIR_H)
#include <sys/ndir.h>
#endif
#if defined (HAVE_SYS_DIR_H)
#include <sys/dir.h>
#endif
#if defined (HAVE_NDIR_H)
#include <ndir.h>
#endif
#endif

#include <ctype.h>
#include "common.h"
#ifdef LC_X11
#include <X11/cursorfont.h>
#include <lcx11.h>
#endif
#include "lctypes.h"
#include "lin-city.h"
#include "cliglobs.h"
#include "engglobs.h"
#include "fileutil.h"

/* GCS: This is from dcgettext.c in the gettext package.      */
/* XPG3 defines the result of `setlocale (category, NULL)' as:
   ``Directs `setlocale()' to query `category' and return the current
     setting of `local'.''
   However it does not specify the exact format.  And even worse: POSIX
   defines this not at all.  So we can use this feature only on selected
   system (e.g. those using GNU C Library).  */
#ifdef _LIBC
# define HAVE_LOCALE_NULL
#endif

#define DEBUG_PRINTF_TO_FILE 0
void debug_printf (char* fmt, ...);

/* ---------------------------------------------------------------------- *
 * Private Fn Prototypes
 * ---------------------------------------------------------------------- */
void dump_screen (void);
void verify_package (void);
static const char *guess_category_value (int category, 
					 const char *categoryname);

/* ---------------------------------------------------------------------- *
 * Public Global Variables
 * ---------------------------------------------------------------------- */
#ifdef LIBDIR
#undef LIBDIR
#endif
char LIBDIR[LC_PATH_MAX];

char *lc_save_dir;
int lc_save_dir_len;
static char *lc_temp_filename;

char given_scene[LC_PATH_MAX];
char colour_pal_file[LC_PATH_MAX];
char opening_pic[LC_PATH_MAX];
char graphic_path[LC_PATH_MAX];
char fontfile[LC_PATH_MAX];
char opening_path[LC_PATH_MAX];
char help_path[LC_PATH_MAX];
char message_path[LC_PATH_MAX];
char lc_textdomain_directory[LC_PATH_MAX];
char lincityrc_file[LC_PATH_MAX];

/* The variable make_dir_ok_flag has 2 uses.

   First, it is initialized to 1, and set to zero if the directory 
   already exists.  Later, if it is found to be 1, the "create directory 
   help page (ask-dir) is display, and then set to zero.  

   Next, in the help handler, if the user says it's OK to create the 
   directory, it is reset to 1.  Finally, in the main loop, if it is 
   found to have value 1, the directory is created and it is reset to 0.
   */
#if defined (commentout)
int make_dir_ok_flag;
#endif

/* ---------------------------------------------------------------------- *
 * Public Functions
 * ---------------------------------------------------------------------- */
#if defined (__BORLANDC__)
int
_chdir (const char *dirname)
{
    return chdir (dirname);
}

int 
_access (const char *path, int mode)
{
    return access (path, mode)
}
#endif

/* Executes a system command */
int
execute_command (char *cmd, char *p1, char *p2, char *p3)
{
  char *sys_cmd = (char *) malloc (strlen (cmd) + strlen (p1) + strlen (p2)
				   + strlen (p3) + 4);
  int ret_value;

  if (sys_cmd == 0) {
    malloc_failure ();
  }
  sprintf (sys_cmd, "%s %s %s %s", cmd, p1, p2, p3);
  ret_value = system (sys_cmd);
/* fprintf(stderr, "system(%s)=%i\n", sys_cmd, ret_value); */
  free (sys_cmd);
  return ret_value;
}

void
copy_file (char *f1, char *f2)
{
  int ret_value = execute_command ("cp", f1, f2, "");
  if (ret_value != 0)
    {
      /* GCS FIX:  Need to make do_error into var_args fn? */
      printf ("Tried to cp %s %s\n", f1, f2);
      do_error ("Can't copy requested file");
    }
}

void
gunzip_file (char *f1, char *f2)
{
  int ret_value = execute_command ("gzip -c -d", f1, ">", f2);
  if (ret_value != 0)
    {
      /* GCS FIX:  Need to make do_error into var_args fn? */
      printf ("Tried to gzip -c -d %s > %s\n", f1, f2);
      do_error ("Can't gunzip requested file");
    }
}

FILE* 
fopen_read_gzipped (char* fn)
{
    FILE* fp;

#if defined (HAVE_GZIP) && defined (HAVE_POPEN)
#ifdef __EMX__
    const char* cmd_str = "gzip -d -c < %s 2> nul";
#else
    const char* cmd_str = "gzip -d -c < %s 2> /dev/null";
#endif
    char *cmd = (char*) malloc (strlen (cmd_str) + strlen (fn) + 1);
    
    sprintf (cmd, cmd_str, fn);
#ifdef __EMX__
    fp=popen(cmd,"rb");
#else
    fp=popen(cmd,"r");
#endif
    if (fp==NULL) {
       fprintf(stderr, "Failed to open pipe cmd: %s\n", cmd);
    }
    free(cmd);

#elif defined (HAVE_GZIP) && !defined (HAVE_POPEN)
    gunzip_file (fn, lc_temp_filename);
    fp = fopen (lc_temp_filename, "rb");

#else /* No gzip */
    fp = fopen (fn, "rb");
#endif

    return fp;
}

void 
fclose_read_gzipped (FILE* fp)
{
#if defined (HAVE_GZIP) && defined (HAVE_POPEN)
    pclose (fp);
#elif defined (HAVE_GZIP) && !defined (HAVE_POPEN)
    fclose (fp);
    remove (lc_temp_filename);
#else
    fclose (fp);
#endif
}

int
directory_exists (char *dir)
{
#if defined (WIN32)
    if (_chdir (dir) == -1) {
	_chdir (LIBDIR);		/* go back... */
	return 0;
    }
    _chdir (LIBDIR);		/* go back... */
#else /* UNIX */
    DIR *dp;
    if ((dp = opendir (dir)) == NULL) {
	return 0;
    }
    closedir (dp);
#endif
    return 1;
}

int
file_exists (char *filename)
{
    FILE* fp;
    fp = fopen (filename,"rb");
    if (fp == NULL) {
	return 0;
    }
    fclose (fp);
    return 1;
}

#if defined (WIN32)
void
find_libdir (void)
{
    const char searchfile[] = "Colour.pal";
    /* default_dir will be something like "C:\\LINCITY1.11" */
    const char default_dir[] = "C:\\LINCITY" VERSION;
//    const char default_dir[] = "D:\\LINCITY";	/* For GCS's use */

    /* Check 1: environment variable */
    _searchenv (searchfile, "LINCITY_HOME", LIBDIR);
    if (*LIBDIR != '\0') {
	int endofpath_offset = strlen (LIBDIR) - strlen (searchfile) - 1;
	LIBDIR[endofpath_offset] = '\0';
	return;
    }

    /* Check 2: default location */
    if ((_access (default_dir, 0)) != -1) {
	strcpy (LIBDIR, default_dir);
	return;
    }

    /* Finally give up */
    HandleError (_("Error. Can't find LINCITY_HOME"), FATAL);
}

#elif defined (__EMX__)
void
find_libdir (void)
{
    strcpy(LIBDIR, __XOS2RedirRoot(OS2_DEFAULT_LIBDIR));
}

#else /* Unix with configure */
void
find_libdir (void)
{
    const char searchfile[] = "colour.pal";
    char *home_dir, *cwd;
    char cwd_buf[LC_PATH_MAX];
    char filename_buf[LC_PATH_MAX];
    #ifdef ENABLE_BINRELOC
    char *datadir_buf;
    #endif

    /* Check 1: environment variable */
    home_dir = getenv ("LINCITY_HOME");
    if (home_dir) {
	snprintf (filename_buf, LC_PATH_MAX, "%s%c%s", 
		  home_dir, PATH_SLASH, searchfile);
	if (file_exists(filename_buf)) {
	    strncpy (LIBDIR, home_dir, LC_PATH_MAX);
	    return;
	}
    }

    /* Check 2: current working directory */
    cwd = getcwd (cwd_buf, LC_PATH_MAX);
    if (cwd) {
	snprintf (filename_buf, LC_PATH_MAX, "%s%c%s", 
		  cwd_buf, PATH_SLASH, searchfile);
	if (file_exists(filename_buf)) {
	    strncpy (LIBDIR, cwd_buf, LC_PATH_MAX);
	    return;
	}
    }

    /* Check 3: default (configuration) directory */
    #ifdef ENABLE_BINRELOC
    datadir_buf=br_strcat(DATADIR,"/lincity");

    snprintf (filename_buf, LC_PATH_MAX, "%s%c%s", 
	      datadir_buf, PATH_SLASH, searchfile);

    if (file_exists(filename_buf)) {
	strncpy (LIBDIR, datadir_buf, LC_PATH_MAX);
	return;
    }
    free(datadir_buf);

    #else
    snprintf (filename_buf, LC_PATH_MAX, "%s%c%s", 
	      DEFAULT_LIBDIR, PATH_SLASH, searchfile);
    if (file_exists(filename_buf)) {
	strncpy (LIBDIR, DEFAULT_LIBDIR, LC_PATH_MAX);
	return;
    }
    #endif
    /* Finally give up */
    HandleError (_("Error. Can't find LINCITY_HOME"), FATAL);
}
#endif


/* GCS:  This function comes from dcgettext.c in the gettext package.      */
/* Guess value of current locale from value of the environment variables.  */
/* GCS Feb 23, 2003.  This was updated in gettext, but I'm going with the  */
/* old version here. */
static const char *
guess_category_value (int category, const char *categoryname)
{
    const char *retval;

    /* The highest priority value is the `LANGUAGE' environment
       variable.  This is a GNU extension.  */
    retval = getenv ("LANGUAGE");
    if (retval != NULL && retval[0] != '\0')
	return retval;

    /* `LANGUAGE' is not set.  So we have to proceed with the POSIX
       methods of looking to `LC_ALL', `LC_xxx', and `LANG'.  On some
       systems this can be done by the `setlocale' function itself.  */
#if defined HAVE_SETLOCALE && defined HAVE_LC_MESSAGES && defined HAVE_LOCALE_NULL
    retval = setlocale (category, NULL);
    if (retval != NULL)
      return retval;
    else
      return "C";
#else
    /* Setting of LC_ALL overwrites all other.  */
    retval = getenv ("LC_ALL");
    if (retval != NULL && retval[0] != '\0')
	return retval;

    /* Next comes the name of the desired category.  */
    retval = getenv (categoryname);
    if (retval != NULL && retval[0] != '\0')
	return retval;

    /* Last possibility is the LANG environment variable.  */
    retval = getenv ("LANG");
    if (retval != NULL && retval[0] != '\0')
	return retval;

    /* We use C as the default domain.  POSIX says this is implementation
       defined.  */
    return "C";
#endif
}

/* GCS:  This function is modified from gettext.  It finds the language 
   portion of the locale. */
static void 
lincity_nl_find_language (char *name)
{
  while (name[0] != '\0' && name[0] != '_' && name[0] != '@'
	 && name[0] != '+' && name[0] != ',')
    ++name;

  *name = '\0';
}


void
find_localized_paths (void)
{
  int messages_done = 0;
  int help_done = 0;

  const char* intl_suffix = "";
  char intl_lang[128];

  /* First, try the locale "as is" */
#if defined (ENABLE_NLS) && defined (HAVE_LC_MESSAGES)
  intl_suffix = guess_category_value(LC_MESSAGES,"LC_MESSAGES");
#else
  intl_suffix = guess_category_value(0,"LC_MESSAGES");
#endif
  debug_printf ("GUESS 1 -- intl_suffix is %s\n", intl_suffix);
  if (strcmp(intl_suffix,"C") && strcmp(intl_suffix,"")) {
    sprintf (message_path, "%s%c%s%c%s%c", LIBDIR, PATH_SLASH, "messages",
	     PATH_SLASH, intl_suffix, PATH_SLASH);
    debug_printf ("Trying Message Path %s\n", message_path);
    if (directory_exists(message_path)) {
      debug_printf ("Set Message Path %s\n", message_path);
      messages_done = 1;
    }
    sprintf (help_path, "%s%c%s%c%s%c", LIBDIR, PATH_SLASH, "help",
	     PATH_SLASH, intl_suffix, PATH_SLASH);
    debug_printf ("Trying Help Path %s\n", help_path);
    if (directory_exists(help_path)) {
      debug_printf ("Set Help Path %s\n", help_path);
      help_done = 1;
    }
  }
  if (messages_done && help_done) return;

  /* Next, try stripping off the country suffix */
  strncpy (intl_lang, intl_suffix, 128);
  intl_lang[127] = '\0';
  lincity_nl_find_language (intl_lang);
  intl_suffix = intl_lang;
  debug_printf ("GUESS 2 -- intl_suffix is %s\n", intl_suffix);
  if (strcmp(intl_suffix,"C") && strcmp(intl_suffix,"")) {
    if (!messages_done) {
      sprintf (message_path, "%s%c%s%c%s%c", LIBDIR, PATH_SLASH, "messages",
	       PATH_SLASH, intl_suffix, PATH_SLASH);
      debug_printf ("Trying Message Path %s\n", message_path);
      if (directory_exists(message_path)) {
	debug_printf ("Set Message Path %s\n", message_path);
	messages_done = 1;
      }
    }
    if (!help_done) {
      sprintf (help_path, "%s%c%s%c%s%c", LIBDIR, PATH_SLASH, "help",
	       PATH_SLASH, intl_suffix, PATH_SLASH);
      debug_printf ("Trying Help Path %s\n", help_path);
      if (directory_exists(help_path)) {
	debug_printf ("Set Help Path %s\n", help_path);
	help_done = 1;
      }
    }
  }
  if (messages_done && help_done) return;
    
  /* Finally, settle for default English messages */
  if (!messages_done) {
    sprintf (message_path, "%s%c%s%c", LIBDIR, PATH_SLASH, "messages",
	     PATH_SLASH);
    debug_printf ("Settling for message Path %s\n", message_path);
  }
  if (!help_done) {
    sprintf (help_path, "%s%c%s%c", LIBDIR, PATH_SLASH, "help",
	     PATH_SLASH);
    debug_printf ("Settling for help Path %s\n", help_path);
  }
}


void
init_path_strings (void)
{
    char* homedir = NULL;
    char* dm = NULL;
    char* td = NULL;

    find_libdir ();

#if defined (WIN32)
    homedir = LIBDIR;
#elif defined (__EMX__)
    homedir = getenv ("HOME");
#else
    homedir = getenv ("HOME");
#endif

    /* Various dirs and files */
    lc_save_dir_len = strlen (homedir) + strlen (LC_SAVE_DIR) + 1;
    if ((lc_save_dir = (char *) malloc (lc_save_dir_len + 1)) == 0)
	malloc_failure ();
    sprintf (lc_save_dir, "%s%c%s", homedir, PATH_SLASH, LC_SAVE_DIR);
    sprintf (colour_pal_file, "%s%c%s", LIBDIR, PATH_SLASH, "colour.pal");
    sprintf (opening_path, "%s%c%s", LIBDIR, PATH_SLASH, "opening");
#if defined (WIN32)
    sprintf (opening_pic, "%s%c%s",opening_path,PATH_SLASH,"open.tga");
#else
    sprintf (opening_pic, "%s%c%s",opening_path,PATH_SLASH,"open.tga.gz");
#endif
    sprintf (graphic_path, "%s%c%s%c", LIBDIR, PATH_SLASH, "icons",
	     PATH_SLASH);
    sprintf (lincityrc_file, "%s%c%s", homedir, PATH_SLASH, 
	LINCITYRC_FILENAME);

    /* Paths for message & help files, etc */
    find_localized_paths ();

    /* Font stuff */
    sprintf (fontfile, "%s%c%s", opening_path, PATH_SLASH,
	     "iso8859-1-8x8.raw");
#if defined (WIN32)
    /* GCS: Use windows font for extra speed */
    strcpy (windowsfontfile, LIBDIR);
#if defined (commentout)
    if (!pix_double)
	strcat (windowsfontfile, "\\opening\\iso8859-1-8x8.fnt");
    else
	strcat (windowsfontfile, "\\opening\\iso8859-1-9x15.fnt");
#endif
    if (!pix_double)
	strcat (windowsfontfile, "\\opening\\winfont_8x8.fnt");
    else
	strcat (windowsfontfile, "\\opening\\winfont_16x16.fnt");
#endif

    /* Temp file for results */
    lc_temp_filename = (char *) malloc (lc_save_dir_len + 16);
    if (lc_temp_filename == 0) {
	malloc_failure ();
    }
    sprintf (lc_temp_filename, "%s%c%s", lc_save_dir, PATH_SLASH, "tmp-file");

    /* Path for localization */
#if defined (ENABLE_NLS)

    sprintf (lc_textdomain_directory, "%s%c%s", LIBDIR, PATH_SLASH, "locale");

    dm = bindtextdomain (PACKAGE, lc_textdomain_directory);
    debug_printf ("Bound textdomain directory is %s\n", dm);
    td = textdomain (PACKAGE);
    debug_printf ("Textdomain is %s\n", td);
#endif
}

void
verify_package (void)
{
    FILE *fp = fopen (colour_pal_file,"rb");
    if (!fp) {
	do_error (_("Error verifying package. Can't find colour.pal."));
    }
    fclose (fp);
}

void
make_savedir (void)
{
#if !defined (WIN32)
    DIR *dp;
#endif

#if defined (commentout)
    if (make_dir_ok_flag == 0)
	return;
#endif

#if defined (WIN32)
    if (_mkdir (lc_save_dir)) {
	printf (_("Couldn't create the save directory %s\n"), lc_save_dir);
	exit (-1);
    }
#else
    mkdir (lc_save_dir, 0755);
    chown (lc_save_dir, getuid (), getgid ());
    if ((dp = opendir (lc_save_dir)) == NULL)
    {
	/* change this to a screen message. */
	printf (_("Couldn't create the save directory %s\n"), lc_save_dir);
	exit (1);
    }
    closedir (dp);
#endif

#if defined (commentout)
    make_dir_ok_flag = 0;
#endif
}

void
check_savedir (void)
{
#if defined (commentout)
    int i = 0, j, k, r, l;
#endif

    if (!directory_exists (lc_save_dir)) {
	make_savedir ();
#if defined (commentout)
	l = lc_save_dir_len;
	if (l > 160) {
	    i = l - 160;
	    l = 160;
	}
	askdir_lines = l / 40 + ((l % 40) ? 1 : 0);
	r = l / askdir_lines + ((l % askdir_lines) ? 1 : 0);
	for (j = 0; j < askdir_lines; j++) {
	    if ((askdir_path[j] = (char *) malloc (r + 1)) == 0)
		malloc_failure ();
	    for (k = 0; k < r; k++, i++)
		*(askdir_path[j] + k) = lc_save_dir[i];
	    *(askdir_path[j] + k) = 0;
	}
	return;
#endif
    }
#if defined (commentout)
    make_dir_ok_flag = 0;		/* don't load the ask-dir */
#endif
}

void
malloc_failure (void)
{
  printf (_("Out of memory: malloc failure\n"));
  exit (1);
}

char*
load_graphic(char *s)
{
    int x,l;
    char ss[LC_PATH_MAX],*graphic;
    FILE *inf;
    strcpy(ss,graphic_path);
    strcat(ss,s);
    if ((inf=fopen(ss,"rb"))==NULL)
    {
	strcat(ss," -- UNABLE TO LOAD");
	do_error(ss);
    }
    fseek(inf,0L,SEEK_END);
    l=ftell(inf);
    fseek(inf,0L,SEEK_SET);
    graphic=(char *)malloc(l);
    for (x=0;x<l;x++)
	*(graphic+x)=fgetc(inf);
    fclose(inf);
    return(graphic);
}

void
load_lincityrc (void)
{
    FILE *fp;
    int arg;
    char buf[128];

    if ((fp = fopen (lincityrc_file, "r")) == 0) {
	save_lincityrc();
	return;
    }
    while (fgets (buf,128,fp)) {
	if (sscanf(buf,"overwrite_transport=%d",&arg)==1) {
	    overwrite_transport_flag = !!arg;
	    continue;
	}
	if (sscanf(buf,"no_init_help=%d",&arg)==1) {
	    /* Careful here ... */
	    no_init_help = !!arg;
	    continue;
	}
	if (sscanf(buf,"skip_splash_screen=%d",&arg)==1) {
	    skip_splash_screen = !!arg;
	    continue;
	}
	if (sscanf(buf,"suppress_firsttime_module_help=%d",&arg)==1) {
	    suppress_firsttime_module_help = !!arg;
	    continue;
	}
	if (sscanf(buf,"suppress_popups=%d",&arg)==1) {
	    suppress_popups = !!arg;
	    continue;
	}
	if (sscanf(buf,"time_multiplex_stats=%d",&arg)==1) {
	    time_multiplex_stats = !!arg;
	    continue;
	}
	if (sscanf(buf,"x_confine_pointer=%d",&arg)==1) {
	    confine_flag = !!arg;
	    continue;
	}
	if (sscanf(buf,"pix_double=%d",&arg)==1) {
	    pix_double = !!arg;
	    continue;
	}
	if (sscanf(buf,"borderx=%d",&arg)==1) {
	    if (borderx >= 0) {
		borderx = arg;
	    }
	    continue;
	}
	if (sscanf(buf,"bordery=%d",&arg)==1) {
	    if (bordery >= 0) {
		bordery = arg;
	    }
	    continue;
	}
    }
    fclose (fp);
}

void
save_lincityrc (void)
{
    FILE *fp;

    if ((fp = fopen (lincityrc_file, "w")) == 0) {
	return;
    }

    fprintf (fp, 
	"# Set this if you want to be able to overwrite one\n"
	"# kind of transport with another.\n"
	"overwrite_transport=%d\n\n",
	overwrite_transport_flag);
    fprintf (fp, 
	"# Set this if you don't want the opening help screen.\n"
	"no_init_help=%d\n\n",
	no_init_help
	);
    fprintf (fp,
	"# Set this if you don't want the opening splash screen.\n"
	"skip_splash_screen=%d\n\n",
	skip_splash_screen
	);
    fprintf (fp,
	"# Set this if you don't want help the first time you\n"
	"# click to place an item.\n"
	"suppress_firsttime_module_help=%d\n\n", 
	suppress_firsttime_module_help
	);
    fprintf (fp,
	"# Set this if don't want modal dialog boxes which you\n"
	"# are required to click OK.  Instead, report the dialog\n"
	"# box information to the message area.\n"
	"suppress_popups=%d\n\n",
	suppress_popups
	);
    fprintf (fp,
	"# Set this if want the different statistic windows to cycle\n"
	"# through the right panel.\n"
	"time_multiplex_stats=%d\n\n",
	time_multiplex_stats
	);
    fprintf (fp,
	"# (X Windows and WIN32 only) Set this if you want pix doubling,\n"
	"# where each pixel is drawn as a 2x2 square.\n"
	"pix_double=%d\n\n",
	pix_double
	);
    fprintf (fp,
	"# (X Windows and WIN32 only) Set this if you want a blank area\n"
	"# around the playing area.\n"
	"borderx=%d\n"
	"bordery=%d\n\n",
	borderx,
	bordery
	);
    fprintf (fp,
	"# (X Windows only) Set this if you want to confine the pointer\n"
	"# to within the window.\n"
	"x_confine_pointer=%d\n\n",
	confine_flag
	);

    fclose (fp);
}

void
undosify_string (char *s)
{
    /* Convert '\r\n' to '\n' in string */
    char prev_char = 0;
    char *p = s, *q = s;
    while (*p) {
	if (*p != '\r') {
	    if (prev_char == '\r' && *p != '\n') {
		*q++ = '\n';
	    }
	    *q++ = *p;
	}
	prev_char = *p;
        p++;
    }
    if (prev_char == '\r') {
	*q++ = '\n';
    }
    *q = '\0';
}

void
debug_printf (char* fmt, ...)
{
#if (DEBUG_PRINTF_TO_FILE)
    static int initialized = 0;
    char* filename = "debug.txt";
    FILE* fp;
#endif
    va_list argptr;

#if (DEBUG_PRINTF_TO_FILE)
    va_start (argptr, fmt);
    fp = fopen(filename, "a");
    if (!initialized) {
	initialized = 1;
	fprintf (fp, "=========================\n");
    }
    vfprintf (fp, fmt, argptr);
#endif

    if (command_line_debug) {
#if (!DEBUG_PRINTF_TO_FILE)
      va_start (argptr, fmt);
#endif
      vprintf (fmt, argptr);
#if (!DEBUG_PRINTF_TO_FILE)
      va_end (argptr);
#endif
    }

#if (DEBUG_PRINTF_TO_FILE)
    va_end (argptr);
    fclose (fp);
#endif
}

