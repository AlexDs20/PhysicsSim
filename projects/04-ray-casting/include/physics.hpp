#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include <glm/glm.hpp>
#include "object.hpp"

float energy(glm::vec3 pos, glm::vec3 v);
void collision(Sphere& s1, Sphere& s2);
void move(Sphere& s, float dt);

#endif
