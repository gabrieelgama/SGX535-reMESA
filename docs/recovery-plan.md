# Plano de recuperação para experimentos futuros

Este plano não autoriza acesso ativo. Os itens marcados **RECOMMENDATION** são
procedimento operacional; `CONFIRMED` descreve apenas comportamento documentado
do Linux ou do gma500.

## Preparação fora da máquina alvo

1. **RECOMMENDATION:** manter uma entrada de boot com kernel conhecido como
   funcional e confirmar, antes do teste, que ela inicia sem intervenção.
2. **RECOMMENDATION:** usar SSH a partir de outro host e, se disponível, console
   serial. Não depender do display que compartilha a função PCI.
3. **RECOMMENDATION:** copiar código, relatório inicial, hash do kernel/config e
   comandos de coleta antes de iniciar. Manter dados de teste fora do único
   filesystem gravável importante.
4. **RECOMMENDATION:** sincronizar filesystems, encerrar workloads e evitar
   gravações não essenciais. Se a instalação permitir, executar de um sistema
   descartável ou root read-only com logs remotos.
5. **RECOMMENDATION:** validar separadamente que reboot remoto, SysRq e a entrada
   de kernel de recuperação funcionam. Não descobrir isso depois de um hang.

## Logging prévio

Registrar, sem fazer MMIO:

```text
uname -a
cat /proc/cmdline
cat /proc/version
python3 tools/sgx535-probe/sgx535_probe.py --json
journalctl -k -b
dmesg --ctime
```

Redirecionar a captura para outro host quando possível. Test Vector Zero não
exige root e não lê debugfs.

## Canais de recuperação

**CONFIRMED — P4-017:** com `CONFIG_MAGIC_SYSRQ`, `/proc/sys/kernel/sysrq`
controla as operações de teclado. SysRq pode despejar tasks (`t`), tasks
bloqueadas (`w`), locks (`d`), sincronizar (`s`), remontar read-only (`u`) e
rebootar imediatamente (`b`). O reboot `b` não sincroniza nem desmonta
filesystems (`Documentation/admin-guide/sysrq.rst:9-46,93-165`).

Sequência futura recomendada, somente se a máquina ainda responder:

1. coletar `d`, `w`, `t` e logs por SSH/console;
2. encerrar o experimento se userspace ainda responder;
3. sincronizar e remontar read-only (`s`, `u`) antes do reboot;
4. usar reboot imediato apenas como último recurso.

**CONFIRMED — P4-018:** quando `ramoops/pstore` já estiver configurado, registros
persistentes podem ser lidos como `dmesg-ramoops-N`; persistent ftrace também
pode ajudar em hangs (`Documentation/admin-guide/ramoops.rst:145-167`). Sua
configuração é tarefa anterior ao experimento, não parte do harness SGX.

## Classificação de falhas

| classe | sinais operacionais | coleta possível | recuperação esperada |
|---|---|---|---|
| GPU hang | kernel/SSH e display respondem; SGX não progride ou faulta | journal/dmesg, estado do processo, pstore depois | terminar experimento; reboot planejado |
| display hang | tela para; SSH/serial e kernel respondem | logs remotos, tasks/locks | não tentar reset SGX; reboot limpo remoto |
| kernel hang | SSH e userspace param; SysRq/serial ainda respondem | SysRq `d/w/t`, console, pstore | sync/remount/reboot se possível |
| machine hang | sem display, SSH, serial ou SysRq | somente watchdog externo/pstore pós-reboot | power-cycle; boot no kernel conhecido |

Esses são critérios de triagem, não diagnósticos automáticos. Um display parado
não prova que a SGX causou a falha.

## Watchdogs e reset

O inventário não encontrou um watchdog/recovery SGX no gma500 atual. Isso não
prova ausência de watchdog da plataforma. Seu estado é **UNKNOWN**. O reset
`psb_spank()` toca BIF, DPM, TA, USE, ISP, TSP e 2D
(`psb_drv.c:97-125`), portanto não é mecanismo de recuperação aprovado. Nunca
presumir que “reset SGX” recupera display, kernel ou máquina.

## Após reboot

Antes de iniciar novo teste:

1. guardar `journalctl -k -b -1` quando journald persistente existir;
2. copiar `/sys/fs/pstore/*` sem apagá-lo durante a primeira coleta;
3. registrar motivo e método do reboot/power-cycle;
4. coletar Test Vector Zero novamente e comparar PCI/BAR/IRQ/driver/PM;
5. confirmar integridade de filesystem e boot no kernel esperado;
6. arquivar stdout/stderr, commit do harness, kernel config e logs remotos.

