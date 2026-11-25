#include "reverso.h"
#include <stdlib.h>

Grafo* grafo_criar_reverso(const Grafo *original) {
    if (!original) {
        return NULL;
    }

    Grafo *rev = grafo_criar(original->num_vertices);
    if (!rev) {
        return NULL;
    }

    for (int i = 0; i < original->num_vertices; i++) {
        rev->vertices[i] = original->vertices[i];
        rev->vertices[i].lista_adj = NULL;
    }

    for (int u = 0; u < original->num_vertices; u++) {
        NoAresta *atual = original->vertices[u].lista_adj;
        while (atual) {
            if (!grafo_adicionar_aresta(rev, atual->id_destino, u)) {
                grafo_liberar(rev);
                return NULL;
            }
            atual = atual->prox;
        }
    }

    return rev;
}

ResultadoImpacto* reverso_analisar_impacto(const Grafo *reverso, int id_alvo) {
    if (!reverso || id_alvo < 0 || id_alvo >= reverso->num_vertices) {
        return NULL;
    }

    int n = reverso->num_vertices;
    int *visitado = (int *)calloc((size_t)n, sizeof(int));
    int *fila = (int *)malloc((size_t)n * sizeof(int));
    int *afetados = (int *)malloc((size_t)n * sizeof(int));
    ResultadoImpacto *res = (ResultadoImpacto *)malloc(sizeof(ResultadoImpacto));

    if (!visitado || !fila || !afetados || !res) {
        free(visitado);
        free(fila);
        free(afetados);
        free(res);
        return NULL;
    }

    int ini = 0;
    int fim = 0;
    int qtd = 0;

    visitado[id_alvo] = 1;
    fila[fim++] = id_alvo;

    while (ini < fim) {
        int u = fila[ini++];
        NoAresta *atual = reverso->vertices[u].lista_adj;
        while (atual) {
            int v = atual->id_destino;
            if (!visitado[v]) {
                visitado[v] = 1;
                fila[fim++] = v;
                afetados[qtd++] = v;
            }
            atual = atual->prox;
        }
    }

    free(visitado);
    free(fila);

    if (qtd == 0) {
        free(afetados);
        res->ids_afetados = NULL;
    } else {
        int *ajustado = (int *)realloc(afetados, (size_t)qtd * sizeof(int));
        res->ids_afetados = ajustado ? ajustado : afetados;
    }
    res->quantidade = qtd;

    return res;
}

void reverso_liberar_impacto(ResultadoImpacto *res) {
    if (!res) {
        return;
    }
    free(res->ids_afetados);
    free(res);
}
