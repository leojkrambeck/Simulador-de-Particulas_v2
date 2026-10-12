#include "GerenciadorFisico.h"
#include "Constantes.h"
#include <cmath>
#include <cstddef>
#include <limits>

GerenciadorFisico::GerenciadorFisico(double k, double softening, double atrito)
    : k(k), softening(softening), atrito(atrito)
{
}

namespace
{
    // Escreve em forcaX/forcaY a forca de Coulomb resultante sobre cada particula.
    //   F = k q_i q_j / (r^2 + s^2)   ao longo do versor que liga o par
    // q_i q_j > 0 -> repulsao ; < 0 -> atracao (o sinal ja sai da conta).
    void calcularForcas(std::vector<Particula*>& ps, double k, double soft)
    {
        const std::size_t n = ps.size();
        const double s2 = soft * soft;

        for (std::size_t i = 0; i < n; ++i)
            ps[i]->zerarForca();

        for (std::size_t i = 0; i < n; ++i)
        {
            for (std::size_t j = i + 1; j < n; ++j)
            {
                double dx = ps[j]->getPosX() - ps[i]->getPosX();
                double dy = ps[j]->getPosY() - ps[i]->getPosY();

                double r2 = dx * dx + dy * dy;
                double r  = std::sqrt(r2);

                double forca = k * ps[i]->getCarga() * ps[j]->getCarga() / (r2 + s2);

                double ux = (r > 0.0) ? dx / r : 1.0;
                double uy = (r > 0.0) ? dy / r : 0.0;

                ps[i]->aplicarForca(-forca * ux, -forca * uy);
                ps[j]->aplicarForca( forca * ux,  forca * uy);
            }
        }
    }

    // trava a rapidez em 99% de c (Verlet nao e relativistico; perto do nucleo
    // "softado" a aceleracao ainda e enorme e mandaria a particula alem de c)
    void limitarVelocidade(double& vx, double& vy)
    {
        const double lim = 0.99 * constantes::luz;
        double v2 = vx * vx + vy * vy;
        if (v2 > lim * lim)
        {
            double f = lim / std::sqrt(v2);
            vx *= f;
            vy *= f;
        }
    }

    // reflexao elastica numa parede em +/-lim: espelha a posicao e inverte a velocidade
    void refletirEixo(double& pos, double& vel, double lim)
    {
        if (lim <= 0.0)
            return;
        if (pos < -lim) { pos = -2.0 * lim - pos; vel = -vel; }
        else if (pos > lim) { pos = 2.0 * lim - pos; vel = -vel; }
        if (pos < -lim) pos = -lim; // seguranca se passou de raspao
        else if (pos > lim) pos = lim;
    }
}

void GerenciadorFisico::prepararForcas(std::vector<Particula*>& particulas)
{
    calcularForcas(particulas, k, softening);
}

double GerenciadorFisico::menorSeparacao(const std::vector<Particula*>& ps) const
{
    double menor = std::numeric_limits<double>::infinity();
    const std::size_t n = ps.size();
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = i + 1; j < n; ++j)
        {
            double dx = ps[j]->getPosX() - ps[i]->getPosX();
            double dy = ps[j]->getPosY() - ps[i]->getPosY();
            double r = std::sqrt(dx * dx + dy * dy);
            if (r < menor)
                menor = r;
        }
    return menor;
}

double GerenciadorFisico::energiaCinetica(const std::vector<Particula*>& ps) const
{
    double e = 0.0;
    for (const Particula* p : ps)
    {
        double v = p->getRapidez();
        e += 0.5 * p->getMassa() * v * v;
    }
    return e;
}

double GerenciadorFisico::energiaPotencial(const std::vector<Particula*>& ps) const
{
    const double s2 = softening * softening;
    double e = 0.0;
    const std::size_t n = ps.size();
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = i + 1; j < n; ++j)
        {
            double dx = ps[j]->getPosX() - ps[i]->getPosX();
            double dy = ps[j]->getPosY() - ps[i]->getPosY();
            e += k * ps[i]->getCarga() * ps[j]->getCarga() / std::sqrt(dx * dx + dy * dy + s2);
        }
    return e;
}

void GerenciadorFisico::atualizar(std::vector<Particula*>& particulas, double dt)
{
    const std::size_t n = particulas.size();

    // 1) meio-chute com F(t) + deslocamento
    for (std::size_t i = 0; i < n; ++i)
    {
        Particula* p = particulas[i];
        double m = (p->getMassa() > 0.0) ? p->getMassa() : 1.0;

        double ax = p->getForcaX() / m;
        double ay = p->getForcaY() / m;

        double vx = p->getVelX() + 0.5 * ax * dt;
        double vy = p->getVelY() + 0.5 * ay * dt;
        limitarVelocidade(vx, vy);

        double px = p->getPosX() + vx * dt;
        double py = p->getPosY() + vy * dt;

        if (bordasSolidas)
        {
            refletirEixo(px, vx, limiteX);
            refletirEixo(py, vy, limiteY);
        }

        p->setVelocidade(vx, vy);
        p->setPosicao(px, py);
    }

    // 2) F(t + dt) nas novas posicoes
    calcularForcas(particulas, k, softening);

    // 3) segundo meio-chute com F(t + dt)
    for (std::size_t i = 0; i < n; ++i)
    {
        Particula* p = particulas[i];
        double m = (p->getMassa() > 0.0) ? p->getMassa() : 1.0;

        double ax = p->getForcaX() / m;
        double ay = p->getForcaY() / m;

        double vx = (p->getVelX() + 0.5 * ax * dt) * atrito;
        double vy = (p->getVelY() + 0.5 * ay * dt) * atrito;
        limitarVelocidade(vx, vy);

        p->setVelocidade(vx, vy);
    }
}
