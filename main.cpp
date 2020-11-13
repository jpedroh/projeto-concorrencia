#include <bits/stdc++.h>
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>

#define endl '\n'

using namespace std;

struct Musica {
    string nome;
    string artista;
    int duracao_em_segundos;
};

// Recursos globais
WINDOW *win1, *win2, *win3;
int MAX_X, MAX_Y;
int DURACAO_ATUAL = 0;
int IS_PLAYING = 0;
vector <Musica> FILA;
Musica MUSICA_ATUAL;
int IS_RANDOM = 0;
int INDICE_ATUAL = 0;

unordered_set<string> JA_TOCADAS;

// Temos mutexes para regiões críticas do programa que serão acessadas por diversas partes da aplicação: A fila de reprodução, o estado atual: PLAY/PAUSE e a o quanto da da música atual já foi tocado.
pthread_mutex_t MUTEX_FILA = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MUTEX_IS_PLAYING = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MUTEX_DURACAO_ATUAL = PTHREAD_MUTEX_INITIALIZER;

// Esta condição é utilizada para que o worked da UI somente trabalhe quando ocorrer mudanças relevantes para o usuário
pthread_cond_t UI_SIGNAL = PTHREAD_COND_INITIALIZER;

void adicionar_musica(Musica musicaLeitura) {
    // Travamos a fila para evitar acessos por outras threads
    while (pthread_mutex_trylock(&MUTEX_FILA) == 0);
    FILA.emplace_back(musicaLeitura);
    // A thread de UI é informada de que hove uma mudanca na fila de reprodução, de modo que ela precisa ser re-renderizada
    pthread_cond_signal(&UI_SIGNAL);
    pthread_mutex_unlock(&MUTEX_FILA);
}

bool musica_ja_tocada(Musica musica) {
    return JA_TOCADAS.find(musica.nome) != JA_TOCADAS.end();
}

pair<Musica, int> escolher_musica_aleatoriamente() {
    int indice = rand() % FILA.size();
    auto musica_escolhida = FILA.at(indice);

    if(JA_TOCADAS.size() == FILA.size()) {
        JA_TOCADAS.clear();
    }

    while(musica_ja_tocada(musica_escolhida)) {
        indice = rand() % FILA.size();
        musica_escolhida = FILA.at(indice);
    }

    return pair<Musica, int>(musica_escolhida, indice);
}

void pular_musica() {
    // Travamos a fila para evitar acessos por outras threads
    while (pthread_mutex_trylock(&MUTEX_FILA) == 0);
    while (pthread_mutex_trylock(&MUTEX_DURACAO_ATUAL) == 0);
    if (!FILA.empty()) {
        if(IS_RANDOM) {
            auto escolha_aleatoria = escolher_musica_aleatoriamente();

            MUSICA_ATUAL = escolha_aleatoria.first;
            INDICE_ATUAL = escolha_aleatoria.second;
        } else {
            INDICE_ATUAL = (INDICE_ATUAL + 1) % FILA.size();
            MUSICA_ATUAL = FILA.at(INDICE_ATUAL);
        }

        DURACAO_ATUAL = 0;
        JA_TOCADAS.insert(MUSICA_ATUAL.nome);
    }

    // A thread de UI é informada de que hove uma mudanca na fila de reprodução, de modo que ela precisa ser re-renderizada
    pthread_cond_signal(&UI_SIGNAL);
    pthread_mutex_unlock(&MUTEX_FILA);
    pthread_mutex_unlock(&MUTEX_DURACAO_ATUAL);
}

void remover_musica(int indice) {
    // Travamos a fila para evitar acessos por outras threads
    while (pthread_mutex_trylock(&MUTEX_FILA) == 0);
    if(indice >= 0 && indice < FILA.size()) {
        auto musicaRemovida = FILA.at(indice);
        FILA.erase(FILA.begin() + indice);

        JA_TOCADAS.erase(musicaRemovida.nome);
        if(MUSICA_ATUAL.nome == musicaRemovida.nome) {
            pular_musica();
        }
    }
    // A thread de UI é informada de que hove uma mudanca na fila de reprodução, de modo que ela precisa ser re-renderizada
    pthread_cond_signal(&UI_SIGNAL);
    pthread_mutex_unlock(&MUTEX_FILA);
}

