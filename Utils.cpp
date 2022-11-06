#include "Includes.h"

s32 StringLength(s8 *String) {
    s32 Length = 0;
    while(*String) {
        String++;
        Length++;
    }
    return Length;
}

bool StringCompare(s8 *A, s8 *B) {
    while(*A) {
        if(*A != *B) return false;
        A++;
        B++;
    }
    
    return true;
}

void Set(void *Dest, s32 Value, s64 Size) {
    s8 *Place = (s8 *)Dest;
    while(Size--) {
        *Place++ = Value;
    }
}

void Zero(void *Dest, s64 Size) {
    s8 *Place = (s8 *)Dest;
    while(Size--) {
        *Place++ = 0;
    }
}

void FastCopy(void *Dest, void *Source, s64 Size) {
    __m256i *Ptr = (__m256i *)Dest;
    __m256i *Src = (__m256i *)Source;
    s32 Num = Size / 32;
    s32 Remainder = Size % 32;
    while(Num--) _mm256_store_si256(Ptr++, *Src++);
    s32 *Rest = (s32 *)Ptr;
    s32 *RestSrc = (s32 *)Src;
    while(Remainder--) {
        *Rest++ = *RestSrc++;
    }
}

void Copy(void *Dest, void *Source, s64 Size) {
    s8 *SourcePlace = (s8 *)Source;
    s8 *DestPlace = (s8 *)Dest;
    
    while(Size--) {
        *DestPlace++ = *SourcePlace++;
    }
}

s8 *Format(s8 *Format, ...) {
    static s8 Buffer[1024];
    va_list Arguments;
    
    va_start(Arguments, Format);
    stb_vsnprintf(Buffer, 1024, Format, Arguments);
    va_end(Arguments);
    
    return Buffer;
}