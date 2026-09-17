# Poulsbo: ABI histórico e proveniência

Fase 3 — análise estática, 2026-09-17. `CONFIRMED` significa o que a fonte declara/implementa; `INFERRED` é interpretação; `UNKNOWN` é lacuna. IDs P3 remetem à [matriz](evidence-matrix.csv), com repositório, commit, arquivo e linhas. Os snapshots preservam a numeração original; ver [proveniência](poulsbo-data/sources.json). Nenhum procedimento abaixo foi executado na GPU.

**CONFIRMED [P3-040]** — O header PSB histórico declara pacote 5.0.0.0045 e comandos CMDBUF=0, XHW_INIT=1, XHW=2, SCENE_UNREF=3, KMS_OFF=4, KMS_ON=5, HW_INFO=6. O dispatch autentica CMDBUF e restringe XHW a root. Fontes: [PSB_psb_drm_h:32-40](poulsbo-data/PSB_psb_drm_h.txt); [PSB_psb_drm_h:338-361](poulsbo-data/PSB_psb_drm_h.txt); [PSB_psb_drv_c:86-111](poulsbo-data/PSB_psb_drv_c.txt).

**CONFIRMED [P3-041]** — drm_psb_cmdbuf_arg transporta listas de buffers/cliprects, scene/fence, handles/offsets/tamanhos TA, OOM, comando e relocations, engine e feedback. drm_psb_reloc contém operação, destino, máscara, shift e parâmetros. Define tipos de memória MMU/PDS/APER/RASTGEOM, engines e fences TA/raster/scene. Fontes: [PSB_psb_drm_h:47-54](poulsbo-data/PSB_psb_drm_h.txt); [PSB_psb_drm_h:120-184](poulsbo-data/PSB_psb_drm_h.txt); [PSB_psb_drm_h:206-263](poulsbo-data/PSB_psb_drm_h.txt).

**CONFIRMED [P3-042]** — XHW_INIT recebe handle de buffer; o kernel faz lookup/map e o chama de buffer de comunicação com o X server. Há operações fire raster, bind scene, memória TA, reset DPM, OOM, terminate, vistest, resume e lockup; o ioctl espera trabalho e usa o buffer compartilhado. Fontes: [PSB_psb_drm_h:265-361](poulsbo-data/PSB_psb_drm_h.txt); [PSB_psb_xhw_c:415-475](poulsbo-data/PSB_psb_xhw_c.txt); [PSB_psb_xhw_c:543-629](poulsbo-data/PSB_psb_xhw_c.txt).

**CONFIRMED [P3-043]** — psb_sgx.c valida a lista de buffer objects e aplica relocations, verificando índices de origem/destino e operações específicas PDS/USE. Isso documenta o caminho de submissão, não uma auditoria de segurança do ABI. Fontes: [PSB_psb_sgx_c:410-464](poulsbo-data/PSB_psb_sgx_c.txt); [PSB_psb_sgx_c:631-779](poulsbo-data/PSB_psb_sgx_c.txt).

**CONFIRMED [P3-044]** — O PSB histórico tem watchdog para lockup e reset workqueue; consulta o canal XHW e solicita reset DPM após reconfiguração MMU. Fontes: [PSB_psb_reset_c:145-177](poulsbo-data/PSB_psb_reset_c.txt); [PSB_psb_reset_c:231-279](poulsbo-data/PSB_psb_reset_c.txt).

**CONFIRMED [P3-045]** — DDK 1.14 define comandos DRM Services/display/buffer-class/is-master/unpriv/debug como 0..5 no modo próprio; DRM_EXT usa DRM_PVR_RESERVED1..6. Não há valores numéricos universais para o modo externo nesse header. Fontes: [DRMS:43-70](poulsbo-data/DRMS.txt); [DRMC:462-475](poulsbo-data/DRMC.txt).

**CONFIRMED [P3-046]** — PVRSRV_BRIDGE_PACKAGE transporta BridgeID, tamanho, ponteiros/tamanhos de entrada/saída e hKernelServices. ALLOCDEVICEMEM usa device/heap handles, atributos, tamanho, alinhamento e informação de chunks; não é um GEM handle moderno. Fontes: [BRIDGE:312-322](poulsbo-data/BRIDGE.txt); [BRIDGE:482-497](poulsbo-data/BRIDGE.txt).

