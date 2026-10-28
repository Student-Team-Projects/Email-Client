#!/usr/bin/env bash
# Local test mail server (GreenMail in a container) for developing the client.
#
# Certificates are issued by a local dev CA instead of being self-signed, so the
# client can verify them properly: `run` points it at the CA via SSL_CERT_FILE.
# Nothing has to be added to the system trust store.
#
# Usage: dev/mail-server.sh <command>
#   up                   start the server (generates certificates on first use)
#   down                 stop the server (all mail is lost, GreenMail keeps it in memory)
#   status               show whether the server is running
#   logs                 follow server logs
#   send [to] [subject]  send a test mail from bob (default: to alice@localhost)
#   run [args...]        run ./build/email_client against the server (starts it if needed),
#                        with a separate config
#   reset                remove the client config and cache used by `run`
#   certs                regenerate certificates

set -euo pipefail

DEV_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$DEV_DIR")"
CERT_DIR="$DEV_DIR/.certs"
STATE_DIR="$DEV_DIR/.state"

CONTAINER=email-client-greenmail
IMAGE=greenmail/standalone:2.1.2
KEYSTORE_PASSWORD=changeit

SMTPS_PORT=3465
IMAPS_PORT=3993
# name:password pairs, the login is name@localhost
USERS=(alice:alice bob:bob)

RUNTIME="${CONTAINER_RUNTIME:-$(command -v docker || command -v podman || true)}"

die() { echo "error: $*" >&2; exit 1; }

need_runtime() {
  [[ -n "$RUNTIME" ]] || die "docker or podman is required (or set CONTAINER_RUNTIME)"
}

is_running() {
  [[ "$("$RUNTIME" inspect -f '{{.State.Running}}' "$CONTAINER" 2>/dev/null)" == true ]]
}

system_ca_bundle() {
  # Same locations as src/backend/certificate.h
  for path in /etc/ssl/cert.pem /etc/ssl/certs/ca-certificates.crt \
              /etc/pki/tls/certs/ca-bundle.crt /etc/ssl/ca-bundle.pem; do
    [[ -f "$path" ]] && { echo "$path"; return; }
  done
}

gen_certs() {
  command -v openssl >/dev/null || die "openssl is required"
  rm -rf "$CERT_DIR"
  mkdir -p "$CERT_DIR"
  (
    cd "$CERT_DIR"
    openssl req -x509 -newkey rsa:2048 -nodes -days 3650 -subj "/CN=Email-Client Dev CA" \
      -addext "basicConstraints=critical,CA:TRUE" -addext "keyUsage=critical,keyCertSign,cRLSign" \
      -keyout ca.key -out ca.pem 2>/dev/null

    openssl req -newkey rsa:2048 -nodes -subj "/CN=localhost" \
      -keyout server.key -out server.csr 2>/dev/null
    printf '%s\n' "subjectAltName=DNS:localhost,IP:127.0.0.1" "basicConstraints=CA:FALSE" \
      "keyUsage=critical,digitalSignature,keyEncipherment" "extendedKeyUsage=serverAuth" > server.ext
    openssl x509 -req -in server.csr -CA ca.pem -CAkey ca.key -CAcreateserial -days 825 \
      -extfile server.ext -out server.pem 2>/dev/null

    openssl pkcs12 -export -in server.pem -inkey server.key -certfile ca.pem -name greenmail \
      -passout "pass:$KEYSTORE_PASSWORD" -out greenmail.p12
    # The container runs as a different user and has to read it
    chmod 644 greenmail.p12

    # System roots + dev CA, so other accounts keep working during `run`
    local bundle
    bundle="$(system_ca_bundle)"
    { [[ -n "$bundle" ]] && cat "$bundle"; cat ca.pem; } > ca-bundle.pem
    rm -f server.csr server.ext ca.srl
  )
  echo "Certificates generated in $CERT_DIR"
}

ensure_certs() {
  if [[ ! -f "$CERT_DIR/greenmail.p12" || ! -f "$CERT_DIR/ca-bundle.pem" ]] ||
     ! openssl x509 -checkend 86400 -noout -in "$CERT_DIR/server.pem" >/dev/null 2>&1; then
    gen_certs
  fi
}

wait_for_server() {
  for _ in $(seq 1 60); do
    if curl -s --cacert "$CERT_DIR/ca.pem" "imaps://localhost:$IMAPS_PORT/" \
         --user alice@localhost:alice >/dev/null 2>&1; then
      return 0
    fi
    sleep 0.5
  done
  die "server did not come up, check: $0 logs"
}

