#ifndef REVERSO_H
#define REVERSO_H

#include "grafo.h"

typedef struct ResultadoImpacto {
    int *ids_afetados;
    int quantidade;
} ResultadoImpacto;

Grafo* grafo_criar_reverso(const Grafo *original);
ResultadoImpacto* reverso_analisar_impacto(const Grafo *reverso, int id_alvo);
void reverso_liberar_impacto(ResultadoImpacto *res);

#endif
