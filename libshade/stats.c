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
#include <ctype.h>
#include "rayshade.h"
#include "options.h"
#include "stats.h"

RSStats Stats;			/* Statistical information */
Geom *GeomRep = NULL;	/* Linked list of object representatives */

static void PrintGeomStats();

void
StatsPrint()
{
	extern void PrintMemoryStats();
	unsigned long TotalRays;

#ifndef LINDA
	RSGetCpuTime(&Stats.Utime, &Stats.Stime);
#endif
	ShadowStats(&Stats.ShadowRays, &Stats.ShadowHits,
		    &Stats.CacheHits, &Stats.CacheMisses);
	IntersectStats(&Stats.BVTests);
	
	TotalRays = Stats.EyeRays + Stats.ShadowRays + Stats.ReflectRays
			 + Stats.RefractRays;
	Stats.ShadowHits += Stats.CacheHits;
	Stats.HitRays += Stats.ShadowHits;
#ifdef LINDA
	fprintf(Stats.fstats,"Workers:\t\t\t%d\n",Options.workers);
#endif
	fprintf(Stats.fstats,"Eye rays:\t\t\t%lu\n", Stats.EyeRays);
	fprintf(Stats.fstats,"Shadow rays:\t\t\t%lu\n",Stats.ShadowRays);
	fprintf(Stats.fstats,"Reflected rays:\t\t\t%lu\n",Stats.ReflectRays);
	fprintf(Stats.fstats,"Refracted rays:\t\t\t%lu\n",Stats.RefractRays);
	fprintf(Stats.fstats,"Total rays:\t\t\t%lu\n", TotalRays);
	if (TotalRays != 0)
		fprintf(Stats.fstats,"Intersecting rays:\t\t%lu (%3.3f%%)\n",
			Stats.HitRays,
			100. * (float)Stats.HitRays / (float)TotalRays);
	if (Stats.ShadowRays != 0) {
		if (Options.cache)
			fprintf(Stats.fstats,
				"Shadow cache hits:\t\t%lu (%lu misses)\n",
				Stats.CacheHits, Stats.CacheMisses);
		fprintf(Stats.fstats,"Total shadow hits:\t\t%lu (%3.3f%%)\n",
			Stats.ShadowHits, 100.*(float)Stats.ShadowHits /
			(float)Stats.ShadowRays);
	}
	fprintf(Stats.fstats,"Supersampled pixels:\t\t%lu\n",
		Stats.SuperSampled);
	fprintf(Stats.fstats,"B.V. intersection tests:\t%lu\n",Stats.BVTests);
	PrintGeomStats();
#ifdef LINDA
	fprintf(Stats.fstats,"Average CPU time/processor:\t");
#else
	fprintf(Stats.fstats,"Total CPU time (sec):\t\t");
#endif
	fprintf(Stats.fstats,"%2.2f (%2.2fu + %2.2fs)\n",
		Stats.Utime+Stats.Stime, Stats.Utime, Stats.Stime);
	if (TotalRays != 0.)
		fprintf(Stats.fstats,"Seconds / ray:\t\t\t%4.4f\n",
			(Stats.Utime + Stats.Stime) / (Float)TotalRays);
	if (Stats.HitRays != 0.)
		fprintf(Stats.fstats,"Seconds / intersecting ray:\t%4.4f\n",
			(Stats.Utime + Stats.Stime)/(Float)Stats.HitRays);
	PrintMemoryStats(Stats.fstats);
}

static void
PrintGeomStats()
{
	Geom *otmp;
	unsigned long tests, hits, totaltests, totalhits;
	char *name;
	extern void GeomStats();

	totaltests = totalhits = 0;

	for (otmp = GeomRep; otmp; otmp = otmp->next) {
		GeomStats(otmp, &tests, &hits);
		if (tests <= 0)
			continue;
		name = GeomName(otmp);
		fprintf(Stats.fstats,
			"%c%s intersection tests:\t%lu (%lu hit, %f%%)\n",
				toupper((int)name[0]), &name[1], tests, hits,
				100.*(float)hits/(float)tests);
		if (!IsAggregate(otmp)) {
			totaltests += tests;
			totalhits += hits;
		}
	}
	fprintf(Stats.fstats,"Total prim. intersection tests:\t%lu",
		totaltests);
	if (totaltests == 0)
		fprintf(Stats.fstats,"\n");
	else
		fprintf(Stats.fstats," (%lu hit, %f%%)\n", totalhits,
			100.*(float)totalhits/(float)totaltests);
}

void
StatsAddRep(obj)
Geom *obj;
{
	Geom *otmp;

	for (otmp = GeomRep; otmp; otmp = otmp->next) {
		if (otmp->methods->stats == obj->methods->stats)
			return;
	}

	/*
	 * Stats method didn't match anything found so far.  Add
	 * a copy of obj to head of GeomRep list.
	 */
	otmp = GeomCopy(obj);
	otmp->next = GeomRep;
	GeomRep = otmp;
}
