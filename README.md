# lg-tgbot
A simple Looking-Glass for BIRD2 based on Telegram Bot

# Require
```bash
sudo apt install g++ make binutils cmake libboost-system-dev libssl-dev zlib1g-dev libcurl4-openssl-dev traceroute bc tcptraceroute
```

> [!Important]
> You need to install `tcping` manually.

# Config Sample
## Master
```json
{
    "loglevel-print":0,
    "loglevel-write":0,
    "token": "<Your Telegram Bot Token>",
    "nodes":[
        {
            "name": "<Node1 name>",
            "url": "<Node1 agent http://host:port>",
            "uuid": "<Node1 UUID>"
        },
        {
            "name": "<Node2 name>",
            "url": "<Node2 agent http://host:port>",
            "uuid": "<Node2 UUID>"
        }
    ]
}
```

## Agent
```json
{
    "loglevel-print":0,
    "loglevel-write":0,
    "bind":"0.0.0.0",
    "port":8080,
    "uuid":"<UUID. Empty to auto generate>"
}
```

## Parameters
| Parameter | Description |
| --- | --- |
| `loglevel-print` | The level of logs printed to the console |
| `loglevel-write` | The level of logs recorded in the log file |

## Log Level tabel
| Level | Value |
| --- | --- |
| Verbose | 0 |
| Info | 1 |
| Debug | 2 |
| Warn | 3 |
| Error | 4 |
| Fatal | 5 |
| OFF | 6 |

To disable log record, please set level to `OFF`.  
Default `loglevel-print` is **Warn**; Default `loglevel-write` is **Error**.