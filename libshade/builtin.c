/*
 * Copyright (C) 1989-2015, Craig E. Kolb, Rod G. Bogart
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

Float
SumExpr(a, b)
Float a, b;
{
	return a + b;
}
Float
DiffExpr(a, b)
Float a, b;
{
	return a - b;
}

Float
MultExpr(a, b)
Float a, b;
{
	return a * b;
}
Float
DivideExpr(a, b)
Float a, b;
{
	return a / b;
}

Float
ModExpr(a, b)
Float a, b;
{
	return (Float)((int)a % (int)b);
}

Float
NegateExpr(a)
Float a;
{
	return -a;
}

Float
LinearTime(starttime, startval, endtime, endval)
Float starttime, endtime, startval, endval;
{
	if (TimeExpr->value < starttime)
		return startval;
	if (TimeExpr->value > endtime)
		return endval;
	if (equal(endtime, starttime))
		return startval;
	return startval + (endval - startval) * 
		(TimeExpr->value - starttime) / (endtime - starttime);
}
