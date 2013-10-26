# forza
The Monitoring Agent.

## Build

### Pull in third-party submodules

	git submodule init
	git submodule update

### `./configure` options

  * `--with-plugin <plugin>` - enable plugin `<plugin>` (see Plugins section)
  * `--interposed-dest-cpu` - CPU architecture to build `libinterposed` for. Valid values are: ia32 and x64.

```bash
./configure
make
```

## Usage
```bash
./forza -h 127.0.0.1 -p 1337 -- node test/fixtures/listen.js
```

## Plugins

### `uptime`

Sends:

```json
{
  "metric": <seconds of process uptime>,
  "service": "health/process/uptime"
}
```

### `mem`

Sends:

```json
{
  "metric": <used-memory-on-the-machine>,
  "service": "health/machine/memory"
}
```

### `logs`

Sends:

```json
{
  "metric": 1.0,
  "service": "logs/stdout",
  "description": <log-message>
}
```

## Testing

Tests require all plugins:

```bash
./build
npm test
```
