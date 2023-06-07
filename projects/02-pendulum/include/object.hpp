#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <vector>
#include <glm/glm.hpp>

struct Sphere {
    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec3 color;
    float radius;
    float m;
};
struct Cube {
    Cube(glm::vec3 size = glm::vec3(1.0f)): size(size){};
    glm::vec3 pos;
    glm::vec3 size;
    glm::vec3 color;
    float m;
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

#endif
