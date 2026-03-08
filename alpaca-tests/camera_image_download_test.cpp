/*******************************************************************************
  ASCOM Alpaca Camera Image Download Test

  This test program downloads an image from an ASCOM Alpaca camera device.
  It tries both the ImageBytes binary protocol and the JSON ImageArray fallback.

*******************************************************************************/

#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>

#ifdef _USE_SYSTEM_JSONLIB
#include <nlohmann/json.hpp>
#else
#include <indijson.hpp>
#endif

using json = nlohmann::json;

// ImageBytes metadata structure (44 bytes) as per ASCOM Alpaca API v10 section 8.7
#pragma pack(push, 1)
struct ImageBytesMetadata
{
    int32_t MetadataVersion;        // Must be 1
    int32_t ErrorNumber;            // ASCOM error code
    int32_t ClientTransactionID;    // Client transaction ID
    int32_t ServerTransactionID;    // Server transaction ID
    int32_t ImageElementType;       // Element type of the actual image data
    int32_t TransmissionElementType; // Element type being transmitted
    int32_t Rank;                   // Number of dimensions (2 or 3)
    int32_t Dimension1;             // Width
    int32_t Dimension2;             // Height
    int32_t Dimension3;             // Planes (0 if Rank=2)
    int32_t DataStart;              // Offset to image data (should be 44)
};
#pragma pack(pop)

// Configuration
const std::string ALPACA_HOST = "seestar.local";
const int ALPACA_PORT = 32323;
const int DEVICE_NUMBER = 0;
const std::string DEVICE_TYPE = "camera";

// Global transaction counter
int g_transaction_id = 0;

int getTransactionId()
{
    return ++g_transaction_id;
}

std::string getAlpacaURL(const std::string& method)
{
    return "/api/v1/" + DEVICE_TYPE + "/" + std::to_string(DEVICE_NUMBER) + method;
}

std::string getAlpacaQueryParams()
{
    return "?ClientID=" + std::to_string(getpid()) + 
           "&ClientTransactionID=" + std::to_string(getTransactionId());
}

bool sendAlpacaGET(httplib::Client& client, const std::string& method, json& response)
{
    std::string url = getAlpacaURL(method) + getAlpacaQueryParams();
    
    std::cout << "GET: " << url << std::endl;
    
    auto result = client.Get(url.c_str());
    
    if (!result)
    {
        std::cerr << "HTTP request failed: " << httplib::to_string(result.error()) << std::endl;
        return false;
    }
    
    if (result->status != 200)
    {
        std::cerr << "HTTP status: " << result->status << std::endl;
        return false;
    }
    
    try
    {
        response = json::parse(result->body);
        
        if (response["ErrorNumber"].get<int>() != 0)
        {
            std::cerr << "Alpaca error: " << response["ErrorMessage"].get<std::string>() << std::endl;
            return false;
        }
        
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return false;
    }
}

bool sendAlpacaPUT(httplib::Client& client, const std::string& method, const std::string& params, json& response)
{
    std::string url = getAlpacaURL(method);
    std::string body = params + "&ClientID=" + std::to_string(getpid()) + 
                      "&ClientTransactionID=" + std::to_string(getTransactionId());
    
    std::cout << "PUT: " << url << std::endl;
    std::cout << "Body: " << body << std::endl;
    
    auto result = client.Put(url.c_str(), body, "application/x-www-form-urlencoded");
    
    if (!result)
    {
        std::cerr << "HTTP request failed: " << httplib::to_string(result.error()) << std::endl;
        return false;
    }
    
    if (result->status != 200)
    {
        std::cerr << "HTTP status: " << result->status << std::endl;
        return false;
    }
    
    try
    {
        response = json::parse(result->body);
        
        if (response["ErrorNumber"].get<int>() != 0)
        {
            std::cerr << "Alpaca error: " << response["ErrorMessage"].get<std::string>() << std::endl;
            return false;
        }
        
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return false;
    }
}

