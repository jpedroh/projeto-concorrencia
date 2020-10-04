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

vector<Musica> fila;
pthread_mutex_t mutexx = PTHREAD_MUTEX_INITIALIZER;

void* ui(void* arg) {
    while(true) {
        while(pthread_mutex_trylock(&mutexx));

        cout << "# \t Artista \t Musica \t Duracao" << endl;
        if(fila.empty()) {
            cout << "A fila de execucao esta vazia :(" << endl;
        } else {
            int i = 0;
            for(auto musica : fila) {
                int minutos = musica.duracao_em_segundos / 60;
                int segundos = musica.duracao_em_segundos % 60;

                cout << i << '\t' << musica.artista << '\t' << musica.nome << '\t' << minutos << ':' << segundos <<  endl;
                i++;
            }
        }
        pthread_mutex_unlock(&mutexx);
        sleep(1);
    }
}

void adicionar_musica() {
    int minutos;
    int segundos;
    Musica musicaLeitura;

    cout << "Digite o nome da musica" << endl;
    cin.ignore();
    getline(cin, musicaLeitura.nome);

    cout << "Digite o artista" << endl;
    getline(cin, musicaLeitura.artista);

    cout << "Digite a duraçao de MINUTOS da musica e depois SEGUNDOS" << endl;
    cin >> minutos >> segundos;
    musicaLeitura.duracao_em_segundos = (minutos*60)+segundos;

    fila.emplace_back(musicaLeitura);
}

void remover_musica() {
    int indice;
    cout << "Qual o indice da musica a ser removida?" << endl;
    cin >> indice;
    fila.erase(fila.begin() + indice);
}

void* receberInput(void* arg) {
    while(true) {
        while(pthread_mutex_trylock(&mutexx));

            cout << "Escolha uma opção" << endl;
            cout << "A de Add \t R de Remove \t Q de Quit" << endl;
            char selecao;
            cin >> selecao;

            switch (selecao) {
                case 'a':
                case 'A':
                    adicionar_musica();
                    break;
                case 'R':
                case 'r':
                    remover_musica();
                    break;
                case 'q':
                case 'Q':
                    exit(0);
                    break;
                default:
                    cout << "Opcao invalida :(" << endl;
            }

        pthread_mutex_unlock(&mutexx);
    }
}

int main(int argc, char *argv[]) {
    cin.tie(nullptr);

    pthread_t threads[2];

    pthread_create(&threads[0], NULL, &ui, NULL);
    pthread_create(&threads[1], NULL, &receberInput, NULL);

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    return 0;
}