# Weather CLI (C)

Simple command-line weather tool in basic C that calls a web service (`wttr.in`) and prints current conditions by ZIP code.

## Requirements

- C compiler (`cc`/`gcc`/`clang`)
- `libcurl` development package

On Debian/Ubuntu:

```bash
sudo apt update
sudo apt install -y build-essential libcurl4-openssl-dev
```

## Build

```bash
make
```

## Run

```bash
./weather 10001
```

Specify country code (default is `US`):

```bash
./weather 10001 CA
```

JSON mode:

```bash
./weather --json 10001 US
```

Example output:

```text
ZIP 10001 (US): Partly cloudy +46°F
```

## Notes

- ZIP validation is basic (5 digits).
- Country code supports 2-3 letters (for example: `US`, `CA`, `GBR`).
- Uses this endpoint pattern:
  - Text: `https://wttr.in/<ZIP>,<COUNTRY>?format=%C+%t`
  - JSON: `https://wttr.in/<ZIP>,<COUNTRY>?format=j1`
