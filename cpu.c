#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// These are just simple values that can be moved around for removing software defined memory limits for turing completeness
#define STACK_SIZE 65536
#define CALL_STACK_SIZE 65536
#define RAM_SIZE 65536

struct STACK {
    uint32_t SP; // Set this to zero on declaration
    uint64_t *data; // Declare this to all zeros on declaration
    uint64_t *CallStack;
    uint32_t CSP;
};

struct CPU {
    struct STACK stack;
    uint8_t running;
    uint8_t* code;
    uint64_t ip;
    uint64_t reg;
    uint64_t RamPointer;
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

// ALL of these need debuged

int DUP(struct CPU *cpu) {
    if(cpu->stack.SP == 0) {return 1;}
    cpu->stack.data[cpu->stack.SP] = cpu->stack.data[cpu->stack.SP - 1];
    cpu->stack.SP++;
    return 0;
}
int DRP(struct CPU *cpu) {
    if(cpu->stack.SP == 0) {return 1;}
    cpu->stack.SP--;
}
int ROT(struct CPU *cpu) {
    if(cpu->stack.SP < 3) {return 1;}
    cpu->stack.SP--;
    int A = cpu->stack.data[cpu->stack.SP--];    
    int B = cpu->stack.data[cpu->stack.SP--];
    int C = cpu->stack.data[cpu->stack.SP--];

    cpu->stack.data[cpu->stack.SP++] = A;
    cpu->stack.data[cpu->stack.SP++] = C;
    cpu->stack.data[cpu->stack.SP++] = B;

    return 0;
}
int NIP(struct CPU *cpu) {
    if(cpu->stack.SP < 2) {return 1;}
    cpu->stack.SP--;
    int A = cpu->stack.data[cpu->stack.SP--];
    cpu->stack.data[cpu->stack.SP++] = A;
    return 0;
}
int IMT(struct CPU *cpu) {
    if(cpu->stack.SP == 0) {
        cpu->reg = 0;
        return 0;
    }
    cpu->reg = 1;
    return 0;
}
int SIZ(struct CPU *cpu) {
    cpu->reg = cpu->stack.SP;
}

int ADD(struct CPU *cpu) {
    if(cpu->stack.SP == 0) {
        cpu->stack.data[cpu->stack.SP] = cpu->reg;
        cpu->stack.SP++;
    } else {
        cpu->stack.data[cpu->stack.SP-1] = cpu->stack.data[cpu->stack.SP-1] + cpu->reg;
    }
    return 0;
}
int SUB(struct CPU *cpu) {
    if(cpu->stack.SP == 0) {
        cpu->stack.data[cpu->stack.SP] = 0xffffffffffffffff - cpu->reg;
        cpu->stack.SP++;
    } else {
        cpu->stack.data[cpu->stack.SP-1] = cpu->stack.data[cpu->stack.SP-1] - cpu->reg;
    }
        return 0;
}

int INC(struct CPU *cpu) {
    cpu->reg++;
    return 0;
}
int DEC(struct CPU *cpu) {
    cpu->reg--;
    return 0;
}

int ICS(struct CPU *cpu) {
    if(cpu->stack.SP == 0) {return 1;}
    cpu->stack.data[cpu->stack.SP-1]++;
    return 0;
}
int DCS(struct CPU *cpu) {
    if(cpu->stack.SP == 0) {return 1;}
    cpu->stack.data[cpu->stack.SP-1]--;
    return 0;
}

int LDI(struct CPU *cpu) {
    uint64_t total = 0;

    cpu->ip++;
    total = total + ((uint64_t)(cpu->code[cpu->ip]) << 24);
    cpu->ip++;
    total = total + ((uint64_t)(cpu->code[cpu->ip]) << 16);
    cpu->ip++;
    total = total + ((uint64_t)(cpu->code[cpu->ip]) << 8);
    cpu->ip++;
    total = total + ((uint64_t)(cpu->code[cpu->ip]));

    cpu->reg = total;
    return 0;
}
int JMP(struct CPU *cpu) {
    uint64_t total = 0;

    cpu->ip++;
    total = total + (((uint64_t)cpu->code[cpu->ip]) << 24);
    cpu->ip++;
    total = total + (((uint64_t)cpu->code[cpu->ip]) << 16);
    cpu->ip++;
    total = total + (((uint64_t)cpu->code[cpu->ip]) << 8);
    cpu->ip++;
    total = total + (((uint64_t)cpu->code[cpu->ip]));

    if(total > cpu->CodeSize) {
        printf("JUMP tried to go out of bounds, exiting now.\n");
        exit(7);
    }
    cpu->ip = total;
    return 0;
}
int JEQ(struct CPU *cpu) {
    if(cpu->stack.SP == 0) {
        printf("Jump failed due to stack underflow, exiting now.\n");
        exit(10);
    }
    if(cpu->reg == cpu->stack.data[cpu->stack.SP - 1]) {
        uint64_t total = 0;

        cpu->ip++;
        total = total + (((uint64_t)cpu->code[cpu->ip]) << 24);
        cpu->ip++;
        total = total + (((uint64_t)cpu->code[cpu->ip]) << 16);
        cpu->ip++;
        total = total + (((uint64_t)cpu->code[cpu->ip]) << 8);
        cpu->ip++;
        total = total + (((uint64_t)cpu->code[cpu->ip]));

        if(total > cpu->CodeSize) {
            printf("JUMP tried to go out of bounds, exiting now.\n");
            exit(7);
        }

        cpu->ip = total;
    } else {
        cpu->ip++;
    }
    return 0;
}
int SLT(struct CPU *cpu) {
    if(cpu->stack.SP == 0) {cpu->reg = 0;}
    else if(cpu->stack.data[cpu->stack.SP-1] < cpu->reg) {cpu->reg = 1;}
    else {cpu->reg = 0;}
    return 0;
}
int STR(struct CPU *cpu) {
    if(cpu->stack.SP == 0) {
        cpu->ram[0] = cpu->reg;
    } else {
        cpu->ram[cpu->stack.data[cpu->stack.SP-1]] = cpu->reg;
    }
    return 0;
}
int LDR(struct CPU *cpu) {
    if(cpu->stack.SP == 0) {
        cpu->reg = cpu->ram[0];
    } else {
        cpu->reg = cpu->ram[cpu->stack.data[cpu->stack.SP-1]];
    }
    return 0;
}
void HLT(struct CPU *cpu) {
    exit(cpu->reg);
}
int LDP(struct CPU *cpu) {
    if(cpu->RamPointer > RAM_SIZE) {
        cpu->reg = cpu->ram[RAM_SIZE - 1];
    } else {
        cpu->reg = cpu->ram[cpu->RamPointer];
    }
    return 0;
}
int STP(struct CPU *cpu) {
    if(cpu->RamPointer > RAM_SIZE) {
        cpu->ram[RAM_SIZE - 1] = cpu->reg;
    } else {
        cpu->ram[cpu->RamPointer] = cpu->reg;
    }
    return 0;
}
int ICP(struct CPU *cpu) {
    if(cpu->RamPointer == RAM_SIZE - 1) {
        cpu->RamPointer = 0;
    } else {
        cpu->RamPointer++;
    }
    return 0;
}
int DCP(struct CPU *cpu) {
    if(cpu->RamPointer == 0) {
        cpu->RamPointer = RAM_SIZE - 1;
    } else {
        cpu->RamPointer--;
    }
    return 0;
}
int SRP(struct CPU *cpu) {
    if(cpu->reg >= RAM_SIZE) {
        cpu->RamPointer = RAM_SIZE - 1;
    } else {
        cpu->RamPointer = cpu->reg;
    }
    return 0;
}

int CALL(struct CPU *cpu) {
    cpu->stack.CallStack[cpu->stack.CSP++] = cpu->ip;
    JMP(cpu);
    return 0;
}
int RET(struct CPU *cpu) {
    if(cpu->stack.CSP == 0) {
        HLT(cpu);
    }
    cpu->stack.CSP--;
    cpu->ip = cpu->stack.CallStack[cpu->stack.CSP];
    return 0;
}

int interupt(struct CPU *cpu) {
    switch (cpu->reg) {
        case 0:
            if(cpu->stack.SP == 0) {
                printf("%c", 0);
            } else {
                cpu->stack.SP--;
                printf("%c", cpu->stack.data[cpu->stack.SP]);
            }
            break;
        case 1:
            if(cpu->stack.SP == 0) {
                printf("%c", 0);
            } else {
                cpu->stack.SP--;
                printf("%llu", cpu->stack.data[cpu->stack.SP]);
            }
            break;
        case 2:
            cpu->stack.SP = 0;
            break;
        case 3:
            if(cpu->stack.SP == 0) {
                srand(0);
            } else {
                cpu->stack.SP--;
                srand(cpu->stack.data[cpu->stack.SP]);
            }
            break;
        case 4:
            if(cpu->stack.SP == 0 || cpu->stack.SP == 1) {
                printf("Stack underflow, exiting now.\n");
                exit(9);
            } else {
                cpu->stack.SP--;
                uint64_t MIN = cpu->stack.data[cpu->stack.SP--];
                uint64_t MAX = cpu->stack.data[cpu->stack.SP--];
                if(MAX > MIN) {
                    cpu->reg = (rand() % (MAX - MIN + 1)) + MIN;
                } else {
                    cpu->reg = (rand() % (MIN - MAX + 1)) + MAX;
                }
            }
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
    srand(time(NULL));

    uint8_t* code;

    struct CPU cpu = {
        .stack = {
            .SP = 0,
            .data = calloc(STACK_SIZE, 8),
            .CallStack = calloc(CALL_STACK_SIZE, 8),
            .CSP = 0
        },
        .running = 1,
        .ip = 0,
        .reg = 0,
        .RamPointer = 0,
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
            case 12:
                LDP(&cpu);
                break;
            case 13:
                STP(&cpu);
                break;
            case 14:
                ICP(&cpu);
                break;
            case 15:
                DCP(&cpu);
                break;
            case 16:
                SRP(&cpu);
                break;
            case 17:
                CALL(&cpu);
                break;
            case 18:
                RET(&cpu);
                break;
            case 19:
                INC(&cpu);
                break;
            case 20:
                DEC(&cpu);
                break;
            case 21:
                INC(&cpu);
                break;
            case 22:
                DEC(&cpu);
                break;
            case 23:
                DUP(&cpu);
                break;
            case 24:
                DRP(&cpu);
                break;
            case 25:
                ROT(&cpu);
                break;
            case 26:
                NIP(&cpu);
                break;
            case 27:
                IMT(&cpu);
                break;
            case 28:
                SIZ(&cpu);
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
