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
#include <libobj/geom.h>
#include <libobj/list.h>
#include <libcommon/sampling.h>

static void GeomBounds(), GeomBoundsAnimated();
void GeomResolveAssoc();	/* probably static */

Geom *
GeomCreate(objptr, methods)
GeomRef objptr;
Methods *methods;
{
	Geom *obj;

	if (objptr == (GeomRef)NULL)
		return (Geom *)NULL;
		
	obj = (Geom *)calloc(1, sizeof(Geom));
	obj->obj = objptr;
	obj->methods = methods;
	obj->animtrans = FALSE;
	obj->trans = obj->transtail = (Trans *) NULL;
	obj->frame = -1;	/* impossible value */
	BoundsInit(obj->bounds);
#ifdef SHAREDMEM
	/*
	 * If the counter is in shared memory, processes will
	 * be modifying it left-and-right.  So, we cheat and
	 * make counter a pointer to a non-shared location and
	 * store the value there.
	 */
	new->counter = (unsigned long *)malloc(sizeof(unsigned long));
	*new->counter = 0;
#endif
	return obj;
}

/*
 * Return a copy of the given object.
 * Note that surface, texturing, and transformation information
 * is copied by reference.
 */
Geom *
GeomCopy(obj)
Geom *obj;
{
	Geom *new;

	new = GeomCreate(obj->obj, obj->methods);
	/* Share texturing, name, #prims, surface info */
	new->name = obj->name;
	new->texture = obj->texture;
	new->surf = obj->surf;
	new->prims = obj->prims;
	new->trans = obj->trans;
	new->animtrans = obj->animtrans;
	new->transtail = obj->transtail;
	/* copy bounds */
	BoundsCopy(obj->bounds, new->bounds);
	return new;
}

/*
 * Report bounding box and number of primitives in object.
 */
void
AggregatePrintInfo(obj, fp)
Geom *obj;
FILE *fp;
{
	if (fp) {
		if (obj->name && obj->name[0])
			fprintf(fp,"%s \"%s\":\n", GeomName(obj), obj->name);
		else
			fprintf(fp,"%s:\n", GeomName(obj));
		if (!UNBOUNDED(obj))
			BoundsPrint(obj->bounds, fp);
		fprintf(fp,"\t%lu primitive%c\n",obj->prims,
			obj->prims == 1 ? ' ' : 's');
	}
}

/*
 * Convert the given object from a linked list of objects to
 * the desired aggregate type.
 */
int AggregateConvert(Geom *obj, Geom *objlist)
{
	if (!IsAggregate(obj)) {
		RLerror(RL_ABORT, "A %s isn't an aggregate.\n",
			GeomName(obj));
		return 0;
	}

	return (*obj->methods->convert)(obj->obj, objlist);
}

/*
 * This should really be called
 * GeomInitialize
 * or something.
 */
void GeomComputeBounds(Geom *obj)
{
	if (obj->frame == Sampling.framenum)
		return;

	if (!obj->animtrans) {
		/*
		 * If it isn't animated,
		 * just compute bbox directly 
		 */
		GeomBounds(obj, obj->bounds);
	} else {
		/*
		 * Animated things are gonna get a bbox
		 * which is large enough to enclose all
		 * the places where the object goes.
		 */
		GeomBoundsAnimated(obj);
	}
	/*
	 * Enlarge by EPSILON in each direction just to
	 * be on the safe side.
	 */
	obj->bounds[LOW][X] -= EPSILON;
	obj->bounds[HIGH][X] += EPSILON;
	obj->bounds[LOW][Y] -= EPSILON;
	obj->bounds[HIGH][Y] += EPSILON;
	obj->bounds[LOW][Z] -= EPSILON;
	obj->bounds[HIGH][Z] += EPSILON;
	/*
	 * Mark the fact that that the obj is initialized
	 * for this frame.
	 */
	obj->frame = Sampling.framenum;
	obj->counter = 0;
}

static void
GeomBoundsAnimated(obj)
Geom *obj;
{
	int i, m;
	Float newbounds[2][3];
	Float window, subwindow, jitter, subjitter;

	/*
	 * For each possible screen sample,
	 * choose TIME_SUB_SAMPLES times and recompute the
	 * bounds of obj at that time,
	 * expanding the computed bounding box appropriately.
	 */
	BoundsInit(obj->bounds);
	jitter = Sampling.shutter / Sampling.totsamples;
	subjitter = jitter / (Float)TIME_SUB_SAMPLES;
	window = Sampling.starttime;
	for (i = 0; i < Sampling.totsamples; i++, window += jitter) {
		subwindow = window;
		for (m = 0; m < TIME_SUB_SAMPLES; m++, subwindow += subjitter) {
			/*
			 * Set the current time.
			 */
			TimeSet(subwindow + subjitter*nrand());
			/*
			 * Resolve the objects geometric associations
			 */
			GeomResolveAssoc(obj);
			/*
			 * Compute bounds and expand current bounds.
			 */
			GeomBounds(obj, newbounds);
			BoundsEnlarge(obj->bounds, newbounds);
		}
	}
	/*
	 * Also sample at time extremes, as for many
	 * movements, extremes occur at beginning/end times.
	 */
	TimeSet(Sampling.starttime);
	GeomResolveAssoc(obj);
	GeomBounds(obj, newbounds);
	BoundsEnlarge(obj->bounds, newbounds);

	TimeSet(Sampling.starttime + Sampling.shutter);
	GeomResolveAssoc(obj);
	GeomBounds(obj, newbounds);
	BoundsEnlarge(obj->bounds, newbounds);
}

