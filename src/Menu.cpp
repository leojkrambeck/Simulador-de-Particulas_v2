#include "Menu.h"
#include "Proton.h"
#include "Eletron.h"
#include "Paleta.h"
#include "Render.h"
#include "Formato.h"
#include "Constantes.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <string>

Menu::Menu(float largura, float altura, const Vista& vista)
    : largura(largura), altura(altura), vista(vista)
{
}

namespace
{
    // cria um sf::Text ja configurado; se 'centro', ancora pelo meio
    sf::Text fazTexto(const sf::Font& fonte, const std::string& s, unsigned tam,
                      sf::Color cor, float x, float y, bool centro = false)
    {
        sf::Text t(s, fonte, tam);
        t.setFillColor(cor);
        if (centro)
        {
            sf::FloatRect b = t.getLocalBounds();
            t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
        }
        t.setPosition(x, y);
        return t;
    }

    void retangulo(sf::RenderWindow& janela, float x, float y, float w, float h,
                   sf::Color preenchimento,
                   sf::Color contorno = sf::Color::Transparent, float espessura = 0.f)
    {
        sf::RectangleShape r(sf::Vector2f(w, h));
        r.setPosition(x, y);
        r.setFillColor(preenchimento);
        if (espessura > 0.f)
        {
            r.setOutlineColor(contorno);
            r.setOutlineThickness(espessura);
        }
        janela.draw(r);
    }

    bool contem(float x, float y, float w, float h, float px, float py)
    {
        return px >= x && px <= x + w && py >= y && py <= y + h;
    }
}

