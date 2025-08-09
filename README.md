# lg-tgbot
A simple Looking-Glass for BIRD2 based on Telegram Bot

# Require
## Development
```bash
sudo apt install g++ make binutils cmake libboost-system-dev libssl-dev zlib1g-dev libcurl4-openssl-dev traceroute bc tcptraceroute whois
```

## Production
```bash
sudo apt install traceroute bc tcptraceroute whois
```

> [!Important]
> You need to install `tcping` manually.

# Config Sample
## Master
```json
{
    "autodelete":30,
    "default-server":0,
    "timeout-connection":300,
    "timeout-read":5000,
    "timeout-all":5000,
    "loglevel-print":0,
    "loglevel-write":0,
    "token": "<Your Telegram Bot Token>",
    "nodes":[
        {//id: 0
            "name": "<Node1 name>",
            "url": "<Node1 agent http://host:port>",
            "uuid": "<Node1 UUID>"
        },
        {//id: 1
            "name": "<Node2 name>",
            "url": "<Node2 agent http://host:port>",
            "uuid": "<Node2 UUID>"
        }
        // etc.
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
    "uuid":"<UUID. Empty to auto generate>",
    "whois-server":"whois.akae.re"
}
```

## Parameters

_Parameter which has a **default** value is **optional**._

| Parameter | Description | Default |
| --- | --- | --- |
| `autodelete` | Auto delete message timeout (unit:second). Set `0` to disable auto delete | `30` |
| `default-server` | The ID of default server. Default server will be used to process `whois`. The ID starts from 0 according to the node configuration sequence  | `0` |
| `loglevel-print` | The level of logs printed to the console | `3` |
| `loglevel-write` | The level of logs recorded in the log file | `4` |
| `timeout-connection` | Maximum time to connect to an agent (unit: milliseconds) | `300` |
| `timeout-read` | Maximum time to read from an agent (unit: milliseconds) | `3000` |
| `timeout-all` | The maximum time during the entire connection process (unit: milliseconds) | `5000` |
| `whois-server` | Whois server address | `whois.akae.re` |

## Log Level table
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