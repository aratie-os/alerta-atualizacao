# Auditoria do pacote original

Origem auditada: `debian-binary`, `control.tar.xz` e `data.tar.xz` do pacote
original. Depois da conclusao da auditoria, esses artefatos e o conteudo extraido
foram removidos da arvore atual; permanecem disponiveis no historico Git.

## Metadados Debian

- Pacote: `system-upgrade`
- Versao: `1.0.4`
- Arquitetura: `all`
- Secao: `admin`
- Prioridade: `optional`
- Depends: `yad`
- Maintainer: `Elton Fabricio <https://github.com/eltonfabricio10>`
- Descricao: `Update notifier to Debian based`

O pacote nao contem codigo-fonte original. Ele e uma montagem binaria simples com dois scripts shell, dois arquivos `.desktop` e uma regra sudoers.

## Arquivos instalados

Arquivos em `data.tar.xz`, todos com dono original `root/root`:

- `/usr/bin/system-upgrade`, modo `0755`: script Bash principal.
- `/usr/bin/system-upgrade-exe`, modo `0755`: helper Bash executado via `sudo`.
- `/usr/share/applications/system-upgrade.desktop`, modo `0755`: launcher manual.
- `/etc/xdg/autostart/system-upgrade.desktop`, modo `0755`: autostart de sessao.
- `/etc/sudoers.d/systemupdate`, modo `0755`: regra sudoers.

Nao foram encontrados binarios ELF, servicos systemd, timers, cron, icones proprios, arquivos de configuracao persistente, scripts `preinst`, `postinst`, `prerm`, `postrm` ou `conffiles`.

## Inicializacao

O programa inicia automaticamente via XDG Autostart:

```ini
[Desktop Entry]
Type=Application
Terminal=false
Name=System Upgrade
Exec=/usr/bin/system-upgrade
Icon=utilities-terminal
```

Tambem ha um launcher de menu:

```ini
Exec=/usr/bin/system-upgrade --gui
Icon=system-software-update
Categories=Applications;System
```

Sem `--gui`, ele roda em modo silencioso no login. Com `--gui`, mostra dialogs YAD.

## Funcionamento do script principal

`/usr/bin/system-upgrade`:

1. Testa conectividade com `ping -c 1 google.com | grep [0-9]`.
2. Sai sem acao em sistemas live se `/rofs` existir.
3. Se nao houver ping:
   - com `--gui`, mostra dialog YAD "Sem conexao com a internet";
   - sem `--gui`, sai silenciosamente.
4. Se houver rede:
   - executa `sudo /usr/bin/system-upgrade-exe`;
   - le `/tmp/system-update/save`;
   - tenta detectar atualizacoes procurando `[kM]B` nas ultimas linhas;
   - monta textos com `cat`, `tail`, `awk`, `sed` e parsing dependente de idioma;
   - se houver atualizacoes, mostra dialog YAD perguntando se deseja instalar;
   - se o usuario aceitar, executa `plasma-discover --mode=Update`;
   - se nao houver atualizacoes, so mostra "O sistema esta atualizado" quando chamado com `--gui`.

Durante `--gui`, o script envolve a funcao de verificacao em um pipe para `yad --progress --pulsate`.

## Helper privilegiado

`/usr/bin/system-upgrade-exe`:

```bash
[[ -d /tmp/system-update ]] && rm -rf /tmp/system-update
mkdir -p /tmp/system-update
apt-get update
apt-get -u dist-upgrade --assume-no >> /tmp/system-update/save
sed -i '1,4d' /tmp/system-update/save
sed -i '$d' /tmp/system-update/save
sed -i '$d' /tmp/system-update/save

FLAT_UP="$(LANG=C flatpak update|tail -n1)"
[ "$FLAT_UP" != "Nothing to do." ] && > /tmp/system-update/flatpak
```

Ele atualiza os indices APT, simula `dist-upgrade`, grava saida textual em `/tmp/system-update/save` e tenta detectar Flatpak criando `/tmp/system-update/flatpak`. O script principal nao usa esse marcador Flatpak, portanto essa deteccao nao produz comportamento visivel.

## Privilegios

O pacote instala:

```sudoers
ALL ALL=NOPASSWD: /usr/bin/system-upgrade-exe
```

Isso permite que qualquer usuario execute o helper como root sem senha. Embora o caminho do helper seja fixo, o helper remove recursivamente `/tmp/system-update` como root e grava arquivos em `/tmp`, o que cria uma superficie de ataque desnecessaria.

## Estado e configuracao

Nao ha configuracao por usuario ou global. O unico estado temporario fica em `/tmp/system-update`, com arquivos:

- `save`: saida textual editada de `apt-get -u dist-upgrade --assume-no`.
- `flatpak`: marcador criado mas nao consumido.

Nao ha uso de XDG config/cache/data.

## Comportamento por cenario

- Sem atualizacoes:
  - modo autostart: sai silenciosamente;
  - modo `--gui`: mostra "O sistema esta atualizado!".
- Com atualizacoes:
  - mostra dialog YAD perguntando se deseja instalar;
  - ao aceitar, abre `plasma-discover --mode=Update`.
- Sem conexao:
  - modo autostart: sai silenciosamente;
  - modo `--gui`: mostra erro YAD.
- APT/dpkg ocupados:
  - nao ha tratamento especifico; erros do `apt-get` entram ou deixam de entrar em `/tmp/system-update/save`, podendo produzir resultado incorreto.
- PackageKit/Discover rodando:
  - nao ha integracao; o script chama APT diretamente.
- Residente/tray:
  - nao fica residente;
  - nao possui icone de bandeja.
- Notificacoes:
  - nao usa o sistema de notificacoes; usa janelas YAD sem decoracao.

## Problemas encontrados

- GUI baseada em shell/YAD, nao Qt.
- Usa `sudo` dentro da aplicacao.
- Instala NOPASSWD sudoers para todos os usuarios.
- Executa operacoes de APT como root a partir de uma GUI nao estruturada.
- Usa `/tmp/system-update` com `rm -rf` como root.
- Faz parsing de saida humana e localizada de `apt-get`.
- Usa `ping google.com` como teste de conectividade.
- Nao trata locks de APT/dpkg corretamente.
- Nao trata erros de rede/repositorios de forma confiavel.
- Nao usa PackageKit/PolicyKit.
- Nao usa notificacoes Plasma.
- Nao possui single-instance.
- Arquivos `.desktop` instalados como executaveis e categorias inadequadas.
- Sem testes, sem CMake, sem fonte, sem empacotamento Debian de fonte.

## Conclusao funcional

A finalidade util do programa original e verificar atualizacoes de sistema para uma distribuicao Debian/Ubuntu, avisar o usuario quando houver atualizacoes e abrir o Discover na pagina de atualizacoes. A reimplementacao deve preservar esse fluxo, removendo o helper root, a regra sudoers, o uso direto de APT pela GUI, o parsing textual e o uso inseguro de `/tmp`.
