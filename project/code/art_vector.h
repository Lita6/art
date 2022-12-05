/* date = November 29th 2022 6:36 pm */

#ifndef ART_VECTOR_H
#define ART_VECTOR_H

#include <intrin.h>

struct v2
{
	r32 x;
	r32 y;
};

struct v3
{
	r32 x;
	r32 y;
	r32 z;
};

v2
V2
(r32 x, r32 y)
{
	v2 result = {};
	result.x = x;
	result.y = y;
	return(result);
}

v3
V3
(r32 x, r32 y, r32 z)
{
	v3 result = {};
	result.x = x;
	result.y = y;
	result.z = z;
	return(result);
}

v2
operator-
(v2 a, v2 b)
{
	v2 result = {};
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	return(result);
}

v3
operator-
(v3 a, v3 b)
{
	v3 result = {};
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	return(result);
}

v2
operator+
(v2 a, v2 b)
{
	v2 result = {};
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	return(result);
}

v3
operator+
(v3 a, v3 b)
{
	v3 result = {};
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	return(result);
}

v2
operator*
(r32 a, v2 b)
{
	v2 result = {};
	result.x = b.x * a;
	result.y = b.y * a;
	return(result);
}

v3
operator*
(r32 a, v3 b)
{
	v3 result = {};
	result.x = b.x * a;
	result.y = b.y * a;
	result.z = b.z * a;
	return(result);
}

v2
operator*
(v2 b, r32 a)
{
	v2 result = {};
	result.x = b.x * a;
	result.y = b.y * a;
	return(result);
}

v3
operator*
(v3 b, r32 a)
{
	v3 result = {};
	result.x = b.x * a;
	result.y = b.y * a;
	result.z = b.z * a;
	return(result);
}

v2
operator*
(v2 a, v2 b)
{
	v2 result = {};
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	return(result);
}

v3
operator*
(v3 a, v3 b)
{
	v3 result = {};
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	result.z = a.z * b.z;
	return(result);
}

v2
operator/
(r32 a, v2 b)
{
	v2 result = {};
	result.x = b.x / a;
	result.y = b.y / a;
	return(result);
}

v3
operator/
(r32 a, v3 b)
{
	v3 result = {};
	result.x = b.x / a;
	result.y = b.y / a;
	result.z = b.z / a;
	return(result);
}

v2
operator/
(v2 b, r32 a)
{
	v2 result = {};
	result.x = b.x / a;
	result.y = b.y / a;
	return(result);
}

v3
operator/
(v3 b, r32 a)
{
	v3 result = {};
	result.x = b.x / a;
	result.y = b.y / a;
	result.z = b.z / a;
	return(result);
}

r32
DotProduct
(v2 a, v2 b)
{
	v2 temp = a * b;
	r32 result = temp.x + temp.y;
	return(result);
}

r32
Dot
(v3 a, v3 b)
{
	v3 temp = a * b;
	r32 result = temp.x + temp.y + temp.z;
	return(result);
}

v3
CrossProduct
(v3 v, v3 w)
{
	v3 result = {};
	result.x = (v.y * w.z) - (v.z * w.y);
	result.y = (v.x * w.z) - (v.z * w.x);
	result.z = (v.x * w.y) - (v.y * w.x);
	return(result);
}

r32
sqrtr32
(r32 a)
{
	r32 result = 0;
	
	__m128 float_reg = _mm_load_ss(&a);
	float_reg = _mm_sqrt_ss(float_reg);
	result = _mm_cvtss_f32(float_reg);
	
	return(result);
}

r32
absr32
(r32 a)
{
	r32 result = (a < 0) ? (r32)(a*-1.0) : a;
	return(result);
}

r32
v2Length
(v2 a)
{
	r32 result = DotProduct(a, a);
	result = sqrtr32(result);
	result = absr32(result);
	return(result);
}

r32
v3Length
(v3 a)
{
	r32 result = Dot(a, a);
	result = sqrtr32(result);
	result = absr32(result);
	return(result);
}

#endif //ART_VECTOR_H
