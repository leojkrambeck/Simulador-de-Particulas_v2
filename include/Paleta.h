#pragma once
#include <SFML/Graphics/Color.hpp>

// Cores compartilhadas entre o editor (Menu) e a simulacao (Principal),
// pra tudo ter a mesma cara.
namespace paleta
{
    inline const sf::Color fundo        (15, 18, 28);
    inline const sf::Color grade        (28, 33, 48);
    inline const sf::Color painel       (22, 27, 40);
    inline const sf::Color painelLinha  (58, 66, 92);
    inline const sf::Color texto        (228, 232, 240);
    inline const sf::Color textoFraco   (140, 150, 170);
    inline const sf::Color proton       (235, 86, 86);
    inline const sf::Color eletron      (86, 150, 235);
    inline const sf::Color acento       (110, 170, 255);
    inline const sf::Color iniciar      (55, 170, 110);
    inline const sf::Color iniciarHover (80, 200, 132);
}
