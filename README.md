# GitHub Webhook Handler

## Simple C++ WebAPI to work with GitHub Webhooks

Currently creating a local copy of remote files on every push

## Usage

### Run prebuilt binary

Head over to the [Releases Page](https://github.com/TiagoRG/gh-wh-handler/releases) and download the desired binary.

Run the application using your configuration file:
```console
/path/to/gh-wh-handler.<arch> /path/to/config.json
```

You can see the config file format below.

### Run from source

Install the dependencies:

- [CrowCpp](https://crowcpp.org/master/)
- [nlohmann::json](https://github.com/nlohmann/json)

Build the application:
```console
cmake -S . -B build
cmake --build build
```

Run the application using your configuration file:
```console
bin/gh-wh-handler.<arch> /path/to/config.json
```

Note: default config file path is `/etc/gh-wh-handler/config.json`

## Config File

The configuration file should be a JSON file with the following format:

```json
{
    "port": 8080,
    "repos": {
        "repo_full_name": {
            "branch": "target_branch",
            "files": {
                "remote_path": "local_path"
            }
        }
    },
    "tokens": {
        "repo_full_name": "github_token"
    },
    "actions": {
        "repo_full_name": [
            "command1",
            "command2"
        ]
    }
}
```

## Endpoint

Currently, the only endpoint for the application is `/update-files`
