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