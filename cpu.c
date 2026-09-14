#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STACK_SIZE 65536
#define RAM_SIZE 65536

struct STACK {
    uint32_t SP; // Set this to zero on declaration
    uint64_t *data; // Declare this to all zeros on declaration
};

struct CPU {
    struct STACK stack;
    uint8_t running;
    uint8_t* code;
    uint64_t ip;
    uint64_t reg;
    uint64_t *ram;
    uint32_t CodeSize;
};

int POP(struct CPU *cpu) {
    if(cpu->stack.SP == 0) {
        cpu->reg = 0;
        return 0;
    } else {
        cpu->stack.SP--;
        cpu->reg = cpu->stack.data[cpu->stack.SP];
        return 0;
    }
}
int PSH(struct CPU *cpu) {
    if(cpu->stack.SP == STACK_SIZE) {
        printf("\nSTACK OVERFLOW\nEXITING NOW\n");
        exit(2);
    } else {
        cpu->stack.data[cpu->stack.SP] = cpu->reg;
        cpu->stack.SP++;
        return 0;
    }
}
int ADD(struct CPU *cpu) {
    cpu->stack.data[cpu->stack.SP-1] = cpu->stack.data[cpu->stack.SP-1] + cpu->reg;
    return 0;
}
int SUB(struct CPU *cpu) {
    cpu->stack.data[cpu->stack.SP-1] = cpu->stack.data[cpu->stack.SP-1] - cpu->reg;
    return 0;
}
int LDI(struct CPU *cpu) {
    uint64_t total = 0;

    cpu->ip++;
    total = total + (cpu->code[cpu->ip] << 24);
    cpu->ip++;
    total = total + (cpu->code[cpu->ip] << 16);
    cpu->ip++;
    total = total + (cpu->code[cpu->ip] << 8);
    cpu->ip++;
    total = total + (cpu->code[cpu->ip]);

    cpu->reg = total;
    return 0;
}
int JMP(struct CPU *cpu) {
    uint64_t total = 0;

    cpu->ip++;
    total = total + (cpu->code[cpu->ip] << 24);
    cpu->ip++;
    total = total + (cpu->code[cpu->ip] << 16);
    cpu->ip++;
    total = total + (cpu->code[cpu->ip] << 8);
    cpu->ip++;
    total = total + (cpu->code[cpu->ip]);

    cpu->ip = total;
    return 0;
}
int JEQ(struct CPU *cpu) {
    if(cpu->reg == cpu->stack.data[cpu->stack.SP - 1]) {
        uint64_t total = 0;

        cpu->ip++;
        total = total + (cpu->code[cpu->ip] << 24);
        cpu->ip++;
        total = total + (cpu->code[cpu->ip] << 16);
        cpu->ip++;
        total = total + (cpu->code[cpu->ip] << 8);
        cpu->ip++;
        total = total + (cpu->code[cpu->ip]);

        cpu->ip = total;
        return 0;
    } else {
        return 0;
    }
}
int SLT(struct CPU *cpu) {
    if(cpu->stack.SP == 0) {cpu->reg = 0;}
    if(cpu->stack.data[cpu->stack.SP-1] == cpu->reg) {cpu->reg = 1;}
    else {cpu->reg = 0;}
    return 0;
}
int STR(struct CPU *cpu) {
    cpu->ram[cpu->stack.data[cpu->stack.SP-1]] = cpu->reg;
    return 0;
}
int LDR(struct CPU *cpu) {
    cpu->reg = cpu->ram[cpu->stack.data[cpu->stack.SP-1]];
    return 0;
}
void HLT(struct CPU *cpu) {
    exit(cpu->reg);
}
int interupt(struct CPU *cpu) {
    switch (cpu->reg) {
        case 0:
            cpu->stack.SP--;
            printf("%c", cpu->stack.data[cpu->stack.SP]);
            break;
        case 1:
            cpu->stack.SP = 0;
            break;
        default:
            break;
    }
    return 0;
}

void ReadFile(struct CPU *cpu) {
    char FilePath[512];
    printf("What is the file that should be run:\n");
    
    if (fgets(FilePath, sizeof(FilePath), stdin) == NULL) {
        printf("Failed to read user input, exiting now.\n");
        exit(5);
    }

    FilePath[strcspn(FilePath, "\n")] = '\0';
    
    FILE *fptr;
    fptr = fopen(FilePath, "rb");

    if(fptr == NULL) {printf("Sorry, but there was an error reading that file, exiting now.\n"); exit(1);}

    fseek(fptr, 0, SEEK_END);
    long FileSize = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);

    if (FileSize == 0) {
        printf("The file is empty, exiting now.\n");
        exit(6);
    }

    uint8_t *code = calloc(FileSize, 1);
    if(code == NULL) {printf("calloc failed in ReadFile function. exiting now.\n"); exit(2);}
    fread(code, FileSize, 1, fptr);

    fclose(fptr);

    cpu->code = code;
    cpu->CodeSize = FileSize;

    return;
}

int main() {

    uint8_t* code;

    struct CPU cpu = {
        .stack = {
            .SP = 0,
            .data = calloc(STACK_SIZE, 8)
        },
        .running = 1,
        .ip = 0,
        .reg = 0,
        .ram = calloc(RAM_SIZE, 8)
    };
    ReadFile(&cpu);
    
    if (cpu.stack.data == NULL) {printf("calloc failure on cpu.stack.data. exiting now.\n"); exit(3);}
    if (cpu.ram == NULL) {printf("calloc failure on cpu.ram. exiting now.\n"); exit(4);}

    // These are all of the variables that will be changed throughout the running loop
    uint8_t command;

    while(cpu.running) {
        // Put the switch case here

        command = cpu.code[cpu.ip];
        switch (command) {
            case 0:
                interupt(&cpu);
                cpu.ip++;
                break;
            case 1:
                POP(&cpu);
                cpu.ip++;
                break;
            case 2:
                PSH(&cpu);
                cpu.ip++;
                break;
            case 3:
                ADD(&cpu);
                cpu.ip++;
                break;
            case 4:
                SUB(&cpu);
                cpu.ip++;
                break;
            case 5:
                LDI(&cpu);
                cpu.ip++;
                break;
            case 6:
                JMP(&cpu);
                break;
            case 7:
                JEQ(&cpu);
                break;
            case 8:
                SLT(&cpu);
                cpu.ip++;
                break;
            case 9:
                STR(&cpu);
                cpu.ip++;
                break;
            case 10:
                LDR(&cpu);
                cpu.ip++;
                break;
            case 11:
                HLT(&cpu);
                break;
            default:
                break;
        }

        if(cpu.CodeSize == cpu.ip) {cpu.running = 0;}
    }

    free(cpu.stack.data);
    free(cpu.ram);
    return 0;
}