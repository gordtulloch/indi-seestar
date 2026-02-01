# cpp-switchbot Quick Reference

## Project Structure

```
cpp-switchbot/
├── CMakeLists.txt          # CMake build configuration
├── build.sh                # Build script
├── LICENSE                 # MIT License
├── README.md               # Full documentation
├── .gitignore             # Git ignore file
├── include/switchbot/      # Public headers
│   ├── switchbot.hpp       # Main SwitchBot class
│   ├── switchbot_client.hpp # HTTP client
│   ├── device.hpp          # Device classes
│   ├── remote.hpp          # Remote classes
│   ├── scene.hpp           # Scene class
│   └── utils.hpp           # Utility functions
├── src/                    # Implementation files
│   ├── switchbot.cpp
│   ├── switchbot_client.cpp
│   ├── device.cpp
│   ├── remote.cpp
│   ├── scene.cpp
│   └── utils.cpp
├── examples/               # Example code
│   └── example.cpp
└── python-switchbot/       # Original Python library (reference)
```

## Quick Build & Run

```bash
# Build the project
./build.sh

# Or manually:
mkdir build && cd build
cmake ..
make

# Run example
./build/switchbot_example
```

## Quick API Reference

### Initialize

```cpp
#include <switchbot/switchbot.hpp>

switchbot::SwitchBot sb("YOUR_TOKEN", "YOUR_SECRET");
```

### Devices

```cpp
// List all devices
auto devices = sb.devices();

// Get specific device
auto device = sb.device("DEVICE_ID");

// Get status
auto status = device->status();

// Send command
device->command("turn_on");
```

### Bot Device

```cpp
auto bot = dynamic_cast<switchbot::Bot*>(device.get());
bot->turn("on");
bot->turn("off");
bot->press();
bot->toggle();
```

### Lock Device

```cpp
auto lock = dynamic_cast<switchbot::Lock*>(device.get());
lock->lock();
lock->unlock();
lock->toggle();
```

### Remotes

```cpp
// List all remotes
auto remotes = sb.remotes();

// Get specific remote
auto remote = sb.remote("REMOTE_ID");

// Send command
remote->command("turn_on");
```

### Scenes

```cpp
// List all scenes
auto scenes = sb.scenes();

// Execute scene
auto scene = sb.scene("SCENE_ID");
scene.execute();
```

## Key Differences from Python Version

| Python | C++ |
|--------|-----|
| `requests` library | libcurl |
| `pyhumps` | Custom camelCase/snake_case conversion |
| Dynamic typing | Static typing with inheritance |
| Garbage collection | Smart pointers (unique_ptr, shared_ptr) |
| Lists | std::vector |
| Dicts | nlohmann::json |
| Exceptions | C++ exceptions (std::runtime_error) |

## Dependencies

- **libcurl**: HTTP client library
- **OpenSSL**: Cryptography (HMAC-SHA256)
- **nlohmann/json**: JSON parsing (auto-fetched)

## Common Issues

### Build Errors

1. **Missing libcurl**: `sudo apt-get install libcurl4-openssl-dev`
2. **Missing OpenSSL**: `sudo apt-get install libssl-dev`
3. **CMake too old**: Update to CMake 3.15+

### Runtime Errors

1. **Authentication failed**: Check token and secret
2. **Network errors**: Check internet connection and firewall
3. **Device not found**: Verify device ID is correct

## Testing

Replace the token and secret in `examples/example.cpp`:

```cpp
std::string token = "YOUR_ACTUAL_TOKEN";
std::string secret = "YOUR_ACTUAL_SECRET";
```

Then rebuild and run:

```bash
./build.sh
./build/switchbot_example
```

## Integration with Your Project

### Option 1: Install System-Wide

```bash
cd build
sudo make install
```

Then in your CMakeLists.txt:

```cmake
find_package(switchbot REQUIRED)
target_link_libraries(your_app PRIVATE switchbot::switchbot)
```

### Option 2: Add as Subdirectory

```cmake
add_subdirectory(cpp-switchbot)
target_link_libraries(your_app PRIVATE switchbot)
```

### Option 3: Manual Linking

```bash
g++ -std=c++17 your_app.cpp \
    -I/path/to/cpp-switchbot/include \
    -L/path/to/cpp-switchbot/build \
    -lswitchbot -lcurl -lssl -lcrypto
```

## API Comparison

### Python
```python
from switchbot import SwitchBot

sb = SwitchBot(token=token, secret=secret)
devices = sb.devices()
for device in devices:
    print(device)
    
bot = devices[0]
bot.turn('on')
status = bot.status()
```

### C++
```cpp
#include <switchbot/switchbot.hpp>

switchbot::SwitchBot sb(token, secret);
auto devices = sb.devices();
for (const auto& device : devices) {
    std::cout << device->to_string() << std::endl;
}

auto bot = dynamic_cast<switchbot::Bot*>(devices[0].get());
bot->turn("on");
auto status = bot->status();
```

## Error Handling

All errors throw `std::runtime_error` or derived exceptions:

```cpp
try {
    auto device = sb.device("invalid_id");
} catch (const std::runtime_error& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

## Performance Notes

- Uses persistent HTTP connection (libcurl session)
- Smart pointers minimize memory overhead
- JSON parsing is efficient with nlohmann/json
- Thread-safe: Create separate SwitchBot instances per thread

## Contributing

The library follows modern C++ best practices:
- C++17 features
- RAII and smart pointers
- Const correctness
- Exception safety
- Header/implementation separation
