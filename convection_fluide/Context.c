#include "Context.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

// ------------------------------------------------

Particle getParticle(Context* context, int id)
{
  return context->particles[id];
}

// ------------------------------------------------

void addParticle(Context* context, float x, float y, float radius, float mass, int draw_id, int halo_id)
{
    assert(context->num_particles<context->capacity_particles); // currently no resize in context
    context->particles[context->num_particles].position.x = x;
    context->particles[context->num_particles].position.y = y;
    context->particles[context->num_particles].next_pos.x = x;
    context->particles[context->num_particles].next_pos.y = y;
    context->particles[context->num_particles].velocity.x = 0.0F;
    context->particles[context->num_particles].velocity.y = 0.0F;
    context->particles[context->num_particles].inv_mass = 1.0F/mass;
    context->particles[context->num_particles].radius = radius;
    context->particles[context->num_particles].draw_id = draw_id;
    context->particles[context->num_particles].halo_id = halo_id;
    context->particles[context->num_particles].temperature = 0.0F;
    context->particles[context->num_particles].r0 = radius;

    //on actualise la grille 
    

    context->num_particles += 1;
}

void add_particle_carre(Context* context, float x, float y, float radius, float mass, int draw_id){
  //on déinit par les coordoné de la première particule 

}

// ------------------------------------------------

void setDrawId(Context* context, int sphere_id, int draw_id)
{
  context->particles[sphere_id].draw_id = draw_id;
}

// ------------------------------------------------

SphereCollider getGroundSphereCollider(Context* context, int id)
{
  return context->ground_spheres[id];
}

// ------------------------------------------------

Context* initializeContext(int capacity)
{
  Context* context = malloc(sizeof(Context));
  context->num_particles = 0;
  context->capacity_particles = capacity;
  context->particles = malloc(capacity*sizeof(Particle));
  memset(context->particles,0,capacity*sizeof(Particle));

  context->num_ground_sphere = 0; //4
  /*ground sphere
  context->ground_spheres = malloc(4*sizeof(SphereCollider));
  
  Vec2 p0 = {-3.0f, -3.0f};
  context->ground_spheres[0].center = p0;
  context->ground_spheres[0].radius = 1.5f;

  p0.x = 3.0f;
  p0.y = -3.0f;
  context->ground_spheres[1].center = p0;
  context->ground_spheres[1].radius = 1.5f;

  p0.x = -8.0f;
  p0.y = -9.0f;
  context->ground_spheres[2].center = p0;
  context->ground_spheres[2].radius = 1.5f;

  p0.x = 8.0f;
  p0.y = -9.0f;
  context->ground_spheres[3].center = p0;
  context->ground_spheres[3].radius = 1.5f;
  */
  //plane collider 
  context->num_ground_plane = 4;
  context->ground_plane = malloc(4*sizeof(PlaneCollider));

  Vec2 n0 = {0, 1};
  Vec2 point = {0, -10};
  context->ground_plane[0].normale = n0;
  context->ground_plane[0].point = point;

  //grille 
  context->taille_grille = calloc(12, sizeof(int*));
  context->grille = calloc(12, sizeof(Particle*));
  for (int i = 0; i < 12; i++){
    int* tmp = calloc(10, sizeof(int));
    context->taille_grille[i] = tmp;
    tmp = calloc(10, sizeof(Particle*));
    context->grille[i] = tmp;
    for (int j = 0; j<10; j++){
      Particle* part = calloc(capacity, sizeof(Particle));
      context->grille[i][j] = part;
    }
  }

  

  return context;
}

// ------------------------------------------------

void updatePhysicalSystem(Context* context, float dt, int num_constraint_relaxation)
{
  applyExternalForce(context, dt);
  dampVelocities(context);
  updateExpectedPosition(context, dt);
  addDynamicContactConstraints(context);
  addStaticContactConstraints(context);
 
  for(int k=0; k<num_constraint_relaxation; ++k) {
    projectConstraints(context);
  }

  updateVelocityAndPosition(context, dt);
  applyFriction(context);

  deleteContactConstraints(context);
}

// ------------------------------------------------