bool connectToCamera(httplib::Client& client)
{
    json response;
    if (!sendAlpacaPUT(client, "/connected", "Connected=true", response))
    {
        return false;
    }
    
    std::cout << "Camera connected successfully" << std::endl;
    return true;
}

bool startExposure(httplib::Client& client, double duration, bool light)
{
    json response;
    std::string params = "Duration=" + std::to_string(duration) + "&Light=" + (light ? "true" : "false");
    
    if (!sendAlpacaPUT(client, "/startexposure", params, response))
    {
        return false;
    }
    
    std::cout << "Exposure started: " << duration << "s" << std::endl;
    return true;
}

bool getCameraState(httplib::Client& client, int& state)
{
    json response;
    if (!sendAlpacaGET(client, "/camerastate", response))
    {
        return false;
    }
    
    state = response["Value"].get<int>();
    return true;
}

bool isImageReady(httplib::Client& client)
{
    json response;
    if (!sendAlpacaGET(client, "/imageready", response))
    {
        return false;
    }
    
    return response["Value"].get<bool>();
}

bool waitForExposure(httplib::Client& client, double timeout_seconds = 120.0)
{
    std::cout << "Waiting for exposure to complete..." << std::endl;
    
    auto start = std::chrono::steady_clock::now();
    int state = 0;
    
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        if (!getCameraState(client, state))
        {
            std::cerr << "Failed to get camera state" << std::endl;
            return false;
        }
        
        // Camera states: 0=Idle, 1=Waiting, 2=Exposing, 3=Reading, 4=Download, 5=Error
        const char* state_names[] = {"Idle", "Waiting", "Exposing", "Reading", "Download", "Error"};
        std::cout << "Camera state: " << state;
        if (state >= 0 && state <= 5)
        {
            std::cout << " (" << state_names[state] << ")";
        }
        std::cout << std::endl;
        
        if (state == 0) // Idle - exposure complete
        {
            break;
        }
        else if (state == 5) // Error
        {
            std::cerr << "Camera entered error state" << std::endl;
            return false;
        }
        
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();
        if (elapsed > timeout_seconds)
        {
            std::cerr << "Timeout waiting for exposure" << std::endl;
            return false;
        }
    }
    
    std::cout << "Exposure complete, checking if image is ready..." << std::endl;
    
    if (!isImageReady(client))
    {
        std::cerr << "Image is not ready" << std::endl;
        return false;
    }
    
    std::cout << "Image is ready for download" << std::endl;
    return true;
}

