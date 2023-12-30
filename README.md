# GitHub Webhook Handler

## Simple C++ WebAPI to work with GitHub Webhooks

Currently creating a local copy of remote files on every push

## Usage

Compile the application:
```console
cmake .
make
```

Run the application using your configuration file:
```console
./gh-wh-handler /path/to/config.json
```

## Config File

The configuration file should be a JSON file with the following format:

```json
{
    "port": 8080,
    "repos": {
        "repo_full_name": {
            "branch": "target_branch",
            "files": {
                "remote_path": "local_path",
            }
        }
    },
    "tokens": {
        "repo_full_name": "github_token"
    }
}
```