void applyExternalForce(Context* context, float dt)
{
  for (int i = 0; i<context->num_particles; i++){
    context->particles[i].velocity.y -= (dt*13.0F)/context->particles[i].inv_mass; // gravité 
    //context->particles[i].velocity.y += (dt*(exp(context->particles[i].temperature)-1)*20.0F)/context->particles[i].inv_mass; //force de convexion exponentielle
    //context->particles[i].velocity.y += (dt*(pow(context->particles[i].temperature, 2))*27.0F)/context->particles[i].inv_mass; quadratique
    context->particles[i].velocity.y += (dt*context->particles[i].temperature*20.0F)/context->particles[i].inv_mass;
    context->particles[i].temperature = 0.999*(context->particles[i].temperature);

    //on augmente le rayon en fonction de la temperature;
    context->particles[i].radius = (1+(context->particles[i].temperature)/2)*context->particles[i].r0;
    //context->particles[i].radius = (exp(context->particles[i].temperature-1))*context->particles[i].r0;//modele exponentielle (pas stable)
  }
}

void dampVelocities(Context* context)
{
}

void updateExpectedPosition(Context* context, float dt)
{
  
  for (int i = 0; i<context->num_particles; i++){
    context->particles[i].next_pos = add(context->particles[i].position, ext_mul(dt, context->particles[i].velocity));
  }
  
}

void addDynamicContactConstraints(Context* context)
{
  /*
  for (int i = 0; i<context->num_particles; i++){
    for (int j = 0; j<context->num_particles; j++){
      if (i != j){
        Vec2 x_ji = sous(context->particles[j].next_pos, context->particles[i].next_pos);
        float C = norm(x_ji) - (context->particles[i].radius+context->particles[j].radius);
        if (C < 0){
          float sigma_i = (context->particles[j].inv_mass) / (context->particles[j].inv_mass + context->particles[i].inv_mass);
          Vec2 delta_i = ext_mul(-sigma_i, ext_mul(1/norm(x_ji), x_ji));
          float sigma_j = (context->particles[i].inv_mass) / (context->particles[j].inv_mass + context->particles[i].inv_mass);
          Vec2 delta_j = ext_mul(sigma_j, ext_mul(1/norm(x_ji), x_ji));
          context->particles[j].next_pos = add(context->particles[j].next_pos, delta_i);
          context->particles[i].next_pos = add(context->particles[i].next_pos, delta_j);
        }
      }
    }
  }
  */
  
  for (int i = 0; i<context->num_particles; i++){
    for (int j = 0; j<context->num_particles; j++){
          if (i != j){
            float distance = norm(sous(context->particles[j].position, context->particles[i].next_pos));
            float R = context->particles[j].radius;
            float r = context->particles[i].radius;
            if (distance < r+R){
                
                Vec2 v_unit = ext_mul(1/distance, sous(context->particles[i].next_pos, context->particles[j].position));
                Vec2 v_corection = sous(ext_mul(R+r, v_unit), sous(context->particles[i].next_pos, context->particles[j].position));
                
                float dx = v_corection.x;
                float dy = v_corection.y;

                //printf("test : %f, %f, %f, %f\n", context->ground_spheres[i].center.x, context->ground_spheres[i].center.y, context->particles[particle_id].position.x, context->particles[particle_id].position.y);
                
                context->particles[i].next_pos.x += 0.485*dx; //ici le coeficient représente la viscosité su fluide, plus il inférieur a 0.5 plus les colision se propage, devient vite instable
                context->particles[i].next_pos.y += 0.485*dy; //la limite d'instabilité est est de est 0.485

                context->particles[j].next_pos.x -= 0.515*dx;
                context->particles[j].next_pos.y -= 0.515*dy;

                //actualise les température 
                float temperature_i = context->particles[i].temperature;
                float temperature_j = context->particles[j].temperature;

                context->particles[i].temperature = 0.8*temperature_i+0.2*temperature_j;
                context->particles[j].temperature = 0.8*temperature_j+0.2*temperature_i;
            }
          }
    }
  }
  
}