bool downloadImageBytes(httplib::Client& client, const std::string& output_file)
{
    std::cout << "\n=== Trying ImageBytes Protocol ===" << std::endl;
    
    // Set up headers for ImageBytes protocol
    httplib::Headers headers = {{"Accept", "application/imagebytes"}};
    
    std::string url = getAlpacaURL("/imagearray") + getAlpacaQueryParams();
    
    std::cout << "GET with ImageBytes header: " << url << std::endl;
    
    auto result = client.Get(url.c_str(), headers);
    
    if (!result)
    {
        std::cerr << "HTTP request failed: " << httplib::to_string(result.error()) << std::endl;
        return false;
    }
    
    if (result->status != 200)
    {
        std::cerr << "HTTP status: " << result->status << std::endl;
        return false;
    }
    
    // Check Content-Type header
    auto content_type = result->get_header_value("Content-Type");
    std::cout << "Content-Type: " << content_type << std::endl;
    
    if (content_type.find("application/imagebytes") == std::string::npos)
    {
        std::cout << "Server did not return ImageBytes format, will try JSON fallback" << std::endl;
        return false;
    }
    
    std::cout << "Response size: " << result->body.size() << " bytes" << std::endl;
    
    // Check minimum size for metadata
    if (result->body.size() < sizeof(ImageBytesMetadata))
    {
        std::cerr << "Response too small for ImageBytes metadata" << std::endl;
        return false;
    }
    
    // Extract metadata
    ImageBytesMetadata meta;
    std::memcpy(&meta, result->body.data(), sizeof(ImageBytesMetadata));
    
    std::cout << "\nImageBytes Metadata:" << std::endl;
    std::cout << "  MetadataVersion: " << meta.MetadataVersion << std::endl;
    std::cout << "  ErrorNumber: " << meta.ErrorNumber << std::endl;
    std::cout << "  ClientTransactionID: " << meta.ClientTransactionID << std::endl;
    std::cout << "  ServerTransactionID: " << meta.ServerTransactionID << std::endl;
    std::cout << "  ImageElementType: " << meta.ImageElementType << std::endl;
    std::cout << "  TransmissionElementType: " << meta.TransmissionElementType << std::endl;
    std::cout << "  Rank: " << meta.Rank << std::endl;
    std::cout << "  Dimension1 (Width): " << meta.Dimension1 << std::endl;
    std::cout << "  Dimension2 (Height): " << meta.Dimension2 << std::endl;
    std::cout << "  Dimension3 (Planes): " << meta.Dimension3 << std::endl;
    std::cout << "  DataStart: " << meta.DataStart << std::endl;
    
    // Debug: show raw bytes
    std::cout << "\nRaw metadata bytes (first 44):" << std::endl;
    const uint8_t* raw = reinterpret_cast<const uint8_t*>(&meta);
    for (int i = 0; i < 44; i++)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)raw[i] << " ";
        if ((i + 1) % 16 == 0) std::cout << std::endl;
    }
    std::cout << std::dec << std::endl;
    
    // Validate metadata
    if (meta.MetadataVersion != 1)
    {
        std::cerr << "Unsupported metadata version: " << meta.MetadataVersion << std::endl;
        return false;
    }
    
    if (meta.ErrorNumber != 0)
    {
        std::cerr << "Image metadata contains error: " << meta.ErrorNumber << std::endl;
        return false;
    }
    
    // Workaround for Seestar non-standard ImageBytes format
    // Seestar reports Rank=8 which is invalid (should be 2 or 3)
    // It appears to put dimensions in wrong fields
    int actual_width = meta.Dimension3;   // Width is in Dimension3
    int actual_height = meta.Dimension2;  // Height is in Dimension2
    int actual_rank = meta.Dimension1;    // Rank is in Dimension1
    
    std::cout << "\nCorrected dimensions:" << std::endl;
    std::cout << "  Actual Rank: " << actual_rank << std::endl;
    std::cout << "  Actual Width: " << actual_width << std::endl;
    std::cout << "  Actual Height: " << actual_height << std::endl;
    
    // Calculate expected data size
    int bytes_per_element = 0;
    const char* type_name = "Unknown";
    switch (meta.TransmissionElementType)
    {
        case 1: bytes_per_element = 2; type_name = "Int16"; break;
        case 2: bytes_per_element = 4; type_name = "Int32"; break;
        case 3: bytes_per_element = 8; type_name = "Double"; break;
        case 4: bytes_per_element = 4; type_name = "Single"; break;
        case 5: bytes_per_element = 8; type_name = "UInt64"; break;
        case 6: bytes_per_element = 1; type_name = "Byte"; break;
        case 7: bytes_per_element = 8; type_name = "Int64"; break;
        case 8: bytes_per_element = 2; type_name = "UInt16"; break;
        case 9: bytes_per_element = 4; type_name = "UInt32"; break;
        default:
            std::cerr << "Unknown transmission element type: " << meta.TransmissionElementType << std::endl;
            return false;
    }
    
    std::cout << "  Element type: " << type_name << " (" << bytes_per_element << " bytes)" << std::endl;
    
    // Calculate actual image dimensions from data size
    size_t actual_data_size = result->body.size() - meta.DataStart;
    size_t total_elements = actual_data_size / bytes_per_element;
    
    // For Seestar: data appears to be height * width * channels
    // 4147200 bytes / 4 = 1036800 pixels = 1080 * 1920 * 0.5 or possibly 1080 * 960
    // Let's calculate what makes sense
    int calc_width = actual_width;
    int calc_height = actual_height;
    int calc_channels = 1;
    
    if (actual_rank == 3)
    {
        // Color image - need to figure out channel count
        // Try to deduce from total size
        size_t pixels_2d = calc_width * calc_height;
        if (total_elements % pixels_2d == 0)
        {
            calc_channels = total_elements / pixels_2d;
        }
    }
    
    std::cout << "\nCalculated from data size:" << std::endl;
    std::cout << "  Total elements: " << total_elements << std::endl;
    std::cout << "  Calculated channels: " << calc_channels << std::endl;
    std::cout << "  Expected pixels: " << (calc_width * calc_height * calc_channels) << std::endl;
    
    size_t num_pixels = calc_width * calc_height * calc_channels;
    size_t expected_data_size = num_pixels * bytes_per_element;
    
    std::cout << "  Expected data size: " << expected_data_size << " bytes" << std::endl;
    std::cout << "  Actual data size: " << actual_data_size << " bytes" << std::endl;
    
    if (actual_data_size < expected_data_size)
    {
        std::cerr << "Warning: Data size mismatch!" << std::endl;
        std::cout << "Will save available data..." << std::endl;
    }
    
    // Save the raw binary data
    std::ofstream out_file(output_file + ".imagebytes", std::ios::binary);
    if (!out_file)
    {
        std::cerr << "Failed to open output file" << std::endl;
        return false;
    }
    
    out_file.write(result->body.data() + meta.DataStart, actual_data_size);
    out_file.close();
    
    std::cout << "\nImage data saved to: " << output_file << ".imagebytes" << std::endl;
    std::cout << "Image dimensions: " << calc_width << "x" << calc_height;
    if (calc_channels > 1)
    {
        std::cout << "x" << calc_channels;
    }
    std::cout << std::endl;
    
    // Also save metadata as JSON
    json meta_json = {
        {"format", "ImageBytes"},
        {"width", calc_width},
        {"height", calc_height},
        {"channels", calc_channels},
        {"rank", actual_rank},
        {"element_type", meta.TransmissionElementType},
        {"element_type_name", type_name},
        {"bytes_per_element", bytes_per_element},
        {"data_size", actual_data_size},
        {"raw_metadata", {
            {"MetadataVersion", meta.MetadataVersion},
            {"ErrorNumber", meta.ErrorNumber},
            {"ImageElementType", meta.ImageElementType},
            {"TransmissionElementType", meta.TransmissionElementType},
            {"Rank_field", meta.Rank},
            {"Dimension1_field", meta.Dimension1},
            {"Dimension2_field", meta.Dimension2},
            {"Dimension3_field", meta.Dimension3},
            {"DataStart", meta.DataStart}
        }}
    };
    
    std::ofstream meta_file(output_file + ".imagebytes.json");
    meta_file << meta_json.dump(2);
    meta_file.close();
    
    std::cout << "Metadata saved to: " << output_file << ".imagebytes.json" << std::endl;
    
    return true;
}