**CONFIRMED [P3-047]** — sgx_bridge.h define DOKICK em SGX_CMD_BASE+3; TRANSFER +13; INFO_FOR_SRVINIT +15; DEVINITPART2 +16; registros de contexto render +20, 2D +24, transfer +26; SUBMIT2D +23 e PROCESS_QUEUES +28, com condicionais de build. Fontes: [SBRIDGE:63-114](poulsbo-data/SBRIDGE.txt); [SBRIDGE:233-273](poulsbo-data/SBRIDGE.txt); [SBRIDGE:315-351](poulsbo-data/SBRIDGE.txt).

**CONFIRMED [P3-048]** — SGX_BRIDGE_INIT_INFO recebe handles CCB/controle/event-kicker/host-control/TA3D, endereços de handlers, scripts, build options, struct sizes, dados de clock/cache e handles adicionais. SGX_CCB_KICK agrega comando, handle/offset CCB e objetos de sincronização TA/3D. Fontes: [INFO:83-153](poulsbo-data/INFO.txt); [INFO:181-251](poulsbo-data/INFO.txt).

**CONFIRMED [P3-049]** — SGXMKIF_COMMAND contém service address USE, cache control e seis palavras de dados; o kernel CCB contém 256 comandos e controle read/write offset. O produtor verifica espaço e avança write offset módulo 256. Fontes: [MKIF:70-96](poulsbo-data/MKIF.txt); [UTIL:236-254](poulsbo-data/UTIL.txt); [UTIL:552-558](poulsbo-data/UTIL.txt).

**CONFIRMED [P3-050]** — Após publicar o comando, o caminho Services atualiza o event kicker, usa barreira de memória e escreve EVENT_KICK2 sob FIX_HW_BRN_26620 + SYSTEM_CACHE sem bypass, e EVENT_KICK no ramo alternativo. A definição SGX535 citada não habilita MULTI_EVENT_KICK. Fontes: [UTIL:603-633](poulsbo-data/UTIL.txt); [FEATURE:76-88](poulsbo-data/../archaeology-data/FEATURE.txt).

**CONFIRMED [P3-051]** — SGXInitialise executa script parte1, reset, script parte2, kick e espera PVRSRV_USSE_EDM_INIT_COMPLETE em memória compartilhada; os scripts vêm de SGX_BRIDGE_INIT_INFO. Essa evidência não fornece o conteúdo do microkernel nem dos scripts preenchidos pelo UM. Fontes: [INIT:207-220](poulsbo-data/INIT.txt); [INIT:534-574](poulsbo-data/INIT.txt); [INIT:637-730](poulsbo-data/INIT.txt); [INFO:83-100](poulsbo-data/INFO.txt).

**CONFIRMED [P3-052]** — O display-class Poulsbo define ENTER_VT=1, LEAVE_VT=2 e CURSOR_LOAD=3, struct com cmd/dev-id e cursor de tamanho32/pointer64. Fontes: [DCS:46-68](poulsbo-data/DCS.txt).

**CONFIRMED [P3-053]** — EMGD emgd_shared.h fixa PVR_RESERVED1..5 em 0x12..0x16 e RESERVED6 em 0x1e. emgd_drm.h define GMM_ALLOC_REGION=0x0e, ALLOC_SURFACE=0x0f, FREE=0x10, FLUSH_CACHE=0x11, DRIVER_PRE_INIT=0x23, START_PVRSRV=0x25 e PREINIT_MMU=0x39. Fontes: [EMGD_drm_include_emgd_shared_h:50-59](poulsbo-data/EMGD_drm_include_emgd_shared_h.txt); [EMGD_drm_include_emgd_drm_h:679-763](poulsbo-data/EMGD_drm_include_emgd_drm_h.txt).

**CONFIRMED [P3-054]** — emgd_drm_start_pvrsrv_t contém xserver e rtn, descrito como retorno de PVRSRVDrmLoad. O pvrversion.h do espelho declara 1.5.15.3226: não confundir essa versão interna do PVR com EMGD 1.14 ou DDK TI 1.14. Fontes: [EMGD_drm_include_emgd_drm_h:638-647](poulsbo-data/EMGD_drm_include_emgd_drm_h.txt); [EMGD_drm_pvr_include4_pvrversion_h:45-53](poulsbo-data/EMGD_drm_pvr_include4_pvrversion_h.txt).

## Três interfaces, não um ABI único

| Família | Entrada | Objetos/controle | Limite da reconstrução |
|---|---|---|---|
| PSB 5.0.0.0045 | DRM CMDBUF / XHW | BOs, relocations, scenes, fences; dependência de X server | handlers privados userspace não reconstruídos |
| DDK TI 1.14 alvo Poulsbo | bridge DRM Services | heaps, handles, contexts, syncs, scripts/CCB/host-control | integração DRM_EXT incompleta e UM ausente |
| Espelho EMGD / PVR 1.5.15.3226 | IGD + slots PVR | display/GMM + Services/init | não demonstra ABI binário compatível com TI1.14 |

