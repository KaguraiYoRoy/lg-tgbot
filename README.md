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
        "token": "<Your Telegram Bot Token>",
        "debug": false,
        "nodes":[
                {
                        "name": "<Node1 name>",
                        "url": "<Node1 agent http://host:port>"
                },
                {
                        "name": "<Node2 name>",
                        "url": "<Node2 agent http://host:port>"
                }
        ]
}
```