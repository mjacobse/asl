/****************************************************************
Copyright (C) 1997, 1999 Lucent Technologies
All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appear in all
copies and that both that the copyright notice and this
permission notice and warranty disclaimer appear in supporting
documentation, and that the name of Lucent or any of its entities
not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.

LUCENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
IN NO EVENT SHALL LUCENT OR ANY OF ITS ENTITIES BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.
****************************************************************/

#include "jac2dim.h"

 static void
#ifdef KR_headers
INchk(asl, who, i) ASL *asl; char *who; int i;
#else
INchk(ASL *asl, char *who, int i)
#endif
{
	ASL_CHECK(asl, ASL_read_fgh, who);
	if (i < 0 || i >= n_con) {
		fprintf(Stderr, "%s: got I = %d; expected 0 <= I < %d\n",
			who, i, n_con);
		exit(1);
		}
	}

 real
#ifdef KR_headers
con2ival_ASL(a, i, X, nerror) ASL *a; int i; fint *nerror; real *X;
#else
con2ival_ASL(ASL *a, int i, real *X, fint *nerror)
#endif
{
	cde *d;
	expr *e1;
	real f;
	int ij, L;
	cgrad *gr, **gr0;
	Jmp_buf err_jmp0;
	ASL_fgh *asl;
	expr_v *V;

	INchk(a, "con2ival", i);
	asl = (ASL_fgh*)a;
	if (nerror && *nerror >= 0) {
		err_jmp = &err_jmp0;
		ij = setjmp(err_jmp0.jb);
		if (*nerror = ij)
			return 0.;
		}
	want_deriv = want_derivs;
	errno = 0;	/* in case f77 set errno opening files */
	if (!asl->i.x_known)
		x2_check_ASL(asl,X);
	if (!asl->i.ncxval) {
		asl->i.ncxval = (int*)M1alloc(L = n_con*sizeof(int));
		memset(asl->i.ncxval, 0, L);
		}
	if (!(x0kind & ASL_have_conval)) {
		if (comb < combc)
			comeval(asl, comb, combc);
		if (comc1)
			com1eval(asl, 0, comc1);
		x0kind |= ASL_have_conval;
		}
	asl->i.ncxval[i] = asl->i.nxval;
	co_index = i;
	d = con_de + i;
	gr0 = Cgrad + i;
	e1 = d->e;
	f = (*e1->op)(e1 C_ASL);
	gr = *gr0;
	if (asl->i.vscale)
		for(V = var_e; gr; gr = gr->next)
			f += gr->coef * V[gr->varno].v;
	else
		for(; gr; gr = gr->next)
			f += gr->coef * X[gr->varno];
	err_jmp = 0;
	if (asl->i.cscale)
		f *= asl->i.cscale[i];
	return f;
	}

 void
#ifdef KR_headers
con2grd_ASL(a, i, X, G, nerror)
	ASL *a; int i; fint *nerror; real *X, *G;
#else
con2grd_ASL(ASL *a, int i, real *X, real *G, fint *nerror)
#endif
{
	cde *d;
	cgrad *gr, **gr0;
	real *Adjoints, *vscale;
	Jmp_buf err_jmp0;
	int i0, ij, L;
	ASL_fgh *asl;
	real scale;
	static char who[] = "con2grd";

	INchk(a, who, i);
	asl = (ASL_fgh*)a;
	if (!want_derivs)
		No_derivs_ASL(who);
	if (nerror && *nerror >= 0) {
		err_jmp = &err_jmp0;
		ij = setjmp(err_jmp0.jb);
		if (*nerror = ij)
			return;
		}
	errno = 0;	/* in case f77 set errno opening files */
	if (!asl->i.x_known)
		x2_check_ASL(asl,X);
	if (!asl->i.ncxval || asl->i.ncxval[i] != asl->i.nxval) {
		want_deriv = 1;
		con2ival_ASL(a,i,X,nerror);
		if (nerror && *nerror)
			return;
		}
	if (!(x0kind & ASL_have_funnel)) {
		if (f_b)
			funnelset(asl, f_b);
		if (f_c)
			funnelset(asl, f_c);
		x0kind |= ASL_have_funnel;
		}
	Adjoints = adjoints;
	d = con_de + i;
	gr0 = Cgrad + i;
	scale = asl->i.cscale ? asl->i.cscale[i] : 1.;
	for(gr = *gr0; gr; gr = gr->next)
		Adjoints[gr->varno] = gr->coef;
	if (L = d->zaplen) {
		memset(adjoints_nv1, 0, L);
		derprop(d->d);
		}
	if (vscale = asl->i.vscale)
		for(gr = *gr0; gr; gr = gr->next) {
			L = gr->varno;
			Adjoints[L] *= vscale[L];
			}
	gr = *gr0;
	i0 = 0;
	switch(asl->i.congrd_mode) {
	  case 1:
		for(; gr; gr = gr->next)
			G[i0++] = scale * Adjoints[gr->varno];
		break;
	  case 2:
		for(; gr; gr = gr->next)
			G[gr->goff] = scale * Adjoints[gr->varno];
		break;
	  default:
		for(; gr; gr = gr->next) {
			i = gr->varno;
			while(i0 < i)
				G[i0++] = 0;
			G[i] = scale * Adjoints[i];
			i0 = i + 1;
			}
		i = n_var;
		while(i0 < i)
			G[i0++] = 0;
	  }
	err_jmp = 0;
	}
