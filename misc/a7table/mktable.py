#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  untitled.py
#  
#  Copyright 2012 Juan I Carrano <juan@superfreak.com.ar>
#  
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.

import sys
from textwrap import wrap

def _list2carray(l):
	return "\n\t".join(wrap(", ".join("0x%x"%ord(c) for c in l)))

def list2carray(name, l, spec):
	a = (spec + ' 'if spec else '') + 'char ' + name + '[]'
	defn = a + ' = \n\t{' + _list2carray(l) + '};'
	decl = 'extern ' + a + ';'
	
	return defn, decl

def define(f, k, v):
	f.write("#define {0} {1}\n".format(k, v))
	
C_HEADER = """/* Tabla de ascii a 7 segmentos 
*	Grupo 2 - 2012
*	****** Codigo generado automaticamente. NO EDITAR ********
*/

"""

H_HEADER = """/*tabla de ascii a 7 segmentos
*	Grupo 2 - 2012
*	****** Codigo generado automaticamente. NO EDITAR ********
*/

#ifndef _ASCII_7_TBL_
#define _ASCII_7_TBL_

"""

H_FOOTER = """

#endif /* _ASCII_7_TBL_ */
/* fin del codigo generado automaticamente */
"""

C_FOOTER = """

/* fin del codigo generado automaticamente */
"""

LETRAS = open("letras").readlines()
NUMS = open("numeros").readlines()
SIMS = open("sims").readlines()

DATA = [0]*(2**8)

TERM = 0x00

L_OFFSET_u = ord('A')
L_OFFSET_l = ord('a')
N_OFFSET = ord('0')

DATA[0] = TERM

i = 0
for l in LETRAS:
	code = int(l)
	DATA[L_OFFSET_l+i] = code
	DATA[L_OFFSET_u+i] = code
	i += 1

i = 0
	
for l in NUMS:
	code = int(l)
	DATA[N_OFFSET+i] = code
	i += 1

for l in SIMS:
	sim, code = l.split()
	DATA[ord(sim)] = int(code)


xtable = "".join(chr(c) for c in DATA)

def main():
	fc = open('a7table.c', 'wb')
	fh = open('a7table.h', 'wb')
	
	#f = open('xtable', 'wb')
	
	fc.write(C_HEADER)
	fh.write(H_HEADER)
	
	define(fh, 'DOT_CODE', ord(xtable[ord('.')]))
	
	df, dc = list2carray('ascii_to_7', xtable, 'const')
	fc.write(df)
	fh.write(dc)
	
	fh.write(H_FOOTER)
	fc.write(C_FOOTER)
	
	#f.close()
	
	return 0

if __name__ == '__main__':
	main()

