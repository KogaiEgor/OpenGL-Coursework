#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>
#include "stb_image.h"
#include <iostream>

class Texture
{
public:
    Texture();
    Texture(const char* fileLoc);
    Texture(std::string faces[6]);
    ~Texture();

    bool LoadTextureA();
    bool LoadTexture();
    bool LoadCubemap();

    void UseTexture();
    void ClearTexture();

private:
    GLuint textureID;
    int width, height, bitDepth;

    const char* fileLocation;
    std::string facesCubemap[6];
};

#endif
