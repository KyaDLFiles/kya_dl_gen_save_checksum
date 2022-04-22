#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crc32.h"

void int32_to_bytes_BE(unsigned __int8 *array, unsigned __int32 num) { //https://stackoverflow.com/a/3784478
    array[0] = (num >> 24) & 0xFF;
    array[1] = (num >> 16) & 0xFF;
    array[2] = (num >> 8) & 0xFF;
    array[3] = num & 0xFF;
}

void int32_to_bytes_le(unsigned __int8 *array, unsigned __int32 num) {
    array[3] = (num >> 24) & 0xFF;
    array[2] = (num >> 16) & 0xFF;
    array[1] = (num >> 8) & 0xFF;
    array[0] = num & 0xFF;
}

unsigned __int32 bytes_le_to_int32(const unsigned __int8 array[], int size) {
    unsigned int res = 0, mult = 1, i;
    for (i = 0; i < size; i++) {
        res += array[i] * mult;
        mult *= 256;
    }
    return res;
}

void print_buf(unsigned __int8 *buffer, int size) {
    int i;
    for (i = 0; i < size; i++) {
        if (i > 0) printf(":");
            printf("%02X", buffer[i]);
    }
}

unsigned __int64 compute_checksum(const void *buffer, __int64 size) {
    unsigned __int64 result;
    __int64 v4;
    unsigned __int8 v5;
    __int64 v6;

    result = 0xFFFFFFFFFFFFFFFFLL;
    v4 = (int)size - 1;
    if (size) {
        do {
            v5 = *(unsigned __int8 *)buffer;
            v6 = v4;
            v4 = (__int32)v4 - 1;
            buffer = (unsigned __int8 *)buffer + 1;
            result = ((unsigned __int32)result >> 8) ^ (unsigned __int64)crc32_tab[(unsigned __int8)(result ^ v5)];
        } while ( v6 );
    }
    return result;
}

int main(int argc, char *argv[]) {
    //Variable definitions
    struct {
        unsigned __int32 header_csum, header_size,
                data1_csum, data1_size,
                data2_csum, data2_size;
    } header;
    FILE * fp;
    unsigned __int8 *buf, four_bytes[4];

    //Check file path argument
    if (argc < 2) {
        printf("Missing argument!\n"
               "Usage: gensavehash(.exe) /path/to/save/file.dat\n");
        return 1;
    }

    //Open file
    if ((fp = fopen(argv[1], "rb+")) == NULL) {
        printf("Error opening file!\n");
        return 2;
    }


    //Read header size
    //(idk why the size is saved as it's always 0x1C, but just to be safe)
    fseek(fp, 8, SEEK_SET);
    fread(four_bytes, 4, 1, fp);
    header.header_size = bytes_le_to_int32(four_bytes, 4);

    //Read first data block size
    fseek(fp, 4, SEEK_CUR);
    fread(four_bytes, 4, 1, fp);
    header.data1_size = bytes_le_to_int32(four_bytes, 4);

    //Read second data block size
    fseek(fp, 4, SEEK_CUR);
    fread(four_bytes, 4, 1, fp);
    header.data2_size = bytes_le_to_int32(four_bytes, 4);

    //Calculate first data block checksum
    fseek(fp, header.header_size, SEEK_SET);
    if (header.data1_size == 0)
        header.data1_csum = 0;
    else {
        buf = (unsigned __int8*)malloc(header.data1_size);
        fread(buf, header.data1_size, 1, fp);
        header.data1_csum = compute_checksum(buf, header.data1_size);
        free(buf);
    }

    //Calculate second data block checksum
    if (header.data2_size == 0)
        header.data2_csum = 0;
    else {
        buf = (unsigned __int8*)malloc(header.data2_size);
        fread(buf, header.data2_size, 1, fp);
        header.data2_csum = compute_checksum(buf, header.data2_size);
        free(buf);
    }

    //Write first data section checksum to file
    fseek(fp, 0x0C, SEEK_SET);
    int32_to_bytes_le(four_bytes, header.data1_csum);
    fwrite(four_bytes, 4, 1, fp);

    //Write second data section checksum to file
    fseek(fp, 4, SEEK_CUR);
    int32_to_bytes_le(four_bytes, header.data2_csum);
    fwrite(four_bytes, 4, 1, fp);

    //Reread header from file
    fseek(fp, 8, SEEK_SET);
    buf = (unsigned __int8*)malloc(header.header_size - 8);
    fread(buf, header.header_size - 8, 1, fp);

    //Calculate header checksum
    header.header_csum = compute_checksum(buf, header.header_size - 8);
    int32_to_bytes_le(four_bytes, header.header_csum);
    fseek(fp, 4, SEEK_SET);
    fwrite(four_bytes, 4, 1, fp);

    //Print checksums to console
    int32_to_bytes_BE(four_bytes, header.header_csum);
    printf("Header checksum: ");
    print_buf(four_bytes, 4);
    int32_to_bytes_BE(four_bytes, header.data1_csum);
    printf("\nFirst data block checksum: ");
    print_buf(four_bytes, 4);
    int32_to_bytes_BE(four_bytes, header.data2_size);
    printf("\nSecond data block checksum: ");
    print_buf(four_bytes, 4);
    printf("\nKeep in mind they're saved as little endian in the save file!");

    //Close file and exit
    fclose(fp);
    return 0;
}
