// Replacement for the newlib's stdio.h.
//

#ifndef SCR_BSP_STDIO_H
#define SCR_BSP_STDIO_H

#include <stddef.h>
#include <stdarg.h>

/*
 * <sys/reent.h> defines __FILE, _fpos_t.
 * They must be defined there because struct _reent needs them (and we don't
 * want reent.h to include this file.
 */

#include <sys/reent.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PRINTF_LEVEL
#define	stdin  (_REENT->_stdin)
#define	stdout (_REENT->_stdout)
#define	stderr (_REENT->_stderr)

#define _stdin_r(x) ((x)->_stdin)
#define _stdout_r(x) ((x)->_stdout)
#define _stderr_r(x) ((x)->_stderr)
#endif /* PRINTF_LEVEL */

#define	EOF	         (-1)
#define	BUFSIZ       1024
#define	FOPEN_MAX    20
#define	FILENAME_MAX 1024
#define	L_tmpnam     FILENAME_MAX

#define	SEEK_SET     0 /* set file offset to offset */
#define	SEEK_CUR     1 /* set file offset to current plus offset */
#define	SEEK_END     2 /* set file offset to EOF plus offset */

// interlocked variant
int printk(const char *fmt, ...) __attribute__((format (printf, 1, 2)));

/*
 * Functions defined in ANSI C standard.
 */

#ifndef __VALIST
#ifdef __GNUC__
#define __VALIST __gnuc_va_list
#else
#define __VALIST va_list
#endif
#endif

typedef long fpos_t;
typedef __FILE FILE;

FILE * tmpfile (void);
char * tmpnam (char *);

void clearerr (FILE *);
int	feof (FILE *);
int	ferror (FILE *);
void perror (const char *);

int	fclose (FILE *);
int	fflush (FILE *);
FILE * freopen (const char *, const char *, FILE *);
void setbuf (FILE *, char *);
int	setvbuf (FILE *, char *, int, size_t);

int	fsetpos (FILE *, const _fpos_t *);
long ftell (FILE *);
void rewind (FILE *);

int	fseek (FILE *, long, int);

__attribute__ ((__format__ (__printf__, 2, 3)))
int	fprintf (FILE *, const char *, ...);

__attribute__ ((__format__ (__scanf__, 2, 3)))
int	fscanf (FILE *, const char *, ...);

__attribute__ ((__format__ (__printf__, 1, 2)))
int	printf (const char *, ...);

__attribute__ ((__format__ (__scanf__, 1, 2)))
int	scanf (const char *, ...);

__attribute__ ((__format__ (__scanf__, 2, 3)))
int	sscanf (const char *, const char *, ...);

__attribute__ ((__format__ (__printf__, 2, 0)))
int	vfprintf (FILE *, const char *, __VALIST);

__attribute__ ((__format__ (__printf__, 1, 0)))
int	vprintf (const char *, __VALIST);

__attribute__ ((__format__ (__printf__, 2, 0)))
int	vsprintf (char *, const char *, __VALIST);

int	fgetc (FILE *);
char * fgets (char *, int, FILE *);
int	fputc (int, FILE *);
int	fputs (const char *, FILE *);
int	getc (FILE *);
int	getchar (void);
char * gets (char *);
int	putc (int, FILE *);
int	putchar (int);
int	puts (const char *);
int	ungetc (int, FILE *);
size_t fread (void *, size_t _size, size_t _n, FILE *);
size_t fwrite (const void * , size_t _size, size_t _n, FILE *);
FILE * fopen (const char * _name, const char * _type);

int	remove (const char *);
int	rename (const char *, const char *);
int	fgetpos (FILE *, _fpos_t *);

__attribute__ ((__format__ (__printf__, 2, 3)))
int	sprintf (char *, const char *, ...);

__attribute__ ((__format__ (__printf__, 3, 4)))
int	snprintf (char *, size_t, const char *, ...);

__attribute__ ((__format__ (__printf__, 3, 0)))
int	vsnprintf (char *, size_t, const char *, __VALIST);

__attribute__ ((__format__ (__scanf__, 2, 0)))
int	vfscanf (FILE *, const char *, __VALIST);

__attribute__ ((__format__ (__scanf__, 1, 0)))
int	vscanf (const char *, __VALIST);

__attribute__ ((__format__ (__scanf__, 2, 0)))
int	vsscanf (const char *, const char *, __VALIST);

#ifdef __cplusplus
}
#endif

#endif // SCR_BSP_STDIO_H
