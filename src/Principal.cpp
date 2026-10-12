#include "Principal.h"
#include "Menu.h"
#include "Paleta.h"
#include "Render.h"
#include "CampoEletrico.h"
#include "Constantes.h"
#include "Vista.h"
#include "Formato.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <deque>
#include <string>
#include <vector>

Principal::Principal()
{
}

namespace
{
    void desenharSeta(sf::RenderWindow& janela, sf::Vector2f origem, sf::Vector2f dir,
                      float comprimento, sf::Color cor)
    {
        float mag = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (mag < 1e-9f)
            return;

        sf::Vector2f u(dir.x / mag, dir.y / mag);
        sf::Vector2f ponta(origem.x + u.x * comprimento, origem.y + u.y * comprimento);

        sf::Vertex haste[] = { sf::Vertex(origem, cor), sf::Vertex(ponta, cor) };
        janela.draw(haste, 2, sf::Lines);

        sf::Vector2f perp(-u.y, u.x);
        const float t = 7.f;
        sf::ConvexShape cabeca(3);
        cabeca.setPoint(0, ponta);
        cabeca.setPoint(1, sf::Vector2f(ponta.x - u.x * t + perp.x * t * 0.6f,
                                        ponta.y - u.y * t + perp.y * t * 0.6f));
        cabeca.setPoint(2, sf::Vector2f(ponta.x - u.x * t - perp.x * t * 0.6f,
                                        ponta.y - u.y * t - perp.y * t * 0.6f));
        cabeca.setFillColor(cor);
        janela.draw(cabeca);
    }

    sf::Color misturar(sf::Color a, sf::Color b, float t)
    {
        return sf::Color(static_cast<sf::Uint8>(a.r + (b.r - a.r) * t),
                         static_cast<sf::Uint8>(a.g + (b.g - a.g) * t),
                         static_cast<sf::Uint8>(a.b + (b.b - a.b) * t));
    }
}

