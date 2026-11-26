#include "bfs.h"
#include <stdlib.h>

typedef struct NoFila {
    int id;
    struct NoFila *prox;
} NoFila;

typedef struct FilaFIFO {
    NoFila *inicio;
    NoFila *fim;
} FilaFIFO;

static FilaFIFO* fila_criar(void) {
    FilaFIFO *f = (FilaFIFO *)malloc(sizeof(FilaFIFO));
    if (!f) {
        return NULL;
    }
    f->inicio = NULL;
    f->fim = NULL;
    return f;
}

static void fila_liberar(FilaFIFO *f) {
    if (!f) {
        return;
    }
    NoFila *atual = f->inicio;
    while (atual) {
        NoFila *prox = atual->prox;
        free(atual);
        atual = prox;
    }
    free(f);
}

static int fila_enfileirar(FilaFIFO *f, int id) {
    NoFila *novo = (NoFila *)malloc(sizeof(NoFila));
    if (!novo) {
        return 0;
    }
    novo->id = id;
    novo->prox = NULL;

    if (!f->fim) {
        f->inicio = novo;
        f->fim = novo;
    } else {
        f->fim->prox = novo;
        f->fim = novo;
    }
    return 1;
}

static int fila_desenfileirar(FilaFIFO *f, int *id_out) {
    if (!f || !f->inicio) {
        return 0;
    }
    NoFila *remover = f->inicio;
    *id_out = remover->id;
    f->inicio = remover->prox;
    if (!f->inicio) {
        f->fim = NULL;
    }
    free(remover);
    return 1;
}

static ResultadoRota* criar_rota_vazia(void) {
    ResultadoRota *res = (ResultadoRota *)malloc(sizeof(ResultadoRota));
    if (!res) {
        return NULL;
    }
    res->caminho = NULL;
    res->tamanho = 0;
    res->encontrada = 0;
    return res;
}

ResultadoRota* bfs_menor_caminho(const Grafo *g, int id_origem, int id_destino) {
    if (!g || id_origem < 0 || id_origem >= g->num_vertices ||
        id_destino < 0 || id_destino >= g->num_vertices) {
        return NULL;
    }

    if (id_origem == id_destino) {
        ResultadoRota *res = criar_rota_vazia();
        if (!res) return NULL;
        res->caminho = (int *)malloc(sizeof(int));
        if (!res->caminho) {
            free(res);
            return NULL;
        }
        res->caminho[0] = id_origem;
        res->tamanho = 1;
        res->encontrada = 1;
        return res;
    }

    int n = g->num_vertices;
    int *visitado = (int *)calloc((size_t)n, sizeof(int));
    int *pai = (int *)malloc((size_t)n * sizeof(int));
    FilaFIFO *fila = fila_criar();

    if (!visitado || !pai || !fila) {
        free(visitado);
        free(pai);
        fila_liberar(fila);
        return NULL;
    }

    for (int i = 0; i < n; i++) {
        pai[i] = -1;
    }

    visitado[id_origem] = 1;
    if (!fila_enfileirar(fila, id_origem)) {
        free(visitado);
        free(pai);
        fila_liberar(fila);
        return NULL;
    }

    int achou = 0;
    int u = -1;
    while (!achou && fila_desenfileirar(fila, &u)) {
        NoAresta *atual = g->vertices[u].lista_adj;
        while (atual) {
            int v = atual->id_destino;
            if (!visitado[v]) {
                visitado[v] = 1;
                pai[v] = u;
                if (v == id_destino) {
                    achou = 1;
                    break;
                }
                if (!fila_enfileirar(fila, v)) {
                    free(visitado);
                    free(pai);
                    fila_liberar(fila);
                    return NULL;
                }
            }
            atual = atual->prox;
        }
    }

    fila_liberar(fila);
    free(visitado);

    ResultadoRota *res = criar_rota_vazia();
    if (!res) {
        free(pai);
        return NULL;
    }

    if (achou) {
        int tam = 0;
        for (int curr = id_destino; curr != -1; curr = pai[curr]) {
            tam++;
        }

        res->caminho = (int *)malloc((size_t)tam * sizeof(int));
        if (!res->caminho) {
            free(pai);
            free(res);
            return NULL;
        }

        int idx = tam - 1;
        for (int curr = id_destino; curr != -1; curr = pai[curr]) {
            res->caminho[idx--] = curr;
        }
        res->tamanho = tam;
        res->encontrada = 1;
    }

    free(pai);
    return res;
}

void bfs_liberar_rota(ResultadoRota *res) {
    if (!res) {
        return;
    }
    free(res->caminho);
    free(res);
}
