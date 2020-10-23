#include <bits/stdc++.h>
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#define endl '\n'

using namespace std;

WINDOW *win1;
WINDOW *win2;
WINDOW *win3;

int maxX, maxY;

int duracao_atual = 0;
int IS_PLAYING = 0;
pthread_mutex_t mutexx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

struct Musica
{
    string nome;
    string artista;
    int duracao_em_segundos; //a duracao em segundos foi adicionada para uso futuro com a funcao de play/pause
};

vector<Musica> fila;

void imprimir_fila_de_reproducao() {
    wclear(win1);
    box(win1, ' ', '*');

    int safeX = int(maxX - 8)/6;

    mvwprintw(win1, 1, 2, "#");
    mvwprintw(win1, 1, 6, "Musica");
    mvwprintw(win1, 1, 3*safeX, "Artista");
    mvwprintw(win1, 1, 5*safeX, "Duracao");

    if(fila.empty()) {
        mvwprintw(win1, (maxY - 6) / 2, (maxX/2) - 11, "Nenhuma música na fila");
    }

    for(int i = 0; i < fila.size(); i++) {
        Musica musica = fila.at(i);

        mvwprintw(win1, i + 2, 2, "%d", i);
        mvwprintw(win1, i + 2, 6, musica.nome.data());
        mvwprintw(win1, i + 2, 3*safeX, musica.artista.data());

        int minutos = musica.duracao_em_segundos / 60;
        int segundos = musica.duracao_em_segundos % 60;
        mvwprintw(win1, i + 2, 5*safeX, "%d:%02d", minutos, segundos);
    }

    wrefresh(win1);
}

void* receber_input(void* args) {
    while(true) {
        int safeX = int(maxX - 4)/5;
        
        wmove(win3, 1, 0);
        wclrtoeol(win3);
        mvwprintw(win3, 1, 2, "A for ADD");
        mvwprintw(win3, 1, safeX, "R for REMOVE");
        mvwprintw(win3, 1, 2*safeX, "P for PLAY/PAUSE");
        mvwprintw(win3, 1, 3*safeX, "S for SKIP");
        mvwprintw(win3, 1, 4*safeX, "Q for QUIT");
        wrefresh(win3);
        char option = wgetch(win3);

        if(option == 'Q' || option == 'q') {
            endwin();
            exit(0);
        } else if(option == 'A' || option == 'a') {
            Musica musicaLeitura;

            char nome[20];
            char artista[20];

            wmove(win3, 1, 0);
            wclrtoeol(win3);
            mvwprintw(win3, 1, 2, "Digite o nome da musica: ");
            wrefresh(win3);
            wgetnstr(win3, nome, 20);
            musicaLeitura.nome = string(nome);

            wmove(win3, 1, 0);
            wclrtoeol(win3);
            mvwprintw(win3, 1, 2, "Digite o nome do artista: ");
            wrefresh(win3);
            wgetnstr(win3, artista, 20);
            musicaLeitura.artista = string(artista);
            
            int minutos, segundos;
            wmove(win3, 1, 0);
            wclrtoeol(win3);
            mvwprintw(win3, 1, 2, "Digite a duração da musica em minutos e depois em segundos: ");
            wrefresh(win3);
            wscanw(win3, "%d %d", &minutos, &segundos);
            musicaLeitura.duracao_em_segundos = minutos * 60 + segundos;
            fila.emplace_back(musicaLeitura);
        } else if(option == 'R' || option == 'r') {
            int indice;
            
            wmove(win3, 1, 0);
            wclrtoeol(win3);
            mvwprintw(win3, 1, 2, "Digite o índice da musica a ser removida: ");
            wrefresh(win3);
            wscanw(win3, "%d", &indice);
            fila.erase(fila.begin() + indice);
        } else if (option == 'P' || option == 'p') {
            IS_PLAYING = !IS_PLAYING;
        } else if (option == 'S' || option == 's') {
            if(!fila.empty()) {
                duracao_atual = 0;
                fila = vector<Musica>(fila.begin() + 1, fila.end());
            }
        }
    }
}

void* play(void* arg) {
    while(true) {
        if(IS_PLAYING && !fila.empty() && duracao_atual < fila.at(0).duracao_em_segundos) {
            duracao_atual++;
        } else if (!fila.empty() && duracao_atual == fila.at(0).duracao_em_segundos) {
            duracao_atual = 0;
            fila = vector<Musica>(fila.begin() + 1, fila.end());
        } else if (fila.empty()) {
            duracao_atual = 0;
            IS_PLAYING = 0;
        }
        sleep(1);
        pthread_cond_broadcast(&cond);
    }
}

void resetar_barra_de_progresso() {
    wmove(win2, 1, 0);
    wclrtoeol(win2);
}

void imprimir_barra_de_progresso() {
    if(duracao_atual == 0) { 
        resetar_barra_de_progresso();
    }

    int minutos = duracao_atual / 60;
    int segundos = duracao_atual % 60;

    int duracaoTotal = fila.empty() ? INT_MAX : fila.at(0).duracao_em_segundos;

    int minutosTotais = fila.empty() ? 0 : fila.at(0).duracao_em_segundos / 60;
    int segundosTotais = fila.empty() ? 0 : fila.at(0).duracao_em_segundos % 60;

    mvwprintw(win2, 1, 2, "%d:%02d", minutos, segundos);
    mvwprintw(win2, 1, maxX - 7, "%02d:%02d", minutosTotais, segundosTotais);
    mvwhline(win2, 1, 7, '-', int(((float) duracao_atual/duracaoTotal) * (maxX - 16)));
    wrefresh(win2);
}

void* ui(void* arg) {
    int safeX = int(maxX - 4)/5;

    while(true) {
        imprimir_fila_de_reproducao();
        imprimir_barra_de_progresso();
        
        pthread_cond_wait(&cond, &mutexx);
    }
}

int main(int argc, char *argv[]) {
    cin.tie(nullptr);

    pthread_t threads[3];

    initscr();
    getmaxyx(stdscr, maxY, maxX);

    win1 = newwin(maxY - 6, maxX, 0, 0);
    win2 = newwin(3, maxX, maxY - 6, 0);
    win3 = newwin(3, maxX, maxY - 3, 0);
    refresh();

    box(win1, ' ', '*');
    box(win2, ' ', '*');
    box(win3, ' ', '*');
    wrefresh(win1);
    wrefresh(win2);
    wrefresh(win3);
      
    pthread_create(&threads[0], NULL, &ui, NULL);
    pthread_create(&threads[1], NULL, &play, NULL);
    pthread_create(&threads[2], NULL, &receber_input, NULL);

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    pthread_join(threads[2], NULL);

    endwin();

    return 0;
}
