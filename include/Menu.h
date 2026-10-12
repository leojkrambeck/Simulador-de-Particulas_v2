#pragma once
#include "Particula.h"
#include "Vista.h"
#include <vector>
#include <memory>

namespace sf { class RenderWindow; class Font; }

// Fase de edicao grafica: o usuario clica dentro da janela pra posicionar as
// particulas antes da simulacao comecar. Os cliques (pixels) viram posicoes em
// metros pela Vista.
class Menu
{
    private:
        float largura;
        float altura;
        const Vista& vista;

    public:
        Menu(float largura, float altura, const Vista& vista);

        // roda o editor na janela ja aberta e devolve as particulas posicionadas.
        // 'iniciar' vira true quando o usuario aperta ENTER; false se fechou a janela.
        // 'quicarBordas' sai com a escolha do toggle "bordas solidas".
        std::vector<std::unique_ptr<Particula>> montar(sf::RenderWindow& janela,
                                                       const sf::Font* fonte,
                                                       bool& iniciar,
                                                       bool& quicarBordas);
};
