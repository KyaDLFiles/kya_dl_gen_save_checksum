#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "crc32.h"

void int32_to_bytes_BE(uint8_t *array, uint32_t num) { //https://stackoverflow.com/a/3784478
	array[0] = (num >> 24) & 0xFF;
	array[1] = (num >> 16) & 0xFF;
	array[2] = (num >> 8) & 0xFF;
	array[3] = num & 0xFF;
}

void int32_to_bytes_le(uint8_t *array, uint32_t num) {
	array[3] = (num >> 24) & 0xFF;
	array[2] = (num >> 16) & 0xFF;
	array[1] = (num >> 8) & 0xFF;
	array[0] = num & 0xFF;
}

uint32_t bytes_le_to_int32(const uint8_t array[], int size) {
	int res = 0, mult = 1, i;
	for (i = 0; i < size; i++) {
		res += array[i] * mult;
		mult *= 256;
	}
	return res;
}

void print_buf(uint8_t *buffer, int size, char separator[]) {
	int i;
	for (i = 0; i < size; i++) {
		if (i > 0) printf("%s", separator);
			printf("%02X", buffer[i]);
	}
}

uint64_t compute_checksum(const void *buffer, int64_t size) {
	uint64_t result;
	int64_t v4;
	uint8_t v5;
	int64_t v6;

	result = 0xFFFFFFFFFFFFFFFFLL;
	v4 = (int)size - 1;
	if (size) {
		do {
			v5 = *(uint8_t *)buffer;
			v6 = v4;
			v4 = (int32_t)v4 - 1;
			buffer = (uint8_t *)buffer + 1;
			result = ((uint32_t)result >> 8) ^ (uint64_t)crc32_tab[(uint8_t)(result ^ v5)];
		} while ( v6 );
	}
	return result;
}

int main(int argc, char *argv[]) {
	//Variable definitions
	struct {
		uint32_t header_csum, header_size,
				data1_csum, data1_size,
				data2_csum, data2_size;
	} header;
	FILE * fp;
	uint8_t *buf, four_bytes[4], NEDE[4] = {0x4e, 0x45, 0x44, 0x45};
	int i;
	_Bool verbose = 0;

	//Check file path argument
	if (argc < 2) {
		fprintf(stderr,"Missing argument!\n"
					   "Usage: gensavecsum(.exe) /path/to/save/file.dat <--verbose>\n");
		return 2;
	}

	if (argc > 2) {
		for (i = 2; i < argc; i++) { //not really useful now, but may be later
			if (strcmp("--verbose", argv[i]) == 0)
				verbose = 1;
			else {
				fprintf(stderr, "Invalid argument(s)!\n");
				return 5;
			}
		}
	}

	//Open file
	if ((fp = fopen(argv[1], "rb+")) == NULL) {
		fprintf(stderr,"Error opening file!\n");
		return 1;
	}

	//Check if NEDE is present at the begiining of the file, if not quit
	fread(four_bytes, 4, 1, fp);
	if (memcmp(four_bytes, NEDE, 4)) {
		fprintf(stderr, "The file is not a KDL save file. Aborting!\n");
		fclose(fp);
		return 3;
	}

	//Read header size
	//(really not needed for the final and sep 29 builds as it's always 1C, but could be useful for may 12)
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
		buf = (uint8_t*)malloc(header.data1_size);
		fread(buf, header.data1_size, 1, fp);
		header.data1_csum = compute_checksum(buf, header.data1_size);
		free(buf);
	}

	//Calculate second data block checksum
	if (header.data2_size == 0)
		header.data2_csum = 0;
	else {
		buf = (uint8_t*)malloc(header.data2_size);
		fread(buf, header.data2_size, 1, fp);
		header.data2_csum = compute_checksum(buf, header.data2_size);
		free(buf);
	}

	if (feof(fp)) {
		fprintf(stderr, "Invalid section size(s) in header! Save file may be damaged\n");
		fclose(fp);
		return 4;
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
	buf = (uint8_t*)malloc(header.header_size - 8);
	fread(buf, header.header_size - 8, 1, fp);

	//Calculate header checksum
	header.header_csum = compute_checksum(buf, header.header_size - 8);
	int32_to_bytes_le(four_bytes, header.header_csum);
	fseek(fp, 4, SEEK_SET);
	fwrite(four_bytes, 4, 1, fp);
	free(buf);

	//Print checksums to console
	if (verbose) {
		printf("Header size: h");
		int32_to_bytes_BE(four_bytes, header.header_size);
		print_buf(four_bytes, 4, "");

		printf("\nFirst data block size: h");
		int32_to_bytes_BE(four_bytes, header.data1_size);
		print_buf(four_bytes, 4, "");

		printf("\nSecond data block size: h");
		int32_to_bytes_BE(four_bytes, header.data2_size);
		print_buf(four_bytes, 4, "");

		int32_to_bytes_BE(four_bytes, header.header_csum);
		printf("\n\nHeader checksum: h(");
		print_buf(four_bytes, 4, ":");

		int32_to_bytes_BE(four_bytes, header.data1_csum);
		printf(")\nFirst data block checksum: h(");
		print_buf(four_bytes, 4, ":");

		int32_to_bytes_BE(four_bytes, header.data2_size);
		printf(")\nSecond data block checksum: h(");
		print_buf(four_bytes, 4, ":");

		printf(")\n\nKeep in mind they're saved as little endian in the save file!");
	}
	
	//Close file and exit
	fclose(fp);
	return 0;
}
