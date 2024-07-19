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

### Install from source

#### Install the dependencies:

- [CrowCpp](https://crowcpp.org/master/)
- [nlohmann::json](https://github.com/nlohmann/json)

#### Build the application:

1. Clone the repository:

```console
git clone https://github.com/TiagoRG/gh-wh-handler.git
```

2. Move to the build directory:
```console
cd gh-wh-handler/build
```

3. Run CMake:
```console
cmake ..
```

4. Build and install the application:
```console
sudo make install
```

#### Run the application:

The application is running on a systemd service, which is both enabled and started after installation.

You can start, stop, restart, and check the status of the service using the following commands:

```console
sudo systemctl start gh-wh-handler      # Start the service
sudo systemctl stop gh-wh-handler       # Stop the service
sudo systemctl restart gh-wh-handler    # Restart the service
sudo systemctl status gh-wh-handler     # Check the status of the service
```

You can also check the logs of the service using the following command:

```console
journalctl -u gh-wh-handler
```

## Configuration

As of now, the configuration menu is not yet implemented so you have to create the configuration file manually.

### Config File

The configuration file can be found in `/services/gh-wh-handler/config.json` and has the following format:

```json
{
  "port": 65001,
  "update-files": {
    "owner/repo-name": {
      "branch": "main",
      "files": {
        "path/to/remote/file": "/path/to/local/file",
        "...": "..."
      },
      "post-update": [
        "post-update-command",
        "post-update-script",
        "..."
      ]
    }
  },
  "run-scripts": {
    "owner/repo-name": {
      "branch": "main",
      "actions": [
        "command",
        "script",
      ]
    }
  },
  "tokens": {
    "owner/repo-name": "token"
  }
}
```

## Endpoints

Currently, the only endpoint for the application is `/update-files`, which is used to update the local files on every push as well as run post-update scripts.

Since only the `/update-files` endpoint is implemented, the configuration file may not contain the `run-scripts` field.

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.


