# Analisador de Dependências de Pacotes (Ubuntu / APT)

Trabalho prático 3 para a disciplina de Estruturas de Dados 2.

## Apresentação
* **Assista ao Vídeo:** [Apresentação da dupla](link_do_video_aqui) *(a ser gravado)*

## Equipe
*   [Gabriel Diniz](https://github.com/GabrielDiniz12) - 241025630
*   [Pedro Americo](https://github.com/dev-americo) - 241025980

## Descrição do Problema
O objetivo deste projeto é modelar e analisar o grafo real de dependências de pacotes do **Ubuntu 24.04 (Noble)** a partir dos metadados oficiais (`Packages.xz`), permitindo simular operações críticas de um gerenciador de pacotes (`apt`).

O sistema atende a três desafios fundamentais:
1. **Ordem de Instalação e Detecção de Ciclos:** Determinar a sequência segura para instalar um pacote e suas dependências sem violar pré-requisitos, além de detectar dependências circulares (*deadlocks*).
2. **Análise de Impacto de Remoção:** Simular a desinstalação de um pacote e identificar todos os softwares dependentes que seriam quebrados em cascata.
3. **Menor Caminho de Dependência:** Encontrar a menor cadeia de dependências entre dois pacotes quaisquer do repositório.

## Arquitetura da Solução
O sistema adota arquitetura desacoplada em três camadas:
1. **Extração de Dados (Python):** Script responsável pelo download automático de `Packages.xz`, tratamento de `Depends:`/`Pre-Depends:`, resolução de alternativas (`|`), deduplicação de arestas e exportação para `pacotes.csv` e `dependencias.csv`.
2. **Backend (C):** Carrega o grafo em memória em tempo $O(V + E)$ usando Lista de Adjacência e executa os algoritmos de grafos, operando como processo contínuo via pipes de comunicação IPC (`stdin`/`stdout`).
3. **Frontend (Python / CustomTkinter):** Interface gráfica em tema escuro com canvas interativo para visualização dos nós do grafo e destaque visual dos resultados (verde para ordem de instalação, vermelho para impacto de remoção e azul para menor caminho BFS).

## Estruturas de Dados e Algoritmos (C)
* **Grafo Direcionado (Lista de Adjacência):**
  * Estruturas `NoAresta`, `Vertice` e `Grafo` definidas em `back/grafo.h`.
  * Carga direta de vértices e arestas por IDs a partir dos CSVs em tempo $O(V + E)$.
* **Busca em Profundidade (DFS), Ciclos e Ordenação Topológica (`back/dfs.c`):**
  * Coloração em 3 estados (**Branco**, **Cinza**, **Preto**) para detecção de ciclos via arestas de retorno e reconstrução do caminho de deadlock.
  * Ordenação Topológica via pilha de pós-ordem focada no subgrafo de dependências do pacote alvo.
* **Grafo Reverso e Análise de Impacto (`back/reverso.c`):**
  * Transposição de todas as arestas direcionadas em $O(V + E)$ para rastrear dependentes reversos ao simular a remoção de um pacote.
* **Busca em Largura (`back/bfs.c`):**
  * Uso de fila FIFO dinâmica para encontrar o menor caminho de dependência entre um pacote de origem e um pacote de destino.

---

## Como Executar o Projeto

> [!NOTE]
> A estrutura abaixo já reflete os módulos planejados para o backend e frontend; os arquivos de código serão adicionados conforme o avanço das próximas issues.

### Pré-requisitos
* Compilador **GCC** (para o backend em C).
* **Python 3** com `customtkinter` (para a Interface Gráfica):
  ```bash
  pip install customtkinter
  ```
* **Valgrind** *(opcional, para validação de memória nos testes em C)*.

### 1. Extração da Base de Pacotes *(a partir da Issue #2)*
Execute o script de extração em Python para baixar `Packages.xz` do Ubuntu 24.04 e gerar `pacotes.csv` e `dependencias.csv`.

### 2. Compilação do Backend (C) *(a partir das Issues #3–#7)*
Na raiz do repositório, compile os módulos em C gerando o binário `main.out`:
```bash
gcc -Wall -Wextra -O2 back/main.c back/dfs.c back/reverso.c back/bfs.c -o main.out
```

### 3. Execução da Interface Gráfica *(a partir das Issues #8–#10)*
Inicie o frontend em Python (que gerencia o executável `./main.out` automaticamente via `subprocess`):
```bash
python3 front/interface.py
```

### 4. Testes Unitários e Valgrind *(a partir da Issue #6)*
Os programas de teste isolados em `test/` poderão ser compilados com `gcc` e validados com:
```bash
valgrind --leak-check=full ./test_grafo.out
```

---

## Check-list de Desenvolvimento
- [X] Estrutura inicial do projeto, `.gitignore` e `README.md` base.
- [ ] `dados`: Extração e limpeza de pacotes do Ubuntu 24.04 (`Packages.xz` $\rightarrow$ CSVs).
- [ ] `backend`: Contratos (`back/grafo.h`) e leitor de CSVs em $O(V + E)$.
- [ ] `backend`: Implementação de DFS, detecção de ciclos e Ordenação Topológica (`back/dfs.h` / `back/dfs.c`).
- [ ] `backend`: Implementação do Grafo Reverso e BFS (`back/reverso.h` / `back/reverso.c` / `back/bfs.c`).
- [ ] `backend`: Testes unitários isolados e validação de memória com Valgrind.
- [ ] `backend`: Protocolo de comunicação IPC (`ANALISAR`, `IMPACTO`, `ROTA`) em `back/main.c`.
- [ ] `frontend`: Módulo de conexão Python-C via `subprocess.Popen` (`front/conexao_backend.py`).
- [ ] `frontend`: Interface gráfica — Canvas interativo e Aba 1 (Ordenação Topológica).
- [ ] `frontend`: Interface gráfica — Aba 2 (Simulação de Desinstalação) e Aba 3 (Rota BFS).
- [ ] Gravação do vídeo de apresentação (5 min) e documentação final.
