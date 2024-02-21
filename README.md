# GitHub Webhook Handler

## Simple C++ WebAPI to work with GitHub Webhooks

Currently creating a local copy of remote files on every push

## Usage

Install the dependencies:

- [CrowCpp](https://crowcpp.org/master/)
- [nlohmann::json](https://github.com/nlohmann/json)

Compile the application:
```console
make
```

Run the application using your configuration file:
```console
./gh_wh_handler /path/to/config.json
```

Note: default config file path is `/etc/gh_wh_handler/config.json`

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

Currently, the only endpoint for the application is /update-files
