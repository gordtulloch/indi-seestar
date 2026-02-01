# cpp-switchbot

A C++ library to control SwitchBot devices connected to SwitchBot Hub. This is a port of the [python-switchbot](https://github.com/OpenWonderLabs/python-switchbot) library.

## Requirements

- C++17 or later
- CMake 3.15+
- libcurl
- OpenSSL
- nlohmann/json (automatically fetched if not found)
- [A SwitchBot Token](https://github.com/OpenWonderLabs/SwitchBotAPI#getting-started)

## Installation

### Install Dependencies (Ubuntu/Debian)

```bash
sudo apt-get install build-essential cmake libcurl4-openssl-dev libssl-dev
```

### Install Dependencies (Fedora/RHEL)

```bash
sudo dnf install gcc-c++ cmake libcurl-devel openssl-devel
```

### Install Dependencies (macOS)

```bash
brew install cmake curl openssl
```

### Build the Library

```bash
mkdir build
cd build
cmake ..
make
```

### Install System-Wide (Optional)

```bash
sudo make install
```

## Usage

### Basic Example

```cpp
#include <iostream>
#include <switchbot/switchbot.hpp>

int main() {
    // To get the token and secret, please refer to:
    // https://github.com/OpenWonderLabs/SwitchBotAPI#getting-started
    std::string token = "YOUR_SWITCHBOT_TOKEN";
    std::string secret = "YOUR_SWITCHBOT_SECRET";
    
    switchbot::SwitchBot sb(token, secret);
    
    // List all devices
    auto devices = sb.devices();
    for (const auto& device : devices) {
        std::cout << device->to_string() << std::endl;
    }
    // Output:
    // Bot(id=CD0A18B1C291)
    // SmartLock(id=CD0A1221C291)
    // HubMini(id=4CAF08629A21)
    
    return 0;
}
```

### Working with Devices

```cpp
// Get a specific device by ID
auto device = sb.device("5F0B798AEF91");

// Get device status
auto status = device->status();
std::cout << status.dump(2) << std::endl;
// Output: {"power": "off"}

// Send commands
device->command("turn_on");
device->command("turn_off");
device->command("press");
```

### Working with Bot Devices

```cpp
auto devices = sb.devices();
for (auto& device : devices) {
    if (device->get_type() == "Bot") {
        auto bot = dynamic_cast<switchbot::Bot*>(device.get());
        if (bot) {
            bot->turn("on");
            bot->turn("off");
            bot->press();
            bot->toggle();  // Toggle based on current state
        }
    }
}
```

### Working with Lock Devices

```cpp
auto device = sb.device("YOUR_LOCK_ID");
if (device->get_type() == "Smart Lock") {
    auto lock = dynamic_cast<switchbot::Lock*>(device.get());
    if (lock) {
        lock->lock();
        lock->unlock();
        lock->toggle();  // Toggle based on current state
    }
}
```

### Working with Remotes

```cpp
// List all remotes
auto remotes = sb.remotes();
for (const auto& remote : remotes) {
    std::cout << remote->to_string() << std::endl;
}

// Control a remote
auto remote = sb.remote("YOUR_REMOTE_ID");
remote->command("turn_on");
remote->command("turn_off");
```

### Working with Scenes

```cpp
// List all scenes
auto scenes = sb.scenes();
for (const auto& scene : scenes) {
    std::cout << scene.to_string() << std::endl;
}

// Execute a scene
auto scene = sb.scene("YOUR_SCENE_ID");
scene.execute();
```

## Linking with Your Project

### Using CMake

```cmake
find_package(switchbot REQUIRED)
target_link_libraries(your_target PRIVATE switchbot::switchbot)
```

### Manual Linking

```bash
g++ -std=c++17 your_app.cpp -lswitchbot -lcurl -lssl -lcrypto
```

## API Documentation

### Main Classes

- **`SwitchBot`**: Main API wrapper class
  - `devices()`: Get all devices
  - `device(id)`: Get a specific device by ID
  - `remotes()`: Get all remotes
  - `remote(id)`: Get a specific remote by ID
  - `scenes()`: Get all scenes
  - `scene(id)`: Get a specific scene by ID

- **`Device`**: Base class for all devices
  - `status()`: Get device status
  - `command(action, parameter)`: Send command to device
  - Specialized subclasses: `Bot`, `Curtain`, `Lock`

- **`Remote`**: Base class for all remotes
  - `command(action, parameter, customize)`: Send command to remote
  - Specialized subclasses: `SupportedRemote`, `OtherRemote`

- **`Scene`**: Scene class
  - `execute()`: Execute the scene

## Building the Example

```bash
cd build
./switchbot_example
```

Remember to replace `YOUR_SWITCHBOT_TOKEN` and `YOUR_SWITCHBOT_SECRET` in the example.

## Differences from Python Version

1. **Memory Management**: Uses smart pointers (`std::unique_ptr`, `std::shared_ptr`) for automatic memory management
2. **Error Handling**: Uses exceptions instead of Python's exception model
3. **Type System**: Uses C++ static typing with inheritance for device specialization
4. **JSON Handling**: Uses nlohmann/json library for JSON operations
5. **HTTP Client**: Uses libcurl instead of Python's requests library

## License

This project is licensed under the same terms as the original python-switchbot library.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## Acknowledgments

- Original Python library: [python-switchbot](https://github.com/OpenWonderLabs/python-switchbot)
- SwitchBot API: [SwitchBot API Documentation](https://github.com/OpenWonderLabs/SwitchBotAPI)