void addStaticContactConstraints(Context* context){
  for (int i = 0; i<context->num_particles; i++){
    checkContactWithPlane(context, i);
    checkContactWithSphere(context, i);
  }
}

void projectConstraints(Context* context)
{
}

void updateVelocityAndPosition(Context* context, float dt)
{
  
  for (int i = 0; i<context->num_particles; i++){
    context->particles[i].velocity = ext_mul(1/dt, sous(context->particles[i].next_pos, context->particles[i].position));
    context->particles[i].position = context->particles[i].next_pos; 
  }
  
}

void applyFriction(Context* context)
{
}

void deleteContactConstraints(Context* context)
{
}

// ------------------------------------------------


void checkContactWithPlane(Context* context, int particle_id){
    // on calcule centre+r*(-normale) puis la distance de ce point au centre et on corrige 

    /*
    for (int i = 0; i<context->num_ground_plane; i++){
        Vec2 point = context->ground_plane[i].point;
        Vec2 normale = context->ground_plane[i].normale;
        //Vec2 p_depassement = sous(context->particles[particle_id].position, ext_mul(context->particles[particle_id].radius, context->ground_plane[i].normale));
        Vec2 p_depassement = changement_base(context->particles[particle_id].next_pos, normale, point);
        if (p_depassement.y-context->particles[particle_id].radius<0){
          printf("test : %f, %f\n", p_depassement.x, p_depassement.y);
          p_depassement.y = context->particles[particle_id].radius;
          //plan de base :
          Vec2 point_base = {0.0, -10.0};
          Vec2 normale_base = {0.0, 1.0};
          context->particles[particle_id].next_pos = changement_base(p_depassement, normale_base, point_base);
        }
      }
      */
     
     if (context->particles[particle_id].next_pos.y < -(9.0-context->particles[particle_id].radius)){ // sol
      
      
      
      context->particles[particle_id].next_pos.y = -(9.0-context->particles[particle_id].radius)+0.001;
      //context->particles[particle_id].temperature = exp(-pow(context->particles[particle_id].position.x, 2)/60); // on augmente la température loi normale
      context->particles[particle_id].temperature = 0.85*context->particles[particle_id].temperature+0.15; //source de chaleur infini
      //context->particles[particle_id].temperature = 1; // chaleur instantané

      //printf("%f\n", context->particles[particle_id].temperature);  
     }
     if (context->particles[particle_id].next_pos.y > (9.0-context->particles[particle_id].radius)){ //haut
        context->particles[particle_id].next_pos.y = (9.0-context->particles[particle_id].radius)-0.001;
        context->particles[particle_id].temperature = 0;
     }
     
     if (context->particles[particle_id].next_pos.x > (14.0-context->particles[particle_id].radius)){ // droite
        context->particles[particle_id].next_pos.x = (14.0-context->particles[particle_id].radius)-0.001;
        
     }

     if (context->particles[particle_id].next_pos.x < -(14.0-context->particles[particle_id].radius)){ //gauche
        context->particles[particle_id].next_pos.x = -(14.0-context->particles[particle_id].radius)+0.001;
        
     }

    
}

void checkContactWithSphere(Context* context, int particle_id){
    for (int i = 0; i<context->num_ground_sphere; i++){
        float distance = norm(sous(context->ground_spheres[i].center, context->particles[particle_id].next_pos));
        float R = context->ground_spheres[i].radius;
        float r = context->particles[particle_id].radius;
        if (distance < r+R){
            Vec2 v_unit = ext_mul(1/distance, sous(context->particles[particle_id].next_pos, context->ground_spheres[i].center));
            Vec2 v_corection = sous(ext_mul(R+r, v_unit), sous(context->particles[particle_id].next_pos, context->ground_spheres[i].center));
            
            float dx = v_corection.x;
            float dy = v_corection.y;

            //printf("test : %f, %f, %f, %f\n", context->ground_spheres[i].center.x, context->ground_spheres[i].center.y, context->particles[particle_id].position.x, context->particles[particle_id].position.y);
            
            context->particles[particle_id].next_pos.x += dx;
            context->particles[particle_id].next_pos.y += dy;
        }
    }
}