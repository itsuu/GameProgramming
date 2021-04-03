/*
 * perlin noise source: http://www.crownandcutlass.com/features/technicaldetails/frustum.html 
 * All credits to the perlin noise source goes to the creator in the above link. He has made it open source
 *  by others in projects.
 */

#include "PointInFrustum.h"
#include "graphics.h"

bool PointInFrustum2(float x, float y, float z)
{
    int p;

    for (p = 0; p < 6; p++)
        if (frustum[p][0] * x + frustum[p][1] * y + frustum[p][2] * z + frustum[p][3] <= 0)
            return false;
    return true;
}