void Principal::rodar()
{
    sf::RenderWindow janela(sf::VideoMode(LARGURA, ALTURA), "Simulador de Particulas");
    janela.setFramerateLimit(120);

    // fonte do sistema so pra mostrar textos; se nao achar, roda sem eles
    sf::Font fonte;
    bool temFonte = fonte.loadFromFile("C:/Windows/Fonts/arial.ttf")
                 || fonte.loadFromFile("C:/Windows/Fonts/segoeui.ttf");
    const sf::Font* pFonte = temFonte ? &fonte : nullptr;

    const double ESCALA_PADRAO = 1.0e-12; // 1 px = 1 pm  ->  raio de Bohr ~ 53 px
    Vista vista(static_cast<float>(LARGURA), static_cast<float>(ALTURA), ESCALA_PADRAO);
    Menu menu(static_cast<float>(LARGURA), static_cast<float>(ALTURA), vista);

    constexpr int BASE_SUB = 12;              // subpassos de fisica quando nada esta perto
    constexpr int MAX_SUB = 2000;             // teto de subpassos por frame (particulas coladas)
    constexpr int CELULA = 8;                 // px por amostra do mapa de potencial
    constexpr std::size_t MAX_TRILHA = 1500;  // pontos guardados por rastro

    // botao "voltar ao editor" (canto superior direito)
    const float btnW = 104.f, btnH = 30.f;
    const float btnX = static_cast<float>(LARGURA) - btnW - 14.f;
    const float btnY = 12.f;
    auto noBotao = [&](float x, float y)
    { return x >= btnX && x <= btnX + btnW && y >= btnY && y <= btnY + btnH; };

    // cada volta do laco = uma nova simulacao (editor -> roda -> editor -> ...)
    while (janela.isOpen())
    {
        vista.setMetrosPorPixel(ESCALA_PADRAO); // zoom volta ao padrao no editor

        bool iniciar = false;
        bool quicarBordas = false;
        particulas = menu.montar(janela, pFonte, iniciar, quicarBordas);
        if (!iniciar)
            break; // fechou a janela no editor

        std::vector<Particula*> refs;
        refs.reserve(particulas.size());
        for (auto& p : particulas)
            refs.push_back(p.get());

        // caixa = a janela no zoom padrao, fixa no espaco (em metros)
        const double limiteXm = (LARGURA * 0.5) * ESCALA_PADRAO;
        const double limiteYm = (ALTURA  * 0.5) * ESCALA_PADRAO;
        fisica.definirBordas(quicarBordas, limiteXm, limiteYm);

        fisica.prepararForcas(refs); // F(t0) pro Velocity Verlet arrancar certo

        // energia mecanica no inicio, referencia pra medir o drift numerico
        const double energiaRef = fisica.energiaCinetica(refs) + fisica.energiaPotencial(refs);

        struct Ponto { double x, y; }; // ponto no mundo, em metros
        std::vector<Ponto> sondas;
        std::vector<std::deque<Ponto>> trilhas(particulas.size());

        sf::Clock relogio;
        bool pausado = false;
        bool voltarEditor = false;
        float mouseX = LARGURA / 2.f;
        float mouseY = ALTURA / 2.f;

        int nsub = BASE_SUB;             // subpassos usados no ultimo frame (so pro HUD)

        sf::Image imgCampo;
        sf::Texture texCampo;
        sf::Sprite sprCampo;
        bool campoPronto = false;

        while (janela.isOpen() && !voltarEditor)
        {
            sf::Event evento;
            while (janela.pollEvent(evento))
            {
                if (evento.type == sf::Event::Closed)
                    janela.close();
                else if (evento.type == sf::Event::KeyPressed)
                {
                    if (evento.key.code == sf::Keyboard::Escape)
                        voltarEditor = true;
                    else if (evento.key.code == sf::Keyboard::Space)
                        pausado = !pausado;
                    else if (evento.key.code == sf::Keyboard::Up)
                        escalaTempo = std::min(escalaTempo * 1.6, 1.0e-12);
                    else if (evento.key.code == sf::Keyboard::Down)
                        escalaTempo = std::max(escalaTempo / 1.6, 1.0e-19);
                }
                else if (evento.type == sf::Event::MouseMoved)
                {
                    mouseX = static_cast<float>(evento.mouseMove.x);
                    mouseY = static_cast<float>(evento.mouseMove.y);
                }
                else if (evento.type == sf::Event::MouseWheelScrolled)
                {
                    vista.aplicarZoom(evento.mouseWheelScroll.delta > 0 ? 1.15 : 1.0 / 1.15);
                    campoPronto = false; // escala mudou -> mapa desatualizado
                }
                else if (evento.type == sf::Event::MouseButtonPressed)
                {
                    float bx = static_cast<float>(evento.mouseButton.x);
                    float by = static_cast<float>(evento.mouseButton.y);

                    if (evento.mouseButton.button == sf::Mouse::Left && noBotao(bx, by))
                    {
                        voltarEditor = true;
                    }
                    else if (pausado)
                    {
                        double mx, my;
                        vista.paraMundo(bx, by, mx, my);
                        if (evento.mouseButton.button == sf::Mouse::Left)
                            sondas.push_back({ mx, my });
                        else if (evento.mouseButton.button == sf::Mouse::Right && !sondas.empty())
                            sondas.pop_back();
                    }
                }
            }

            if (voltarEditor || !janela.isOpen())
                break;

            float dtReal = relogio.restart().asSeconds();
            if (dtReal > 0.05f)
                dtReal = 0.05f;

            if (!pausado)
            {
                // subpassos adaptativos: quanto mais perto o par mais proximo estiver,
                // menor o dt -> o Verlet atravessa o nucleo sem injetar energia
                double rmin = fisica.menorSeparacao(refs);
                nsub = BASE_SUB;
                if (std::isfinite(rmin))
                {
                    double razao = constantes::raioBohr / std::max(rmin, fisica.getSoftening());
                    nsub = static_cast<int>(std::ceil(BASE_SUB * std::max(1.0, razao)));
                    nsub = std::clamp(nsub, BASE_SUB, MAX_SUB);
                }

                double passo = (static_cast<double>(dtReal) * escalaTempo) / nsub;
                int passoTrilha = std::max(1, nsub / BASE_SUB); // ~BASE_SUB amostras/frame

                for (int s = 0; s < nsub; ++s)
                {
                    fisica.atualizar(refs, passo);

                    if (s % passoTrilha == 0)
                        for (std::size_t i = 0; i < refs.size(); ++i)
                        {
                            std::deque<Ponto>& tr = trilhas[i];
                            tr.push_back({ refs[i]->getPosX(), refs[i]->getPosY() });
                            if (tr.size() > MAX_TRILHA)
                                tr.pop_front();
                        }
                }
            }

            // mapa de potencial: recalcula quando pausado e ainda nao ha um valido
            if (pausado && !campoPronto && !refs.empty())
            {
                const unsigned gw = LARGURA / CELULA;
                const unsigned gh = ALTURA / CELULA;
                const double k = fisica.getK();
                const double soft = fisica.getSoftening();

                std::vector<float> grade(static_cast<std::size_t>(gw) * gh);
                float vmax = 1e-9f;
                for (unsigned j = 0; j < gh; ++j)
                    for (unsigned i = 0; i < gw; ++i)
                    {
                        double mx, my;
                        vista.paraMundo((i + 0.5f) * CELULA, (j + 0.5f) * CELULA, mx, my);
                        float v = static_cast<float>(campoEletrico::potencial(refs, mx, my, k, soft));
                        grade[j * gw + i] = v;
                        vmax = std::max(vmax, std::fabs(v));
                    }

                const float escala = vmax * 0.30f;
                imgCampo.create(gw, gh, sf::Color::Transparent);
                for (unsigned j = 0; j < gh; ++j)
                    for (unsigned i = 0; i < gw; ++i)
                    {
                        float tsig = std::tanh(grade[j * gw + i] / escala); // -1..1
                        float a = std::fabs(tsig);
                        sf::Color base = (tsig >= 0.f) ? paleta::proton : paleta::eletron;
                        sf::Color c = misturar(paleta::fundo, base, a);
                        c.a = static_cast<sf::Uint8>(35.f + 175.f * a);
                        imgCampo.setPixel(i, j, c);
                    }

                texCampo.loadFromImage(imgCampo);
                texCampo.setSmooth(true);
                sprCampo.setTexture(texCampo, true);
                sprCampo.setScale(static_cast<float>(CELULA), static_cast<float>(CELULA));
                campoPronto = true;
            }
            if (!pausado)
                campoPronto = false;

            // ---------------------------------- desenho ----------------------------------
            janela.clear(paleta::fundo);
            desenharGrade(janela, static_cast<float>(LARGURA), static_cast<float>(ALTURA));

            if (pausado && campoPronto)
                janela.draw(sprCampo);

            // paredes da caixa (em coordenadas do mundo, entao acompanham o zoom)
            if (quicarBordas)
            {
                sf::Vector2f a = vista.paraTela(-limiteXm, -limiteYm);
                sf::Vector2f b = vista.paraTela( limiteXm,  limiteYm);
                sf::RectangleShape caixa(sf::Vector2f(b.x - a.x, b.y - a.y));
                caixa.setPosition(a);
                caixa.setFillColor(sf::Color::Transparent);
                caixa.setOutlineThickness(2.f);
                caixa.setOutlineColor(paleta::acento);
                janela.draw(caixa);
            }

            // rastros (cauda transparente -> cabeca opaca)
            for (std::size_t i = 0; i < particulas.size(); ++i)
            {
                const std::deque<Ponto>& tr = trilhas[i];
                if (tr.size() < 2)
                    continue;

                sf::Color cor = (particulas[i]->getSinalCarga() >= 0) ? paleta::proton : paleta::eletron;
                sf::VertexArray linha(sf::LineStrip, tr.size());
                for (std::size_t j = 0; j < tr.size(); ++j)
                {
                    float aa = static_cast<float>(j) / static_cast<float>(tr.size() - 1);
                    linha[j].position = vista.paraTela(tr[j].x, tr[j].y);
                    linha[j].color = sf::Color(cor.r, cor.g, cor.b, static_cast<sf::Uint8>(160.f * aa));
                }
                janela.draw(linha);
            }

            for (auto& p : particulas)
                desenharParticula(janela, *p, vista);

            if (pausado)
            {
                const double k = fisica.getK();
                const double soft = fisica.getSoftening();

                // marcadores fixados
                for (std::size_t idx = 0; idx < sondas.size(); ++idx)
                {
                    sf::Vector2f pt = vista.paraTela(sondas[idx].x, sondas[idx].y);
                    double v = campoEletrico::potencial(refs, sondas[idx].x, sondas[idx].y, k, soft);

                    sf::CircleShape anel(5.f);
                    anel.setOrigin(5.f, 5.f);
                    anel.setPosition(pt);
                    anel.setFillColor(sf::Color::Transparent);
                    anel.setOutlineThickness(2.f);
                    anel.setOutlineColor(paleta::texto);
                    janela.draw(anel);

                    if (pFonte)
                    {
                        char letra = static_cast<char>('A' + static_cast<int>(idx % 26));
                        sf::Text t(std::string(1, letra) + ": " + fmtSci(v) + " V", *pFonte, 12);
                        t.setFillColor(paleta::texto);
                        t.setPosition(pt.x + 9.f, pt.y - 7.f);
                        janela.draw(t);
                    }
                }

                // sonda que segue o cursor
                double mx, my;
                vista.paraMundo(mouseX, mouseY, mx, my);
                double v = campoEletrico::potencial(refs, mx, my, k, soft);
                sf::Vector2f e = campoEletrico::campoVetor(refs, mx, my, k, soft);
                float emag = std::sqrt(e.x * e.x + e.y * e.y);

                sf::CircleShape ponto(3.f);
                ponto.setOrigin(3.f, 3.f);
                ponto.setPosition(mouseX, mouseY);
                ponto.setFillColor(paleta::texto);
                janela.draw(ponto);

                desenharSeta(janela, sf::Vector2f(mouseX, mouseY), e, 42.f, paleta::acento);

                if (pFonte)
                {
                    float px = mouseX + 14.f;
                    float py = mouseY + 14.f;
                    if (px > LARGURA - 172.f) px = mouseX - 172.f;
                    if (py > ALTURA - 48.f)   py = mouseY - 48.f;

                    sf::RectangleShape cx(sf::Vector2f(162.f, 40.f));
                    cx.setPosition(px, py);
                    cx.setFillColor(sf::Color(15, 18, 28, 225));
                    cx.setOutlineThickness(1.f);
                    cx.setOutlineColor(paleta::painelLinha);
                    janela.draw(cx);

                    sf::Text t1("V = " + fmtSci(v) + " V", *pFonte, 13);
                    t1.setFillColor(paleta::texto);
                    t1.setPosition(px + 8.f, py + 4.f);
                    janela.draw(t1);

                    sf::Text t2("|E| = " + fmtSci(emag) + " V/m", *pFonte, 13);
                    t2.setFillColor(paleta::textoFraco);
                    t2.setPosition(px + 8.f, py + 20.f);
                    janela.draw(t2);
                }
            }

            // barra de escala = comprimento do raio de Bohr
            {
                float lb = vista.metrosParaPixels(constantes::raioBohr);
                float x0 = static_cast<float>(LARGURA) - lb - 20.f;
                float y0 = static_cast<float>(ALTURA) - 40.f;

                sf::Vertex barra[] = {
                    sf::Vertex(sf::Vector2f(x0, y0), paleta::textoFraco),
                    sf::Vertex(sf::Vector2f(x0 + lb, y0), paleta::textoFraco)
                };
                janela.draw(barra, 2, sf::Lines);

                if (pFonte)
                {
                    sf::Text t("raio de Bohr", *pFonte, 11);
                    t.setFillColor(paleta::textoFraco);
                    t.setPosition(x0, y0 - 16.f);
                    janela.draw(t);
                }
            }

            // botao "voltar ao editor"
            {
                bool hover = noBotao(mouseX, mouseY);
                sf::RectangleShape b(sf::Vector2f(btnW, btnH));
                b.setPosition(btnX, btnY);
                b.setFillColor(hover ? sf::Color(46, 54, 78) : sf::Color(28, 33, 48, 235));
                b.setOutlineThickness(1.f);
                b.setOutlineColor(paleta::painelLinha);
                janela.draw(b);

                if (pFonte)
                {
                    sf::Text t("< EDITOR", *pFonte, 13);
                    t.setFillColor(paleta::texto);
                    sf::FloatRect tb = t.getLocalBounds();
                    t.setPosition(btnX + (btnW - tb.width) / 2.f - tb.left,
                                  btnY + (btnH - tb.height) / 2.f - tb.top);
                    janela.draw(t);
                }
            }

            // rodape com o estado da simulacao
            if (pFonte)
            {
                double vmaxc = 0.0;
                for (auto& p : particulas)
                    vmaxc = std::max(vmaxc, p->getRapidez());
                vmaxc = vmaxc / constantes::luz * 100.0;

                double eAgora = fisica.energiaCinetica(refs) + fisica.energiaPotencial(refs);
                double deriva = (std::fabs(energiaRef) > 1e-30)
                              ? (eAgora - energiaRef) / std::fabs(energiaRef) * 100.0 : 0.0;

                std::string s = "E: " + fmtSci(eAgora / constantes::e, 4) + " eV  (drift "
                              + fmtSci(deriva, 2) + "%)"
                              + "   |   v_max: " + fmtSci(vmaxc, 2) + "% c"
                              + "   |   subpassos: " + std::to_string(nsub)
                              + "   |   "
                              + (pausado ? "PAUSADO - clique fixa sonda, botao direito remove"
                                         : "Espaco: pausa   Cima/Baixo: tempo   roda: zoom   Esc: novo");

                sf::Text t(s, *pFonte, 12);
                t.setFillColor(paleta::textoFraco);
                t.setPosition(12.f, static_cast<float>(ALTURA) - 22.f);
                janela.draw(t);
            }

            janela.display();
        }
    }
}
