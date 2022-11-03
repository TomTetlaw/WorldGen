#ifndef UTILS_H
#define UTILS_H

#ifdef DEBUG_BUILD
#ifdef WINDOWS_BUILD
#define DebugPlaceholder OutputDebugString("")
#else
#error unimplemented
#endif
#else
#define DebugPlaceholder
#endif

#define ArrayCount(a) (sizeof(a)/sizeof(a[0]))

s32 StringLength(s8 *String);
bool StringCompare(s8 *A, s8 *B);
void Set(void *Dest, s32 Value, s64 Size);
void Copy(void *Dest, void *Source, s64 Size);
void Zero(void *Dest, s64 Size);
s8 *Format(s8 *Format, ...);

#endif