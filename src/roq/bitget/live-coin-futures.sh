#!/usr/bin/env bash

CWD="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"

if [ "$1" == "debug" ]; then
  PREFIX="gdb --args"
else
  PREFIX=
fi

NAME="bitget"

CONFIG="${CONFIG:-$NAME}"

CONFIG_FILE="$ROQ_CONFIG_PATH/roq-bitget/$CONFIG.toml"

FLAG_FILE="../../../share/flags/prod/flags.cfg"

API="COIN-FUTURES"

MARGIN_COIN="USDT"

$PREFIX ./roq-bitget \
  --name "$NAME" \
  --config_file "$CONFIG_FILE" \
  --flagfile "$FLAG_FILE" \
  --api "$API" \
  --cache_dir "$HOME/var/lib/roq/cache" \
  --event_log_dir "$HOME/var/lib/roq/data" \
  --event_log_symlink true \
  --client_listen_address "$HOME/run/$NAME.sock" \
  --service_listen_address "$HOME/run/metrics/${NAME}.sock" \
  $@
