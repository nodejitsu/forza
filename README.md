# godot
THe Monitoring Agent.

## Build

### `./configure options`

  * `--with-plugin <plugin>` - enable plugin `<plugin>` (see Plugins section)

```bash
./configure
make
```

## Usage
```bash
./estragon -h 127.0.0.1:1337 -- node test/fixtures/listen.js
```

## Plugins

### `uptime`

Sends:

```json
{
  "metric": <seconds of process uptime>,
  "host": "pc09.local",
  "service": "health/process/uptime"
}
```

### `memory`

Sends:

```json
{
  "metric": <used-memory-on-the-machine>,
  "host": "pc09.local",
  "service":"health/machine/memory"
}
```

## Testing

Tests require all plugins:

```bash
./configure --with-plugin uptime --with-plugin mem --with-plugin cpu --with-plugin process --with-plugin heartbeat --with-plugin port
npm test
```
