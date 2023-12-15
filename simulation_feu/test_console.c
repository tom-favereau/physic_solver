#include <stdio.h>

#include "Vec2.h"

int main(int argc, const char * argv[]) {
    Vec2 v = {1.0, 0};
    Vec2 normale = {0.0, 1.0};
    Vec2 point = {0.0, -10.0};
    Vec2 w = changement_base(v, normale, point);
    //printf("%f, %f", w.x, w.y);
    return 0;
}
