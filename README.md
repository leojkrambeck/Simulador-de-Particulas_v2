# ⚛️ Simulador de Partículas

Simulador interativo de eletrodinâmica clássica entre prótons e elétrons, em **C++17** com **SFML**. Você posiciona as partículas numa tela gráfica, dá play, e observa a física de Coulomb acontecer em unidades físicas reais (SI) — com integração estável, sonda de campo elétrico, rastros de trajetória e um mapa de potencial visual.

Não é um brinquedo com números inventados: as cargas são a carga elementar de verdade (`1,602 × 10⁻¹⁹ C`), as massas são a massa real do próton e do elétron, e a constante de Coulomb é a constante de Coulomb. A "trapaça" fica só na velocidade com que o tempo passa na tela — senão um picossegundo real levaria um picossegundo pra desenhar.

## Índice

- [Funcionalidades](#funcionalidades)
- [Controles](#controles)
- [Como compilar e rodar](#como-compilar-e-rodar)
- [Estrutura do projeto](#estrutura-do-projeto)
- [Conceitos de física](#conceitos-de-física)
- [Conceitos de engenharia de software](#conceitos-de-engenharia-de-software)
- [Limitações conhecidas](#limitações-conhecidas)

## Funcionalidades

- **Editor visual** — clique para posicionar prótons e elétrons dentro da própria janela; arraste para dar a eles uma velocidade inicial (com prévia em % da velocidade da luz enquanto você arrasta).
- **Física em unidades SI reais** — carga, massa, distância, tempo e força em Coulomb, quilograma, metro, segundo e Newton de verdade. Nada de "carga = 5".
- **Integração por Velocity Verlet** com **passo de tempo adaptativo** — o passo encolhe sozinho quando duas partículas se aproximam, evitando que o integrador injete energia artificial e a partícula "escape" sem motivo físico.
- **Sonda de campo elétrico** — pausa a simulação, passa o mouse por qualquer ponto e veja o potencial elétrico (V) e o vetor campo elétrico (E) naquele ponto. Fixe marcadores pra comparar vários pontos.
- **Mapa de potencial** — ao pausar, o fundo da tela se colore com o potencial elétrico gerado pelas partículas (vermelho = positivo, azul = negativo).
- **Rastros de trajetória** — cada partícula deixa um rastro que esmaece, mostrando a forma real da órbita/trajetória.
- **Paredes elásticas opcionais** — um toggle no editor liga uma "caixa" que reflete as partículas nas bordas em vez de deixá-las fugir pra sempre.
- **Medidor de deriva de energia** — mostra o quanto a energia mecânica total se afastou do valor inicial, pra você avaliar se a simulação está numericamente estável.
- **Zoom e escala de tempo ajustáveis** em tempo real, sem pausar.
- **Editor ↔ simulação em loop** — termina uma simulação e volta direto pro editor pra montar outra, sem reiniciar o programa.

## Controles

### No editor (antes de iniciar)

| Ação | Efeito |
|---|---|
| Clique esquerdo | Adiciona a partícula da ferramenta ativa |
| Clique esquerdo + arrastar | Define a velocidade inicial da partícula (direção e módulo do arrasto) |
| Clique direito | Adiciona o tipo *oposto* ao da ferramenta ativa, sem velocidade |
| `P` / `1` | Seleciona próton como ferramenta |
| `E` / `2` | Seleciona elétron como ferramenta |
| Roda do mouse | Alterna a ferramenta ativa |
| `Backspace` | Desfaz a última partícula |
| `R` | Limpa tudo |
| Toggle "bordas" | Liga/desliga a caixa com paredes elásticas |
| `Enter` / `Espaço` / botão **INICIAR** | Começa a simulação |

### Na simulação

| Ação | Efeito |
|---|---|
| `Espaço` | Pausa / retoma |
| `Cima` / `Baixo` | Acelera / desacelera a escala de tempo |
| Roda do mouse | Zoom |
| `Esc` / botão **‹ EDITOR** | Volta pro editor pra montar outra simulação |
| Clique esquerdo (pausado) | Fixa uma sonda de potencial no ponto |
| Clique direito (pausado) | Remove a última sonda fixada |

## Como compilar e rodar

Pré-requisitos: **CMake ≥ 3.14**, um compilador C++17 (testado com MSVC/Visual Studio) e conexão à internet na primeira configuração — o [SFML](https://www.sfml-dev.org/) 2.6.2 é baixado automaticamente via `FetchContent`, não precisa instalar nada manualmente.

```bash
# configura (escolha -A x64 ou -A ARM64 conforme sua maquina)
cmake -S . -B build -A x64

# compila
cmake --build build --config Release

# roda
./build/Release/app.exe
```

> Em Windows ARM64, o CMake às vezes mistura bibliotecas de arquiteturas diferentes se `-A` não for passado explicitamente — sempre especifique a plataforma na configuração.

## Estrutura do projeto

```
include/
  Particula.h         # classe base: carga, massa, posicao, velocidade, forca (tudo em SI)
  Proton.h / Eletron.h # especializacoes: carga +-e, massa real, raio visual
  GerenciadorFisico.h  # Coulomb + Velocity Verlet + paredes elasticas (sem SFML)
  CampoEletrico.h      # potencial V(x,y) e campo E(x,y) gerados pelas particulas
  Constantes.h         # constantes fisicas em SI (e, k, massas, raio de Bohr, c)
  Vista.h              # conversao mundo (metros) <-> tela (pixels), zoom
  Menu.h               # editor grafico (posicionar particulas antes de rodar)
  Principal.h           # laco principal: editor <-> simulacao
  Render.h / Paleta.h  # desenho e paleta de cores compartilhados
  Formato.h            # formatacao compacta de numeros (notacao cientifica)

src/
  *.cpp                # implementacao de cada header acima
  main.cpp             # ponto de entrada

CMakeLists.txt         # build com FetchContent (baixa o SFML sozinho)
```

## Conceitos de física

### Lei de Coulomb

A força entre duas cargas pontuais é

```
F = k · q₁ · q₂ / r²
```

onde `k ≈ 8,99 × 10⁹ N·m²/C²` é a constante de Coulomb, `q₁, q₂` as cargas (com sinal) e `r` a distância entre elas. O sinal do produto `q₁·q₂` já decide tudo: mesmo sinal repele, sinais opostos atraem. Isso é calculado par a par para toda partícula em [GerenciadorFisico.cpp](src/GerenciadorFisico.cpp).

### Potencial e campo elétrico

Além da força entre partículas, o simulador calcula duas grandezas num **ponto qualquer do espaço** (onde você põe o cursor):

- **Potencial elétrico** `V(x,y) = k · Σ qᵢ / rᵢ` — um escalar, em volts.
- **Campo elétrico** `E(x,y) = k · Σ qᵢ · (P − rᵢ) / rᵢ³` — um vetor, em volts/metro, que aponta pra longe das cargas positivas.

Isso é eletrostática pura: só faz sentido porque, ao pausar, as cargas estão paradas.

### Unidades reais (SI) e a "câmera" que converte pra pixels

Nada na física é escalado — `1 C` é `1 C`. O que existe é uma classe [`Vista`](include/Vista.h) que funciona como uma câmera: converte metros (posição real das partículas) em pixels (onde desenhar na tela), com um fator `metros por pixel` que você pode mudar com o zoom. A física nunca sabe que existe uma tela; o `Vista` é a única ponte entre os dois mundos.

Por padrão, 1 pixel = 1 picômetro, o que deixa o raio de Bohr (`5,29 × 10⁻¹¹ m`, o "tamanho" clássico de um átomo de hidrogênio) em torno de 53 pixels — visível e confortável.

### Por que Velocity Verlet, e não o método de Euler

O jeito mais simples de simular movimento é o método de Euler: `v += a·dt; x += v·dt`. O problema é que ele **vaza energia** — numa órbita, isso aparece como a órbita crescendo ou encolhendo sozinha, sem motivo físico algum.

O **Velocity Verlet** é um integrador *simplético*: ele conserva a energia muito melhor ao longo de muitos passos, porque divide cada passo em dois meio-impulsos de velocidade com o recálculo da força no meio:

```
v(t + dt/2) = v(t) + ½·a(t)·dt
x(t + dt)   = x(t) + v(t + dt/2)·dt
a(t + dt)   = F(x(t + dt)) / m
v(t + dt)   = v(t + dt/2) + ½·a(t + dt)·dt
```

### Softening: suavizando o núcleo

A força de Coulomb vai a infinito quando `r → 0`. Como duas partículas de verdade podem chegar bem perto uma da outra, isso faria a simulação explodir. A solução padrão em simulações de N-corpos é o **softening**: em vez de dividir por `r²`, divide-se por `r² + s²`, onde `s` é um comprimento pequeno fixo. Isso dá um teto físico-mas-artificial pra força, sem alterar quase nada longe do núcleo.

### Passo de tempo adaptativo

Mesmo com softening, perto do núcleo a força muda *rápido demais* dentro de um único passo de tempo fixo — e é exatamente aí que o Verlet "erra" e injeta energia, fazendo um elétron escapar sem razão física. A solução, em [Principal.cpp](src/Principal.cpp), é medir a menor distância entre qualquer par de partículas a cada quadro e **aumentar o número de subpassos** proporcionalmente — quanto mais perto, menor o `dt` efetivo, e a integração atravessa a região violenta com precisão.

### Limite de velocidade

O Verlet aqui não é relativístico. Perto do núcleo "suavizado" a aceleração ainda pode ser gigantesca o bastante pra mandar uma partícula além da velocidade da luz num único passo — fisicamente impossível. Por segurança, a velocidade é travada em 99% de `c`.

### Deriva de energia (drift)

A energia mecânica total (cinética + potencial) de um sistema isolado deveria ser constante. Qualquer integrador numérico tem erro, então ela sempre deriva um pouco. O HUD mostra esse desvio em tempo real (`drift %`) — é a melhor forma de julgar se a simulação está numericamente saudável.

### Paredes elásticas

Quando ligadas, a "caixa" é a própria janela no zoom padrão, fixa no espaço em metros. Uma partícula que cruza a borda tem sua posição espelhada de volta pra dentro e a componente de velocidade perpendicular à parede invertida — uma colisão perfeitamente elástica, sem perda de energia.

### Sobre fidelidade: o que o modelo representa (e o que não representa)

Vale ser honesto: isso é **eletrodinâmica clássica**, não mecânica quântica. Um elétron de verdade ao redor de um próton não segue uma trajetória como um planeta — ele existe como uma nuvem de probabilidade (orbital), e um elétron clássico orbitando perderia energia por radiação e colapsaria no núcleo em frações de segundo. O simulador modela corretamente a eletrostática clássica (Coulomb, potencial, campo, conservação de energia), mas não modela radiação eletromagnética, efeitos relativísticos além do limite de velocidade, nem — principalmente — o comportamento quântico que é o que de fato mantém átomos reais estáveis. É uma ferramenta pra ganhar intuição sobre cargas elétricas interagindo, não um preditor quantitativo da estrutura atômica.

## Conceitos de engenharia de software

### Hierarquia de classes e polimorfismo

`Particula` é a classe base (carga, massa, posição, velocidade, força). `Proton` e `Eletron` herdam dela e só diferem nos valores que colocam no construtor (carga `+e`/`-e`, massa real, raio visual). A coleção de partículas é um `std::vector<std::unique_ptr<Particula>>` — **tem que ser ponteiro**, porque guardar `Particula` por valor cortaria (*object slicing*) as partes específicas de `Proton`/`Eletron` e quebraria o polimorfismo.

### Separação de responsabilidades

- [`GerenciadorFisico`](include/GerenciadorFisico.h) e [`CampoEletrico`](include/CampoEletrico.h) **não incluem nada de SFML** — são física pura, testável isoladamente, sem saber que existe uma janela.
- [`Render.h`](include/Render.h) cuida só de desenhar.
- [`Menu`](include/Menu.h) cuida só da fase de edição.
- [`Principal`](include/Principal.h) orquestra os três: alterna entre editor e simulação, mede o tempo real, e decide quando chamar cada peça.

### `Vista`: o padrão de câmera

A classe [`Vista`](include/Vista.h) é a única responsável por converter coordenadas do mundo físico (metros, `double`) em coordenadas de tela (pixels, `float`) e vice-versa. Isso desacopla completamente a física da apresentação: a física nunca manipula pixels, e o desenho nunca manipula metros diretamente.

### CMake + `FetchContent`

O [`CMakeLists.txt`](CMakeLists.txt) usa `FetchContent` para baixar e compilar o SFML 2.6.2 automaticamente na primeira configuração — não é preciso instalar a biblioteca manualmente nem versionar binários de terceiros no repositório.

## Limitações conhecidas

- Simulação **2D**, não 3D.
- Sem força magnética (só o campo elétrico — não há `v × B`).
- Sem radiação eletromagnética (uma carga acelerada deveria perder energia como luz).
- As constantes de software (passo base, softening, limite de velocidade) são escolhas de engenharia pra manter a simulação estável e visível, documentadas em [`Constantes.h`](include/Constantes.h) e [`GerenciadorFisico.h`](include/GerenciadorFisico.h).
