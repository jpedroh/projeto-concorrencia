# Exercício de Concorrência

## Dupla

- **jphsd** - João Pedro Henrique Santos Duarte
- **lss8** - Lucas e Silva de Souza

## Como executar o programa?

Primeiro é necessário compilar
```bash
g++ main.cpp -o main.o -pthread -lncurses
```

Após isso, basta executar o programa digitando
```bash
./main.o
```

Após isso, será mostrada a interface do usuário e os comandos disponíveis. Basta seguir as instruções da interface para utilizar o programa. 

## Comandos Disponíveis

### ADD
Permite adicionar uma música na fila de execução. Recebe o nome da música, o artista e a duração da música no formato minutos segundos. Exemplo da sequência de adição de uma música:
```
A for ADD
Digite o nome da música: Azul
Digite o nome do artista: Djavan
Digite a duração da musica em minutos e depois em segundos: 2 30
```

### REMOVE
Remove uma música da fila de execução. Ao remover a música atualmente em execução, a música seguinte será executada.
```
R for REMOVE
Digite o índice da musica a ser removida: 0
```
A fila de execução tem a contagem iniciada em 0.

### PLAY/PAUSE
Alterna entre reproduzindo e em pausa.


### SKIP
Permite pular a execução da música atual, movendo para a música imediatamente posterior.


### QUIT
Encerra a execução do programa.