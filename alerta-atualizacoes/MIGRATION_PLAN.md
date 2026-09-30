# Arquitetura atual

A versao 2.0.1 mantem a aplicacao nativa Qt 6 e corrige o fluxo de login para
ser estritamente efemero: checar, alertar apenas quando necessario e encerrar.

## Fluxo

1. `startup.cpp` interpreta `/proc/cmdline`; `boot=casper` encerra antes de toda
   inicializacao da sessao instalada.
2. `UpdateChecker` executa sequencialmente o refresh APT, a consulta APT e as
   consultas Flatpak system/user com `QProcess` assincrono.
3. `UpdateState` separa sucesso das consultas e existencia de atualizacoes.
4. `main.cpp` cria `UpdateNotification` somente quando `hasUpdates()` e true.
5. `UpdateNotification` usa LayerShellQt no Wayland e `availableGeometry()` no
   X11; o Discover e aberto apenas por acao do usuario.

Nao ha daemon, tray, notificacao passiva, atualizacao automatica, telemetria ou
persistencia. O helper shell legado e mantido somente em `legacy/` para auditoria
e nao faz parte do pacote binario.
