#pragma once

#include "Vec2.h"
#include <stdint.h>

// ------------------------------------------------

typedef struct Particle {
  Vec2 position;   // current position of the particle
  Vec2 next_pos;   // only used during update loop
  Vec2 velocity;   // current velocity of the particle
  float inv_mass;  // 1/mass of the particle
  float radius;    // radius of the particle
  int solid_id;    // all particles associated to the same solid should have the same id
  int draw_id;     // id used to identify drawing element associated to the sphere
  int halo_id;     // id used to identify halo asociated to the sphere
  //uint32_t status; // can be used as a bitfield to store additional information
  float temperature; //entre 0 et 1
  float r0; // rayon lors de l'initialisation
} Particle;

typedef struct Particle_carre { // elle est lié a ses voisine ce qui vas faire un carré
  Particle p1;
  Particle p2;
  Particle p3;
  Particle P4;
} Particule_carre;

// ------------------------------------------------

