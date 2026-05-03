#ifndef BASE_MATH_H_
#define BASE_MATH_H_

#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef BASE_MATH_STATIC
#define BASE_MATH_API static
#else
#define BASE_MATH_API extern
#endif /* BASE_MATH_H_ */

typedef struct { float x, y; } Vec2;

BASE_MATH_API Vec2  vec2Add       (Vec2 a, Vec2 b);
BASE_MATH_API Vec2  vec2Sub       (Vec2 a, Vec2 b);
BASE_MATH_API Vec2  vec2Mul       (Vec2 a, Vec2 b);
BASE_MATH_API Vec2  vec2Scale     (Vec2 a, float s);
BASE_MATH_API Vec2  vec2Rotate    (Vec2 a, float rads);
BASE_MATH_API Vec2  vec2Normalize (Vec2 a);
BASE_MATH_API float vec2Dot       (Vec2 a, Vec2 b);
BASE_MATH_API float vec2Len       (Vec2 a);
BASE_MATH_API float vec2LenSq     (Vec2 a);
BASE_MATH_API float vec2Distance  (Vec2 a, Vec2 b);

typedef struct { float x, y, z; } Vec3;

BASE_MATH_API Vec2  vec3ToVec2    (Vec3 a);
BASE_MATH_API Vec3  vec3FromVec2  (Vec2 a, float z);
BASE_MATH_API Vec3  vec3Add       (Vec3 a, Vec3 b);
BASE_MATH_API Vec3  vec3Sub       (Vec3 a, Vec3 b);
BASE_MATH_API Vec3  vec3Mul       (Vec3 a, Vec3 b);
BASE_MATH_API Vec3  vec3Cross     (Vec3 a, Vec3 b);
BASE_MATH_API Vec3  vec3Scale     (Vec3 a, float s);
BASE_MATH_API Vec3  vec3Rotate    (Vec3 a, Vec3 axis, float rads);
BASE_MATH_API Vec3  vec3Normalize (Vec3 a);
BASE_MATH_API float vec3Dot       (Vec3 a, Vec3 b);
BASE_MATH_API float vec3Len       (Vec3 a);
BASE_MATH_API float vec3LenSq     (Vec3 a);
BASE_MATH_API float vec3Distance  (Vec3 a, Vec3 b);

typedef struct { float x, y, z, w; } Vec4;

BASE_MATH_API Vec3  vec4ToVec3    (Vec4 a);
BASE_MATH_API Vec4  vec4FromVec3  (Vec3 a, float w);
BASE_MATH_API Vec4  vec4Add       (Vec4 a, Vec4 b);
BASE_MATH_API Vec4  vec4Sub       (Vec4 a, Vec4 b);
BASE_MATH_API Vec4  vec4Mul       (Vec4 a, Vec4 b);
BASE_MATH_API Vec4  vec4Scale     (Vec4 a, float s);
BASE_MATH_API Vec4  vec4Normalize (Vec4 a);
BASE_MATH_API float vec4Dot       (Vec4 a, Vec4 b);
BASE_MATH_API float vec4Len       (Vec4 a);
BASE_MATH_API float vec4LenSq     (Vec4 a);
BASE_MATH_API float vec4Distance  (Vec4 a, Vec4 b);

typedef struct { float x, y, z, w; } Quat;

BASE_MATH_API Quat  quatIdentity      (void);
BASE_MATH_API Quat  quatFromAxisAngle (Vec3 axis, float rads);
BASE_MATH_API Quat  quatFromEuler     (float pitch, float yaw, float roll);
BASE_MATH_API Quat  quatAdd           (Quat a, Quat b);
BASE_MATH_API Quat  quatSub           (Quat a, Quat b);
BASE_MATH_API Quat  quatMul           (Quat a, Quat b);
BASE_MATH_API Quat  quatScale         (Quat a, float s);
BASE_MATH_API Quat  quatConjugate     (Quat a);
BASE_MATH_API Quat  quatInverse       (Quat a);
BASE_MATH_API Quat  quatNormalize     (Quat a);
BASE_MATH_API Vec3  quatRotateVec3    (Quat a, Vec3 v);
BASE_MATH_API float quatDot           (Quat a, Quat b);
BASE_MATH_API float quatLen           (Quat a);
BASE_MATH_API float quatLenSq         (Quat a);

