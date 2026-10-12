#pragma once
#include "Particula.h"
#include "Constantes.h"
#include <vector>

// Eletrodinamica classica em unidades SI.
// Integrador: Velocity Verlet (simpletico - conserva energia bem melhor que Euler).
// As particulas interagem SO pelo campo eletrico: nao ha colisao de contato.
class GerenciadorFisico
{
    private:
        double k;          // constante de Coulomb [N m^2 / C^2]
        double softening;  // comprimento de suavizacao [m]: limita a forca quando r -> 0
        double atrito;     // fator de velocidade por passo (1.0 = conservativo)

        bool   bordasSolidas = false; // reflete a particula ao bater na parede
        double limiteX = 0.0;         // meia-largura da caixa [m]
        double limiteY = 0.0;         // meia-altura da caixa [m]

    public:
        GerenciadorFisico(double k = constantes::kCoulomb,
                          double softening = 3.0e-12,
                          double atrito = 1.0);

        // paredes elasticas: se 'ativas', a particula reflete ao cruzar +/-limX, +/-limY
        void definirBordas(bool ativas, double limX, double limY)
        {
            bordasSolidas = ativas;
            limiteX = limX;
            limiteY = limY;
        }

        // calcula F(t) nas posicoes atuais. Chamar uma vez antes do primeiro atualizar().
        void prepararForcas(std::vector<Particula*>& particulas);

        // avanca um passo dt (Velocity Verlet). Assume forcaX/forcaY = F(t) do passo atual.
        void atualizar(std::vector<Particula*>& particulas, double dt);

        // menor distancia entre dois pares (pra escolher quantos subpassos usar).
        // devolve +infinito se ha menos de 2 particulas.
        double menorSeparacao(const std::vector<Particula*>& particulas) const;

        // energias mecanicas (o Principal usa pra mostrar o drift numerico no HUD)
        double energiaCinetica(const std::vector<Particula*>& particulas) const;
        double energiaPotencial(const std::vector<Particula*>& particulas) const;

        double getK() const { return k; }
        double getSoftening() const { return softening; }
};
