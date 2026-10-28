build:
  cmake -S . -B build
  cmake --build build --parallel {{num_cpus()}}

buildrun:
  just build
  just run

run:
  ./build/email_client

clean:
  rm -rf build

# Local test mail server, see dev/mail-server.sh
mail-up:
  ./dev/mail-server.sh up

mail-down:
  ./dev/mail-server.sh down

mail-send *args:
  ./dev/mail-server.sh send {{args}}

# Run the client against the local test mail server, with a separate config
dev-run:
  ./dev/mail-server.sh run
