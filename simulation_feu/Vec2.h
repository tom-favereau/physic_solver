#ifndef VEC2_H_
#define VEC2_H_

#include <math.h>
#include <stdio.h>
// ------------------------------------------------

typedef struct Vec2 {
  float x;
  float y;
} Vec2;

// ------------------------------------------------

Vec2 ext_mul(float lambda, Vec2 v);

Vec2 add(Vec2 v, Vec2 w);

Vec2 sous(Vec2 v, Vec2 w);

float dot_product(Vec2 v, Vec2 w);

float norm(Vec2 v);

Vec2 changement_base(Vec2 v, Vec2 normale, Vec2 point);


// ------------------------------------------------

#endif