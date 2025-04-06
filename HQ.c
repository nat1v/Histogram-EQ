#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"
//made proudly in nvim with AI help

#pragma pack(push, 1) //packing bytes with no padding TEMPORARILY = push > pop

// 14 bytes of BMP header (BMP)
typedef struct {
    uint16_t bmp_type;
    uint32_t bmp_size;
    uint16_t bmp_reserved1;
    uint32_t bmp_reserved2;
    uint32_t bmp_offbit;
} BMPHeader;

// 40 bytes of Device Independent Bitmaps / bitmap info header (DIB)
typedef struct {
    uint32_t dib_size;
    int32_t dib_width;
    int32_t dib_height;
    uint16_t dib_planes;
    uint16_t dib_bitcount;
    uint32_t dib_compression;
    uint32_t dib_sizeImage;
    int32_t dib_xPixelPerMeter;
    int32_t dib_yPixelPerMeter;
    uint32_t dib_colorUsed;
    uint32_t dib_colorImportant;
} DIBHeader;

typedef struct { //if bmp is rgb use this
    uint8_t blue;
    uint8_t green;
    uint8_t red;
} RGB;

typedef struct { // if bmp is rgba use this
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
} RGBA;

#pragma pack(pop)

FILE* inputBMP;
FILE* outputBMP;

void* read_BMP(const char* imageFILE, BMPHeader* bmp_Header, DIBHeader* dib_Header, int*is_rgba){
    
    inputBMP = fopen(imageFILE, "rb");
    fread(bmp_Header, sizeof(BMPHeader), 1, inputBMP);  
    fread(dib_Header, sizeof(DIBHeader), 1, outputBMP);
    
    if (dib_Header->dib_bitcount == 24){
        *is_rgba = 0;
    } else {
        *is_rgba = 1;
    }
    
    fseek(inputBMP, bmp_Header->bmp_offbit, SEEK_SET);

    int image_Size = dib_Header->dib_width * dib_Header->dib_height;
    void* theBMPFILE = malloc(image_Size * (dib_Header->dib_bitcount / 8));
    fread(theBMPFILE, dib_Header->dib_bitcount / 8, image_Size, inputBMP);
    fclose(inputBMP);

    return image;
}

void write_BMP(const char* imageFILE, BMPHeader* bmp_Header, DIBHeader* dib_Header, int* is_rgba){
    
    outputBMP = fopen(imageFILE, "wb");
    
    fwrite(bmp_Header, sizeof(BMPHeader), 1, outputBMP);
    fwrite(dib_Header, sizeof(DIBHeader), 1, outputBMP);

    int image_Size = dib_Header->dib_width * dib_Header->dib_height;
    
    if (is_rgba){
        fwrite(image, sizeof(RGBA), image_Size, outputBMP);
    } else {
        fwrite(image, sizeof(RGB), image_Size, outputBMP);
    }
    
    fclose(outputBMP);
}

void grayscale_rgb(RGB* pixel){
    uint8_t gray = (uint8_t)(0.299 * pixel->red + 0.587 * pixel->green + 0.114 * pixel->blue);
    pixel->red = gray;
    pixel->green = gray;
    pixel->blue = gray;
}
void grayscale_rgba(RGBA* pixel){
    uint8_t gray = (uint8_t)(0.299 * pixel->red + 0.587 * pixel->green + 0.114 * pixel->blue);
    pixel->red = gray;
    pixel->green = gray;
    pixel->blue = gray;
}

void histogram_eq_rgb(RGB* theBMPFILE, int width, int height){
    
    int total_pixel = width * height;
    int pixel_freq[256] = {0};
    int cdf[256] = {0};
    int value_of_pixel;
  
    //calculating how many some pixel appeared (histogram)
    for(int i = 0; i < total_pixel; i++){
        value_of_pixel = theBMPFILE[i].green;
        pixel_frequency[value_of_pixel]++
    }
  
    //calculating cumulative distribution function (CDF)
    cdf[0] = pixel_frequency[0];
    for(int i = 0; i < total_pixel; i++){
        cdf[i] = cdf[i - 1] + pixel_frequency[i];
    }

    //applying the newly histogram EQ
    for(int i = 0; i < total_pixel; i++){
        value_of_pixel = theBMPFILE[i].green;
        int new_GRAY_pixel = (int) ((double) (cdf[value_of_pixel] - cdf[0]) / (total_pixel - 1) * 255);
  }
}

void histogram_eq_rgba(RGBA* theBMPFILE, int width, int height){
    
    int total_pixel = width * height;
    int pixel_frequency[256] = {0};
    int cdf[256] = {0};
    int value_of_pixel;
  
    //calculating how many some pixel appeared (histogram)
    for(int i = 0; i < total_pixel; i++){
        value_of_pixel = theBMPFILE[i].green;
        pixel_frequency[value_of_pixel]++
    }
  
    //calculating cumulative distribution function (CDF)
    cdf[0] = pixel_frequency[0];

    for(int i = 0; i < total_pixel; i++){
        cdf[i] = cdf[i - 1] + pixel_frequency[i];
    }

    //applying the newly histogram EQ
    for(int i = 0; i < total_pixel; i++){
        value_of_pixel = theBMPFILE[i].green;
        int new_GRAY_pixel = (int)(((double)(cdf[value_of_pixel] - cdf[0]) / (total_pixel - 1)) * 255);
  }
}

int int main(int argc, char *argv[])
{
    if (argc != 3){
        printf("penggunaan: %s <file input.bmp> <file output.bmp>\n", argv[0]);
        return 1;
    }
    
    BMPHeader bmp_Header;
    DIBHeader dib_Header;
    int is_rgba = 0;

    void* image = read_bmp(argv[1], &bmp_Header, &dib_Header, &is_rgba);
    
    int width = dib_Header.dib_width;
    int height = dib_Header.dib_height;
    int imageSize = width * height;
    
    if (is_rgba){
        RGBA* theRGBA_IMAGE = (RGBA*)image;
        
        for (int i = 0; i < image_Size; i++){
            grayscale_rgba(theRGBA_IMAGE);
        }

        histogram_eq_rgba(theRGBA_IMAGE, width, height);

        write_BMP(argv[2], &bmp_Header, &dib_Header, theRGBA_IMAGE, 1);
            
    } else {
        RGB* theRGB_IMAGE = (RGB*)image;
        
        for (int i = 0; i < image_Size; i++){
            grayscale_rgba(theRGB_IMAGE);
        }

        histogram_eq_rgba(theRGB_IMAGE, width, height);

        write_BMP(argv[2], &bmp_Header, &dib_Header, theRGB_IMAGE, 0);

    }
    

    free(image);

    printf("complet!")
    return 0;
}


