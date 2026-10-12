#pragma once

// Grandezas em SI: carga [C], massa [kg], posicao [m], velocidade [m/s], forca [N].
// 'raioPx' e so pro desenho e nao tem papel nenhum na fisica.
class Particula
{
    protected:
        double carga;
        double massa;
        float  raioPx;

        double posX, posY;
        double velX, velY;

        // forca acumulada durante o passo (zerada e recalculada a cada F(t))
        double forcaX, forcaY;

    public:
        Particula();
        virtual ~Particula() {}

        void setPosicao(double x, double y) { posX = x; posY = y; }
        double getPosX() const { return posX; }
        double getPosY() const { return posY; }

        void setVelocidade(double vx, double vy) { velX = vx; velY = vy; }
        double getVelX() const { return velX; }
        double getVelY() const { return velY; }
        double getRapidez() const;

        void setCarga(double q) { carga = q; }
        double getCarga() const { return carga; }
        int getSinalCarga() const { return (carga > 0.0) - (carga < 0.0); }

        void setMassa(double m) { massa = m; }
        double getMassa() const { return massa; }

        float getRaioPx() const { return raioPx; }

        void zerarForca() { forcaX = 0.0; forcaY = 0.0; }
        void aplicarForca(double fx, double fy) { forcaX += fx; forcaY += fy; }
        double getForcaX() const { return forcaX; }
        double getForcaY() const { return forcaY; }
};
