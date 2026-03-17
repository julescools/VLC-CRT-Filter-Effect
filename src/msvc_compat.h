/*****************************************************************************
 * msvc_compat.h : Compatibility shims for building VLC plugins with MSVC
 *****************************************************************************
 * VLC headers assume GCC/MinGW and use POSIX types and macros that MSVC
 * does not provide. This header bridges the gap.
 *****************************************************************************/

#ifndef MSVC_COMPAT_H
#define MSVC_COMPAT_H

/* ssize_t: POSIX signed size type, not defined in MSVC C headers.
 * Used by vlc_arrays.h and vlc_configuration.h */
#include <stdint.h>
#if !defined(ssize_t) && !defined(_SSIZE_T_DEFINED)
  typedef intptr_t ssize_t;
  #define _SSIZE_T_DEFINED
#endif

/* N_(): VLC uses this for gettext translatable strings.
 * In plugins it's a no-op passthrough. */
#ifndef N_
  #define N_(x) (x)
#endif

#endif /* MSVC_COMPAT_H */
