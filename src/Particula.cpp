#include "Particula.h"
#include <cmath>

Particula::Particula()
{
    carga = 0.0;
    massa = 1.0;
    raioPx = 10.f;

    posX = 0.0;
    posY = 0.0;
    velX = 0.0;
    velY = 0.0;
    forcaX = 0.0;
    forcaY = 0.0;
}

double Particula::getRapidez() const
{
    return std::sqrt(velX * velX + velY * velY);
}