print_info() {
  cat <<EOF
GreenMail is running.
  SMTP  smtps://localhost:$SMTPS_PORT
  IMAP  imaps://localhost:$IMAPS_PORT
  Accounts (login / password):
EOF
  for user in "${USERS[@]}"; do
    echo "    ${user%%:*}@localhost / ${user#*:}"
  done
  echo "  CA certificate: $CERT_DIR/ca.pem"
}

cmd_up() {
  need_runtime
  ensure_certs
  if is_running; then
    print_info
    return
  fi
  "$RUNTIME" rm -f "$CONTAINER" >/dev/null 2>&1 || true

  local users=()
  for user in "${USERS[@]}"; do users+=("${user%%:*}:${user#*:}@localhost"); done

  "$RUNTIME" run -d --rm --name "$CONTAINER" \
    -v "$CERT_DIR/greenmail.p12:/certs/greenmail.p12:ro" \
    -e GREENMAIL_OPTS="-Dgreenmail.setup.test.all -Dgreenmail.hostname=0.0.0.0 \
-Dgreenmail.tls.keystore.file=/certs/greenmail.p12 -Dgreenmail.tls.keystore.password=$KEYSTORE_PASSWORD \
-Dgreenmail.users=$(IFS=,; echo "${users[*]}") -Dgreenmail.users.login=email" \
    -p 127.0.0.1:3025:3025 -p 127.0.0.1:3143:3143 \
    -p "127.0.0.1:$SMTPS_PORT:3465" -p "127.0.0.1:$IMAPS_PORT:3993" \
    "$IMAGE" >/dev/null
  wait_for_server
  # A fresh server has empty mailboxes, so the client cache would be stale
  rm -rf "$STATE_DIR/data"
  print_info
}

cmd_down() {
  need_runtime
  "$RUNTIME" stop "$CONTAINER" >/dev/null 2>&1 && echo "Stopped." || echo "Not running."
}

cmd_status() {
  need_runtime
  if is_running; then print_info; else echo "Not running."; fi
}

cmd_logs() {
  need_runtime
  "$RUNTIME" logs -f "$CONTAINER"
}

cmd_send() {
  local to="${1:-alice@localhost}"
  local subject="${2:-Test $(date +%H:%M:%S)}"
  is_running || die "server is not running, start it with: $0 up"
  printf 'From: bob@localhost\r\nTo: %s\r\nSubject: %s\r\nDate: %s\r\n\r\nHello from %s!\r\n' \
    "$to" "$subject" "$(date -R)" "$0" |
    curl -sS --cacert "$CERT_DIR/ca.pem" "smtps://localhost:$SMTPS_PORT" \
      --user bob@localhost:bob --mail-from bob@localhost --mail-rcpt "$to" --upload-file -
  echo "Sent \"$subject\" to $to"
}

write_client_config() {
  local config="$STATE_DIR/config/email_client/config.json"
  [[ -f "$config" ]] && return
  mkdir -p "$(dirname "$config")"
  local entries=()
  for user in "${USERS[@]}"; do
    entries+=("$(printf '  {"username": "%s@localhost", "password": "%s", "smtpHost": "smtps://localhost:%s", "imapHost": "imaps://localhost:%s"}' \
      "${user%%:*}" "${user#*:}" "$SMTPS_PORT" "$IMAPS_PORT")")
  done
  { echo "["; (IFS=$'\n'; echo "${entries[*]}" | sed '$!s/$/,/'); echo "]"; } > "$config"
}

cmd_run() {
  local bin="$ROOT_DIR/build/email_client"
  [[ -x "$bin" ]] || die "$bin not found, build it first: just build"
  is_running || cmd_up
  write_client_config
  XDG_CONFIG_HOME="$STATE_DIR/config" XDG_DATA_HOME="$STATE_DIR/data" \
    SSL_CERT_FILE="$CERT_DIR/ca-bundle.pem" exec "$bin" "$@"
}

cmd_reset() {
  rm -rf "$STATE_DIR"
  echo "Removed $STATE_DIR"
}

case "${1:-}" in
  up)     cmd_up ;;
  down)   cmd_down ;;
  status) cmd_status ;;
  logs)   cmd_logs ;;
  send)   shift; cmd_send "$@" ;;
  run)    shift; cmd_run "$@" ;;
  reset)  cmd_reset ;;
  certs)  gen_certs ;;
  *)      sed -n '2,18p' "$0" | sed 's/^# \{0,1\}//'; exit 1 ;;
esac
