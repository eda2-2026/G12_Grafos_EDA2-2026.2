#include "../back/grafo.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static int contar_adjacentes(const Grafo *g, int id) {
    int cont = 0;
    NoAresta *atual = g->vertices[id].lista_adj;
    while (atual) {
        cont++;
        atual = atual->prox;
    }
    return cont;
}

static void testar_grafo_basico(void) {
    Grafo *g = grafo_criar(3);
    assert(g != NULL);
    assert(g->num_vertices == 3);
    assert(g->num_arestas == 0);

    assert(grafo_adicionar_aresta(g, 0, 1) == 1);
    assert(grafo_adicionar_aresta(g, 0, 2) == 1);
    assert(grafo_adicionar_aresta(g, 5, 1) == 0);
    assert(g->num_arestas == 2);
    assert(contar_adjacentes(g, 0) == 2);

    grafo_liberar(g);
    printf("[OK] Teste basico de grafo\n");
}

static void testar_carga_csv(void) {
    clock_t inicio = clock();
    Grafo *g = carregar_grafo_csv("dados/pacotes.csv", "dados/dependencias.csv");
    clock_t fim = clock();

    assert(g != NULL);
    assert(g->num_vertices == 6099);
    assert(g->num_arestas == 21623);

    int id_acc = grafo_buscar_id_por_nome(g, "accountsservice");
    assert(id_acc == 0);
    assert(strcmp(g->vertices[id_acc].nome, "accountsservice") == 0);
    assert(g->vertices[id_acc].tamanho_kb == 524);
    assert(contar_adjacentes(g, id_acc) == 4);

    int id_libc = grafo_buscar_id_por_nome(g, "libc6");
    assert(id_libc == 1869);
    assert(grafo_buscar_id_por_nome(g, "pacote-inexistente-xyz") == -1);

    double tempo_ms = ((double)(fim - inicio) / CLOCKS_PER_SEC) * 1000.0;
    printf("[OK] Carga CSV: %d vertices, %d arestas em %.2f ms\n",
           g->num_vertices, g->num_arestas, tempo_ms);

    grafo_liberar(g);
}

int main(void) {
    testar_grafo_basico();
    testar_carga_csv();
    return 0;
}
