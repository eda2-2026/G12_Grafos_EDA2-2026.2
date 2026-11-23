#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import lzma
import re
import shutil
import urllib.request
from pathlib import Path

UBUNTU_PACKAGES_URL = (
    "http://archive.ubuntu.com/ubuntu/dists/noble/main/binary-amd64/Packages.xz"
)

DIRETORIO_DADOS = Path(__file__).resolve().parent
ARQUIVO_PACKAGES_XZ = DIRETORIO_DADOS / "Packages.xz"
ARQUIVO_PACOTES_CSV = DIRETORIO_DADOS / "pacotes.csv"
ARQUIVO_DEPENDENCIAS_CSV = DIRETORIO_DADOS / "dependencias.csv"

REGEX_VERSAO = re.compile(r"\s*\([^)]*\)")


def baixar_packages_xz(url: str, destino: Path, forcar: bool = False) -> Path:
    if destino.exists() and not forcar:
        print(f"[INFO] Arquivo já existe em {destino}. Reutilizando download local.")
        return destino

    print(f"[INFO] Baixando {url} ...")
    requisicao = urllib.request.Request(
        url,
        headers={"User-Agent": "EDA2-APT-Dependency-Inspector/1.0"},
    )
    with urllib.request.urlopen(requisicao, timeout=60) as resposta:
        with destino.open("wb") as arquivo_saida:
            shutil.copyfileobj(resposta, arquivo_saida)

    tamanho_mb = destino.stat().st_size / (1024 * 1024)
    print(f"[OK] Download concluído: {destino} ({tamanho_mb:.2f} MB)")
    return destino


def limpar_texto_csv(texto: str) -> str:
    limpo = texto.replace(";", ",").replace("\r", " ").replace("\n", " ")
    return " ".join(limpo.split())


def extrair_dependencias(campo_raw: str) -> list[str]:
    if not campo_raw:
        return []

    dependencias: list[str] = []
    for item in campo_raw.split(","):
        alternativa_padrao = item.split("|", maxsplit=1)[0]
        sem_versao = REGEX_VERSAO.sub("", alternativa_padrao)
        sem_arch = sem_versao.split(":", maxsplit=1)[0].strip()
        if sem_arch:
            dependencias.append(sem_arch)

    return dependencias


def parsear_packages_xz(caminho_xz: Path) -> dict[str, dict]:
    pacotes: dict[str, dict] = {}
    bloco_atual: dict[str, str] = {}
    chave_anterior: str | None = None

    def finalizar_bloco() -> None:
        nome = bloco_atual.get("Package", "").strip()
        if not nome:
            return

        deps = extrair_dependencias(bloco_atual.get("Depends", ""))
        pre_deps = extrair_dependencias(bloco_atual.get("Pre-Depends", ""))

        tamanho_str = bloco_atual.get("Installed-Size", "0").strip()
        tamanho_kb = tamanho_str if tamanho_str.isdigit() else "0"

        pacotes[nome] = {
            "nome": nome,
            "versao": limpar_texto_csv(bloco_atual.get("Version", "")),
            "tamanho_kb": tamanho_kb,
            "descricao": limpar_texto_csv(bloco_atual.get("Description", "")),
            "dependencias": deps + pre_deps,
        }

    with lzma.open(caminho_xz, mode="rt", encoding="utf-8", errors="replace") as f:
        for linha in f:
            linha = linha.rstrip("\r\n")

            if not linha:
                if bloco_atual:
                    finalizar_bloco()
                    bloco_atual = {}
                    chave_anterior = None
                continue

            if linha[0] in (" ", "\t"):
                if chave_anterior in ("Depends", "Pre-Depends"):
                    bloco_atual[chave_anterior] += " " + linha.strip()
                continue

            if ":" in linha:
                chave, valor = linha.split(":", maxsplit=1)
                chave = chave.strip()
                bloco_atual[chave] = valor.strip()
                chave_anterior = chave

    if bloco_atual:
        finalizar_bloco()

    return pacotes


def exportar_csvs(
    pacotes: dict[str, dict],
    caminho_pacotes: Path,
    caminho_dependencias: Path,
) -> tuple[int, int, int]:

    nomes_ordenados = sorted(pacotes.keys())
    nome_para_id = {nome: idx for idx, nome in enumerate(nomes_ordenados)}

    arestas: set[tuple[int, int]] = set()
    ignoradas = 0

    for nome_origem in nomes_ordenados:
        id_origem = nome_para_id[nome_origem]
        for nome_destino in pacotes[nome_origem]["dependencias"]:
            id_destino = nome_para_id.get(nome_destino)
            if id_destino is None or id_destino == id_origem:
                ignoradas += 1
                continue
            arestas.add((id_origem, id_destino))

    with caminho_pacotes.open("w", encoding="utf-8", newline="") as f_pac:
        escritor = csv.writer(f_pac, delimiter=";", lineterminator="\n")
        escritor.writerow(["id", "nome", "versao", "tamanho_kb", "descricao"])
        for nome in nomes_ordenados:
            info = pacotes[nome]
            escritor.writerow(
                [
                    nome_para_id[nome],
                    info["nome"],
                    info["versao"],
                    info["tamanho_kb"],
                    info["descricao"],
                ]
            )

    with caminho_dependencias.open("w", encoding="utf-8", newline="") as f_dep:
        escritor = csv.writer(f_dep, delimiter=";", lineterminator="\n")
        escritor.writerow(["id_origem", "id_destino", "nome_origem", "nome_destino"])
        for id_origem, id_destino in sorted(arestas):
            escritor.writerow(
                [
                    id_origem,
                    id_destino,
                    nomes_ordenados[id_origem],
                    nomes_ordenados[id_destino],
                ]
            )

    return len(nomes_ordenados), len(arestas), ignoradas


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Extrai pacotes e dependências do Ubuntu 24.04 (Noble) para CSV."
    )
    parser.add_argument(
        "--forcar-download",
        action="store_true",
        help="Força um novo download de Packages.xz mesmo que já exista localmente.",
    )
    args = parser.parse_args()

    baixar_packages_xz(
        url=UBUNTU_PACKAGES_URL,
        destino=ARQUIVO_PACKAGES_XZ,
        forcar=args.forcar_download,
    )

    print("[INFO] Processando blocos de pacotes...")
    pacotes = parsear_packages_xz(ARQUIVO_PACKAGES_XZ)

    total_v, total_e, ignoradas = exportar_csvs(
        pacotes=pacotes,
        caminho_pacotes=ARQUIVO_PACOTES_CSV,
        caminho_dependencias=ARQUIVO_DEPENDENCIAS_CSV,
    )

    print(f"[OK] {ARQUIVO_PACOTES_CSV.name}: {total_v} pacotes (vértices)")
    print(f"[OK] {ARQUIVO_DEPENDENCIAS_CSV.name}: {total_e} dependências únicas (arestas)")
    print(f"[INFO] Referências virtuais/auto-laços ignorados: {ignoradas}")


if __name__ == "__main__":
    main()