void trocar_play_pause() {
    // Travamos o recurso IS_PLAYING para evitar acessos por outras threads
    while (pthread_mutex_trylock(&MUTEX_IS_PLAYING) == 0);
    IS_PLAYING = !IS_PLAYING;
    // Liberamos o recurso IS_PLAYING para uso por outas threads
    pthread_mutex_unlock(&MUTEX_IS_PLAYING);
    // Notemos que neste caso não existe uma atualização da UI. Desta forma, a chamada ao sinal da UI é desnecessário
}

void *receber_input(void *args) {
    while (true) {
        int safeX = int(MAX_X - 4) / 6;

        wmove(win3, 1, 0);
        wclrtoeol(win3);
        wattron(win3, COLOR_PAIR(1));
        mvwprintw(win3, 1, 2, "A for ADD");
        mvwprintw(win3, 1, safeX, "R for REMOVE");
        mvwprintw(win3, 1, 2 * safeX, string("P for ").append(IS_PLAYING ? "PAUSE" : "PLAY").data());
        mvwprintw(win3, 1, 3 * safeX, "N for NEXT");
        mvwprintw(win3, 1, 4 * safeX, string("S for ").append(IS_RANDOM ? "SEQUENTIAL" : "SHUFFLE").data());
        mvwprintw(win3, 1, 5 * safeX, "Q for QUIT");
        wattroff(win3, COLOR_PAIR(1));
        wrefresh(win3);
        char option = wgetch(win3);

        if (option == 'Q' || option == 'q') {
            endwin();
            exit(0);
        } else if (option == 'A' || option == 'a') {
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

            adicionar_musica(musicaLeitura);
        } else if (option == 'R' || option == 'r') {
            int indice;

            wmove(win3, 1, 0);
            wclrtoeol(win3);
            mvwprintw(win3, 1, 2, "Digite o índice da musica a ser removida: ");
            wrefresh(win3);
            wscanw(win3, "%d", &indice);

            remover_musica(indice);
        } else if (option == 'P' || option == 'p') {
            trocar_play_pause();
        } else if (option == 'N' || option == 'n') {
            pular_musica();
        } else if (option == 'S' || option == 's') {
            IS_RANDOM = !IS_RANDOM;
        }
    }
}

void imprimir_fila_de_reproducao() {
    wclear(win1);

    wattron(win1, COLOR_PAIR(3));
    box(win1, ' ', '*');
    wattroff(win1, COLOR_PAIR(3));

    int safeX = int(MAX_X - 8) / 6;

    mvwprintw(win1, 1, 2, "#");
    mvwprintw(win1, 1, 6, "Musica");
    mvwprintw(win1, 1, 3 * safeX, "Artista");
    mvwprintw(win1, 1, 5 * safeX, "Duracao");

    if (FILA.empty()) {
        mvwprintw(win1, (MAX_Y - 6) / 2, (MAX_X / 2) - 11, "Nenhuma música na fila");
    }

    for (int i = 0; i < FILA.size(); i++) {
        Musica musica = FILA.at(i);

        if (musica.nome == MUSICA_ATUAL.nome) {
            wattron(win1, COLOR_PAIR(2));
        }

        mvwprintw(win1, i + 2, 2, "%d", i);
        mvwprintw(win1, i + 2, 6, musica.nome.data());
        mvwprintw(win1, i + 2, 3 * safeX, musica.artista.data());

        int minutos = musica.duracao_em_segundos / 60;
        int segundos = musica.duracao_em_segundos % 60;
        mvwprintw(win1, i + 2, 5 * safeX, "%02d:%02d", minutos, segundos);

        wattroff(win1, COLOR_PAIR(2));
    }

    wrefresh(win1);
}

