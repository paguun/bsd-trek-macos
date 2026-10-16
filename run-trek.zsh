#!/bin/zsh
# Compile if needed, then run the original C game in this directory.
emulate -LR zsh
setopt ERR_EXIT NO_UNSET PIPE_FAIL
trek_dir=${0:A:h}
cd -- "$trek_dir"

if [[ $(uname -s) == Darwin ]] && ! /usr/bin/xcode-select -p >/dev/null 2>&1; then
    print -u2 -- 'Apple Command Line Tools が必要です。Terminalで次を実行してください：'
    print -u2 -- '  xcode-select --install'
    print -u2 -- 'インストール後、もう一度このスクリプトを実行してください。'
    exit 1
fi
if ! command -v make >/dev/null 2>&1 || ! command -v cc >/dev/null 2>&1; then
    print -u2 -- 'Cコンパイラとmakeが必要です。Macでは xcode-select --install を実行してください。'
    exit 1
fi
make -s all
exec "$trek_dir/bin/trek" "$@"
