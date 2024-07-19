# GitHub Webhook Handler

## Simple C++ WebAPI to work with GitHub Webhooks

This application is a simple C++ WebAPI that listens for GitHub Webhooks and performs actions based on the received data and configuration.

## Installation

### Use installation script (recommended)

Run the installation script to install the application:

```console
curl -fsSL https://cdn.tiagorg.pt/gh-wh-handler/install.sh | sudo sh
```

You can uninstall the application using the following command:

```console
curl -fsSL https://cdn.tiagorg.pt/gh-wh-handler/uninstall.sh | sudo sh
```

### Run prebuilt binary

Head over to the [Releases Page](https://github.com/TiagoRG/gh-wh-handler/releases) and download the desired binary.

Run the application using your configuration file:
```console
/path/to/gh-wh-handler.<arch> /path/to/config.json /path/to/logs_dir
```

You can see the config file format below.

### Install from source

#### Install the dependencies:

- [CrowCpp](https://crowcpp.org/master/)
- [nlohmann::json](https://github.com/nlohmann/json)

#### Build and install the application:

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

If you want to uninstall the application, you can run the following command:
```console
sudo make uninstall
```

## Usage

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

The configuration file can be found in `/services/gh-wh-handler/config.json` and has the following base format:

```json
{
  "port": 65001,
  "tokens": {
    "owner/repo-name": "token"
  }
}
```

This configuration will then have more fields for each endpoint that you want to configure.

Note: Tokens are only required for private repositories.

## Endpoints

Currently, the only endpoint for the application is `/update-files`, which is used to update the local files on every push as well as run post-update scripts.

### `/update-files`

#### Webhook event: `push`

This endpoint allows the application to update specific files on the server when a push to a specific branch is made. This way, there's no need to manually update the files on the server or to pull the entire repository.

It also allows the application to run post-update scripts after the files are updated.

The configuration file must contain the `update-files` field, which is an object with the following format:

```json
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
}
```

### `/run-actions`

#### Webhook event: `push`

This endpoint allows the application to run specific actions when a push to a specific branch is made. This way, there's no need to manually run the actions on the server.

The configuration file must contain the `run-actions` field, which is an object with the following format:

```json
"run-actions": {
  "owner/repo-name": {
    "branch": "main",
    "actions": [
      {
        "name": "action-name",
        "command": "command-to-run",
        "args": [
          "arg1",
          "arg2",
          "..."
        ]
      }
    ]
  }
}
```

Note: if you don't want to use the `args` field, just leave an empty array such as `"args": []`.

## Nginx Configuration

If you want to use Nginx as a reverse proxy for the application, you can use the following example configuration:

```nginx
server {
    listen 80;
    server_name services.example.com;

    location /gh-wh-handler {
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        add_header Front-End-Https on;

        proxy_headers_hash_max_size 512;
        proxy_headers_hash_bucket_size 64;

        proxy_buffering off;
        proxy_redirect off;
        proxy_max_temp_file_size 0;
        rewrite /gh-wh-handler/(.*) /$1  break;
        proxy_pass http://127.0.0.1:65001;
    }
}
```

This way, you will be able to access the application using the URL `http://services.example.com/gh-wh-handler/end-point`.

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.


