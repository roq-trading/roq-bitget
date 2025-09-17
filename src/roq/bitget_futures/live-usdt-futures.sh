#!/usr/bin/env bash

CWD="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"

if [ "$1" == "debug" ]; then
  PREFIX="gdb --args"
else
  PREFIX=
fi

NAME="bitget-futures"

CONFIG="${CONFIG:-$NAME}"

CONFIG_FILE="$ROQ_CONFIG_PATH/roq-bitget-futures/$CONFIG.toml"

FLAG_FILE="../../../share/flags/prod/flags.cfg"

API="USDT-FUTURES"

MARGIN_COIN="USDT"

$PREFIX ./roq-bitget-futures \
  --name "$NAME" \
  --config_file "$CONFIG_FILE" \
  --flagfile "$FLAG_FILE" \
  --api "$API" \
  --margin_coin "$MARGIN_COIN" \
  --cache_dir "$HOME/var/lib/roq/cache" \
  --event_log_dir "$HOME/var/lib/roq/data" \
  --event_log_symlink true \
  --client_listen_address "$HOME/run/$NAME.sock" \
  --service_listen_address "$HOME/run/metrics/${NAME}.sock" \
  $@
