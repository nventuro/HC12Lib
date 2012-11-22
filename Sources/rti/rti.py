#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  untitled.py
#  
#  Lab de micros - Grupo 2 - 2012
#
# Lasagna code!!!!!!!!!!!!!!!!!!!
#

import sys
from textwrap import wrap
from collections import OrderedDict

CHNAME = "RTI_SRV_TBL"
DPREFIX = "SRV_"
DEF_SRV_FILE = "services"
COMMENT_CHAR = '#'
CMD_CHAR = '%'
DEF_INCLUDES = ('"../common.h"','"rti2.h"')

def ts(f):
	def ff(x, k, p):
		 p[k] = x
	
	return ff

def iden(x, k, p):
	p[k] = x

tsint = ts(int)

def fo_parse(s, k, par):
	try:
		nsteps = int(s)
		msecs = None
	except ValueError:
		nsteps = None
		if s.endswith('ms'):
			msecs = int(s.split('ms')[0])
		elif s.endswith('s'):
			msecs = int(s.split('s')[0])*1000
		else:
			raise ValueError("invalid unit: %s"%s)
		
		smsecs = "MS2PERIOD(%d)"%msecs
	
	par[k] = {'steps': nsteps, 'msecs': msecs, 'toprint': nsteps or smsecs}

def step_parse(s, k, par):
	try:
		dummy = int(s)
		v = s
	except ValueError:
		if s.endswith('ms'):
			msecs = int(s.split('ms')[0])
		elif s.endswith('s'):
			msecs = int(s.split('s')[0])*1000
		else:
			raise ValueError("invalid unit: %s"%s)
		
		if par['f_osc']['steps'] is not None:
			v = "(MS2PERIOD(%s)/%s)"%(msecs, str(par['f_osc']['steps']))
		else:
			v = "(%s/%s)"%(msecs, str(par['f_osc']['msecs']))
	
	par[k] = v

_srv_params = (('name', iden), ('enabled', tsint), ('f_osc', fo_parse), 
		('pha_osc', tsint), ('r_init', tsint), ('r_steps', step_parse),
			('r_ths', tsint), ('func', iden), ('data', iden))

srv_params = OrderedDict()
srv_params.update(_srv_params)

TBL_ELEMENT = (
"""/* {name} */
	{{{enabled}, 0, {{{f_osc[toprint]}, {pha_osc}}}, {r_init}, {r_steps}, {r_ths}, {func}, {data}}}"""
)

TBL_DEF = (
"""struct rti_srv rti_srv_tbl[] = {
%s	
};
"""
)

#fun_tbl_decl = "extern rti_srv rti_funcs[];\n"

def define(f, k, v):
	f.write("#define {0} {1}\n".format(k, v))

def include(f, header):
	f.write("#include %s\n"%header)

def enum(f, l):
	f.write('\n\t'.join(wrap("enum {%s};"%(", ".join(l)))))
	f.write("\n")

def h_start(f, name):
	f.write("#ifndef __%s__\n"%name)
	f.write("#define __%s__\n"%name)
	f.write("\n")

def h_end(f, name):
	f.write("#endif /* __%s__ */\n"%name)

def mk_tbl_line(e):
	return TBL_ELEMENT.format(**e)


def mk_tbl(l):
	return TBL_DEF%(',\n'.join(mk_tbl_line(e) for e in l))
	
C_HEADER = """/* Tabla de servicios de la RTI
 *	Grupo 2 - 2012
 *	****** Codigo generado automaticamente. NO EDITAR ********
 */

"""

H_HEADER = """/*Tabla de servicios de la RTI
 *	Definiciones
 *	Grupo 2 - 2012
 *	****** Codigo generado automaticamente. NO EDITAR ********
 */
"""

H_FOOTER = """
/* fin del codigo generado automaticamente, espero que no lo hayas editado.*/
"""

C_FOOTER = H_FOOTER

def send_err(s):
	sys.stderr.write("Invalid formatting: \n%s\n"%s)
	
def send_line(lineno):
	sys.stderr.write("In line %d:\n"%(lineno+1))

def main():
	try:
		srvfn = sys.argv[1]
		srvf = open(srvfn if srvfn not in ('-', '--') else DEF_SRV_FILE)
	except IndexError:
		srvf = open(DEF_SRV_FILE)
	
	srv_table = []
	includes = list(DEF_INCLUDES)
	
	for lineno, li in enumerate(srvf):
		if not li or li.isspace():
			continue
		elif li[0] == COMMENT_CHAR:
			continue
		elif li[0] == CMD_CHAR:
			if li[1:].startswith('include'):
				includes.append(li.split('include')[1].strip())
			else:
				send_line(lineno)
				send_err(li)
				exit(1)
		else:
			params = {}
			srv_tmpdata = {}
			raw_params = li.strip().split()
			
			if not len(srv_params) == len(raw_params):
				send_line(lineno)
				send_err(li)
				sys.stderr.write(
					"Error: I have %d colums, I need %d\n" %
					(len(raw_params), len(srv_params), ))
				exit(1)
			
			try:
				for (k, uparser), rv in zip(srv_params.items(), raw_params):
					uparser(rv, k, params)
			except ValueError as ex:
				send_line(lineno)
				send_err(li)
				sys.stderr.write(ex.message + '\n')
				exit(1)
			
			srv_table.append(params)
	
	fun_table = mk_tbl(srv_table);
	
	if 'write' in sys.argv[2:]:
		fc = open('rti_srvs.c', 'wb')
		fh = open('rti_srvs.h', 'wb')
	else:
		fc = sys.stdout
		fh = fc
	
	fc.write(C_HEADER)
	for header in includes:
		include(fc, header)
	
	fc.write('\n')
	
	fc.write(fun_table)

	fc.write(C_FOOTER)
	
	
	fh.write(H_HEADER)
	h_start(fh, CHNAME)
	
	snames = [DPREFIX+v['name'] for v in srv_table]
	snames.append("RTI_N_SRVS")
	
	enum(fh, snames)
	
	fh.write('\n')
	
	#fh.write(fun_tbl_decl)
	
	#fh.write('\n')
	
	h_end(fh, CHNAME)
	fh.write(H_FOOTER)
	
	#f.close()
	
	return 0

if __name__ == '__main__':
	main()