typedef struct { float m[4][4]; } Mat4;

BASE_MATH_API Mat4  mat4Identity  (void);
BASE_MATH_API Mat4  mat4Zero      (void);
BASE_MATH_API Mat4  mat4FromQuat  (Quat a);
BASE_MATH_API Mat4  mat4Transpose (Mat4 a);
BASE_MATH_API Mat4  mat4Mul       (Mat4 a, Mat4 b);
BASE_MATH_API Mat4  mat4RotateX   (Mat4 a, float rads);
BASE_MATH_API Mat4  mat4RotateY   (Mat4 a, float rads);
BASE_MATH_API Mat4  mat4RotateZ   (Mat4 a, float rads);
BASE_MATH_API Mat4  mat4Scale     (Mat4 a, float s);
BASE_MATH_API bool  mat4Inverse   (Mat4 a, Mat4 *out);
BASE_MATH_API Vec4  mat4MulVec4   (Mat4 a, Vec4 v);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BASE_MATH_H_ */

#ifdef BASE_MATH_IMPLEMENTATION

BASE_MATH_API Vec2 vec2Add(Vec2 a, Vec2 b) {
  return (Vec2) { a.x + b.x, a.y + b.y };
}

BASE_MATH_API Vec2 vec2Sub(Vec2 a, Vec2 b) {
  return (Vec2) { a.x - b.x, a.y - b.y };
}

BASE_MATH_API Vec2 vec2Mul(Vec2 a, Vec2 b) {
  return (Vec2) { a.x * b.x, a.y * b.y };
}

BASE_MATH_API Vec2  vec2Scale(Vec2 a, float s) {
  return (Vec2) { a.x * s, a.y * s };
}

BASE_MATH_API Vec2 vec2Rotate(Vec2 a, float rads) {
  float c = cosf(rads);
  float s = sinf(rads);
  return (Vec2) { a.x * c - a.y * s, a.x * s + a.y * c };
}

BASE_MATH_API Vec2 vec2Normalize (Vec2 a) {
  float lsq = vec2LenSq(a);
  if (lsq == 0.f) return (Vec2) { 0 };
  return vec2Scale(a, 1.f / sqrtf(lsq));
}

BASE_MATH_API float vec2Dot(Vec2 a, Vec2 b) {
  return a.x * b.x + a.y * b.y;
}

BASE_MATH_API float vec2Len(Vec2 a) {
  return sqrtf(vec2LenSq(a));
}

BASE_MATH_API float vec2LenSq(Vec2 a) {
  return a.x * a.x + a.y * a.y;
}

BASE_MATH_API float vec2Distance(Vec2 a, Vec2 b) {
  return vec2Len(vec2Sub(a, b));
}

BASE_MATH_API Vec2 vec3ToVec2(Vec3 a) {
  return (Vec2) { a.x, a.y };
}

BASE_MATH_API Vec3 vec3FromVec2(Vec2 a, float z) {
  return (Vec3) { a.x, a.y, z };
}

BASE_MATH_API Vec3 vec3Add(Vec3 a, Vec3 b) {
  return (Vec3) { a.x + b.x, a.y + b.y, a.z + b.z };
}

BASE_MATH_API Vec3 vec3Sub(Vec3 a, Vec3 b) {
  return (Vec3) { a.x - b.x, a.y - b.y, a.z - b.z };
}

BASE_MATH_API Vec3 vec3Mul(Vec3 a, Vec3 b) {
  return (Vec3) { a.x * b.x, a.y * b.y, a.z * b.z };
}

BASE_MATH_API Vec3 vec3Cross(Vec3 a, Vec3 b) {
  return (Vec3) {
    a.y * b.z - a.z * b.y,
    a.z * b.x - a.x * b.z,
    a.x * b.y - a.y * b.x,
  };
}

BASE_MATH_API Vec3 vec3Scale(Vec3 a, float s) {
  return (Vec3) { a.x * s, a.y * s, a.z * s };
}

BASE_MATH_API Vec3 vec3Rotate(Vec3 a, Vec3 axis, float rads) {
  axis = vec3Normalize(axis);
  Vec3 aPar = vec3Scale(axis, vec3Dot(a, axis));
  Vec3 aPerp = vec3Sub(a, aPar);
  Vec3 w = vec3Cross(axis, aPerp);
  return (Vec3) {
    aPar.x + cosf(rads) * aPerp.x + sinf(rads) * w.x,
    aPar.y + cosf(rads) * aPerp.y + sinf(rads) * w.y,
    aPar.z + cosf(rads) * aPerp.z + sinf(rads) * w.z,
  };
}

