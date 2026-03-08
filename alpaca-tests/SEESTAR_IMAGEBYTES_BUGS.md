# Seestar S50 ImageBytes Protocol Bugs

## Summary

The Seestar S50 (Alpaca v1.1.2-1) has a non-standard implementation of the ASCOM Alpaca ImageBytes protocol. While it does return images successfully, the metadata fields are incorrectly populated.

## Test Results

Using the `camera_image_download_test` program, we successfully downloaded a 1920x1080 image from the Seestar camera.

**Actual Data:**
- File size: 4,147,244 bytes
- Image: 1920 x 1080 pixels
- Format: Int16 (2 bytes per pixel)
- Total image data: 1920 × 1080 × 2 = 4,147,200 bytes
- Metadata header: 44 bytes
- Total: 4,147,244 bytes ✓

## ImageBytes Metadata Bugs

The Seestar reports the following metadata (44-byte header):

```
Raw bytes: 01 00 00 00 00 00 00 00 09 00 00 00 1b 00 00 00 
           2c 00 00 00 02 00 00 00 08 00 00 00 02 00 00 00 
           38 04 00 00 80 07 00 00 00 00 00 00
```

### Standard ASCOM ImageBytes Format (Expected)

| Offset | Field                    | Expected | Actual | Notes |
|--------|--------------------------|----------|--------|-------|
| 0-3    | MetadataVersion          | 1        | 1      | ✓ Correct |
| 4-7    | ErrorNumber              | 0        | 0      | ✓ Correct |
| 8-11   | ClientTransactionID      | 9        | 9      | ✓ Correct |
| 12-15  | ServerTransactionID      | varies   | 27     | ✓ Correct |
| 16-19  | ImageElementType         | varies   | 44     | Unknown value |
| 20-23  | TransmissionElementType  | 1 (Int16)| 2 (Int32) | **❌ WRONG - reports Int32, sends Int16** |
| 24-27  | Rank                     | 2        | 8      | **❌ WRONG - invalid value** |
| 28-31  | Dimension1 (Width)       | 1920     | 2      | **❌ WRONG - contains Rank instead** |
| 32-35  | Dimension2 (Height)      | 1080     | 1080   | ✓ Correct |
| 36-39  | Dimension3 (Planes)      | 0 or 1   | 1920   | **❌ WRONG - contains Width instead** |
| 40-43  | DataStart                | 44       | 0      | **❌ WRONG - should be 44** |

### Workaround Mapping

To correctly interpret Seestar ImageBytes data:

```cpp
// Standard Alpaca field -> Actual Seestar meaning
int actual_rank = meta.Dimension1;          // Rank is in Dimension1 field
int actual_height = meta.Dimension2;        // Height is correct
int actual_width = meta.Dimension3;         // Width is in Dimension3 field
int actual_bytes_per_pixel = 2;             // Always Int16 despite TransmissionElementType
int actual_data_start = 44;                 // Always 44 bytes, ignore DataStart field
```

### Correct Image Dimensions

Using the workaround:
- **Rank:** 2 (monochrome image)
- **Width:** 1920 pixels
- **Height:** 1080 pixels
- **Element Type:** Int16 (2 bytes per pixel)
- **Data starts at:** byte 44
- **Total size:** 4,147,244 bytes

## Recommendations for INDI Driver

The `indi_alpaca_ccd` driver should:

1. **Detect Seestar devices** (check device name/description)
2. **Apply workaround** when ImageBytes format is returned:
   - Swap dimension fields: `width = Dimension3`, `rank = Dimension1`
   - Force `bytes_per_element = 2` (ignore TransmissionElementType)
   - Force `data_start = 44` (ignore DataStart field)
3. **Validate data size** matches expected: `(width × height × 2) + 44`

## Alternative: Use JSON ImageArray

The Seestar also supports the standard JSON `ImageArray` protocol as a fallback. This may be more reliable but is slower and uses more bandwidth.

## Testing

To test image download:
```bash
cd alpaca-tests/build
./camera_image_download_test 1.0 output_filename
```

This will save:
- `output_filename.imagebytes` - Raw binary image data
- `output_filename.imagebytes.json` - Metadata including the workaround dimensions

## Files

- `camera_image_download_test.cpp` - Test program that downloads images
- This document - Analysis of the ImageBytes bugs