bool downloadImageArrayJSON(httplib::Client& client, const std::string& output_file)
{
    std::cout << "\n=== Trying JSON ImageArray Protocol ===" << std::endl;
    
    json response;
    if (!sendAlpacaGET(client, "/imagearray", response))
    {
        return false;
    }
    
    std::cout << "Got JSON response" << std::endl;
    
    auto imageArray = response["Value"];
    
    if (imageArray.empty() || !imageArray[0].is_array())
    {
        std::cerr << "Invalid image array format" << std::endl;
        return false;
    }
    
    int width = imageArray.size();
    int height = imageArray[0].size();
    
    std::cout << "Image dimensions: " << width << "x" << height << std::endl;
    
    // Determine if this is a color image (3D array)
    int planes = 0;
    bool is_3d = false;
    
    if (!imageArray[0][0].is_number())
    {
        if (imageArray[0][0].is_array())
        {
            is_3d = true;
            planes = imageArray[0][0].size();
            std::cout << "Color image with " << planes << " planes" << std::endl;
        }
        else
        {
            std::cerr << "Unexpected array element type" << std::endl;
            return false;
        }
    }
    else
    {
        std::cout << "Monochrome image" << std::endl;
        planes = 1;
    }
    
    // Extract pixel data
    size_t num_pixels = width * height * planes;
    std::vector<int32_t> pixel_data;
    pixel_data.reserve(num_pixels);
    
    std::cout << "Extracting pixel data..." << std::endl;
    
    if (is_3d)
    {
        // 3D array: imageArray[x][y][plane]
        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                for (int p = 0; p < planes; p++)
                {
                    pixel_data.push_back(imageArray[x][y][p].get<int32_t>());
                }
            }
        }
    }
    else
    {
        // 2D array: imageArray[x][y]
        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                pixel_data.push_back(imageArray[x][y].get<int32_t>());
            }
        }
    }
    
    std::cout << "Extracted " << pixel_data.size() << " pixel values" << std::endl;
    
    // Save the raw pixel data
    std::ofstream out_file(output_file + ".imagearray.raw", std::ios::binary);
    if (!out_file)
    {
        std::cerr << "Failed to open output file" << std::endl;
        return false;
    }
    
    out_file.write(reinterpret_cast<const char*>(pixel_data.data()), 
                   pixel_data.size() * sizeof(int32_t));
    out_file.close();
    
    std::cout << "Image data saved to: " << output_file << ".imagearray.raw" << std::endl;
    
    // Save metadata
    json meta_json = {
        {"format", "JSON ImageArray"},
        {"width", width},
        {"height", height},
        {"planes", planes},
        {"is_3d", is_3d},
        {"pixel_count", pixel_data.size()},
        {"data_type", "int32"}
    };
    
    std::ofstream meta_file(output_file + ".imagearray.json");
    meta_file << meta_json.dump(2);
    meta_file.close();
    
    std::cout << "Metadata saved to: " << output_file << ".imagearray.json" << std::endl;
    
    return true;
}

