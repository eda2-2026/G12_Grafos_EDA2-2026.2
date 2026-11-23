#include "grafo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void copiar_campo(char *dest, const char *src, size_t max_len) {
    if (!src) {
        dest[0] = '\0';
        return;
    }
    strncpy(dest, src, max_len - 1);
    dest[max_len - 1] = '\0';
}

static int contar_capacidade_vertices(FILE *fp) {
    char linha[512];
    int max_id = -1;

    if (!fgets(linha, sizeof(linha), fp)) {
        return 0;
    }

    while (fgets(linha, sizeof(linha), fp)) {
        if (linha[0] == '\n' || linha[0] == '\r') {
            continue;
        }
        int id = atoi(linha);
        if (id > max_id) {
            max_id = id;
        }
    }
    return max_id + 1;
}

Grafo* grafo_criar(int num_vertices) {
    if (num_vertices < 0) {
        return NULL;
    }

    Grafo *g = (Grafo *)malloc(sizeof(Grafo));
    if (!g) {
        return NULL;
    }

    g->num_vertices = num_vertices;
    g->num_arestas = 0;
    g->vertices = NULL;

    if (num_vertices > 0) {
        g->vertices = (Vertice *)calloc((size_t)num_vertices, sizeof(Vertice));
        if (!g->vertices) {
            free(g);
            return NULL;
        }
        for (int i = 0; i < num_vertices; i++) {
            g->vertices[i].id = -1;
        }
    }

    return g;
}

void grafo_liberar(Grafo *g) {
    if (!g) {
        return;
    }

    for (int i = 0; i < g->num_vertices; i++) {
        NoAresta *atual = g->vertices[i].lista_adj;
        while (atual) {
            NoAresta *prox = atual->prox;
            free(atual);
            atual = prox;
        }
    }

    free(g->vertices);
    free(g);
}

int grafo_adicionar_aresta(Grafo *g, int origem, int destino) {
    if (!g || origem < 0 || origem >= g->num_vertices || destino < 0 || destino >= g->num_vertices) {
        return 0;
    }

    NoAresta *nova = (NoAresta *)malloc(sizeof(NoAresta));
    if (!nova) {
        return 0;
    }

    // Insercao no inicio da lista garante O(1) por aresta.
    nova->id_destino = destino;
    nova->prox = g->vertices[origem].lista_adj;
    g->vertices[origem].lista_adj = nova;
    g->num_arestas++;
    return 1;
}

int grafo_buscar_id_por_nome(const Grafo *g, const char *nome) {
    if (!g || !nome) {
        return -1;
    }

    for (int i = 0; i < g->num_vertices; i++) {
        if (g->vertices[i].id != -1 && strcmp(g->vertices[i].nome, nome) == 0) {
            return g->vertices[i].id;
        }
    }
    return -1;
}

static int parse_linha_pacote(char *linha, Vertice *v) {
    size_t len = strlen(linha);
    while (len > 0 && (linha[len - 1] == '\n' || linha[len - 1] == '\r')) {
        linha[--len] = '\0';
    }

    char *p1 = strchr(linha, ';');
    if (!p1) return 0;
    *p1 = '\0';

    char *p2 = strchr(p1 + 1, ';');
    if (!p2) return 0;
    *p2 = '\0';

    char *p3 = strchr(p2 + 1, ';');
    if (!p3) return 0;
    *p3 = '\0';

    char *p4 = strchr(p3 + 1, ';');
    if (!p4) return 0;
    *p4 = '\0';

    v->id = atoi(linha);
    copiar_campo(v->nome, p1 + 1, TAM_NOME);
    copiar_campo(v->versao, p2 + 1, TAM_VERSAO);
    v->tamanho_kb = atol(p3 + 1);
    copiar_campo(v->descricao, p4 + 1, TAM_DESC);
    v->lista_adj = NULL;
    return 1;
}

Grafo* carregar_grafo_csv(const char *caminho_pacotes, const char *caminho_dependencias) {
    if (!caminho_pacotes || !caminho_dependencias) {
        return NULL;
    }

    FILE *fp_pac = fopen(caminho_pacotes, "r");
    if (!fp_pac) {
        return NULL;
    }

    // Primeira passagem define o tamanho exato para alocar o vetor de vertices de uma vez
    int num_vertices = contar_capacidade_vertices(fp_pac);
    if (num_vertices <= 0) {
        fclose(fp_pac);
        return NULL;
    }

    Grafo *g = grafo_criar(num_vertices);
    if (!g) {
        fclose(fp_pac);
        return NULL;
    }

    rewind(fp_pac);
    char linha[512];
    if (!fgets(linha, sizeof(linha), fp_pac)) {
        fclose(fp_pac);
        grafo_liberar(g);
        return NULL;
    }

    while (fgets(linha, sizeof(linha), fp_pac)) {
        if (linha[0] == '\n' || linha[0] == '\r') {
            continue;
        }
        Vertice temp;
        if (parse_linha_pacote(linha, &temp) && temp.id >= 0 && temp.id < g->num_vertices) {
            g->vertices[temp.id] = temp;
        }
    }
    fclose(fp_pac);

    FILE *fp_dep = fopen(caminho_dependencias, "r");
    if (!fp_dep) {
        grafo_liberar(g);
        return NULL;
    }

    if (fgets(linha, sizeof(linha), fp_dep)) {
        while (fgets(linha, sizeof(linha), fp_dep)) {
            if (linha[0] == '\n' || linha[0] == '\r') {
                continue;
            }
            int origem = -1;
            int destino = -1;
            if (sscanf(linha, "%d;%d", &origem, &destino) == 2) {
                grafo_adicionar_aresta(g, origem, destino);
            }
        }
    }
    fclose(fp_dep);

    return g;
}
