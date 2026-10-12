#pragma once
#include <SFML/Graphics.hpp>
#include <algorithm>
#include "Particula.h"
#include "Paleta.h"
#include "Vista.h"

// Helpers de desenho usados tanto no editor quanto na simulacao.

inline void desenharGrade(sf::RenderWindow& janela, float largura, float altura,
                          float passo = 40.f)
{
    sf::RectangleShape vertical(sf::Vector2f(1.f, altura));
    vertical.setFillColor(paleta::grade);
    for (float x = 0.f; x <= largura; x += passo)
    {
        vertical.setPosition(x, 0.f);
        janela.draw(vertical);
    }

    sf::RectangleShape horizontal(sf::Vector2f(largura, 1.f));
    horizontal.setFillColor(paleta::grade);
    for (float y = 0.f; y <= altura; y += passo)
    {
        horizontal.setPosition(0.f, y);
        janela.draw(horizontal);
    }
}

// Circulo colorido + contorno leve + sinal "+" (proton) ou "-" (eletron) no centro.
// A posicao da particula esta em metros; a Vista converte pra pixels.
inline void desenharParticula(sf::RenderWindow& janela, const Particula& p, const Vista& vista)
{
    float raio = p.getRaioPx();
    bool proton = p.getSinalCarga() >= 0;
    sf::Vector2f pos = vista.paraTela(p.getPosX(), p.getPosY());

    sf::CircleShape corpo(raio);
    corpo.setOrigin(raio, raio);
    corpo.setPosition(pos);
    corpo.setFillColor(proton ? paleta::proton : paleta::eletron);
    corpo.setOutlineThickness(2.f);
    corpo.setOutlineColor(sf::Color(255, 255, 255, 45));
    janela.draw(corpo);

    sf::Color tinta(255, 255, 255, 210);
    float comprimento = raio * 0.8f;
    float espessura = std::max(2.f, raio * 0.16f);

    sf::RectangleShape traco(sf::Vector2f(comprimento, espessura));
    traco.setOrigin(comprimento / 2.f, espessura / 2.f);
    traco.setPosition(pos);
    traco.setFillColor(tinta);
    janela.draw(traco);

    if (proton) // barra vertical -> vira um "+"
    {
        sf::RectangleShape vertical(sf::Vector2f(espessura, comprimento));
        vertical.setOrigin(espessura / 2.f, comprimento / 2.f);
        vertical.setPosition(pos);
        vertical.setFillColor(tinta);
        janela.draw(vertical);
    }
}
