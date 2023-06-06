#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <vector>

struct Sphere {
    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec3 color;
    float radius;
    float m;
};


class Cube {
    public:
        Cube();
        ~Cube();
        void Draw() const;

    private:
        unsigned int VBO, EBO, VAO;
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        void construct();
        void setup();
};

class Sphere2 {
    public:
        Sphere2(unsigned int sectorCount=32, unsigned int stackCount=16);
        ~Sphere2();
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