std::vector<std::unique_ptr<Particula>> Menu::montar(sf::RenderWindow& janela,
                                                    const sf::Font* fonte,
                                                    bool& iniciar,
                                                    bool& quicarBordas)
{
    std::vector<std::unique_ptr<Particula>> particulas;
    iniciar = false;
    quicarBordas = false;

    bool ferramentaProton = true; // ferramenta ativa (clique esquerdo)
    float mouseX = largura / 2.f;
    float mouseY = altura / 2.f;

    // arrastar depois de soltar uma particula define a velocidade inicial dela.
    // deslocamento (em metros) dividido por esse tempo de referencia -> m/s.
    Particula* arrastando = nullptr;
    float arr0x = 0.f, arr0y = 0.f;
    const double tempoRef = 2.0e-17;

    // raios de referencia sem precisar instanciar toda hora
    static Proton amostraProton;
    static Eletron amostraEletron;

    const float barraTopo = 64.f;
    const float barraBase = 44.f;

    const float chipY = 14.f, chipH = 36.f, chipW = 124.f;
    const float chipX1 = 280.f, chipX2 = chipX1 + chipW + 10.f;

    const float togX = chipX2 + chipW + 8.f, togW = 96.f; // toggle "bordas solidas"

    const float btnW = 132.f, btnH = 34.f;
    const float btnX = largura - btnW - 16.f, btnY = 15.f;

    auto dentroCanvas = [&](float y) { return y > barraTopo && y < altura - barraBase; };

    auto adiciona = [&](bool proton, float px, float py) -> Particula*
    {
        std::unique_ptr<Particula> p;
        if (proton) p = std::make_unique<Proton>();
        else        p = std::make_unique<Eletron>();

        // trava dentro da janela (px), depois converte o ponto pra metros
        float r = p->getRaioPx();
        double mx, my;
        vista.paraMundo(std::clamp(px, r, largura - r), std::clamp(py, r, altura - r), mx, my);
        p->setPosicao(mx, my);
        particulas.push_back(std::move(p));
        return particulas.back().get();
    };

    while (janela.isOpen())
    {
        sf::Event ev;
        while (janela.pollEvent(ev))
        {
            if (ev.type == sf::Event::Closed)
            {
                janela.close();
                return particulas;
            }
            else if (ev.type == sf::Event::MouseMoved)
            {
                mouseX = static_cast<float>(ev.mouseMove.x);
                mouseY = static_cast<float>(ev.mouseMove.y);
            }
            else if (ev.type == sf::Event::MouseWheelScrolled)
            {
                ferramentaProton = !ferramentaProton;
            }
            else if (ev.type == sf::Event::MouseButtonPressed)
            {
                float mx = static_cast<float>(ev.mouseButton.x);
                float my = static_cast<float>(ev.mouseButton.y);

                if (contem(btnX, btnY, btnW, btnH, mx, my))
                {
                    iniciar = true;
                    return particulas;
                }
                if (contem(chipX1, chipY, chipW, chipH, mx, my)) { ferramentaProton = true;  continue; }
                if (contem(chipX2, chipY, chipW, chipH, mx, my)) { ferramentaProton = false; continue; }
                if (contem(togX, chipY, togW, chipH, mx, my))    { quicarBordas = !quicarBordas; continue; }

                if (dentroCanvas(my))
                {
                    if (ev.mouseButton.button == sf::Mouse::Left)
                    {
                        arrastando = adiciona(ferramentaProton, mx, my);
                        sf::Vector2f t = vista.paraTela(arrastando->getPosX(), arrastando->getPosY());
                        arr0x = t.x;
                        arr0y = t.y;
                    }
                    else if (ev.mouseButton.button == sf::Mouse::Right)
                        adiciona(!ferramentaProton, mx, my); // atalho: tipo oposto, sem velocidade
                }
            }
            else if (ev.type == sf::Event::MouseButtonReleased)
            {
                if (arrastando && ev.mouseButton.button == sf::Mouse::Left)
                {
                    double dxm = vista.pixelsParaMetros(mouseX - arr0x);
                    double dym = vista.pixelsParaMetros(mouseY - arr0y);
                    arrastando->setVelocidade(dxm / tempoRef, dym / tempoRef);
                    arrastando = nullptr;
                }
            }
            else if (ev.type == sf::Event::KeyPressed)
            {
                switch (ev.key.code)
                {
                    case sf::Keyboard::P:
                    case sf::Keyboard::Num1: ferramentaProton = true;  break;
                    case sf::Keyboard::E:
                    case sf::Keyboard::Num2: ferramentaProton = false; break;
                    case sf::Keyboard::BackSpace:
                        if (!particulas.empty()) particulas.pop_back();
                        arrastando = nullptr;
                        break;
                    case sf::Keyboard::R: particulas.clear(); arrastando = nullptr; break;
                    case sf::Keyboard::Enter:
                    case sf::Keyboard::Space:
                        iniciar = true;
                        return particulas;
                    default: break;
                }
            }
        }

        // ---------------------------- desenho ----------------------------
        janela.clear(paleta::fundo);
        desenharGrade(janela, largura, altura);

        // moldura da area jogavel (destaca quando as bordas viram parede)
        retangulo(janela, 4.f, barraTopo + 4.f,
                  largura - 8.f, altura - barraTopo - barraBase - 8.f,
                  sf::Color::Transparent,
                  quicarBordas ? paleta::acento : sf::Color(60, 68, 92, 90),
                  quicarBordas ? 2.f : 1.f);

        if (particulas.empty() && fonte)
        {
            janela.draw(fazTexto(*fonte, "clique na area para adicionar particulas", 15,
                                 sf::Color(110, 120, 140),
                                 largura / 2.f, (barraTopo + altura - barraBase) / 2.f, true));
        }

        for (auto& p : particulas)
            desenharParticula(janela, *p, vista);

        // previa "fantasma" no cursor (some enquanto arrasta uma velocidade)
        if (!arrastando && dentroCanvas(mouseY))
        {
            float r = ferramentaProton ? amostraProton.getRaioPx() : amostraEletron.getRaioPx();
            sf::Color base = ferramentaProton ? paleta::proton : paleta::eletron;

            sf::CircleShape fantasma(r);
            fantasma.setOrigin(r, r);
            fantasma.setPosition(std::clamp(mouseX, r, largura - r),
                                 std::clamp(mouseY, r, altura - r));
            fantasma.setFillColor(sf::Color(base.r, base.g, base.b, 70));
            fantasma.setOutlineThickness(1.5f);
            fantasma.setOutlineColor(sf::Color(base.r, base.g, base.b, 170));
            janela.draw(fantasma);
        }

        // seta de velocidade sendo arrastada
        if (arrastando)
        {
            sf::Vertex ln[] = {
                sf::Vertex(sf::Vector2f(arr0x, arr0y), paleta::acento),
                sf::Vertex(sf::Vector2f(mouseX, mouseY), paleta::acento)
            };
            janela.draw(ln, 2, sf::Lines);

            if (fonte)
            {
                double dxm = vista.pixelsParaMetros(mouseX - arr0x);
                double dym = vista.pixelsParaMetros(mouseY - arr0y);
                double vel = std::sqrt(dxm * dxm + dym * dym) / tempoRef;
                janela.draw(fazTexto(*fonte, fmtSci(vel / constantes::luz * 100.0, 2) + "% c",
                                     12, paleta::texto, mouseX + 10.f, mouseY + 6.f));
            }
        }

        // ---------- barra superior ----------
        retangulo(janela, 0.f, 0.f, largura, barraTopo, paleta::painel);
        retangulo(janela, 0.f, barraTopo - 2.f, largura, 2.f, paleta::acento);

        if (fonte)
        {
            double larguraNm = vista.pixelsParaMetros(largura) * 1e9;
            janela.draw(fazTexto(*fonte, "SIMULADOR DE PARTICULAS", 16, paleta::texto, 20.f, 11.f));
            janela.draw(fazTexto(*fonte,
                                 "modo edicao   -   area " + fmtSci(larguraNm, 2) + " nm de largura",
                                 12, paleta::textoFraco, 20.f, 35.f));
        }

        // contagem por tipo (mostrada dentro de cada chip)
        int nP = 0, nE = 0;
        for (auto& p : particulas) (p->getSinalCarga() < 0 ? nE : nP)++;

        // seletor de ferramenta (2 chips), com o total de cada tipo a direita
        auto chip = [&](float x, const std::string& rotulo, sf::Color cor, bool ativo, int n)
        {
            retangulo(janela, x, chipY, chipW, chipH,
                      ativo ? sf::Color(40, 48, 68) : sf::Color(26, 31, 46),
                      ativo ? paleta::acento : sf::Color(50, 58, 82),
                      ativo ? 2.f : 1.f);

            sf::CircleShape ponto(6.f);
            ponto.setOrigin(6.f, 6.f);
            ponto.setPosition(x + 16.f, chipY + chipH / 2.f);
            ponto.setFillColor(cor);
            janela.draw(ponto);

            if (fonte)
            {
                janela.draw(fazTexto(*fonte, rotulo, 13,
                                     ativo ? paleta::texto : paleta::textoFraco,
                                     x + 28.f, chipY + 9.f));

                sf::Text cont = fazTexto(*fonte, std::to_string(n), 14,
                                         ativo ? paleta::texto : paleta::textoFraco, 0.f, chipY + 8.f);
                cont.setPosition(x + chipW - 12.f - cont.getLocalBounds().width, chipY + 8.f);
                janela.draw(cont);
            }
        };
        chip(chipX1, "proton",  paleta::proton,  ferramentaProton,  nP);
        chip(chipX2, "eletron", paleta::eletron, !ferramentaProton, nE);

        // toggle "bordas solidas"
        {
            retangulo(janela, togX, chipY, togW, chipH,
                      quicarBordas ? sf::Color(40, 48, 68) : sf::Color(26, 31, 46),
                      quicarBordas ? paleta::acento : sf::Color(50, 58, 82),
                      quicarBordas ? 2.f : 1.f);

            float cq = 14.f;
            retangulo(janela, togX + 12.f, chipY + (chipH - cq) / 2.f, cq, cq,
                      quicarBordas ? paleta::acento : sf::Color::Transparent,
                      sf::Color(90, 100, 130), 1.f);

            if (fonte)
                janela.draw(fazTexto(*fonte, "bordas", 13,
                                     quicarBordas ? paleta::texto : paleta::textoFraco,
                                     togX + 34.f, chipY + 9.f));
        }

        // botao iniciar
        bool hoverBtn = contem(btnX, btnY, btnW, btnH, mouseX, mouseY);
        retangulo(janela, btnX, btnY, btnW, btnH,
                  hoverBtn ? paleta::iniciarHover : paleta::iniciar);
        if (fonte)
            janela.draw(fazTexto(*fonte, "INICIAR  >", 15, sf::Color::White,
                                 btnX + btnW / 2.f, btnY + btnH / 2.f, true));

        // ---------- barra inferior: dicas em badges ----------
        retangulo(janela, 0.f, altura - barraBase, largura, barraBase, paleta::painel);
        retangulo(janela, 0.f, altura - barraBase, largura, 1.f, paleta::painelLinha);

        if (fonte)
        {
            float bx = 20.f;
            float by = altura - barraBase + 11.f;
            auto badge = [&](const std::string& tecla, const std::string& rotulo)
            {
                sf::Text kt = fazTexto(*fonte, tecla, 12, paleta::texto, 0.f, 0.f);
                float kw = kt.getLocalBounds().width + 16.f;

                retangulo(janela, bx, by, kw, 22.f,
                          sf::Color(38, 44, 63), sf::Color(60, 68, 92), 1.f);
                kt.setPosition(bx + 8.f, by + 3.f);
                janela.draw(kt);

                sf::Text lt = fazTexto(*fonte, rotulo, 12, paleta::textoFraco,
                                       bx + kw + 7.f, by + 3.f);
                janela.draw(lt);

                bx += kw + 7.f + lt.getLocalBounds().width + 20.f;
            };
            badge("clique", "adiciona");
            badge("arraste", "da velocidade inicial");
            badge("Backspace", "desfazer");
            badge("R", "limpar");
            badge("Enter", "iniciar");
        }

        janela.display();
    }

    return particulas;
}
