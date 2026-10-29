build:
  cmake -S . -B build
  cmake --build build --parallel {{num_cpus()}}

buildrun:
  just build
  just run

# Run the client with your real config and the system CA certificates
run:
  ./build/email_client

# Build with the unit tests and run them; extra args go to ctest, e.g. `just test -R HtmlParser`
test *args:
  cmake -S . -B build -DBUILD_TESTS=ON
  cmake --build build --parallel {{num_cpus()}}
  ctest --test-dir build/tests --output-on-failure {{args}}

clean:
  rm -rf build

# Local test mail server, see dev/mail-server.sh
mail-up:
  ./dev/mail-server.sh up

mail-down:
  ./dev/mail-server.sh down

mail-send *args:
  ./dev/mail-server.sh send {{args}}

# Run the client against the local test mail server (started if needed),
# with a separate config and the dev CA
dev-run:
  ./dev/mail-server.sh run
