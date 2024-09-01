/**
 * @file ConvenienceMacros.h
 * @author amarioguy (Arminder Singh)
 * 
 * Macros used for convenience/readability reasons in the code.
 * Some are borrowed from Linux or m1n1 (for example to help with bitmasks)
 * @version 1.0
 * @date 2022-10-09
 * 
 * @copyright Copyright (c) amarioguy (Arminder Singh), 2022.
 * 
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 * 
 */

#ifndef CONVENIENCE_MACROS_H
#define CONVENIENCE_MACROS_H

//borrowed from m1n1
#define BIT(x) (1UL << (x))
#define GENMASK(msb, lsb) ((BIT((msb + 1) - (lsb)) - 1) << (lsb))

//While EDK2 does have code that reads bitfields from values, the m1n1 bitfield macros
//seem easier to deal with, so put these in the code anyways
#define _FIELD_LSB(field)      ((field) & ~(field - 1))

#define FIELD_PREP(field, val) ((val) * (_FIELD_LSB(field)))
#define FIELD_GET(field, val)  (((val) & (field)) / _FIELD_LSB(field))

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

#define ALIGN(x,a)		__ALIGN_MASK((x),(typeof(x))(a)-1)
#define ALIGN_DOWN(x, a)	ALIGN((x) - ((a) - 1), (a))
#define __ALIGN_MASK(x,mask)	(((x)+(mask))&~(mask))
#define PTR_ALIGN(p, a)		((typeof(p))ALIGN((unsigned long)(p), (a)))

#endif //CONVENIENCE_MACROS_H