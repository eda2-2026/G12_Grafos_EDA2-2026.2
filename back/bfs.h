#ifndef BFS_H
#define BFS_H

#include "grafo.h"

typedef struct ResultadoRota {
    int *caminho;
    int tamanho;
    int encontrada;
} ResultadoRota;

ResultadoRota* bfs_menor_caminho(const Grafo *g, int id_origem, int id_destino);
void bfs_liberar_rota(ResultadoRota *res);

#endif
