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
#include "graphics_ssse3.h"

#include <stdio.h>

#ifdef USE_X86_GFX
# if defined(__SSSE3__)
#  define _M_SSE 0x301
# elif defined(__SSE2__)
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
    for (int i = 0; i < length; i++) {
        dst[i] = mean_pixel(src1[i], src2[i]);
    }
}

void imageFilterAddTo_Basic(unsigned char *src, unsigned char *dst, int length) {
    for (int i = 0; i < length; i++) {
        addto_pixel(dst[i], src[i]);
    }
}

void imageFilterSubFrom_Basic(unsigned char *src, unsigned char *dst, int length) {
    for (int i = 0; i < length; i++) {
        subfrom_pixel(dst[i], src[i]);
    }
}

void imageFilterBlend_Basic(Uint32 *dst_buffer, Uint32 *src_buffer,
                            Uint8 *alphap, int alpha, int length)
{
    int n = length + 1;
    BASIC_BLEND();
}

bool alphaMaskBlend_Basic(SDL_Surface* dst, SDL_Surface *s1, SDL_Surface *s2, SDL_Surface *mask_surface, const SDL_Rect& rect, Uint32 mask_value) {
    return false;
}

void alphaMaskBlendConst_Basic(SDL_Surface* dst, SDL_Surface *s1, SDL_Surface *s2, const SDL_Rect& rect, Uint32 mask_value)
{
    int end_x = rect.x + rect.w;
    int end_y = rect.y + rect.h;
    for (int y = rect.y; y < end_y; y++) {
        Uint32* s1p = getPointerToRow<Uint32>(s1, y);
        Uint32* s2p = getPointerToRow<Uint32>(s2, y);
        Uint32* dstp = getPointerToRow<Uint32>(dst, y);
        for (int x = rect.x; x < end_x; x++) {
            dstp[x] = blendMaskOnePixel(s1p[x], s2p[x], 0, mask_value);
        }
    }
}

#ifdef USE_X86_GFX
enum Manufacturer {
    MF_UNKNOWN,
    MF_INTEL,
    MF_AMD,
};

static bool hasFastPSHUFB(Manufacturer mf, int eax, int ecx) {
    if (!(ecx & bit_SSSE3)) { return false; }
    if (mf != MF_INTEL) { return true; }
    static const uint8_t SLOW_PSHUFB[] = { // From https://en.wikichip.org/wiki/intel/cpuid
        0x0F, 0x16, // Merom
        0x1C, 0x2C, // Bonnell
        0x27, 0x35, 0x36, // Saltwell
        0x37, 0x4A, 0x4D, 0x5A, 0x5D, // Silvermont
        0x4C, // Airmont
    };
    uint8_t family = (eax >> 8) & 0xF;
    if (family != 6) { return true; }
    uint8_t model = (eax >> 4) & 0xF;
    model |= (eax >> 12) & 0xF0;
    for (int i = 0; i < sizeof(SLOW_PSHUFB); i++) {
        if (SLOW_PSHUFB[i] == model) {
            return false;
        }
    }
    return true;
}
#endif

AcceleratedGraphicsFunctions AcceleratedGraphicsFunctions::accelerated() {
    AcceleratedGraphicsFunctions out;

#ifdef USE_X86_GFX
    Manufacturer mf = MF_UNKNOWN;
    unsigned int func, eax, ebx, ecx, edx;
    if (__get_cpuid(0, &eax, &ebx, &ecx, &edx) != 0) {
        if (ebx == 0x68747541 && edx == 0x69746e65 && ecx == 0x444d4163) {
            mf = MF_AMD;
        }
        if (ebx == 0x756e6547 && edx == 0x49656e69 && ecx == 0x6c65746e) {
            mf = MF_INTEL;
        }
    }
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
            out._alphaMaskBlend = alphaMaskBlend_SSE2;
            out._alphaMaskBlendConst = alphaMaskBlendConst_SSE2;
        }
        if (_M_SSE >= 0x301 || hasFastPSHUFB(mf, eax, ecx)) {
            printf("SSSE3 ");
            out._imageFilterBlend = imageFilterBlend_SSSE3;
            out._alphaMaskBlend = alphaMaskBlend_SSSE3;
            out._alphaMaskBlendConst = alphaMaskBlendConst_SSSE3;
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