Os números acima são índices relativos conforme cada definição; um número de comando não é o ioctl Linux completo. Direção, tamanho, DRM_COMMAND_BASE, ponteiros nativos, IMG_HANDLE, IMG_SIZE_T e macros de build participam do ABI. Não inventar sizeof/packing, nem equivalência x86/ARM ou 32/64 bits.

**UNKNOWN:** snapshot completo do lado userspace PSB XHW; opções geradas do build instalado; layouts exatos dos objetos de firmware; conteúdo/licença/revisão dos microkernels Poulsbo; scripts preenchidos e handshake integral UM/KM. O canal XHW não deve ser chamado de firmware: é comunicação kernel/X server, segundo P3-042. A presença de firmware de vídeo MSVDX em outros pacotes não identificaria firmware SGX.

**INFERRED:** um driver moderno precisaria desenhar validação, lifetime, isolamento de VA, fences e recuperação próprios. Isso não é autorização para implementar ou reutilizar o ABI histórico nesta fase.

## Licença e proveniência

Decisões abaixo são política de separação de material deste projeto, baseada nos avisos por arquivo. Não atribuir uma licença única ao diretório EMGD nem inferir direitos a partir de disponibilidade pública.


**CONFIRMED [P3-055]** — sgx535defs.h preserva aviso dual MIT/GPLv2, com opção MIT e manutenção dos avisos. Fontes: [H535:1-40](poulsbo-data/../archaeology-data/H535.txt).

**CONFIRMED [P3-056]** — gtt.c atual tem SPDX GPL-2.0-only; psb_drm.h histórico traz GPL versão2. Não transplantar código desses arquivos para uma futura implementação Mesa sob licença permissiva. Fontes: [GTT:1-8](poulsbo-data/GTT.txt); [PSB_psb_drm_h:1-21](poulsbo-data/PSB_psb_drm_h.txt).

**CONFIRMED [P3-057]** — Os headers EMGD emgd_drm.h e SGX535 contêm avisos permissivos com exigência de preservação de copyright/licença. O License.txt também separa o kernel DRM e aponta GPLv2; tratar a combinação por arquivo e proveniência, sem licença global presumida. Fontes: [EMGD_drm_include_emgd_drm_h:1-30](poulsbo-data/EMGD_drm_include_emgd_drm_h.txt); [EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h:1-21](poulsbo-data/EMGD_drm_pvr_services4_srvkm_hwdefs_sgx535defs_h.txt); [EMGD_License_txt:1-9](poulsbo-data/EMGD_License_txt.txt).

**CONFIRMED [P3-058]** — A licença userspace no readme.txt EMGD permite binários sem modificação, restringe hardware e proíbe reverse engineering/decompilação/disassembly; License.txt também contém condições para redistribuição binária. Fontes: [EMGD_readme_txt:1-16](poulsbo-data/EMGD_readme_txt.txt); [EMGD_License_txt:32-42](poulsbo-data/EMGD_License_txt.txt).


| Classe | Uso nesta fase | Política para implementação futura |
|---|---|---|
| Código com origem/commit/aviso preservado | documentação de comportamento do software | revisar licença individual antes de copiar |
| TI dual MIT/GPLv2, headers permissivos IMG/Intel identificados | candidatos técnicos, sem port nesta fase | potencialmente reutilizáveis pela opção permissiva, preservando avisos e verificando origem |
| Linux GPL-only e PSB GPL | evidência e comparação | não copiar implementação para Mesa permissiva; manter separação documental |
| EMGD com proveniência apenas de espelho | evidência do conteúdo do espelho | autenticidade/licença da versão exata ainda precisa confirmação |
| UM TI e UM EMGD binários | inventário histórico, licença e metadados | não copiar payload, desassemblar, executar ou carregar nesta fase |
| Documentos Intel | contexto e evidência do que declaram | não são licença de firmware nem descrição completa do núcleo |

Os `.txt` em poulsbo-data são snapshots de referência com avisos originais, não código novo de driver. As licenças deles não são substituídas pela licença da documentação. Não foram importados binários externos.


**CONFIRMED [P3-064]** — O README do espelho comunitário se apresenta como EMGD 1.18 e declara reunir binários, fontes e patches; não é uma release Intel autenticada pelo projeto. [EMGD README:1–7](poulsbo-data/EMGD_README_md.txt). Distinguir esse rótulo, o PVR interno 1.5.15.3226 (P3-054) e o documento oficial de funcionalidades EMGD1.14 (P3-060).
