# Poulsbo: evidência direta e comparação com SGX535

Convenção: **CONFIRMED** confirma conteúdo/histórico das fontes, não funcionamento no hardware; **INFERRED** identifica uma dedução; **UNKNOWN** é o que ainda não foi estabelecido. Identificadores de fonte como H535 e PBUILD resolvem para repositório, commit completo, arquivo e hash no [catálogo de fontes](source-archaeology.md#catálogo-de-fontes). Os snapshots documentais preservam as linhas originais e os avisos de licença; não são arquivos de implementação.

As duas âncoras são o TI KM 1.14 `cb46ba4d0c900f89f7ec0284f9803d476bfa98de` e o Linux local `9b87fdc9af2fbfcdb5c24a64139685ef80f6573f`. H535 é citado pela primeira versão local encontrada (1.13); seu blob é idêntico na âncora 1.14. Não usamos SGX540/544 como substitutos.

## Integração Intel explicitamente publicada no KM

| Tema | DDK Poulsbo | Linux gma500 | Confidence / limite |
| --- | --- | --- | --- |
| Core/revisão de build | Poulsbo D0 → 535/121, [PBUILD:44–50](archaeology-data/PBUILD.txt#L44) | comentário SGX535 e IDs 8108/8109, [LDRVC:43–59](archaeology-data/LDRVC.txt#L43) | CONFIRMED; identidade de cada stepping UNKNOWN |
| MMIO SGX | offset 0x40000, tamanho 0x4000, [PSYS:51–55](archaeology-data/PSYS.txt#L51) | offset 0x40000, tamanho 0x8000, [LDRVH:41–53](archaeology-data/LDRVH.txt#L41) | CONFIRMED: mesma base relativa, extensões diferentes; razão UNKNOWN |
| PCI | vendor 8086, device 8108; índice MMADR=4, [PSYS:64–71](archaeology-data/PSYS.txt#L64) | recurso MMIO 0; IDs 8108/8109, [LDRVH:49–53](archaeology-data/LDRVH.txt#L49), [LDRVC:58–59](archaeology-data/LDRVC.txt#L58) | CONFIRMED; índice DDK é convertido antes de usar recurso |
| IRQ da integração | IER 20a0, IIR 20a4, IMR 20a8; THALIA bit18, [PSYS:81–94](archaeology-data/PSYS.txt#L81) | mesmas posições e SGX bit18, [LDRVH:91–111](archaeology-data/LDRVH.txt#L91) | CONFIRMED: concordância de definições e uso |
| Host port | feature habilitada, tamanho 0x8000000, VA base 0xd0000000, [PSYS:48–49](archaeology-data/PSYS.txt#L48), [PSYS:108–112](archaeology-data/PSYS.txt#L108) | não equivale ao slave port 2D 0x4000, [LDRVH:61](archaeology-data/LDRVH.txt#L61) | CONFIRMED como declarações distintas; necessidade no novo desenho UNKNOWN |
| Clock nominal | constante 200000000, [PINFO:46–49](archaeology-data/PINFO.txt#L46) | não demonstra medição da GPU alvo | CONFIRMED como parâmetro DDK, frequência real UNKNOWN |
| Endereços físicos | CPU/sistema/device convertidos por identidade, [PSYSC:1340–1445](archaeology-data/PSYSC.txt#L1340) | MMU escreve PFN da página, [LMMU:123–157](archaeology-data/LMMU.txt#L123) | CONFIRMED no software, sem prova de todos os domínios DMA |
| Power | ramo DRM externo usa ospm_* para display/graphics islands, [PSYSC:1916–1979](archaeology-data/PSYSC.txt#L1916) | não é a mesma ABI dos callbacks atuais da fase 1 | CONFIRMED como dependência histórica, portabilidade UNKNOWN |

**CONFIRMED:** em Linux, DDK define `POULSBO_ADDR_RANGE_INDEX = MMADR_INDEX - 4`; a base usada para SGX vem desse recurso e soma SGX_REGS_OFFSET. Não interpretar MMADR_INDEX=4 como BAR4. [PSYSC:144–149](archaeology-data/PSYSC.txt#L144); [PSYSC:348–350](archaeology-data/PSYSC.txt#L348); [PSYSC:418–430](archaeology-data/PSYSC.txt#L418).

O ramo `SUPPORT_DRI_DRM_EXT` depende de `psb_drv.h`, `psb_powermgmt.h` e `sys_pvr_drm_export.h` e oferece `SYSPVRServiceSGXInterrupt`, que chama o ISR do device e agenda MISR. [PSYSC:65–69](archaeology-data/PSYSC.txt#L65); [PSYSC:1982–2005](archaeology-data/PSYSC.txt#L1982). **CONFIRMED:** é um ponto concreto de integração com driver PSB histórico, não garantia de encaixe no gma500 atual.

## Offsets: comparação sistemática

A comparação direta encontrou **26 offsets numéricos com nomes correspondentes iguais**, sem divergência nesse subconjunto. Os outros 16 defines numéricos Linux são USE_CODE_BASE0..15: o DDK expressa essas bases por macro indexada. A comparação manual da fórmula confirma `0x0a0c + 4*i`, 16 entradas, campos base `0x01ffffff` e DM `0x06000000` shift25. [LREG:89–114](archaeology-data/LREG.txt#L89); [H535:727–736](archaeology-data/H535.txt#L727). Igualdade da fórmula é **CONFIRMED**; igualdade de efeitos em todos os steppings não foi testada.

A planilha automática marca os 16 nomes individuais como UNKNOWN por ausência de um define literal correspondente; isso não é contradição com a fórmula verificada. [Comparação completa com linhas](archaeology-data/register-offset-comparison.tsv).

| Grupo | Offsets/valores comparados | Fontes | Confidence |
| --- | --- | --- | --- |
| Clock/identificação/reset | 0000, 0010, 0014, 0018, 001c, 0080; reset bits 0–6 | [LREG:13–56](archaeology-data/LREG.txt#L13); [H535:45–128](archaeology-data/H535.txt#L45) | CONFIRMED como defines |
| Eventos | 0110, 0114, 0118, 012c, 0130, 0134; fault BIF bit4; SW_EVENT bit14 | [LREG:60–84](archaeology-data/LREG.txt#L60); [H535:129–374](archaeology-data/H535.txt#L129) | CONFIRMED como defines |
| Kick/PDS | 0ab8, 0ac4, 0ac8; NOW bit0 | [LREG:116–124](archaeology-data/LREG.txt#L116); [H535:375–386](archaeology-data/H535.txt#L375) | CONFIRMED como defines |
| BIF | 0c00, 0c04, 0c08; FLUSH bit2, INVALDC bit3, CLEAR_FAULT bit4 | [LREG:128–147](archaeology-data/LREG.txt#L128); [H535:417–458](archaeology-data/H535.txt#L417) | CONFIRMED como defines |
| Bancos/diretórios/bases | 0c38, 0c78, 0c7c, 0c84, 0c88, 0cac | [LREG:126–153](archaeology-data/LREG.txt#L126); [H535:539–682](archaeology-data/H535.txt#L539) | CONFIRMED como defines |
| 2D | status 0e04: busy bit24, complete 23:0; SOCIF 0e18: freespace 7:0 | [LREG:155–163](archaeology-data/LREG.txt#L155); [H535:693–720](archaeology-data/H535.txt#L693) | CONFIRMED como defines |

## BIF: dúvida agora mais precisa

**CONFIRMED:** H535 nomeia BASE1=0x0c38 e BASE2=0x0c3c, enquanto o Linux seleciona `BASE1 + hw_context*4` para contexto não zero. Com contexto 1 ele escreve, portanto, no registrador chamado BASE2 pelo header SGX535. O reset TI usa `BASE1 + (índice-1)*4`. [H535:539–546](archaeology-data/H535.txt#L539); [LMMU:123–135](archaeology-data/LMMU.txt#L123); [RESET:125–135](archaeology-data/RESET.txt#L125).

**INFERRED:** a divergência não vem de offsets BASE1 diferentes entre essas fontes; vem da fórmula/convenção de indexação. **UNKNOWN:** intenção do contexto 1, convenção do driver, bug histórico ou efeito no silício. Sem o histórico anterior do Linux local e uma explicação do contrato não há base para corrigir o código.

O header ainda confirma BANK0/BANK1 com campos EDM 3:0, TA 7:4, HOST 11:8, 3D 15:12 e 2D 19:16; o ramo SGX535 declara 16 dirlists e uma entrada de cache PT por linha. [H535:603–626](archaeology-data/H535.txt#L603); [FEATURE:76–88](archaeology-data/FEATURE.txt#L76). A política de segurança/isolamento efetiva continua **UNKNOWN**.

## Informação adicional que não deve virar suposição de hardware

**CONFIRMED:** H535 declara máscara de endereço BIF_FAULT `0xfffff000`, dirlist bases `0xfffff000`, request bases 2D/3D `0xfff00000`, PDS_EXEC_BASE `0xfff00000` e EVENT_KICKER `0xfffffff0`. [H535:455–458](archaeology-data/H535.txt#L455); [H535:539–546](archaeology-data/H535.txt#L539); [H535:631–642](archaeology-data/H535.txt#L631); [H535:679–682](archaeology-data/H535.txt#L679); [H535:375–382](archaeology-data/H535.txt#L375). Esses campos reduzem lacunas documentais da fase 1, mas não estabelecem efeitos laterais ou sequência segura de escrita.

**CONFIRMED:** o header SGX535 e o Linux concordam nos bits básicos PDE/PTE: valid=1, WO=2, RO=4 e cache=8 no contrato MMU comum. [MMU:48–93](archaeology-data/MMU.txt#L48); [LDRVH:75–84](archaeology-data/LDRVH.txt#L75). **UNKNOWN:** coerência completa e permissões EDM em cada integração; usar o header comum com a seleção 535 não equivale a teste.

## Artefato externo prioritário

O artefato de maior valor agora é **um pacote-fonte user-mode e microkernel do IMG SGX DDK 1.14 build 3699939, especificamente configurado para `pc_i686_poulsbo_d0_linux`, SGX535 rev121, com licença e proveniência verificáveis**. Deve conter o inicializador SGX que preenche scripts/handlers, fontes ou geração reproduzível dos programas microkernel/PDS e headers completos da ABI correspondente ao KM.

Essa especificação deriva das âncoras locais [PBUILD:44–52](archaeology-data/PBUILD.txt#L44) e [VER14:51–60](archaeology-data/VER14.txt#L51); a existência pública de tal pacote é **UNKNOWN**. Ele é prioritário porque o header e a integração KM já foram encontrados, enquanto boot e execução ainda dependem do lado ausente. Se só um arquivo puder ser obtido, priorizar o **`sgxinit.c` user-mode dessa mesma entrega/alvo**, acompanhado de seus includes e identificação de build; não confundir com o `sgxinit.c` kernel já presente. Essa escolha é uma recomendação de investigação (**INFERRED**), não uma alegação sobre onde o artefato está disponível.