void
GeomResolveAssoc(obj)
Geom *obj;
{
	/*
	 * PrimResolveAssoc(obj);
	 */
	TransResolveAssoc(obj->trans);
}

/*
 * Set "bounds" of object to be the extent of the primitive.
 */
static void
GeomBounds(obj, bounds)
Geom *obj;
Float bounds[2][3];
{
	Trans *trans;

	if (!obj || !obj->methods->bounds)
		RLerror(RL_ABORT, "Can't compute bounds of \"%s\".\n",
			GeomName(obj));
	(*obj->methods->bounds) (obj->obj, bounds);
	bounds[LOW][X] -= EPSILON;
	bounds[LOW][Y] -= EPSILON;
	bounds[LOW][Z] -= EPSILON;
	bounds[HIGH][X] += EPSILON;
	bounds[HIGH][Y] += EPSILON;
	bounds[HIGH][Z] += EPSILON;
	if (obj->trans) {
		for (trans = obj->trans; trans; trans = trans->next)
			BoundsTransform(&trans->trans, bounds);
	}
}

char *
GeomName(obj)
Geom *obj;
{
	if (obj->methods->name)
		return (*obj->methods->name)();

	return "unknown";
}

void
GeomStats(obj, tests, hits)
Geom *obj;
unsigned long *tests, *hits;
{
	if (obj && obj->methods->stats)
		(*obj->methods->stats)(tests, hits);
	else {
		*tests = *hits = 0;
	}
}

/*
 * Push an object onto the head of the given stack, returning
 * the new head.
 */
GeomList *
GeomStackPush(obj, list)
Geom *obj;
GeomList *list;
{
	GeomList *new;
	/*
	 * Pretty simple.
	 * Make new element point to old head and return new head.
	 */
	new = (GeomList *)malloc(sizeof(GeomList));
	new->obj = obj;
	new->next = list;
	return new;
}

/*
 * Pop the topmost object off of the given stack, returning the new head.
 * The old head is freed, but the object it points to is not.
 */
GeomList *
GeomStackPop(list)
GeomList *list;
{
	GeomList *ltmp;

	ltmp = list->next;	/* Save new head. */
	free((void *)list);	/* Free old head. */
	return ltmp;		/* Return new head. */
}

Methods *
MethodsCreate()
{
	return (Methods *)calloc(1, sizeof(Methods));
}

/*
 * Call appropriate routine to compute UV and, if non-null,
 * dpdu and dpdv at given point on the given primitive.  The
 * normal is used to facilitate computation of u, v, and the
 * partial derivatives.
 */
void
PrimUV(prim, pos, norm, uv, dpdu, dpdv)
Geom *prim;
Vector *pos, *norm, *dpdu, *dpdv;
Vec2d *uv;
{
	/*
	 * Call appropriate inverse mapping routine
	 */
	if (prim->methods->uv == NULL) {
		uv->u = uv->v = 0.;
		if (dpdu) {
			dpdu->y = dpdu->z = 0.;
			dpdu->x = 1.;
		}
		if (dpdv) {
			dpdv->x = dpdv->z = 0.;
			dpdv->y = 1.;
		}	
	} else
		(*prim->methods->uv)(prim->obj,pos,norm,uv,dpdu,dpdv);
}

int
PrimNormal(prim, pos, norm, gnorm)
Geom *prim;
Vector *pos, *norm, *gnorm;
{
	/*
	 * Call appropriate normal routine
	 */
	return (*prim->methods->normal) (prim->obj, pos, norm, gnorm);
}

int PrimEnter(Geom *obj, Ray *ray, Float mind, Float hitd)
{
	/*
	 * Call appropriate enter/leave routine
	 */
	if (obj->methods->enter == NULL) {
		Vector pos, nrm, gnrm;
		/*
		 * Sleazy method:  Use hit point, find normal 
		 * and take dot prod with ray 
		 */
		VecAddScaled(ray->pos, hitd, ray->dir, &pos);
		PrimNormal(obj, &pos, &nrm, &gnrm);

		return dotp(&ray->dir, &gnrm) < 0.0;
	}
	else
		return (*obj->methods->enter) (obj->obj, ray, mind, hitd);
}

/*
 * Walk through a linked-list of objects.  If the object is unbounded,
 * unlink it it from the list and add it to the 'unbounded' list.
 * If the object is bounded, enlarge the given bounding box if
 * necessary.  Return pointer to unbounded list.
 */
Geom *
GeomComputeAggregateBounds(bounded, unbounded, bounds)
Geom **bounded, *unbounded;
Float bounds[2][3];
{
	Geom *ltmp, *prev, *nextobj;

	BoundsInit(bounds);

	prev = (Geom *)0;

	for (ltmp = *bounded; ltmp; ltmp = nextobj) {
		nextobj = ltmp->next;
		GeomComputeBounds(ltmp);
		if (UNBOUNDED(ltmp)) {
			/*
			 * Geom is unbounded -- unlink it...
			 */
			if (prev)
				prev->next = ltmp->next;
			else
				*bounded = ltmp->next;
			/*
			 * And add it to unbounded object list.
			 */
			ltmp->next = unbounded;
			unbounded = ltmp;
		} else {
			/*
			 * Geom is bounded.
			 */
			BoundsEnlarge(bounds, ltmp->bounds);
			prev = ltmp;
		}
	}
	return unbounded;
}

/*
 * Find 'highest' animated object on the hitlist.
 */
int
FirstAnimatedGeom(hitlist)
HitList *hitlist;
{
	int i;

	for (i = hitlist->nodes -1; i; i--)
		/*
		 * If object itself is animated, have
		 * to check other flag, too...
		 */
		if (hitlist->data[i].obj->animtrans)
			return i;
	return 0;
}