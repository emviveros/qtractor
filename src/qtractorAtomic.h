// qtractorAtomic.h
//
/****************************************************************************
   Copyright (C) 2005-2006, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*****************************************************************************/

#ifndef __qtractorAtomic_h
#define __qtractorAtomic_h

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct { volatile int value; } qtractorAtomic;

#if defined(__GNUC__)

#if defined(powerpc) || defined(__ppc__)

static inline int ATOMIC_CAS ( volatile void *pAddr,
	volatile void *pOldValue, void *pNewValue )
{
	register int result;
	asm volatile (
		"# ATOMIC_CAS			\n"
		"	lwarx	r0, 0, %1	\n"
		"	cmpw	r0, %2		\n"
		"	bne-	1f          \n"
		"	sync            	\n"
		"	stwcx.	%3, 0, %1	\n"
		"	bne-	1f          \n"
		"   li      %0, 1       \n"
		"	b		2f          \n"
		"1:                     \n"
		"   li      %0, 0       \n"
		"2:                     \n"
	:"=r" (result)
	: "r" (pAddr), "r" (pOldValue), "r" (pNewValue)
	: "r0"
	);
	return result;
}

#elif defined(__i386__) || defined(__x86_64__)

static inline int ATOMIC_CAS ( volatile void *pAddr,
	volatile void *pOldValue, void *pNewValue )
{
	register char result;
	__asm__ __volatile__ (
		"# ATOMIC_CAS            \n\t"
		"lock ; cmpxchg %2, (%1) \n\t"
		"sete %0                 \n\t"
		:"=a" (result)
		:"c" (pAddr), "d" (pNewValue), "a" (pOldValue)
	);
	return result;
}

#else
#   error "qtractorAtomic.h: unsupported target compiler processor (GNU)."
#endif

#elif defined(WIN32) || defined(__WIN32__) || defined(_WIN32)

#if defined(__BORLANDC__)

static inline int ATOMIC_CAS ( volatile void *pAddr,
	volatile void *pOldValue, void *pNewValue )
{
	// Dummy CAS (NON ATOMIC whatsoever;)
	unsigned int *pValue = (unsigned int *) pAddr;
	if (*pValue == (unsigned int) pOldValue) {
	    *pValue =  (unsigned int) pNewValue;
	    return 1;
	} else {
		return 0;
	}
}

#else

static inline int ATOMIC_CAS ( volatile void *pAddr,
	volatile void *pOldValue, void *pNewValue )
{
	register char result;
	__asm {
		push	ebx
		push	esi
		mov		esi, pAddr
		mov		eax, pOldValue
		mov		ebx, pNewValue
		lock	cmpxchg dword ptr [esi], ebx
		sete	result
		pop		esi
		pop		ebx
	}
	return result;
}

#endif

#else
#   error "qtractorAtomic.h: unsupported target compiler processor (WIN32)."
#endif

#define ATOMIC_GET(a)	((a)->value)
#define ATOMIC_SET(a,v)	((a)->value = (v))

static inline int ATOMIC_ADD ( qtractorAtomic *pVal, int add )
{
	volatile int val;
	do {
		val = pVal->value;
	} while (!ATOMIC_CAS(&pVal->value, (void *) val, (void *) (val + add)));
	return val;
}

#if defined(__cplusplus)
}
#endif


#endif // __qtractorAtomic_h

// end of qtractorAtomic.h
