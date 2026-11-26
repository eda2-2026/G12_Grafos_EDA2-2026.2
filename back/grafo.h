#ifndef GRAFO_H
#define GRAFO_H

#define TAM_NOME 64
#define TAM_VERSAO 64
#define TAM_DESC 256

typedef struct NoAresta {
    int id_destino;
    struct NoAresta *prox;
} NoAresta;

typedef struct Vertice {
    int id;
    char nome[TAM_NOME];
    char versao[TAM_VERSAO];
    long tamanho_kb;
    char descricao[TAM_DESC];
    NoAresta *lista_adj;
} Vertice;

typedef struct Grafo {
    int num_vertices;
    int num_arestas;
    Vertice *vertices;
} Grafo;

Grafo* grafo_criar(int num_vertices);
void grafo_liberar(Grafo *g);
int grafo_adicionar_aresta(Grafo *g, int origem, int destino);
int grafo_buscar_id_por_nome(const Grafo *g, const char *nome);
Grafo* carregar_grafo_csv(const char *caminho_pacotes, const char *caminho_dependencias);

#endif
