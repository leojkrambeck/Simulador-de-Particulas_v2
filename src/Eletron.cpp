#include "Eletron.h"
#include "Constantes.h"

Eletron::Eletron() : Particula()
{
    carga  = -constantes::e;            // -e
    massa  = constantes::massaEletron;
    raioPx = 9.f;                       // so visual
}
