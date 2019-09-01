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
#include <libcommon/common.h>
#include <libcommon/translate.h>

TransMethods *iTranslateMethods;
void TranslationMatrix();

Translate *
TranslateCreate()
{
	Translate *res;

	res = (Translate *)malloc(sizeof(Translate));
	res->x = res->y = res->z = 0.;
	return res;
}

TransMethods *
TranslateMethods()
{
	if (iTranslateMethods == (TransMethods *)NULL) {
		iTranslateMethods = (TransMethods *)malloc(sizeof(TransMethods));
		iTranslateMethods->create = (TransCreateFunc *)TranslateCreate;
		iTranslateMethods->propagate = TranslatePropagate;
	}
	return iTranslateMethods;	
}

void
TranslatePropagate(translate, trans, itrans)
Translate *translate;
RSMatrix *trans, *itrans;
{
	TranslationMatrix(translate->x, translate->y, translate->z, trans);
	/*
	 * Build the inverse...
	 */
	MatrixInit(itrans);
	itrans->translate.x = -translate->x;
	itrans->translate.y = -translate->y;
	itrans->translate.z = -translate->z;
}

void
TranslationMatrix(x, y, z, mat)
Float x, y, z;
RSMatrix *mat;
{
	MatrixInit(mat);
	mat->translate.x = x;
	mat->translate.y = y;
	mat->translate.z = z;
}