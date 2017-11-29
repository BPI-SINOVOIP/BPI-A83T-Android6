/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * All rights reserved.
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

#include "defs.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/utsname.h>

/* Bits of module.flags.  */

#define MOD_UNINITIALIZED	0
#define MOD_RUNNING		1
#define MOD_DELETED		2
#define MOD_AUTOCLEAN		4
#define MOD_VISITED		8
#define MOD_USED_ONCE		16
#define MOD_JUST_FREED		32
#define MOD_INITIALIZING	64

/* Values for query_module's which.  */

#define QM_MODULES	1
#define QM_DEPS		2
#define QM_REFS		3
#define QM_SYMBOLS	4
#define QM_INFO		5

struct module_symbol
{
	unsigned long value;
	const char *name;
};

struct module_info
{
	unsigned long addr;
	unsigned long size;
	unsigned long flags;
	long usecount;
};

#include "xlat/qm_which.h"
#include "xlat/modflags.h"
#include "xlat/delete_module_flags.h"

SYS_FUNC(query_module)
{
	if (entering(tcp)) {
		printstr(tcp, tcp->u_arg[0], -1);
		tprints(", ");
		printxval(qm_which, tcp->u_arg[1], "QM_???");
		tprints(", ");
	} else {
		size_t ret;

		if (!verbose(tcp) || syserror(tcp) ||
		    umove(tcp, tcp->u_arg[4], &ret) < 0) {
			tprintf("%#lx, %lu, %#lx", tcp->u_arg[2],
				tcp->u_arg[3], tcp->u_arg[4]);
		} else if (tcp->u_arg[1]==QM_INFO) {
			struct module_info	mi;
			if (umove(tcp, tcp->u_arg[2], &mi) < 0) {
				tprintf("%#lx, ", tcp->u_arg[2]);
			} else {
				tprintf("{address=%#lx, size=%lu, flags=",
					mi.addr, mi.size);
				printflags(modflags, mi.flags, "MOD_???");
				tprintf(", usecount=%lu}, ", mi.usecount);
			}
			tprintf("%lu", (unsigned long)ret);
		} else if ((tcp->u_arg[1]==QM_MODULES) ||
			   (tcp->u_arg[1]==QM_DEPS) ||
			   (tcp->u_arg[1]==QM_REFS)) {
			tprints("{");
			if (!abbrev(tcp)) {
				char*	data	= malloc(tcp->u_arg[3]);
				char*	mod	= data;
				size_t	idx;

				if (!data) {
					fprintf(stderr, "out of memory\n");
					tprintf(" /* %lu entries */ ", (unsigned long)ret);
				} else {
					if (umoven(tcp, tcp->u_arg[2],
						tcp->u_arg[3], data) < 0) {
						tprintf(" /* %lu entries */ ", (unsigned long)ret);
					} else {
						for (idx = 0; idx < ret; idx++) {
							tprintf("%s%s",
								(idx ? ", " : ""),
								mod);
							mod += strlen(mod)+1;
						}
					}
					free(data);
				}
			} else
				tprintf(" /* %lu entries */ ", (unsigned long)ret);
			tprintf("}, %lu", (unsigned long)ret);
		} else if (tcp->u_arg[1]==QM_SYMBOLS) {
			tprints("{");
			if (!abbrev(tcp)) {
				char*			data	= malloc(tcp->u_arg[3]);
				struct module_symbol*	sym	= (struct module_symbol*)data;
				size_t			idx;

				if (!data) {
					fprintf(stderr, "out of memory\n");
					tprintf(" /* %lu entries */ ", (unsigned long)ret);
				} else {
					if (umoven(tcp, tcp->u_arg[2],
						tcp->u_arg[3], data) < 0) {
						tprintf(" /* %lu entries */ ", (unsigned long)ret);
					} else {
						for (idx = 0; idx < ret; idx++) {
							tprintf("%s{name=%s, value=%lu}",
								(idx ? " " : ""),
								data+(long)sym->name,
								sym->value);
							sym++;
						}
					}
					free(data);
				}
			} else
				tprintf(" /* %lu entries */ ", (unsigned long)ret);
			tprintf("}, %ld", (unsigned long)ret);
		} else {
			printstr(tcp, tcp->u_arg[2], tcp->u_arg[3]);
			tprintf(", %#lx", tcp->u_arg[4]);
		}
	}
	return 0;
}

SYS_FUNC(create_module)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", %lu", tcp->u_arg[1]);
	}
	return RVAL_HEX;
}

SYS_FUNC(delete_module)
{
	if (entering(tcp)) {
		printstr(tcp, tcp->u_arg[0], -1);
		tprints(", ");
		printflags(delete_module_flags, tcp->u_arg[1], "O_???");
	}
	return 0;
}

SYS_FUNC(init_module)
{
	if (entering(tcp)) {
		tprintf("%#lx, %lu, ", tcp->u_arg[0], tcp->u_arg[1]);
		printstr(tcp, tcp->u_arg[2], -1);
	}
	return 0;
}

#define MODULE_INIT_IGNORE_MODVERSIONS  1
#define MODULE_INIT_IGNORE_VERMAGIC     2

#include "xlat/module_init_flags.h"

SYS_FUNC(finit_module)
{
	if (exiting(tcp))
		return 0;

	/* file descriptor */
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	/* param_values */
	printstr(tcp, tcp->u_arg[1], -1);
	tprints(", ");
	/* flags */
	printflags(module_init_flags, tcp->u_arg[2], "MODULE_INIT_???");

	return 0;
}
