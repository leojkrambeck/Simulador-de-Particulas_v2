#pragma once
#include <SFML/System/Vector2.hpp>

// Converte entre coordenadas do mundo (metros, double) e da tela (pixels, float).
// A origem (0,0) do mundo fica no centro da janela.
class Vista
{
    private:
        double metrosPorPixel;
        float larguraPx;
        float alturaPx;

    public:
        Vista(float larguraPx, float alturaPx, double metrosPorPixel)
            : metrosPorPixel(metrosPorPixel), larguraPx(larguraPx), alturaPx(alturaPx)
        {
        }

        double getMetrosPorPixel() const { return metrosPorPixel; }
        void setMetrosPorPixel(double m) { metrosPorPixel = m; }

        // fator > 1 aproxima a camera (menos metros por pixel)
        void aplicarZoom(double fator) { metrosPorPixel /= fator; }

        sf::Vector2f paraTela(double mx, double my) const
        {
            return sf::Vector2f(larguraPx * 0.5f + static_cast<float>(mx / metrosPorPixel),
                                alturaPx  * 0.5f + static_cast<float>(my / metrosPorPixel));
        }

        void paraMundo(float px, float py, double& mx, double& my) const
        {
            mx = (static_cast<double>(px) - larguraPx * 0.5) * metrosPorPixel;
            my = (static_cast<double>(py) - alturaPx  * 0.5) * metrosPorPixel;
        }

        double pixelsParaMetros(float px) const { return px * metrosPorPixel; }
        float  metrosParaPixels(double m) const { return static_cast<float>(m / metrosPorPixel); }
};
