#ifndef CONSTRAINT_H_
#define CONSTRAINT_H_



// ------------------------------------------------

typedef struct SphereCollider {
  Vec2 center;
  float radius;
} SphereCollider;

typedef struct PlaneCollider {
  Vec2 point; // un point du plan 
  Vec2 normale; // sa normale 
} PlaneCollider;

// ------------------------------------------------




#endif