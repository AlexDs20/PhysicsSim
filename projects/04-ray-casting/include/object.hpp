#pragma once

#include <glm/ext/quaternion_geometric.hpp>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include "ray.hpp"


class Sphere {
    public:
        glm::vec3 pos;
        glm::vec3 prev_pos;
        glm::vec3 vel;
        glm::vec3 color;
        float radius;
        float m;

        bool intersects(const Ray& ray, float& t){
            // I just solve that the length of the vector which direction is from center of sphere to the ray is exactly the radius of the sphere.
            glm::vec3 OS = ray.O - this->pos;

            float delta = glm::pow(2 * glm::dot(ray.dir, OS), 2) - 4 * (glm::length2(OS) - radius*radius);

            if (delta < 0)
                return false;

            delta = glm::sqrt(delta);

            // TODO: Understand why t is negative here...
            t = glm::min(-glm::dot(ray.O, OS) + delta/2, -glm::dot(ray.O, OS) - delta/2);
            return true;
        };
};
class Cube {
    public:
        Cube(glm::vec3 size = glm::vec3(1.0f)): size(size){};
        glm::vec3 pos;
        glm::vec3 size;
        glm::vec3 color;
        float m;

        // TODO: Implement
        bool intersects(const Ray& r, float& t);
};


class MeshCube {
    public:
        MeshCube();
        ~MeshCube();
        void Draw() const;

    private:
        unsigned int VBO, EBO, VAO;
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        void construct();
        void setup();
};

class MeshSphere {
    public:
        MeshSphere(unsigned int sectorCount=32, unsigned int stackCount=16);
        ~MeshSphere();
        void Draw() const;

    private:
        std::vector<float> vertices;
        std::vector<float> normals;
        std::vector<float> texCoords;
        std::vector<int> indices;
        unsigned int VBO, VAO, EBO;

        unsigned int sectorCount;
        unsigned int stackCount;

        void constructVertices();
        void constructIndices();
        void setup();
};
