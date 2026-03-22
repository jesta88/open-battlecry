/*
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Copyright (c) Arvid Gerstmann. All rights reserved.
 */
#ifndef _WINDOWS_
#ifndef WINDOWS_INTRIN_H
#define WINDOWS_INTRIN_H

/* ========================================================================== */
/* Intrinsics                                                                 */
/* ========================================================================== */
extern void _mm_pause(void);
#pragma intrinsic(_mm_pause)

extern void _ReadWriteBarrier(void);
#pragma intrinsic(_ReadWriteBarrier)

#endif /* WINDOWS_INTRIN_H */
#endif /* _WINDOWS_ */

