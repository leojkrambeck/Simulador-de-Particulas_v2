#include "Proton.h"
#include "Constantes.h"

Proton::Proton() : Particula()
{
    carga  = constantes::e;             // +e
    massa  = constantes::massaProton;
    raioPx = 16.f;                      // so visual
}