int main(int argc, char* argv[])
{
    double exposure_duration = 1.0; // default 1 second
    std::string output_file = "alpaca_image";
    
    if (argc > 1)
    {
        exposure_duration = std::atof(argv[1]);
    }
    if (argc > 2)
    {
        output_file = argv[2];
    }
    
    std::cout << "=== ASCOM Alpaca Camera Image Download Test ===" << std::endl;
    std::cout << "Host: " << ALPACA_HOST << ":" << ALPACA_PORT << std::endl;
    std::cout << "Device: " << DEVICE_TYPE << "/" << DEVICE_NUMBER << std::endl;
    std::cout << "Exposure duration: " << exposure_duration << "s" << std::endl;
    std::cout << "Output file base: " << output_file << std::endl;
    std::cout << std::endl;
    
    // Create HTTP client
    httplib::Client client(ALPACA_HOST, ALPACA_PORT);
    client.set_connection_timeout(5, 0);
    client.set_read_timeout(60, 0);
    client.set_write_timeout(5, 0);
    
    // Connect to camera
    if (!connectToCamera(client))
    {
        std::cerr << "Failed to connect to camera" << std::endl;
        return 1;
    }
    
    // Start exposure
    if (!startExposure(client, exposure_duration, true))
    {
        std::cerr << "Failed to start exposure" << std::endl;
        return 1;
    }
    
    // Wait for exposure to complete
    if (!waitForExposure(client, exposure_duration + 30.0))
    {
        std::cerr << "Failed waiting for exposure" << std::endl;
        return 1;
    }
    
    // Try to download the image
    bool success = false;
    
    // Try ImageBytes first
    if (downloadImageBytes(client, output_file))
    {
        success = true;
    }
    // Fallback to JSON
    else if (downloadImageArrayJSON(client, output_file))
    {
        success = true;
    }
    else
    {
        std::cerr << "\nFailed to download image via both methods" << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Test completed successfully ===" << std::endl;
    
    return 0;
}
