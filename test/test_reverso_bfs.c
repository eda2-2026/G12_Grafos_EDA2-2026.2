#include "../back/bfs.h"
#include "../back/grafo.h"
#include "../back/reverso.h"
#include <assert.h>
#include <stdio.h>

static int contem_id(const int *vetor, int n, int alvo) {
    for (int i = 0; i < n; i++) {
        if (vetor[i] == alvo) {
            return 1;
        }
    }
    return 0;
}

static int existe_aresta(const Grafo *g, int u, int v) {
    NoAresta *atual = g->vertices[u].lista_adj;
    while (atual) {
        if (atual->id_destino == v) {
            return 1;
        }
        atual = atual->prox;
    }
    return 0;
}

static void testar_reverso_manual(void) {
    /* 0 -> 1 -> 2, 3 -> 1, 4 isolado */
    Grafo *g = grafo_criar(5);
    assert(g != NULL);
    grafo_adicionar_aresta(g, 0, 1);
    grafo_adicionar_aresta(g, 1, 2);
    grafo_adicionar_aresta(g, 3, 1);

    Grafo *rev = grafo_criar_reverso(g);
    assert(rev != NULL);
    assert(rev->num_vertices == 5);
    assert(rev->num_arestas == 3);
    assert(existe_aresta(rev, 1, 0));
    assert(existe_aresta(rev, 2, 1));
    assert(existe_aresta(rev, 1, 3));

    /* Remover 2 quebra 1, 0 e 3; 4 permanece intacto */
    ResultadoImpacto *imp = reverso_analisar_impacto(rev, 2);
    assert(imp != NULL);
    assert(imp->quantidade == 3);
    assert(contem_id(imp->ids_afetados, imp->quantidade, 0));
    assert(contem_id(imp->ids_afetados, imp->quantidade, 1));
    assert(contem_id(imp->ids_afetados, imp->quantidade, 3));
    assert(!contem_id(imp->ids_afetados, imp->quantidade, 4));
    reverso_liberar_impacto(imp);

    /* Remover 0 nao quebra ninguem */
    ResultadoImpacto *imp_folha = reverso_analisar_impacto(rev, 0);
    assert(imp_folha != NULL);
    assert(imp_folha->quantidade == 0);
    assert(imp_folha->ids_afetados == NULL);
    reverso_liberar_impacto(imp_folha);

    grafo_liberar(rev);
    grafo_liberar(g);
    printf("[OK] Teste manual de Grafo Reverso e Impacto\n");
}

static void testar_bfs_manual(void) {
    /* 0 -> 1 -> 2 -> 3 e atalho 0 -> 3; 4 isolado */
    Grafo *g = grafo_criar(5);
    assert(g != NULL);
    grafo_adicionar_aresta(g, 0, 1);
    grafo_adicionar_aresta(g, 1, 2);
    grafo_adicionar_aresta(g, 2, 3);
    grafo_adicionar_aresta(g, 0, 3);

    ResultadoRota *curta = bfs_menor_caminho(g, 0, 3);
    assert(curta != NULL);
    assert(curta->encontrada == 1);
    assert(curta->tamanho == 2);
    assert(curta->caminho[0] == 0 && curta->caminho[1] == 3);
    bfs_liberar_rota(curta);

    ResultadoRota *mesmo = bfs_menor_caminho(g, 2, 2);
    assert(mesmo != NULL);
    assert(mesmo->encontrada == 1);
    assert(mesmo->tamanho == 1);
    assert(mesmo->caminho[0] == 2);
    bfs_liberar_rota(mesmo);

    ResultadoRota *sem_rota = bfs_menor_caminho(g, 0, 4);
    assert(sem_rota != NULL);
    assert(sem_rota->encontrada == 0);
    assert(sem_rota->tamanho == 0);
    bfs_liberar_rota(sem_rota);

    grafo_liberar(g);
    printf("[OK] Teste manual de BFS (menor caminho)\n");
}

static void testar_csv_real(void) {
    Grafo *g = carregar_grafo_csv("dados/pacotes.csv", "dados/dependencias.csv");
    assert(g != NULL);

    Grafo *rev = grafo_criar_reverso(g);
    assert(rev != NULL);
    assert(rev->num_vertices == g->num_vertices);
    assert(rev->num_arestas == g->num_arestas);

    int id_libc = grafo_buscar_id_por_nome(g, "libc6");
    assert(id_libc >= 0);

    ResultadoImpacto *imp = reverso_analisar_impacto(rev, id_libc);
    assert(imp != NULL);
    assert(imp->quantidade > 1000);

    int id_acc = grafo_buscar_id_por_nome(g, "accountsservice");
    assert(id_acc >= 0);

    ResultadoRota *rota = bfs_menor_caminho(g, id_acc, id_libc);
    assert(rota != NULL);
    assert(rota->encontrada == 1);
    assert(rota->tamanho == 2);
    assert(rota->caminho[0] == id_acc);
    assert(rota->caminho[rota->tamanho - 1] == id_libc);

    for (int i = 0; i < rota->tamanho - 1; i++) {
        assert(existe_aresta(g, rota->caminho[i], rota->caminho[i + 1]));
    }

    printf("[OK] Teste real CSV: impacto libc6 = %d pacotes | rota accountsservice -> libc6 = %d nos\n",
           imp->quantidade, rota->tamanho);

    bfs_liberar_rota(rota);
    reverso_liberar_impacto(imp);
    grafo_liberar(rev);
    grafo_liberar(g);
}

int main(void) {
    testar_reverso_manual();
    testar_bfs_manual();
    testar_csv_real();
    return 0;
}
