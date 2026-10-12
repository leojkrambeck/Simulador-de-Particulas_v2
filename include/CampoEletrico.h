#pragma once
#include <cmath>
#include <vector>
#include <SFML/System/Vector2.hpp>
#include "Particula.h"

// Potencial e campo eletrico (SI) gerados por um conjunto de particulas paradas.
// 'soft' e o mesmo comprimento de suavizacao usado na forca.
namespace campoEletrico
{
    // V(x,y) = k * soma( q_i / sqrt(r_i^2 + s^2) )   [volts]
    inline double potencial(const std::vector<Particula*>& ps,
                            double x, double y, double k, double soft)
    {
        double s2 = soft * soft;
        double v = 0.0;
        for (const Particula* p : ps)
        {
            double dx = x - p->getPosX();
            double dy = y - p->getPosY();
            v += p->getCarga() / std::sqrt(dx * dx + dy * dy + s2);
        }
        return k * v;
    }

    // E(x,y) [V/m] -> aponta para longe de carga positiva
    inline sf::Vector2f campoVetor(const std::vector<Particula*>& ps,
                                   double x, double y, double k, double soft)
    {
        double s2 = soft * soft;
        double ex = 0.0, ey = 0.0;
        for (const Particula* p : ps)
        {
            double dx = x - p->getPosX();
            double dy = y - p->getPosY();
            double d = dx * dx + dy * dy + s2;
            double inv32 = 1.0 / (d * std::sqrt(d)); // (r^2 + s^2)^(-3/2)
            ex += p->getCarga() * dx * inv32;
            ey += p->getCarga() * dy * inv32;
        }
        return sf::Vector2f(static_cast<float>(k * ex), static_cast<float>(k * ey));
    }
}
