/* -*- C++ -*-
 *
 *  graphics_accelerated.cpp - Accelerated graphics function chooser
 *
 *  Copyright (c) 2021 TellowKrinkle
 *
 *  tellowkrinkle@gmail.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, see <http://www.gnu.org/licenses/>
 *  or write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "graphics_accelerated.h"
#include "graphics_common.h"

#include "graphics_altivec.h"
#include "graphics_mmx.h"
#include "graphics_sse2.h"

#include <stdio.h>

#ifdef USE_X86_GFX
# if defined(__SSE2__)
#  define _M_SSE 0x200
# elif defined(__SSE__)
#  define _M_SSE 0x100
# elif defined(__MMX__)
#  define _M_SSE 0x001
# else
#  define _M_SSE 0x000
#endif

# include <cpuid.h>
#elif defined(USE_PPC_GFX)
# if defined(__linux__) || (defined(__FreeBSD__) && __FreeBSD__ >= 12)
#  ifdef __linux__
#include <asm/cputable.h>
#  else
#include <machine/cpu.h>
#  endif
#include <sys/auxv.h>
# elif defined(MACOSX) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#  if defined(__NetBSD__) || defined(__OpenBSD__)
#include <machine/cpu.h>
#  endif
#include <sys/sysctl.h>
# endif
#endif

void imageFilterMean_Basic(unsigned char *src1, unsigned char *src2, unsigned char *dst, int length) {
    int n = length + 1;
    BASIC_MEAN();
}

void imageFilterAddTo_Basic(unsigned char *src, unsigned char *dst, int length) {
    int n = length + 1;
    BASIC_ADDTO();
}

void imageFilterSubFrom_Basic(unsigned char *src, unsigned char *dst, int length) {
    int n = length + 1;
    BASIC_SUBFROM();
}

void imageFilterBlend_Basic(Uint32 *dst_buffer, Uint32 *src_buffer,
                            Uint8 *alphap, int alpha, int length)
{
    int n = length + 1;
    BASIC_BLEND();
}

AcceleratedGraphicsFunctions AcceleratedGraphicsFunctions::accelerated() {
    AcceleratedGraphicsFunctions out;

#ifdef USE_X86_GFX
    unsigned int func, eax, ebx, ecx, edx;
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx) != 0) {
        printf("System info: Intel CPU, with functions: ");
        if (_M_SSE >= 0x001 || edx & bit_MMX) {
            printf("MMX ");
            out._imageFilterMean = imageFilterMean_MMX;
            out._imageFilterAddTo = imageFilterAddTo_MMX;
            out._imageFilterSubFrom = imageFilterSubFrom_MMX;
        }
        if (_M_SSE >= 0x100 || edx & bit_SSE) {
            printf("SSE ");
        }
        if (_M_SSE >= 0x200 || edx & bit_SSE2) {
            printf("SSE2 ");
            out._imageFilterMean = imageFilterMean_SSE2;
            out._imageFilterAddTo = imageFilterAddTo_SSE2;
            out._imageFilterSubFrom = imageFilterSubFrom_SSE2;
            out._imageFilterBlend = imageFilterBlend_SSE2;
        }
        printf("\n");
    }
#elif defined(USE_PPC_GFX)
    bool has_altivec = false;
# if defined(__linux__) || (defined(__FreeBSD__) && __FreeBSD__ >= 12)
    // Determine if this PPC CPU supports AltiVec
    {
        unsigned long hwcap = 0;
#  ifdef __linux__
        hwcap = getauxval(AT_HWCAP);
#  else
        elf_aux_info(AT_HWCAP, &hwcap, sizeof(hwcap));
#  endif
        if (hwcap & PPC_FEATURE_HAS_ALTIVEC) {
            has_altivec = true;
            printf("System info: PowerPC CPU, supports altivec\n");
        } else {
            printf("System info: PowerPC CPU, DOES NOT support altivec\n");
        }
    }
# elif defined(MACOSX) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    // Determine if this PPC CPU supports AltiVec (Roto)
    {
        int altivec_present = 0;

        size_t length = sizeof(altivec_present);
#  if defined(MACOSX)
        int error = sysctlbyname("hw.optional.altivec", &altivec_present, &length, NULL, 0);
#  elif defined(__FreeBSD__)
        int error = sysctlbyname("hw.altivec", &altivec_present, &length, NULL, 0);
#  else
        int mib[] = { CTL_MACHDEP, CPU_ALTIVEC };
        int error = sysctl(mib, sizeof(mib)/sizeof(mib[0]), &altivec_present, &length, NULL, 0);
#  endif
        if (error) {
            return;
        }
        if (altivec_present) {
            has_altivec = true;
            printf("System info: PowerPC CPU, supports altivec\n");
        } else {
            printf("System info: PowerPC CPU, DOES NOT support altivec\n");
        }
    }
# endif
    if (has_altivec) {
        out._imageFilterMean = imageFilterMean_Altivec;
        out._imageFilterAddTo = imageFilterAddTo_Altivec;
        out._imageFilterSubFrom = imageFilterSubFrom_Altivec;
    }
#endif
    return out;
}
