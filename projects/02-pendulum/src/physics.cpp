#include <glm/glm.hpp>

#include "object.hpp"
#include <iostream>


float energy(glm::vec3 pos, glm::vec3 v){
    glm::vec3 g(0.0f, -10.f, 0.0f);
    return -g.y*pos.y + 0.5 * dot(v, v);
};


void collision(Sphere& s1, Sphere& s2){
    // elasticity
    float e = 0.95f;

    glm::vec3 normal(s2.pos - s1.pos);
    float d = glm::length(normal);

    if (d > s1.radius + s2.radius)
        return;

    normal /= d;

    float corr = (s1.radius + s2.radius - d) / 2.0f;

    s1.pos -= corr * normal;
    s2.pos += corr * normal;

    float v1n = glm::dot(s1.vel, normal);
    float v2n = glm::dot(s2.vel, normal);

    float v1n_new = ((s1.m * v1n) + (s2.m * v2n) - s2.m * (v1n - v2n) * e)/(s1.m + s2.m);
    float v2n_new = ((s1.m * v1n) + (s2.m * v2n) - s1.m * (v2n - v1n) * e)/(s1.m + s2.m);

    glm::vec3 v1t = s1.vel - glm::dot(s1.vel, normal) * normal;
    glm::vec3 v2t = s2.vel - glm::dot(s2.vel, normal) * normal;

    s1.vel = v1t + v1n_new * normal;
    s2.vel = v2t + v2n_new * normal;
};



void move(Sphere& s, float dt, glm::vec3 center, float r){
    /*
    F = ma
    F = m dv/dt
        dv = (F/m) * dt
        vi - vi-1 = (F/m) * dt
        vi = vi-1 + (F/m) * dt
    v = dx/dt
        dx = v * dt
        xi = xi-1 + v * dt

    In practice:
    v = v + (F/m) * dt
    p <- x                  save initial position
    x = x + v * dt          move the particle
    x = constraint(x)       apply constraints
    v = (x - p) / dt
    */
    glm::vec3 g(0.0f, -10.f, 0.0f);

    s.vel += g * dt;
    glm::vec3 p = s.pos;
    s.pos += s.vel * dt;

    // constraint
    glm::vec3 dir = glm::normalize(s.pos - center);
    s.pos = center + r * dir;

    s.vel = (s.pos - p) / dt;
}
