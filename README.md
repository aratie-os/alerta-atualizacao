# System Upgrade

`system-upgrade` e um alerta de atualizacoes para Kubuntu 26.04 com KDE
Plasma 6. A aplicacao usa C++ e Qt 6 para consultar atualizacoes APT e Flatpak
no inicio da sessao e abrir o KDE Discover quando o usuario decide atualizar.

```text
Login no Plasma -> APT + Flatpak -> sem atualizacoes: encerra
                                -> com atualizacoes: alerta -> Discover
```

O projeto consulta instalacoes Flatpak de sistema e de usuario. Snap e seu
backend para o Discover nao sao utilizados deliberadamente.

Pacotes oficiais devem ser baixados pela pagina de
[GitHub Releases](https://github.com/aratie-os/alerta-atualizacao/releases).
Instrucoes de compilacao, arquitetura e empacotamento estao no
[README tecnico](alerta-atualizacoes/README.md).

Licenciado sob a GNU General Public License, versao 3 ou posterior.
