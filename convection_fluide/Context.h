#pragma once

#include <math.h>
#include "Vec2.h"
#include "Particle.h"
#include "Constraint.h"



// ------------------------------------------------

typedef struct Context {
  int num_particles;
  int capacity_particles;
  Particle* particles;

  // Ground colliders 
  int num_ground_sphere;
  SphereCollider* ground_spheres;

  // Plan colliders 
  int num_ground_plane;
  PlaneCollider* ground_plane;

  //grille 
  int** taille_grille; // nombre de particule dans la cas i, j 
  Particle*** grille;  // liste des particule dans la case i, j

  
} Context;

// ------------------------------------------------

Context* initializeContext(int capacity);

// ------------------------------------------------

void addParticle(Context* context, float x, float y, float radius, float mass, int draw_id, int halo_id);

void add_particle_carre(Context* context, float x, float y, float radius, float mass, int draw_id);

// ------------------------------------------------

Particle getParticle(Context* context, int id);

// ------------------------------------------------

void setDrawId(Context* context, int sphere_id, int draw_id);

// ------------------------------------------------

void updatePhysicalSystem(Context* context, float dt, int num_constraint_relaxation);

// ------------------------------------------------
// Methods below are called by updatePhysicalSystem
// ------------------------------------------------

void applyExternalForce(Context* context, float dt);
void dampVelocities(Context* context);
void updateExpectedPosition(Context* context, float dt);
void addDynamicContactConstraints(Context* context);
void addStaticContactConstraints(Context* context);
void projectConstraints(Context* context);
void updateVelocityAndPosition(Context* context, float dt);
void applyFriction(Context* context);
void deleteContactConstraints(Context* context);

// ------------------------------------------------

void checkContactWithPlane(Context* context, int particle_id);

void checkContactWithSphere(Context* context, int particle_id);

