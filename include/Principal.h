#pragma once
#include "Particula.h"
#include "GerenciadorFisico.h"
#include <vector>
#include <memory>

class Principal
{
    public:
        static constexpr unsigned LARGURA = 800;
        static constexpr unsigned ALTURA  = 600;

    private:
        // um vetor unico e polimorfico - precisa ser ponteiro pra nao fatiar (slicing)
        std::vector<std::unique_ptr<Particula>> particulas;
        GerenciadorFisico fisica;

        // segundos de simulacao por segundo real (ajustavel com Cima/Baixo).
        // ~5e-17 deixa uma orbita de eletron durar alguns segundos na tela.
        double escalaTempo = 5.0e-17;

    public:
        Principal();
        void rodar();
};