BASE_MATH_API Vec3 vec3Normalize(Vec3 a) {
  float lsq = vec3LenSq(a);
  if (lsq == 0.f) return (Vec3) { 0 };
  return vec3Scale(a, 1.f / sqrtf(lsq));
}

BASE_MATH_API float vec3Dot(Vec3 a, Vec3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

BASE_MATH_API float vec3Len(Vec3 a) {
  return sqrtf(vec3LenSq(a));
}

BASE_MATH_API float vec3LenSq(Vec3 a) {
  return a.x * a.x + a.y * a.y + a.z * a.z;
}

BASE_MATH_API float vec3Distance(Vec3 a, Vec3 b) {
  return vec3Len(vec3Sub(a, b));
}

BASE_MATH_API Vec3 vec4ToVec3(Vec4 a) {
  return (Vec3) { a.x, a.y, a.z };
}

BASE_MATH_API Vec4 vec4FromVec3(Vec3 a, float w) {
  return (Vec4) { a.x, a.y, a.z, w };
}

BASE_MATH_API Vec4 vec4Add(Vec4 a, Vec4 b) {
  return (Vec4) { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

BASE_MATH_API Vec4 vec4Sub(Vec4 a, Vec4 b) {
  return (Vec4) { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

BASE_MATH_API Vec4 vec4Mul(Vec4 a, Vec4 b) {
  return (Vec4) { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
}

BASE_MATH_API Vec4 vec4Scale(Vec4 a, float s) {
  return (Vec4) { a.x * s, a.y * s, a.z * s, a.w * s };
}

BASE_MATH_API Vec4 vec4Normalize (Vec4 a) {
  float lsq = vec4LenSq(a);
  if (lsq == 0.f) return (Vec4) { 0 };
  return vec4Scale(a, 1.f / sqrtf(lsq));
}

BASE_MATH_API float vec4Dot(Vec4 a, Vec4 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

BASE_MATH_API float vec4Len(Vec4 a) {
  return sqrtf(vec4LenSq(a));
}

BASE_MATH_API float vec4LenSq(Vec4 a) {
  return a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w;
}

BASE_MATH_API float vec4Distance(Vec4 a, Vec4 b) {
  return vec4Len(vec4Sub(a, b));
}

BASE_MATH_API Quat quatIdentity(void) {
  return (Quat) { 0.f, 0.f, 0.f, 1.f };
}
BASE_MATH_API Quat quatFromAxisAngle(Vec3 axis, float rads) {
  axis = vec3Normalize(axis);
  float s = sinf(rads * 0.5f);
  float c = cosf(rads * 0.5f);
  return (Quat) { axis.x * s, axis.y * s, axis.z * s, c };
}

BASE_MATH_API Quat quatFromEuler(float pitch, float yaw, float roll) {
  float cosP = cosf(pitch * 0.5f);
  float sinP = sinf(pitch * 0.5f);
  float cosY = cosf(yaw * 0.5f);
  float sinY = sinf(yaw * 0.5f);
  float cosR = cosf(roll * 0.5f);
  float sinR = sinf(roll * 0.5f);

  return (Quat) {
    sinR * cosP * cosY - cosR * sinP * sinY,
    cosR * sinP * cosY - sinR * cosP * sinY,
    cosR * cosP * sinY - sinR * sinP * cosY,
    cosR * cosP * cosY - sinR * sinP * sinY
  };
}

BASE_MATH_API Quat quatAdd(Quat a, Quat b) {
  return (Quat) { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

BASE_MATH_API Quat quatSub(Quat a, Quat b) {
  return (Quat) { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

BASE_MATH_API Quat quatMul(Quat a, Quat b) {
  return (Quat) {
    a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
    a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
    a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
    a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
  };
}

BASE_MATH_API Quat quatScale(Quat a, float s) {
  return (Quat) { a.x * s, a.y * s, a.z * s, a.w * s };
}

BASE_MATH_API Quat quatConjugate(Quat a) {
  return (Quat) { -a.x, -a.y, -a.z, a.w };
}

BASE_MATH_API Quat quatInverse(Quat a) {
  float lsq = quatLenSq(a);
  if (lsq == 0.f) return quatIdentity();
  return quatScale(quatConjugate(a), 1.f / lsq);
}

BASE_MATH_API Quat quatNormalize(Quat a) {
  float lsq = quatLenSq(a);
  if (lsq == 0.f) return quatIdentity();
  return quatScale(a, 1.f / sqrtf(lsq));
}

BASE_MATH_API Vec3 quatRotateVec3(Quat a, Vec3 v) {
  Vec3 vecQuat = (Vec3) { a.x, a.y, a.z };
  Vec3 t = vec3Scale(vec3Cross(vecQuat, v), 2.f);
  Vec3 u = vec3Cross(vecQuat, t);
  return (Vec3) {
    v.x + a.w * t.x + u.x,
    v.y + a.w * t.y + u.y,
    v.z + a.w * t.z + u.z,
  };
}

BASE_MATH_API float quatDot(Quat a, Quat b) {
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

BASE_MATH_API float quatLen(Quat a) {
  return sqrtf(quatLenSq(a));
}

BASE_MATH_API float quatLenSq(Quat a) {
  return a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w;
}

BASE_MATH_API Mat4 mat4Identity(void) {
  return (Mat4) {
    {{ 1.f, 0.f, 0.f, 0.f },
     { 0.f, 1.f, 0.f, 0.f },
     { 0.f, 0.f, 1.f, 0.f },
     { 0.f, 0.f, 0.f, 1.f }}
  };
}

BASE_MATH_API Mat4 mat4Zero(void) {
  return (Mat4) { 0 };
}

BASE_MATH_API Mat4 mat4FromQuat(Quat a) {
  float lsq = quatLenSq(a);
  if (lsq == 0.f) return mat4Identity();

  float qInvLen = 1.f / sqrtf(lsq);
  float x = a.x * qInvLen;
  float y = a.y * qInvLen;
  float z = a.z * qInvLen;
  float w = a.w * qInvLen;

  float xx = x * x;
  float yy = y * y;
  float zz = z * z;
  float xy = x * y;
  float xz = x * z;
  float yz = y * z;
  float wx = w * x;
  float wy = w * y;
  float wz = w * z;

  Mat4 m = mat4Identity();

  m.m[0][0] = 1.f - 2.f * (yy + zz);
  m.m[0][1] = 2.f * (xy - wz);
  m.m[0][2] = 2.f * (xz + wy);

  m.m[1][0] = 2.f * (xy + wz);
  m.m[1][1] = 1.f - 2.f * (xx + zz);
  m.m[1][2] = 2.f * (yz - wx);

  m.m[2][0] = 2.f * (xz - wy);
  m.m[2][1] = 2.f * (yz + wx);
  m.m[2][2] = 1.f - 2.f * (xx + yy);

  return m;
}

BASE_MATH_API Mat4 mat4Transpose(Mat4 a) {
  return (Mat4) {
    {{ a.m[0][0], a.m[1][0], a.m[2][0], a.m[3][0] },
     { a.m[0][1], a.m[1][1], a.m[2][1], a.m[3][1] },
     { a.m[0][2], a.m[1][2], a.m[2][2], a.m[3][2] },
     { a.m[0][3], a.m[1][3], a.m[2][3], a.m[3][3] }}
  };
}

BASE_MATH_API Mat4 mat4Mul(Mat4 a, Mat4 b) {
  Mat4 res = { 0 };
  for (size_t i = 0; i < 4; i++) {
    for (size_t j = 0; j < 4; j++) {
      for (size_t k = 0; k < 4; k++) {
        res.m[i][j] += a.m[i][k] * b.m[k][j];
      }
    }
  }
  return res;
}

BASE_MATH_API Mat4 mat4RotateX(Mat4 a, float rads) {
  Mat4 rotationMatrix = {
    {{ 1.f, 0.f,        0.f,         0.f },
     { 0.f, cosf(rads), -sinf(rads), 0.f },
     { 0.f, sinf(rads), cosf(rads),  0.f },
     { 0.f, 0.f,        0.f,         1.f }}
  };

  return mat4Mul(a, rotationMatrix);
}

BASE_MATH_API Mat4 mat4RotateY(Mat4 a, float rads) {
  Mat4 rotationMatrix = {
    {{ cosf(rads),  0.f, sinf(rads), 0.f },
     { 0.f,         1.f, 0.f,        0.f },
     { -sinf(rads), 0.f, cosf(rads), 0.f },
     { 0.f,         0.f, 0.f,        1.f }}
  };

  return mat4Mul(a, rotationMatrix);
}

BASE_MATH_API Mat4 mat4RotateZ(Mat4 a, float rads) {
  Mat4 rotationMatrix = {
    {{ cosf(rads), -sinf(rads), 0.f, 0.f },
     { sinf(rads), cosf(rads),  0.f, 0.f },
     { 0.f,        0.f,         1.f, 0.f },
     { 0.f,        0.f,         0.f, 1.f }}
  };

  return mat4Mul(a, rotationMatrix);
}

BASE_MATH_API Mat4 mat4Scale(Mat4 a, float s) {
  for (size_t i = 0; i < 4; i++) {
    for (size_t j = 0; j < 4; j++) {
      a.m[i][j] *= s;
    }
  }
  return a;
}

BASE_MATH_API bool mat4Inverse(Mat4 a, Mat4 *out) {
  BASE_ASSERT(out);
  float *m = &a.m[0][0];

  float t0  =  m[10] * m[15] - m[11] * m[14];
  float t1  =  m[9]  * m[15] - m[11] * m[13];
  float t2  =  m[9]  * m[14] - m[10] * m[13];
  float t3  =  m[8]  * m[15] - m[11] * m[12];
  float t4  =  m[8]  * m[14] - m[10] * m[12];
  float t5  =  m[8]  * m[13] - m[9]  * m[12];

  float c00 =  m[5]  * t0 - m[6] * t1 + m[7] * t2;
  float c01 = -(m[4] * t0 - m[6] * t2 + m[7] * t3);
  float c02 =  m[4]  * t1 - m[5] * t3 + m[7] * t4;
  float c03 = -(m[4] * t2 - m[5] * t4 + m[6] * t5);

  float det = m[0] * c00 + m[1] * c01 + m[2] * c02 + m[3] * c03;
  if (det == 0.f) return false;

  float invDet = 1.f / det;

  float *o = &out->m[0][0];

  o[0]  = c00 * invDet;
  o[1]  = -(m[1] * t0 - m[2] * t1  + m[3] * t2)  * invDet;
  o[2]  =  (m[1] * t0 - m[2] * t1  + m[3] * t2)  * invDet;
  o[3]  = -(m[1] * t2 - m[2] * t4  + m[3] * t5)  * invDet;

  o[4]  = c01 * invDet;
  o[5]  =  (m[0] * t0 - m[2] * t3  + m[3] * t4)  * invDet;
  o[6]  = -(m[0] * t0 - m[1] * t3  + m[3] * t5)  * invDet;
  o[7]  =  (m[0] * t2 - m[1] * t4  + m[2] * t5)  * invDet;

  o[8]  = c02 * invDet;
  o[9]  = -(m[0] * t1 - m[1] * t3  + m[3] * t5)  * invDet;
  o[10] =  (m[0] * t0 - m[1] * t3  + m[2] * t5)  * invDet;
  o[11] = -(m[0] * t1 - m[1] * t2  + m[2] * t3)  * invDet;

  o[12] = c03 * invDet;
  o[13] =  (m[0] * t2 - m[1] * t4  + m[2] * t5)  * invDet;
  o[14] = -(m[0] * t1 - m[1] * t3  + m[2] * t4)  * invDet;
  o[15] =  (m[0] * t0 - m[1] * t1  + m[2] * t2)  * invDet;

  return true;
}

BASE_MATH_API Vec4 mat4MulVec4(Mat4 a, Vec4 v) {
  return (Vec4) {
    a.m[0][0] * v.x + a.m[0][1] * v.y + a.m[0][2] * v.z + a.m[0][3] * v.w,
    a.m[1][0] * v.x + a.m[1][1] * v.y + a.m[1][2] * v.z + a.m[1][3] * v.w,
    a.m[2][0] * v.x + a.m[2][1] * v.y + a.m[2][2] * v.z + a.m[2][3] * v.w,
    a.m[3][0] * v.x + a.m[3][1] * v.y + a.m[3][2] * v.z + a.m[3][3] * v.w
  };
}

#endif /* BASE_MATH_IMPLEMENTATION */
