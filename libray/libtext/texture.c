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
#include "texture.h"

/*
 * Transformation structures used to map from texture space to
 * model/primitive/world space.
 */
Trans prim2model, model2text, prim2text, world2text;

#define ApplyMapping(m,o,p,n,c,u,v)	(*m->method)(m, o, p, n, c, u, v)

Texture *
TextCreate(data, meth)
TextRef data;
void (*meth)();
{
	Texture *res;

	res = (Texture *)share_calloc(1, sizeof(Texture));
	res->data = data;
	res->method = meth;
	res->trans = (Trans *)NULL; 
	res->next = (Texture *)NULL;
	res->animtrans = FALSE;
	return res;
}

/*
 * Apply appropriate textures to a surface.
 */
void
TextApply(tlist, prim, ray, pos, norm, gnorm, surf, p2model, world2model)
Texture *tlist;				/* Textures */
Geom *prim;
Ray *ray;
Vector *pos, *norm, *gnorm;		/* pos, shading norm, geo. norm */
Surface *surf;
Trans *p2model, *world2model;
{
	Vector ptmp;
	Texture *ttmp;

	prim2model = *p2model;
	/*
	 * Walk down texture list, applying each in turn.
	 */
	for (ttmp = tlist; ttmp; ttmp = ttmp->next) {
		/*
		 * Make copies of pos & ray to pass to the texturing function.
		 */
		ptmp = *pos;
		if (ttmp->trans) {
			/*
			 * 'take' the inverse of ttmp->trans, since
			 * transforming a texture means applying the
			 * inverse of the transformation
			 * to the point of intersection, etc.
			 */
			if (ttmp->animtrans) {
				/*
				 * Resolve animated associations.
				 * We currently do not store a time
				 * for the texture, so we can't know if
				 * we're already resolved for the current
				 * ray->time.
				 */
				TransResolveAssoc(ttmp->trans);
				TransComposeList(ttmp->trans, &model2text);
				TransInvert(&model2text, &model2text);
			} else
				TransInvert(ttmp->trans, &model2text);
			/*
			 * We compose ttmp->trans, which maps from model to
			 * texture space, with prim2model and world2model
			 * to get prim2text and world2text.
			 */
			TransCompose(&model2text, &prim2model, &prim2text);
			TransCompose(&model2text, world2model, &world2text);
			/*
			 * Transform intersection point to texture space.
			 * Ray and normal are passed in model space.
			 */
			ModelPointToText(&ptmp);
		} else {
			/*
		 	 * By default, texture and model space are identical.
		 	 */
			TransInit(&model2text);
			TransCopy(&prim2model, &prim2text);
			TransCopy(world2model, &world2text);
		}

		/*
		 * Call texture function.
		 */
		(*ttmp->method) (ttmp->data,prim,ray,&ptmp,norm,gnorm,surf);
	}
}

/*
 * Compute UV at 'pos' on given primitive.
 */
TextToUV(mapping, prim, pos, norm, u, v, dpdu, dpdv)
Mapping *mapping;
Geom *prim;
Vector *pos, *norm, *dpdu, *dpdv;
Float *u, *v;
{
	Vec2d uv;
	Vector ptmp;
	RSMatrix t;

	ptmp = *pos;

	if (mapping->flags & PRIMSPACE) {
		/*
	 	 * Convert point and normal to primitive space.
	 	 */
		TextPointToPrim(&ptmp);
	} else {
		/*
		 * Convert point and normal to object space.
		 */
		TextPointToModel(&ptmp);
	}

	ApplyMapping(mapping, prim, &ptmp, norm, &uv, dpdu, dpdv);

	/*
	 * Transform UV by model2text.  We set X = u and Y = v,
	 * while Z = 0.
	 * Although the UV coordinates may be in prim space,
	 * we treat them as if they are model-space coords.
	 * This is due to the fact that we want the texture
	 * to be applied in model space.
	 */
	ptmp.x = uv.u;
	ptmp.y = uv.v;
	ptmp.z = 0.;
	PointTransform(&ptmp, &model2text.trans);
	*u = ptmp.x;
	*v = ptmp.y;
	if (dpdu == (Vector *)NULL || dpdv == (Vector *)NULL)
		return;
	/*
	 * Here's the ugly part.
	 * Build initial UVN-->XYZ matrix...
	 */
	ArbitraryMatrix(dpdu->x, dpdu->y, dpdu->z,
			 dpdv->x, dpdv->y, dpdv->z,
			 norm->x, norm->y, norm->z, 0., 0., 0., &t);
	/*
	 * ...transform to model space...
	 */
	MatrixMult(&t, &prim2model.trans, &t);
	/*
	 * ... apply model2text in UVN space.
	 */
	MatrixMult(&model2text.itrans, &t, &t);
	dpdu->x = t.matrix[0][0];
	dpdu->y = t.matrix[0][1];
	dpdu->z = t.matrix[0][2];
	dpdv->x = t.matrix[1][0];
	dpdv->y = t.matrix[1][1];
	dpdv->z = t.matrix[1][2];
	(void)VecNormalize(dpdu);
	(void)VecNormalize(dpdv);
}

/*
 * Append 'text' to the given linked list of textures.
 * Note that 'text' may be a list, too.
 */
Texture *
TextAppend(text, list)
Texture *text, *list;
{
	Texture *tp;

	if (list) {
		/*
		 * Walk to the end of the list
		 */
		for (tp = list;tp->next ;tp = tp->next)
				;
		tp->next = text;
		return list;
	}
	/* else */
	return text;
}
