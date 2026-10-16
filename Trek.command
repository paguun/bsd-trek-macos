#!/bin/zsh
emulate -LR zsh
exec /bin/zsh "${0:A:h}/run-trek.zsh" "$@"
