#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

uint16_t* readInstructions(FILE* fp, int *size) {
    /* 1. Determine the length of the file. */
    fseek(fp, 0, SEEK_END);
    int length = ftell(fp) / 2; // There are bytesize/2 instructions
    fseek(fp, 0, SEEK_SET);

    /* 2. Allocate memory to store the instructions */
    uint16_t* arr = (uint16_t*) malloc(length * sizeof(uint16_t));
    if (arr == NULL) {
        return NULL;
    }

    /* 3. Read the instructions into the array, at once. */
    fread(arr, 2, length, fp);

    /* 4. Swap the bytes, as CHIP8 instructions are in Big Endian.*/
    for (int i = 0; i < length; i++) {
        arr[i] = ((arr[i] & 0x00FF) << 8) | (arr[i] >> 8);
    }

    *size = length;
    return arr;
}

void translateOpcode(uint16_t opcode, char* output) {
    // Isolate the single hex digits, in case we need them
    uint8_t p1 = opcode >> 12;
    uint8_t p2 = (opcode & 0x0F00) >> 8;
    uint8_t p3 = (opcode & 0x00F0) >> 4;
    uint8_t p4 = opcode & 0x000F;

    switch (p1) {
        case 0x0:
            switch (opcode & 0x00FF) {
                case 0xE0:
                    sprintf(output, "CLS");
                    break;
                case 0xEE:
                    sprintf(output, "RET");
                    break;
                default:
                    sprintf(output, "[invalid instruction]");
                    break;
            }
            break;
        case 0x1:
            sprintf(output, "JP 0x%03X", opcode & 0x0FFF);
            break;
        case 0x2:
            sprintf(output, "CALL 0x%03X", opcode & 0x0FFF);
            break;
        case 0x3:
            sprintf(output, "SE V%X, 0x%02X", p2, opcode & 0x00FF);
            break;
        case 0x4:
            sprintf(output, "SNE V%X, 0x%02X", p2, opcode & 0x00FF);
            break;
        case 0x5:
            sprintf(output, "SE V%X, V%X", p2, p3);
            break;
        case 0x6:
            sprintf(output, "LD V%X, 0x%02X", p2, opcode & 0x00FF);
            break;
        case 0x7:
            sprintf(output, "ADD V%X, 0x%02X", p2, opcode & 0x00FF);
            break;
        case 0x8:
            switch (p4) {
                case 0x0:
                    sprintf(output, "LD V%X, V%X", p2, p3);
                    break;
                case 0x1:
                    sprintf(output, "OR V%X, V%X", p2, p3);
                    break;
                case 0x2:
                    sprintf(output, "AND V%X, V%X", p2, p3);
                    break;
                case 0x3:
                    sprintf(output, "XOR V%X, V%X", p2, p3);
                    break;
                case 0x4:
                    sprintf(output, "ADD V%X, V%X", p2, p3);
                    break;
                case 0x5:
                    sprintf(output, "SUB V%X, V%X", p2, p3);
                    break;
                case 0x6:
                    sprintf(output, "SHR V%X", p2);
                    break;
                case 0x7:
                    sprintf(output, "SUBN V%X, V%X", p2, p3);
                    break;
                case 0xE:
                    sprintf(output, "SHL V%X", p2);
                    break;
                default:
                    sprintf(output, "[invalid instruction]");
                    break;
            }
            break;
        case 0x9:
            sprintf(output, "SNE V%X, V%X", p2, p3);
            break;
        case 0xA:
            sprintf(output, "LD I, 0x%03X", opcode & 0x0FFF);
            break;
        case 0xB:
            sprintf(output, "JP V0, 0x%03X", opcode & 0x0FFF);
            break;
        case 0xC:
            sprintf(output, "RND V%X, 0x%02X", p2, opcode & 0x00FF);
            break;
        case 0xD:
            sprintf(output, "DRW V%X, V%X, 0x%X", p2, p3, p4);
            break;
        case 0xE:
            switch (opcode & 0x00FF) {
                case 0x9E:
                    sprintf(output, "SKP V%X", p2);
                    break;
                case 0xA1:
                    sprintf(output, "SNKP V%X", p2);
                    break;
                default:
                    sprintf(output, "[invalid instruction]");
                    break;
            }
            break;
        case 0xF:
            switch (opcode & 0x00FF) {
                case 0x07:
                    sprintf(output, "LD V%X, DT", p2);
                    break;
                case 0x0A:
                	sprintf(output, "LD V%X, K", p2);
                	break;
                case 0x15:
                	sprintf(output, "LD DT, V%X", p2);
                	break;
                case 0x18:
                	sprintf(output, "LD ST, V%X", p2);
                	break;
                case 0x1E:
                	sprintf(output, "ADD I, V%X", p2);
                	break;
                case 0x29:
                	sprintf(output, "LD F, V%X", p2);
                	break;
                case 0x33:
                	sprintf(output, "LD B, V%X", p2);
                	break;
                case 0x55:
                	sprintf(output, "LD [I], V%X", p2);
                	break;
                case 0x65:
                	sprintf(output, "LD V%X, [I]", p2);
                	break;
                default:
                    sprintf(output, "[invalid instruction]");
                    break;
            }
            break;
        default:
            sprintf(output, "[invalid instruction]");
            break;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Please provide the filepath to a CHIP8 ROM file.\n");
        return 1;
    }

    FILE* fp = fopen(argv[1], "rb");
    int fsize = 0;
    uint16_t* opcodes = readInstructions(fp, &fsize);
    fclose(fp);

    int out_fn_len = strlen(argv[1]) + 5; // +4 for ".asm" and +1 for '\0'
    char* out_fn = (char*) malloc(out_fn_len * sizeof(char));
    if (out_fn == NULL) {
        printf("Error: Failed to allocate memory.\n");
        return 1;
    }
    sprintf(out_fn, "%s.asm", argv[1]);

    int show_memaddr = 0;
    if (argc >= 3 && (strcmp(argv[2], "--maddr") == 0)) show_memaddr = 1;

    FILE* out_fp = fopen(out_fn, "w");
    char line[32] = "";
    printf("0/%d instructions translated.", fsize);
    for (int i = 0; i < fsize; i++) {
        translateOpcode(opcodes[i], line);

        if (show_memaddr) fprintf(out_fp, "0x%03X: ", 0x200 + i * 2);
        fputs(line, out_fp);
        fputc('\n', out_fp);
        printf("\r%d/%d instructions translated.", i + 1, fsize);
    }
    fclose(out_fp);
    printf("\nThe ROM has been fully translated.");

    return 0;
}
