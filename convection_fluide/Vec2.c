#include "Vec2.h"

// ------------------------------------------------
Vec2 ext_mul(float lambda, Vec2 v){
    Vec2 res = {lambda*v.x, lambda*v.y};
    return res;
}

Vec2 add(Vec2 v, Vec2 w){
    Vec2 res = {v.x+w.x, v.y+w.y};
    return res;
}

Vec2 sous(Vec2 v, Vec2 w){
    Vec2 res = {v.x-w.x, v.y-w.y};
    return res;
}

float dot_product(Vec2 v, Vec2 w){
    return (v.x*w.x)+(v.y*w.y);
}

float norm(Vec2 v){
    return sqrtf(dot_product(v, v));
}

Vec2 changement_base(Vec2 v, Vec2 normale, Vec2 point){
    // renvoie le vecteur v dans la base directe ou normale est le deuxième vecteur (x)
    Vec2 premier_vec = {normale.y, -normale.x};
    //changement de base 
    Vec2 res = {premier_vec.x*v.x+normale.x*v.x, premier_vec.y*v.x+normale.y*v.y};
    res = sous(res, point);
    return res;
}

// ------------------------------------------------
