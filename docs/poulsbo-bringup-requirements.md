# Requisitos para primeiro bring-up controlado

Fase 3 — análise estática, 2026-09-17. `CONFIRMED` significa o que a fonte declara/implementa; `INFERRED` é interpretação; `UNKNOWN` é lacuna. IDs P3 remetem à [matriz](evidence-matrix.csv), com repositório, commit, arquivo e linhas. Os snapshots preservam a numeração original; ver [proveniência](poulsbo-data/sources.json). Nenhum procedimento abaixo foi executado na GPU.

## Decisão

**NÃO temos informação suficiente para autorizar nesta fase um bring-up ativo da SGX535, com tomada de ownership, reset, programação de MMU ou bootstrap.** Esta é uma avaliação **INFERRED** das lacunas abaixo, não uma alegação de inviabilidade do projeto.

Temos material suficiente para especificar um inventário passivo e preparar um futuro teste limitado de identificação. Inventário não equivale a bring-up do núcleo; a falta de firmware não bloqueia inventário nem, por si só, uma leitura de ID. Os critérios devem ser proporcionais ao experimento.

| Etapa futura | Bloqueadores exatos | O que os resolve |
|---|---|---|
| Inventário sem acessos novos | placa/ambiente e logs ainda não fornecidos | registros já existentes de PCI, driver, kernel/BIOS, recursos exportados e boot |
| Leitura nova de ID SGX | B1 revisão/plataforma alvo; B2 ownership e energia de acesso; B3 segurança das leituras e recuperação comprovada | identificar placa; definir cooperação com driver; fundamentar allowlist e preparar console/reboot |
| Reset/MMU controlados | B1–B3 mais B4 sequência clock/reset/errata; B5 índice contextos; B6 memória/DMA/coerência e proteção display | fontes/trace aplicáveis à revisão e plano testável com limites |
| Bootstrap microkernel | anteriores mais B7 scripts/PDS/payload/ABI e licença; B8 timeouts/ack/fault/recovery | pacote correspondente verificável e protocolo completo de inicialização/parada |
| Renderização | anteriores mais streams/ISA/layouts/estado/sync/isolamento | nova fase de engenharia, fora do escopo |

B1 corresponde a U01; B2 a U03/U04; B3 a U02/U16; B4 a U03/U05; B5 a U06/U09; B6 a U07/U08/U20; B7 a U10–U14; B8 a U15/U16. Ver [20 incógnitas](unknowns.md). A divergência P3-026 bloqueia escrever contextos novos, não coletar um log existente. Não exigir ISA completa para ler um ID, nem declarar reset seguro apenas porque existe em gma500.

## Menor experimento proposto — atualizado pela Fase 4

**Etapa zero, passiva:** agora está implementada em `tools/sgx535-probe` como Test Vector Zero. Ela recolhe PCI IDs/revision/subsystem, recursos textuais publicados pelo kernel, IRQ, driver, DRM node, kernel, arquitetura e runtime status sem abrir BAR, PCI config ou DRM. A garantia “não modifica estado” limita-se ao probe: o `gma500` já inicializa ativamente hardware durante bind.

**Etapa seguinte continua BLOCKED:** `CORE_ID` e `CORE_REVISION` têm offsets e uso histórico confirmados, mas a Fase 4 não encontrou contrato de read-side-effects, power/clock ou locking suficiente. Eles não formam uma whitelist aprovada. Se essas lacunas forem fechadas, a instrumentação deverá viver no gma500 e usar uma operação compilada e específica; nunca acesso genérico a MMIO.

O registro deve conter ID PCI/stepping, BAR base/len, offset exato, valor bruto, interpretação separada, estado de energia conhecido, timestamps e erro/timeout. Nenhum scan de offsets, escrita de reset, teste de fault, CCB, firmware, PDS ou stream3D. Não inserir retries ilimitados. Após coleta, o recurso temporário deve ser removível; em falha, parar a sequência e recuperar pelo mecanismo previamente preparado. Reboot é plano de recuperação, não garantia já demonstrada.

Essa etapa MMIO **não está aprovada tecnicamente como pronta**. Nenhum patch ou sequência MMIO foi escrito.

## Próximas fontes e artefato de maior valor

1. **Pacote fonte legítimo UM + microkernel/PDS + inicializador correspondente a `pc_i686_poulsbo_d0_linux`, SGX535 rev121, IMG DDK1.14 build3699939**, com licença explícita, opções geradas, scripts preenchidos, hashes e cabeçalhos ABI. É o artefato externo de maior valor para fechar o bootstrap que o KM local só consome. Não foi demonstrado que esse pacote esteja publicamente disponível.
2. Implementações exatas de `psb_powermgmt.h` / `sys_pvr_drm_export.h` usadas pelo ramo DRM_EXT desse DDK, com versão do driver host; não basta um header homônimo de outra família.
3. Userspace/Xorg que atende XHW do PSB5.0.0.0045, especialmente handlers scene-bind/fire, TA-memory, OOM e reset DPM. Serve para reconstruir fronteiras históricas; não é ABI a adotar.
4. Manual/errata Intel SCH da revisão alvo que descreva requisitos de acesso, clock/power/reset/IRQ e a relação com BIF; feature matrix EMGD não substitui esse manual.
5. No espelho EMGD fixado: build/config gerado, bridge dispatch, common/sysconfig.c, plb init/power e sgxreset.c; correlacionar versão interna PVR1.5.15.3226 com um pacote Intel autenticável. Nenhum binário deve ser tratado como firmware compatível apenas pelo nome.

Não é necessário resolver streams3D/texturas para preparar inventário. Já para executar qualquer payload, origem, licença, revisão, layout e mecanismo de parada precisam ser verificados previamente.
