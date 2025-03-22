#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#pragma pack(push, 1)
typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BMPHeader;

typedef struct {
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} DIBHeader;

typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
} RGB;

typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
} RGBA;

#pragma pack(pop)

void to_grayscale(RGB* pixel) {
    uint8_t gray = (uint8_t)(0.2989 * pixel->red + 0.5870 * pixel->green + 0.1140 * pixel->blue);
    pixel->red = pixel->green = pixel->blue = gray;
}

void to_grayscale_rgba(RGBA* pixel) {
    uint8_t gray = (uint8_t)(0.2989 * pixel->red + 0.5870 * pixel->green + 0.1140 * pixel->blue);
    pixel->red = pixel->green = pixel->blue = gray;
}

void histogram_equalization(RGB* image, int width, int height) {
    int hist[256] = {0};
    int cdf[256] = {0};
    int total_pixels = width * height;

    // Step 1: Calculate histogram
    for (int i = 0; i < total_pixels; i++) {
        int gray_value = image[i].red;  // Since it's grayscale, red, green, and blue values are the same.
        hist[gray_value]++;
    }

    // Step 2: Calculate cumulative distribution function (CDF)
    cdf[0] = hist[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i - 1] + hist[i];
    }

    // Step 3: Apply histogram equalization
    for (int i = 0; i < total_pixels; i++) {
        int gray_value = image[i].red;
        int new_gray_value = (int)(((double)(cdf[gray_value] - cdf[0]) / (total_pixels - 1)) * 255);
        image[i].red = image[i].green = image[i].blue = new_gray_value;
    }
}

void histogram_equalization_rgba(RGBA* image, int width, int height) {
    int hist[256] = {0};
    int cdf[256] = {0};
    int total_pixels = width * height;

    // Step 1: Calculate histogram
    for (int i = 0; i < total_pixels; i++) {
        int gray_value = image[i].red;  // Since it's grayscale, red, green, and blue values are the same.
        hist[gray_value]++;
    }

    // Step 2: Calculate cumulative distribution function (CDF)
    cdf[0] = hist[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i - 1] + hist[i];
    }

    // Step 3: Apply histogram equalization
    for (int i = 0; i < total_pixels; i++) {
        int gray_value = image[i].red;
        int new_gray_value = (int)(((double)(cdf[gray_value] - cdf[0]) / (total_pixels - 1)) * 255);
        image[i].red = image[i].green = image[i].blue = new_gray_value;
    }
}

void write_bmp(const char* filename, BMPHeader* bmpHeader, DIBHeader* dibHeader, void* image, int is_rgba) {
    FILE* output = fopen(filename, "wb");
    if (!output) {
        perror("Error opening output file");
        exit(1);
    }

    fwrite(bmpHeader, sizeof(BMPHeader), 1, output);
    fwrite(dibHeader, sizeof(DIBHeader), 1, output);
    
    int image_size = dibHeader->biWidth * abs(dibHeader->biHeight);
    if (is_rgba) {
        fwrite(image, sizeof(RGBA), image_size, output);
    } else {
        fwrite(image, sizeof(RGB), image_size, output);
    }

    fclose(output);
}

void* read_bmp(const char* filename, BMPHeader* bmpHeader, DIBHeader* dibHeader, int* is_rgba) {
    FILE* input = fopen(filename, "rb");
    if (!input) {
        perror("Error opening input file");
        exit(1);
    }

    fread(bmpHeader, sizeof(BMPHeader), 1, input);
    fread(dibHeader, sizeof(DIBHeader), 1, input);

    if (dibHeader->biBitCount == 24) {
        *is_rgba = 0;  // 24-bit BMP
    } else if (dibHeader->biBitCount == 32) {
        *is_rgba = 1;  // 32-bit BMP
    } else {
        printf("Only 24-bit and 32-bit BMP images are supported.\n");
        exit(1);
    }

    fseek(input, bmpHeader->bfOffBits, SEEK_SET);

    int imageSize = dibHeader->biWidth * abs(dibHeader->biHeight);
    void* image = malloc(imageSize * (dibHeader->biBitCount / 8));
    if (!image) {
        perror("Error allocating memory for image");
        exit(1);
    }

    fread(image, dibHeader->biBitCount / 8, imageSize, input);
    fclose(input);

    return image;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input BMP file> <output BMP file>\n", argv[0]);
        return 1;
    }

    BMPHeader bmpHeader;
    DIBHeader dibHeader;
    int is_rgba = 0; // flag to distinguish between RGB and RGBA

    // Read the input BMP file
    void* image = read_bmp(argv[1], &bmpHeader, &dibHeader, &is_rgba);

    int width = dibHeader.biWidth;
    int height = dibHeader.biHeight;
    int imageSize = width * abs(height);

    if (is_rgba) {
        RGBA* rgba_image = (RGBA*)image;

        // Convert to grayscale for 32-bit image
        for (int i = 0; i < imageSize; i++) {
            to_grayscale_rgba(&rgba_image[i]);
        }

        // Apply histogram equalization for 32-bit image
        histogram_equalization_rgba(rgba_image, width, height);

        // Write the result to the output BMP file
        write_bmp(argv[2], &bmpHeader, &dibHeader, rgba_image, 1);
    } else {
        RGB* rgb_image = (RGB*)image;

        // Convert to grayscale for 24-bit image
        for (int i = 0; i < imageSize; i++) {
            to_grayscale(&rgb_image[i]);
        }

        // Apply histogram equalization for 24-bit image
        histogram_equalization(rgb_image, width, height);

        // Write the result to the output BMP file
        write_bmp(argv[2], &bmpHeader, &dibHeader, rgb_image, 0);
    }

    // Clean up
    free(image);

    printf("Processing complete. Output saved to %s\n", argv[2]);

    return 0;
}
