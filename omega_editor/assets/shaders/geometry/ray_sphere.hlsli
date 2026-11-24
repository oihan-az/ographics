#ifndef OMEGA_RAY_SPHERE_HLSLI
#define OMEGA_RAY_SPHERE_HLSLI

// Assumes a normalized direction and a sphere centered at the origin
bool intersect_sphere(float3 origin, float3 direction, float radius, out float t_near, out float t_far)
{
    float b = dot(origin, direction);
    float c = dot(origin, origin) - radius * radius;
    float discriminant = b * b - c;

    if (discriminant < 0.0f)
    {
        t_near = 0.0f;
        t_far = 0.0f;
        return false;
    }

    float root = sqrt(discriminant);

    t_near = -b - root;
    t_far = -b + root;

    return true;
}

#endif // OMEGA_RAY_SPHERE_HLSLI
