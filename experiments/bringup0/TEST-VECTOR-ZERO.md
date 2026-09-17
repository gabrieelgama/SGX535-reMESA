# TEST VECTOR ZERO

## Objetivo formal

Coletar uma fotografia reproduzível de identidade e binding usando somente
interfaces passivas do Linux. Não é um teste funcional da SGX.

## Pré-condições

- kernel em estado normal e sysfs/procfs montados;
- cópia exata do probe da revisão registrada;
- usuário sem privilégios sempre que as permissões permitirem;
- nenhum outro comando de diagnóstico que abra BAR/DRM misturado à captura.

## Operações permitidas, em ordem

1. enumerar `/sys/bus/pci/devices`;
2. ler `vendor`/`device` e selecionar somente `8086:8108` ou `8086:8109`;
3. ler revision, subsystem, class, IRQ e o arquivo textual `resource`;
4. resolver o symlink `driver`;
5. correlacionar nós em `/sys/class/drm` pelo symlink `device`;
6. ler runtime status/contadores se presentes;
7. ler versão do kernel, obter arquitetura com `uname` e campos DMI públicos
   de sistema/placa/BIOS, sem serial ou UUID;
8. declarar GTT/stolen indisponíveis quando não expostos;
9. produzir relatório humano ou JSON e sair.

Execução futura:

```sh
python3 tools/sgx535-probe/sgx535_probe.py --json >sgx535-probe.json
```

## Operações proibidas

Não abrir PCI `config`, `resourceN`, ROM, `/dev/mem`, DRM node ou debugfs. Não
escrever sysfs. Não resetar, inicializar, mudar clocks/PM/MMU, carregar firmware,
enviar CCB/`EVENT_KICK`, nem executar PDS/USSE/shader.

## Saída e critérios

Sucesso exige ID exato, todos os campos obrigatórios legíveis, relatório válido
e exit `0`. ID desconhecido, ausência, ambiguidade ou erro de parsing retorna
exit `2`; nenhuma tentativa alternativa é feita.

Estado final esperado:

```text
hardware state modified by probe: NO
MMIO reads: NO
MMIO writes: NO
command submission: NO
firmware loading: NO
```

O primeiro campo tem escopo deliberado. Se `gma500` estiver associado, o driver
já executou inicialização ativa antes de publicar o DRM node (P4-006). Portanto
o vetor não certifica “hardware intocado desde boot”.

## Resultado de design da Fase 4

**GO para execução futura.** Os testes funcionais usaram sysfs/procfs
sintéticos. Uma invocação de sanity no ambiente de desenvolvimento falhou
fechada, por permissão, antes de enumerar qualquer função PCI. Nenhum resultado
de Inspiron/Poulsbo é alegado e nenhum atributo de dispositivo foi lido.
