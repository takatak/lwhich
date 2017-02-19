/* Modified by takataka (2017 - )
 *  original : https://opensource.apple.com/source/shell_cmds/shell_cmds-175/which/which.c
 */
/**
 * Copyright (c) 2000 Dan Papasian.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>

#ifdef __FreeBSD__
 __FBSDID("$FreeBSD: src/usr.bin/which/which.c,v 1.6 2005/02/10 16:04:22 ru Exp $");
#endif

#define RC_FIND_ALL      0
#define RC_FIND_SOME     1
#define RC_FIND_NONE     2
#define RC_DISPLAY_USAGE 3
#define RC_UNKNOWN_ERROR 4

#include <sys/stat.h>
#include <sys/param.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

static void	 usage(void);
static int	 print_matches(char *, char *);
static void	 print_with_datetime_byte_link(char *, struct stat *);
	
static int 	silent;
static int 	allpaths;
static int 	showlink;
static int	showlinkrealpath;
static int 	showmtime;
static int 	showbytesize;

int
main(int argc, char **argv)
{
	char	*p,*path;
	ssize_t pathlen;
	int 	opt, status;
	int 	iOk=0,iFileCnt;
	status = EXIT_SUCCESS;
	while ((opt = getopt(argc, argv, "aslrdb")) != -1) {
		switch (opt) {
		case 'a':
			allpaths = 1;
			break;
		case 's':
			silent = 1;
			break;
		case 'l':
			showlink = 1;
			break;
		case 'r':
			showlinkrealpath = 1;
			showlink = 1;
			break;
		case 'd':
			showmtime = 1;
			break;
		case 'b':
			showbytesize = 1;
			break;
		default:
			usage();
			break;
		}
	}

	argv += optind;
	argc -= optind;
	iFileCnt=argc;

	if (argc == 0)
		usage();

	if ((p = getenv("PATH")) == NULL)
		exit(RC_UNKNOWN_ERROR);
	pathlen = strlen(p) + 1;
	path = malloc(pathlen);
	if (path == NULL){
		err(RC_UNKNOWN_ERROR, NULL);
	}else{
		while (argc > 0) {
			memcpy(path, p, pathlen);
	
			if (strlen(*argv) >= FILENAME_MAX ||
				print_matches(path, *argv) == -1){
			}else{
				iOk++;
			}
			argv++;
			argc--;
		}
	}
	if( iOk == iFileCnt ){
		status = RC_FIND_ALL;
	}else if ( iOk > 0 ){
		status = RC_FIND_SOME;
	}else if ( iOk == 0 ){
		status = RC_FIND_NONE;
	}
	exit(status);
}

static void
usage(void)
{
	(void)fprintf(stderr, "usage: lwhich [-aslrb] program ...\n");
	exit(RC_DISPLAY_USAGE);
}

static void 
print_with_datetime_byte_link(char *file , struct stat *pfin){
	if ( file != NULL ){
		struct tm *ts=NULL;
		long long size=-1;
		char linkbuf[PATH_MAX] = {"\0"};
		if ( showlink ){
			struct stat flink;
			if ( lstat(file,&flink) < 0 ){
				// something error!
				fprintf(stderr,"Error: lstat() %s: %s\n", strerror(errno), file);
				exit(RC_UNKNOWN_ERROR);
			}
			if(S_ISLNK(flink.st_mode)){
				if ( showlinkrealpath ){
					if ( realpath(file, linkbuf) == NULL ){
						// something error!
						fprintf(stderr,"Error: realpath() %s: %s\n", strerror(errno), file);
						exit(RC_UNKNOWN_ERROR);
					}
				}else{
					ssize_t len = 0;
					len = readlink(file, linkbuf, sizeof(linkbuf) - 1);
					if( len < 0){
						fprintf(stderr,"Error: readlink() %s: %s\n", strerror(errno), file);
						exit(RC_UNKNOWN_ERROR);
					}
					linkbuf[len] = '\0';
				}
			}
		}
		if ( showmtime ){
			if ( pfin != NULL ){
				ts = localtime(&pfin->st_mtime);
			}else{
				exit(RC_UNKNOWN_ERROR);
			}
		}
		if ( showbytesize ){
			if ( pfin != NULL ){
	 			size =(long long) pfin->st_size;
			}else{
				exit(RC_UNKNOWN_ERROR);
			}
		}

		if ( showmtime ){
			if ( linkbuf[0] != '\0' ){
				if ( showbytesize ){
					printf("%04d-%02d-%02d %02d:%02d:%02d %8lld %s -> %s\n",
						ts->tm_year+1900,ts->tm_mon+1,ts->tm_mday,
						ts->tm_hour,ts->tm_min,ts->tm_sec,
						size,
						file,
						linkbuf
						);
				}else{
					printf("%04d-%02d-%02d %02d:%02d:%02d %s -> %s\n",
						ts->tm_year+1900,ts->tm_mon+1,ts->tm_mday,
						ts->tm_hour,ts->tm_min,ts->tm_sec,
						file,
						linkbuf
						);
				}
			}else{
				if ( showbytesize ){
					printf("%04d-%02d-%02d %02d:%02d:%02d %8lld %s\n", 
						ts->tm_year+1900,ts->tm_mon+1,ts->tm_mday,
						ts->tm_hour,ts->tm_min,ts->tm_sec,
						size,
						file
						);
				}else{
					printf("%04d-%02d-%02d %02d:%02d:%02d %s\n", 
						ts->tm_year+1900,ts->tm_mon+1,ts->tm_mday,
						ts->tm_hour,ts->tm_min,ts->tm_sec,
						file
						);
				}
			}
		}else{
			if ( linkbuf[0] != '\0' ){
				if ( showbytesize ){
					printf("%8lld %s -> %s\n",size, file, linkbuf);
				}else{
					printf("%s -> %s\n", file,linkbuf);
				}
			}else{
				if ( showbytesize ){
					printf("%8lld %s\n", size , file);
				}else{
					printf("%s\n", file);
				}
			}
		}
	}
}

static int
is_there(char *candidate)
{
	struct stat fin;

	/* XXX work around access(2) false positives for superuser */
	if (access(candidate, X_OK) == 0 &&
	    stat(candidate, &fin) == 0 &&
	    S_ISREG(fin.st_mode) &&
	    (getuid() != 0 ||
	    (fin.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) != 0)) {
		if (!silent)
			print_with_datetime_byte_link(candidate, &fin);
		return (1);
	}
	return (0);
}

static int
print_matches(char *path, char *filename)
{
	char candidate[PATH_MAX];
	const char *d;
	int found;
	if (strchr(filename, '/') != NULL)
		return (is_there(filename) ? 0 : -1);
	found = 0;
	while ((d = strsep(&path, ":")) != NULL) {
		if (*d == '\0')
			d = ".";
		if (snprintf(candidate, sizeof(candidate), "%s/%s", d,
		    filename) >= (int)sizeof(candidate))
			continue;
		if (is_there(candidate)) {
			found = 1;
			if (!allpaths)
				break;
		}
	}
	return (found ? 0 : -1);
}
