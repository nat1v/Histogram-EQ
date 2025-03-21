#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// BMP header structures
#pragma pack(push, 1)  // Ensure structures are packed tightly
typedef struct {
    uint16_t signature;      // "BM"
    uint32_t fileSize;       // Size of the file in bytes
    uint16_t reserved1;      // Reserved
    uint16_t reserved2;      // Reserved
    uint32_t dataOffset;     // Offset to image data
} BMPFileHeader;

typedef struct {
    uint32_t headerSize;     // Size of this header in bytes
    int32_t width;           // Width of the image
    int32_t height;          // Height of the image
    uint16_t planes;         // Number of color planes
    uint16_t bitsPerPixel;   // Bits per pixel
    uint32_t compression;    // Compression method
    uint32_t imageSize;      // Size of the image data
    int32_t xPixelsPerMeter; // Horizontal resolution
    int32_t yPixelsPerMeter; // Vertical resolution
    uint32_t colorsUsed;     // Number of colors in the palette
    uint32_t colorsImportant;// Number of important colors
} BMPInfoHeader;
#pragma pack(pop)

// Pixel structure for 32-bit BMP (RGBA)
typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
} Pixel;

// Function to read BMP file
int readBMP(const char* filename, BMPFileHeader* fileHeader, BMPInfoHeader* infoHeader, Pixel** pixels) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return 0;
    }

    // Read the file header
    if (fread(fileHeader, sizeof(BMPFileHeader), 1, file) != 1) {
        fprintf(stderr, "Error reading file header\n");
        fclose(file);
        return 0;
    }

    // Check if it's a BMP file
    if (fileHeader->signature != 0x4D42) { // "BM" in little endian
        fprintf(stderr, "Not a BMP file\n");
        fclose(file);
        return 0;
    }

    // Read the info header
    if (fread(infoHeader, sizeof(BMPInfoHeader), 1, file) != 1) {
        fprintf(stderr, "Error reading info header\n");
        fclose(file);
        return 0;
    }

    // Check if it's a 32-bit BMP
    if (infoHeader->bitsPerPixel != 32) {
        fprintf(stderr, "Not a 32-bit BMP file\n");
        fclose(file);
        return 0;
    }

    // Allocate memory for pixel data
    *pixels = (Pixel*)malloc(infoHeader->width * infoHeader->height * sizeof(Pixel));
    if (!*pixels) {
        fprintf(stderr, "Memory allocation error\n");
        fclose(file);
        return 0;
    }

    // Seek to the beginning of the pixel data
    fseek(file, fileHeader->dataOffset, SEEK_SET);

    // Read the pixel data
    if (fread(*pixels, sizeof(Pixel), infoHeader->width * infoHeader->height, file) != infoHeader->width * infoHeader->height) {
        fprintf(stderr, "Error reading pixel data\n");
        free(*pixels);
        fclose(file);
        return 0;
    }

    fclose(file);
    return 1;
}

// Function to write BMP file
int writeBMP(const char* filename, BMPFileHeader* fileHeader, BMPInfoHeader* infoHeader, Pixel* pixels) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Error creating file %s\n", filename);
        return 0;
    }

    // Write the file header
    if (fwrite(fileHeader, sizeof(BMPFileHeader), 1, file) != 1) {
        fprintf(stderr, "Error writing file header\n");
        fclose(file);
        return 0;
    }

    // Write the info header
    if (fwrite(infoHeader, sizeof(BMPInfoHeader), 1, file) != 1) {
        fprintf(stderr, "Error writing info header\n");
        fclose(file);
        return 0;
    }

    // Write any padding between headers and pixel data
    long currPos = ftell(file);
    if (currPos < fileHeader->dataOffset) {
        int paddingSize = fileHeader->dataOffset - currPos;
        uint8_t* padding = (uint8_t*)calloc(paddingSize, 1);
        fwrite(padding, 1, paddingSize, file);
        free(padding);
    }

    // Write the pixel data
    if (fwrite(pixels, sizeof(Pixel), infoHeader->width * infoHeader->height, file) != infoHeader->width * infoHeader->height) {
        fprintf(stderr, "Error writing pixel data\n");
        fclose(file);
        return 0;
    }

    fclose(file);
    return 1;
}

// Function to convert RGB to grayscale
uint8_t rgbToGray(uint8_t r, uint8_t g, uint8_t b) {
    return (uint8_t)(0.299 * r + 0.587 * g + 0.114 * b);
}

// Main function for histogram equalization
void histogramEqualization(Pixel* pixels, int width, int height) {
    int totalPixels = width * height;
    
    // Step 1: Create grayscale image
    uint8_t* grayImage = (uint8_t*)malloc(totalPixels * sizeof(uint8_t));
    for (int i = 0; i < totalPixels; i++) {
        grayImage[i] = rgbToGray(pixels[i].red, pixels[i].green, pixels[i].blue);
    }
    
    // Step 2: Compute histogram
    int histogram[256] = {0};
    for (int i = 0; i < totalPixels; i++) {
        histogram[grayImage[i]]++;
    }
    
    // Step 3: Compute probability distribution
    float probability[256] = {0.0};
    for (int i = 0; i < 256; i++) {
        probability[i] = (float)histogram[i] / totalPixels;
    }
    
    // Step 4: Compute CDF
    float cdf[256] = {0.0};
    cdf[0] = probability[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i-1] + probability[i];
    }
    
    // Step 5: Apply histogram equalization
    uint8_t equalized[256]; // Mapping table
    for (int i = 0; i < 256; i++) {
        equalized[i] = (uint8_t)(cdf[i] * 255.0 + 0.5); // +0.5 for rounding
    }
    
    // Step 6: Convert back to 32-bit BMP
    for (int i = 0; i < totalPixels; i++) {
        uint8_t newValue = equalized[grayImage[i]];
        pixels[i].red = newValue;
        pixels[i].green = newValue;
        pixels[i].blue = newValue;
        // Alpha channel is preserved
    }
    
    free(grayImage);
}

int main(int argc, char* argv[]) {
    // Check command line arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_image.bmp> <output_image.bmp>\n", argv[0]);
        return 1;
    }
    
    const char* inputFilename = argv[1];
    const char* outputFilename = argv[2];
    
    // Structures for BMP file
    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;
    Pixel* pixels = NULL;
    
    // Read the input BMP file
    if (!readBMP(inputFilename, &fileHeader, &infoHeader, &pixels)) {
        fprintf(stderr, "Failed to read input BMP file\n");
        return 1;
    }
    
    printf("Processing image: %dx%d pixels, %d bits per pixel\n", 
           infoHeader.width, infoHeader.height, infoHeader.bitsPerPixel);
    
    // Apply histogram equalization
    histogramEqualization(pixels, infoHeader.width, infoHeader.height);
    
    // Write the output BMP file
    if (!writeBMP(outputFilename, &fileHeader, &infoHeader, pixels)) {
        fprintf(stderr, "Failed to write output BMP file\n");
        free(pixels);
        return 1;
    }
    
    printf("Histogram equalization completed. Output saved to %s\n", outputFilename);
    
    // Clean up
    free(pixels);
    
    return 0;
}