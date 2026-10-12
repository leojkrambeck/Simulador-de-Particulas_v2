#pragma once

// Constantes fisicas em unidades SI (metro, quilograma, segundo, Coulomb).
namespace constantes
{
    inline constexpr double e            = 1.602176634e-19;       // carga elementar [C]
    inline constexpr double kCoulomb     = 8.9875517873681764e9;  // 1/(4*pi*eps0) [N m^2 / C^2]
    inline constexpr double massaEletron = 9.1093837015e-31;      // [kg]
    inline constexpr double massaProton  = 1.67262192369e-27;     // [kg]
    inline constexpr double raioBohr     = 5.29177210903e-11;     // [m]
    inline constexpr double luz          = 299792458.0;           // [m/s]
}
