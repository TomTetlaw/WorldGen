#ifndef _MATH_H
#define _MATH_H

#define Max(A, B) ((A) > (B) ? (A) : (B))
#define Min(A, B) ((A) < (B) ? (A) : (B))

f32 Abs(f32 x) {
    if(x < 0.0f) return x * -1.0f;
    return x;
}

s32 Sign(s32 x) {
    if(x < 0) return -1;
    if(x == 0) return 0;
    return 1;
}

f32 Remap(f32 x, f32 SourceMin, f32 SourceMax, f32 DestMin, f32 DestMax) {
    return DestMin + (x - SourceMin) * (DestMax - DestMin) / (SourceMax - SourceMin);
}

struct vec2i {
    s32 x;
    s32 y;
};

vec2i v2i(s32 x, s32 y) {
    vec2i v;
    v.x = x;
    v.y = y;
    return v;
}

vec2i v2i(s32 xy) {
    vec2i v;
    v.x = xy;
    v.y = xy;
    return v;
}

vec2i Maxv2i(vec2i a, vec2i b) {
    return v2i(Max(a.x, b.x), Max(a.y, b.y));
}

vec2i operator+(vec2i a, vec2i b) { return v2i(a.x + b.x, a.y + b.y); }
vec2i operator-(vec2i a, vec2i b) { return v2i(a.x - b.x, a.y - b.y); }
vec2i operator*(vec2i a, vec2i b) { return v2i(a.x * b.x, a.y * b.y); }

struct vec2 {
    f32 x;
    f32 y;
};

vec2 v2(f32 x, f32 y) {
    vec2 v;
    v.x = x;
    v.y = y;
    return v;
}

vec2 v2(f32 xy) {
    vec2 v;
    v.x = xy;
    v.y = xy;
    return v;
}

vec2 v2(vec2i xy) {
    vec2 v;
    v.x = xy.x;
    v.y = xy.y;
    return v;
}

f32 StepFloat(f32 x, f32 Step) {
    s32 Multiples = x / Step;
    return roundf(x / Step) * Step;
}

vec2 operator+(vec2 a, vec2 b)  { return { a.x + b.x, a.y + b.y }; }
vec2 operator-(vec2 a, vec2 b)  { return { a.x - b.x, a.y - b.y }; }
vec2 operator*(vec2 a, vec2 b)  { return { a.x * b.x, a.y * b.y }; }
vec2 operator*(vec2 a, f32 b)   { return { a.x *   b, a.y *   b }; }
vec2 &operator*=(vec2 &a, f32 b) { a = a * b; return a; }

f32 Length(vec2 v) {
    return sqrt(v.x*v.x + v.y*v.y);
}

vec2 Normalise(vec2 v) {
    f32 InverseLength = 1.0f / Length(v);
    return v * v2(InverseLength);
}

f32 Approach(f32 a, f32 b, f32 dt) {
    f32 Diff = b - a;
    if(Diff > dt) return a + dt;
    if(Diff < -dt) return a - dt;
    return b;
}

vec2 Approach(vec2 a, vec2 b, f32 dt) {
    return v2(Approach(a.x, b.x, dt), Approach(a.y, b.y, dt));
}

struct vec3 {
    f32 x;
    f32 y;
    f32 z;
};

vec3 v3(f32 x, f32 y, f32 z) {
    vec3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}


vec3 v3(f32 xyz) {
    vec3 v;
    v.x = xyz;
    v.y = xyz;
    v.z = xyz;
    return v;
}

struct vec4 {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
};

typedef vec4 colour;

vec4 v4(f32 x, f32 y, f32 z, f32 w) {
    vec4 v;
    v.x = x;
    v.y = y;
    v.z = z;
    v.w = w;
    return v;
}

vec4 v4(vec3 xyz, f32 w) {
    vec4 v;
    v.x = xyz.x;
    v.y = xyz.y;
    v.z = xyz.z;
    v.w = w;
    return v;
}

vec4 v4(f32 xyzw) {
    vec4 v;
    v.x = xyzw;
    v.y = xyzw;
    v.z = xyzw;
    v.w = xyzw;
    return v;
}

colour rgba(s32 r, s32 g, s32 b, s32 a) {
    return v4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

union mat4 {
    float e[16];
    float m[4][4];
    vec4 v[4];
    struct {
        float _00, _01, _02, _03, 
        _10, _11, _12, _13,
        _20, _21, _22, _23,
        _30, _31, _32, _33;
	};
};

mat4 CreateIdentityMatrix() {
    mat4 Out;
    Out._00 = 1; Out._01 = 0; Out._02 = 0; Out._03 = 0;
    Out._10 = 0; Out._11 = 1; Out._12 = 0; Out._13 = 0;
    Out._20 = 0; Out._21 = 0; Out._22 = 1; Out._23 = 0;
    Out._30 = 0; Out._31 = 0; Out._32 = 0; Out._33 = 1;
    return Out;
}

mat4 CreateOrthoMatrix(float Left, float Right, float Bottom, float Top) {
    float NearZ = -999.0f;
    float FarZ = 999.0f;
    mat4 Out;
    
    Out.v[0].x = 2.0 / (Right - Left);
    Out.v[0].y = 0;
    Out.v[0].z = 0;
    Out.v[0].w = 0;
    Out.v[1].x = 0;
    Out.v[1].y = 2.0 / (Top - Bottom);
    Out.v[1].z = 0;
    Out.v[1].w = 0;
    Out.v[2].x = 0;
    Out.v[2].y = 0;
    Out.v[2].z = 2.0 / (FarZ - NearZ);
    Out.v[2].w = 0;
    Out.v[3].x = -(Right + Left) / (Right - Left);
    Out.v[3].y = -(Top + Bottom) / (Top - Bottom);
    Out.v[3].z = -(FarZ + NearZ) / (FarZ - NearZ);
    Out.v[3].w = 1;
    
    return Out;
}

#endif //_MATH_H
