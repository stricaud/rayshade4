/*
 * Copyright (C) 1989-2015, Craig E. Kolb
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "rayshade.h"
#ifdef RUSAGE
#include <sys/time.h>
#include <sys/resource.h>
#else
#ifdef TIMES
#include <sys/types.h>
#include <sys/times.h>
#include <sys/param.h>
#endif
#endif
#include "options.h"
#include "stats.h"

Float RSabstmp;	/* Temporary value used by fabs macro.  Ugly. */
static void RSmessage();

/*
 * Open input file and call yyparse().
 */
void
RSReadInputFile()
{
	extern FILE *yyin;	/* lex/yacc file pointer */
	extern char yyfilename[];

#if defined(CPPSTDIN) && defined(POPEN)
	char cmd[BUFSIZ];

	if (Options.cppargs != (char *)NULL)
		sprintf(cmd, "%s %s ", CPPSTDIN, Options.cppargs);
	else
		/* fromstdin */
		sprintf(cmd, "%s %s ", CPPSTDIN, CPPMINUS);

	if (Options.inputname == (char *)NULL) {
		(void)strcpy(yyfilename, "stdin");
	} else {
		(void)strcpy(yyfilename, Options.inputname);
		(void)strcat(cmd, Options.inputname);
	}

	if (Options.cpp) {
		yyin = popen(cmd, "r");
		if (yyin == (FILE *)NULL)
			RLerror(RL_PANIC, "popen of \"%s\" failed!\n", cmd);
	} else {
#endif
	if (Options.inputname == (char *)NULL) {
		yyin = stdin;
		(void)strcpy(yyfilename, "stdin");
	} else {
		(void)strcpy(yyfilename, Options.inputname);
		yyin = fopen(Options.inputname, "r");
		if (yyin == (FILE *)NULL)
			RLerror(RL_PANIC,
				"Cannot open %s.\n",Options.inputname);
	}
#if defined(CPPSTDIN) && defined(POPEN)
	}
#endif
	/*
	 * Initialize symbol table.
	 */
	SymtabInit();
	(void)yyparse();
}

void
OpenStatsFile()
{
	if (Options.statsname == (char *)NULL || Stats.fstats != stderr)
		return;		/* Not specified or already opened. */

	Stats.fstats = fopen(Options.statsname, "w");
	if (Stats.fstats == (FILE *)NULL) {
		RLerror(RL_PANIC,
			"Cannot open stats file %s.\n", Options.statsname);
	}
}

void
RLerror(level, pat, arg1, arg2, arg3)
int level;
char *pat, *arg1, *arg2, *arg3;
{
	switch (level) {
		case RL_ADVISE:
			if (!Options.quiet)
				RSmessage("Warning", pat, arg1, arg2, arg3);
			break;
		case RL_WARN:
			RSmessage("Warning", pat, arg1, arg2, arg3);
			break;
		case RL_ABORT:
			RSmessage("Error", pat, arg1, arg2, arg3);
			exit(1);
			break;
		case RL_PANIC:
			RSmessage("Fatal error", pat, arg1, arg2, arg3);
			exit(2);
			break;
		default:
			RSmessage("Unknown error", pat, arg1, arg2, arg3);
			exit(3);
	}
}

static void
RSmessage(type, pat, arg1, arg2, arg3)
char *type, *pat, *arg1, *arg2, *arg3;
{
	extern FILE *yyin;
	extern int yylineno;
	extern char yyfilename[];

	if (yyin) {
		/*
		 * cleanup() hasn't nulled yyin, so line #
		 * info is valid.
		 */
		fprintf(stderr,"%s: %s: %s, line %d: ",
			Options.progname, type,
			yyfilename == (char *)NULL ? "stdin" :
				yyfilename, yylineno);
	} else {
		fprintf(stderr,"%s: %s: ", Options.progname, type);
	}
	fprintf(stderr, pat, arg1, arg2, arg3);
}
		
#ifdef RUSAGE
void
RSGetCpuTime(usertime, systime)
Float *usertime, *systime;
{
	struct rusage usage;

	getrusage(RUSAGE_SELF, &usage);

	*usertime = (Float)usage.ru_utime.tv_sec +
			(Float)usage.ru_utime.tv_usec / 1000000.;
	*systime = (Float)usage.ru_stime.tv_sec +
			(Float)usage.ru_stime.tv_usec / 1000000.;
}

#else
#ifdef TIMES

void
RSGetCpuTime(usertime, systime)
Float *usertime, *systime;
{
	extern CLOCKTYPE times();
	struct tms time;

	(void)times(&time);
	*usertime = (Float)time.tms_utime / (Float)HZ;
	*systime = (Float)time.tms_stime / (Float)HZ;
}

#else /* !RUSAGE && !TIMES */

void
RSGetCpuTime(usertime, systime)
Float *usertime, *systime;
{
	*usertime = *systime = 0.;
}

#endif /* TIMES */
#endif /* RUSAGE */
