#pragma once

#include <iostream>
#include <glm/glm.hpp>

class Ray {
    public:
        glm::vec3 O;
        glm::vec3 dir;

        Ray(glm::vec3 O, glm::vec3 dir): O(O), dir(dir){};

        void operator() (glm::vec3 O, glm::vec3 dir) {
            this->O = O;
            this->dir = dir;
        };

        void print() const {print(0);};

        void print(float t) const {
            glm::vec3 V = O + t * dir;
            std::cout << V.x << ", " << V.y << ", " << V.z << std::endl;
        };

    private:

};
