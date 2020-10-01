#include <bits/stdc++.h>
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#define endl '\n'

using namespace std;

struct Musica
{
    string nome;
    string artista;
    int duracao_em_segundos;
};

string getstrteste()
{
    std::string input;

    // let the terminal do the line editing
    nocbreak();
    echo();

    // this reads from buffer after <ENTER>, not "raw" 
    // so any backspacing etc. has already been taken care of
    int ch = getch();

    while ( ch != '\n' )
    {
        input.push_back( ch );
        ch = getch();
    }

    // restore your cbreak / echo settings here

    return input;
}

int main(int argc, char *argv[]) {
    vector<Musica> fila;

                Musica musicaLeitura;
                int minutos;
                int segundos;
                int duracao_em_segundos;

    initscr();

    while(true) {
        printw("Seja bem vindo ao JPLay.\n");

        if(fila.empty()) {
            printw("A fila de execução está vazia\n");
        } else {
            printw("Artista \t Musica \t Duracao \n");
            for(auto musica : fila) {
                printw("%s \t %s \t %d \n", musica.artista.data(), musica.nome.data(), musica.duracao_em_segundos);
            }
            refresh();
        }


        printw("Escolha uma opção\n");
        int ch = getch();

        switch (ch) {
            case 'a':
            case 'A':

                printw("Insira o nome da musica\n");
                refresh();
                musicaLeitura.nome = getstrteste();

                printw("Insira o nome do artista\n");
                refresh();
                musicaLeitura.artista = getstrteste();
                
                printw("Insira a duracao da musica no formato MM:ss\n");
                refresh();
                // getstrteste(duracao);
                // sscanf(duracao, "%d:%d", &minutos, &segundos);
                musicaLeitura.duracao_em_segundos = 0;
                
                fila.emplace_back(musicaLeitura);

                break;
            case 'q':
            case 'Q':
                endwin();
                exit(0);
                break;
        }
        refresh();
    }

    endwin();
    return 0;
}
