# Poulsbo: interrupções e faults

Fase 3 — análise estática, 2026-09-17. `CONFIRMED` significa o que a fonte declara/implementa; `INFERRED` é interpretação; `UNKNOWN` é lacuna. IDs P3 remetem à [matriz](evidence-matrix.csv), com repositório, commit, arquivo e linhas. Os snapshots preservam a numeração original; ver [proveniência](poulsbo-data/sources.json). Nenhum procedimento abaixo foi executado na GPU.

**CONFIRMED [P3-030]** — DDK Poulsbo define IER/IIR/IMR/ISR Intel em 0x20a0/20a4/20a8/20ac e THALIA/SGX bit18; traduz a indicação para DEVICE_SGX_INTERRUPT e limpa IIR. Linux despacha esse bit para status SGX, separado dos eventos de display. Fontes: [PSYS:77-94](poulsbo-data/../archaeology-data/PSYS.txt); [PSYSC:1196-1282](poulsbo-data/../archaeology-data/PSYSC.txt); [LDRVH:91-111](poulsbo-data/../archaeology-data/LDRVH.txt); [LIRQ:198-230](poulsbo-data/../archaeology-data/LIRQ.txt).

**CONFIRMED [P3-031]** — Linux habilita BIF_REQUESTER_FAULT e TWOD_COMPLETE. O handler lê INT_STAT/FAULT, registra tipo, requestor e endereço, depois escreve HOST_CLEAR/2; instala IRQ compartilhada. Fontes: [LIRQ:151-196](poulsbo-data/../archaeology-data/LIRQ.txt); [LIRQ:278-325](poulsbo-data/../archaeology-data/LIRQ.txt).

**CONFIRMED [P3-032]** — No DDK 1.14, SGX_ISRHandler reconhece SW_EVENT; o MISR pode chamar recuperação. A integração DRM_EXT exporta SYSPVRServiceSGXInterrupt, chama o ISR do dispositivo e agenda MISR quando tratado. Fontes: [INIT:1945-2008](poulsbo-data/INIT.txt); [INIT:2097-2125](poulsbo-data/INIT.txt); [PSYSC:1982-2005](poulsbo-data/../archaeology-data/PSYSC.txt).

## Cadeia de responsabilidades

**INFERRED, a partir de P3-030–032:** requestor SGX → estado de evento SGX → agregação THALIA Intel → IRQ PCI → ISR/MISR. A interrupção de vblank pertence à cadeia display; não comprova avanço de CCB nem execução USE.

**UNKNOWN:** ordem necessária de ack/mask sob fault persistente na revisão real; se uma leitura tem efeito lateral em todos os estados de energia; garantia de que HOST_CLEAR remove a causa da falha BIF; sequência segura de recuperação com um job em voo. O handler atual imprime e reconhece o evento; não oferece por si só isolamento e recuperação de jobs 3D.

Não habilitar SW_EVENT nem induzir page fault apenas para testar interrupção. Para uma fase futura, começar por logs já produzidos pelo driver proprietário do dispositivo. Qualquer teste ativo de IRQ exige ownership exclusivo e plano de restauração da máscara compartilhada com o display.