void resetar_barra_de_progresso() {
    wmove(win2, 1, 0);
    wclrtoeol(win2);

    wattron(win2, COLOR_PAIR(2));
    mvwhline(win2, 1, 8, '-', MAX_X - 17);
    wattroff(win2, COLOR_PAIR(2));
}

void imprimir_barra_de_progresso() {
    if (DURACAO_ATUAL == 0) {
        resetar_barra_de_progresso();
    }

    int minutos = DURACAO_ATUAL / 60;
    int segundos = DURACAO_ATUAL % 60;

    int duracaoTotal = FILA.empty() ? INT_MAX : MUSICA_ATUAL.duracao_em_segundos;

    int minutosTotais = FILA.empty() ? 0 : MUSICA_ATUAL.duracao_em_segundos / 60;
    int segundosTotais = FILA.empty() ? 0 : MUSICA_ATUAL.duracao_em_segundos % 60;

    mvwprintw(win2, 1, 2, "%02d:%02d", minutos, segundos);
    mvwprintw(win2, 1, MAX_X - 8, "%02d:%02d", minutosTotais, segundosTotais);
    wattron(win2, COLOR_PAIR(2));
    mvwhline(win2, 1, 8, ACS_CKBOARD, int(((float) DURACAO_ATUAL / duracaoTotal) * (MAX_X - 17)));
    wattroff(win2, COLOR_PAIR(2));
    wrefresh(win2);
}

void *ui(void *arg) {
    while (true) {
        // A thread deve aguardar pela sinalização de algum evento relevante para a UI
        pthread_cond_wait(&UI_SIGNAL, &MUTEX_FILA);

        imprimir_fila_de_reproducao();
        imprimir_barra_de_progresso();
    }
}

void *play(void *arg) {
    while (true) {
        // Travamos a duaração atual pois ela será alterada.
        while (pthread_mutex_trylock(&MUTEX_DURACAO_ATUAL) == 0);
        if (IS_PLAYING && !FILA.empty() && DURACAO_ATUAL < MUSICA_ATUAL.duracao_em_segundos) {
            DURACAO_ATUAL++;
        } else if (!FILA.empty() && DURACAO_ATUAL == MUSICA_ATUAL.duracao_em_segundos) {
            pular_musica();
        } else if (FILA.empty()) {
            DURACAO_ATUAL = 0;
            IS_PLAYING = 0;
        }
        sleep(1);
        // A thread de UI é informada de que um tique de relógio passou e que a barra de progresso deverá ser atualizada
        pthread_cond_signal(&UI_SIGNAL);
        pthread_mutex_unlock(&MUTEX_DURACAO_ATUAL);
    }
}

int main(int argc, char *argv[]) {
    cin.tie(nullptr);

    pthread_t threads[3];

    initscr();
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);

    getmaxyx(stdscr, MAX_Y, MAX_X);

    win1 = newwin(MAX_Y - 6, MAX_X, 0, 0);
    win2 = newwin(3, MAX_X, MAX_Y - 6, 0);
    win3 = newwin(3, MAX_X, MAX_Y - 3, 0);
    refresh();

    wattron(win1, COLOR_PAIR(3));
    wattron(win2, COLOR_PAIR(3));
    wattron(win3, COLOR_PAIR(3));
    box(win1, ' ', '*');
    box(win2, ' ', '*');
    box(win3, ' ', '*');
    wrefresh(win1);
    wrefresh(win2);
    wrefresh(win3);
    wattroff(win1, COLOR_PAIR(3));

    pthread_create(&threads[0], NULL, &ui, NULL);
    pthread_create(&threads[1], NULL, &play, NULL);
    pthread_create(&threads[2], NULL, &receber_input, NULL);

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    pthread_join(threads[2], NULL);

    endwin();

    return 0;
}
