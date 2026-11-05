#ifndef DIRECTIONAL_LIGHT_HPP
#define DIRECTIONAL_LIGHT_HPP

struct DirectionalLight
{
    float direction[4]{0.0f, -1.0f, 0.0f, 0.0f};
    float color[4]{1.0f, 1.0f, 1.0f, 1.0f};
};

#endif