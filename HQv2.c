#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> //macro untuk specify variable, int16 = 2 byte
#include <math.h>

// Agar struktur tidak di-padding oleh compiler
#pragma pack(push, 1)
typedef struct
{
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BMP_HEADER;

typedef struct
{
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
} DIB_HEADER;
#pragma pack(pop)

int main(int argc, char *argv[])
{   
    //arg Validation checking
    if (argc < 3)
    {
        printf("Usage: %s input.bmp output.bmp\n", argv[0]);
        return 1;
    }
    
    
  //  char *inputFile = argv[1];
//  char *outputFile = argv[2];

    FILE *file_bitmap = fopen(argv[1], "rb");
    if (!file_bitmap)
    {
        printf("Error membuka file input.\n");
        return 1;
    }

    // Membaca header file BMP yang 14 bytes
    BMP_HEADER bitmapINIT;

    fread(&bitmapINIT, sizeof(BMP_HEADER), 1, file_bitmap);
    if (bitmapINIT.bfType != 0x4D42)
    { // 0x4D42 adalah "BM"
        printf("File bukan bmp!.\n");
        fclose(file_bitmap);
        return 1;
    }

    // device independent bitmap
    DIB_HEADER bitmapINFO;
    fread(&bitmapINFO, sizeof(DIB_HEADER), 1, file_bitmap);

    int width = bitmapINFO.biWidth;
    int height = abs(bitmapINFO.biHeight); // Bisa negatif jika disimpan terbalik
    int bmp_bit = bitmapINFO.biBitCount;

    if (bmp_bit != 24 && bmp_bit != 32)
    {
        printf("Hanya mendukung BMP 24-bit dan 32-bit.\n");
        fclose(file_bitmap);
        return 1;
    }

    // Menghitung ukuran tiap baris (untuk 24-bit, baris dipadding hingga kelipatan 4 byte)
    int rowSize = ((bmp_bit * width + 31) / 32) * 4;
    unsigned char *pixelData = (unsigned char *)malloc(rowSize * height);
    if (pixelData == NULL)
    {
        printf("Memori tidak cukup.\n");
        fclose(file_bitmap);
        return 1;
    }

    // Pindah ke posisi data pixel di file
    fseek(file_bitmap, bitmapINIT.bfOffBits, SEEK_SET);
    fread(pixelData, 1, rowSize * height, file_bitmap);
    fclose(file_bitmap);

    // Konversi ke grayscale
    for (int y = 0; y < height; y++)
    {
        unsigned char *row = pixelData + y * rowSize;
    // baca dari row paling pertama
        for (int x = 0; x < width; x++)
        {
            int pixelOffset = x * (bmp_bit / 8);
            unsigned char blue = row[pixelOffset];
            unsigned char green = row[pixelOffset + 1];
            unsigned char red = row[pixelOffset + 2];

            unsigned char gray = (red + green + blue) / 3;

            row[pixelOffset] = gray;
            row[pixelOffset + 1] = gray;
            row[pixelOffset + 2] = gray;
        }
    }

    // Hitung histogram frekuensi
    int histogram[256] = {0};
    for (int y = 0; y < height; y++)
    {
        unsigned char *row = pixelData + y * rowSize;
        for (int x = 0; x < width; x++)
        {
            int pixelOffset = x * (bmp_bit / 8);
            unsigned char gray = row[pixelOffset];
            histogram[gray]++;
        }
    }

    // Hitung histogram kumulatif
    int cdf[256] = {0};
    cdf[0] = histogram[0];
    for (int i = 1; i < 256; i++)
    {
        cdf[i] = cdf[i - 1] + histogram[i];
    }

    // Normalisasi dan pemetaan nilai intensitas baru
    int totalPixels = width * height;
    unsigned char newIntensity[256];
    for (int i = 0; i < 256; i++)
    {
        newIntensity[i] = (unsigned char)round(((double)cdf[i] / totalPixels) * 255.0);
    }

    // Terapkan nilai intensitas baru ke gambar
    for (int y = 0; y < height; y++)
    {
        unsigned char *row = pixelData + y * rowSize;
        for (int x = 0; x < width; x++)
        {
            int pixelOffset = x * (bmp_bit / 8);
            unsigned char gray = row[pixelOffset];
            unsigned char newGray = newIntensity[gray];

            row[pixelOffset] = newGray;
            row[pixelOffset + 1] = newGray;
            row[pixelOffset + 2] = newGray;
        }
    }

    // Menyesuaikan header untuk output bit depth
    int outputRowSize = ((bmp_bit * width + 31) / 32) * 4;
    bitmapINFO.biSizeImage = outputRowSize * height;
    bitmapINIT.bfSize = sizeof(BMP_HEADER) + sizeof(DIB_HEADER) + bitmapINFO.biSizeImage;

    // Menulis hasil ke file output BMP
    file_bitmap = fopen(argv[2], "wb");
    if (!file_bitmap)
    {
        printf("Error membuka file output.\n");
        free(pixelData);
        return 1;
    }
    fwrite(&bitmapINIT, sizeof(BMP_HEADER), 1, file_bitmap);
    fwrite(&bitmapINFO, sizeof(DIB_HEADER), 1, file_bitmap);
    fseek(file_bitmap, bitmapINIT.bfOffBits, SEEK_SET);
    fwrite(pixelData, 1, outputRowSize * height, file_bitmap);
    fclose(file_bitmap);
    free(pixelData);

    printf("Histogram Equalization berhasil diterapkan.\n");
    return 0;
}
