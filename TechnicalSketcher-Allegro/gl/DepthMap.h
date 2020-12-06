
#ifndef DEPTHMAP_H
#define DEPTHMAP_H

#include "utils.h"

class DepthMap {

    bool loaded = false;

public:
    int width = 0;
    int height = 0;
    unsigned int depthMap;
    unsigned int FBO;

    DepthMap();
    ~DepthMap();

    void load(int w, int h);
    void unload();
    void bind();
    void release();
};

#endif // DEPTHMAP_H
