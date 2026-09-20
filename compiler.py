def From64To8(number: int):
    binary = []
    for i in range(4):
        binary.append((number >> (24 - (8 * i))) & 0xff)
    return binary

opcodes = {
    "INT":0, 0:"INT",
    "POP":1, 1:"POP",
    "PSH":2, 2:"PSH",
    "ADD":3, 3:"ADD",
    "SUB":4, 4:"SUB",
    "LDI":5, 5:"LDI",
    "JMP":6, 6:"JMP",
    "JEQ":7, 7:"JEQ",
    "SLT":8, 8:"SLT",
    "STR":9, 9:"STR",
    "LDR":10, 10:"LDR",
    "HLT":11, 11:"HLT",
    "LDP":12, 12:"LDP",
    "STP":13, 13:"STP",
    "ICP":14, 14:"ICP",
    "DCP":15, 15:"DCP",
    "SRP":16, 16:"SRP",
    "CALL":17, 17:"CALL",
    "RET":18, 18:"RET",
    "ICS":19, 19:"ICS",
    "DCS":20, 20:"DCS",
    "INC":21, 21:"INC",
    "DEC":22, 22:"DEC",
}

file = input("input the file to compile without file extention:\n")

ReadLines = []
JumpDict = {}

with open(file, 'r') as file:
    for line in file:
        if(line == '\n'):
            continue
        ll=line.split('//', 1)
        lin=(ll[0]).strip()
        if((lin.strip()).split()==[]):
            continue
        # Do comment removeal then do actual command parsing, and also check for a blank line of just '\n' to ignore it above
        ReadLines.append((lin.strip()).split())

# Use "DEF" keyword for defining labels for looping/jumping???

LabelCounter = 0
ByteOffset = 0
for line, value in enumerate(ReadLines):
    if(value[0] == "LDI" or value[0] == "JMP" or value[0] == "JEQ" or value[0] == "CALL"):
        ByteOffset += 4
    if(value[0] == "DEF"):
        JumpDict[value[1]] = (line + ByteOffset - LabelCounter)
        LabelCounter += 1

LineCounter = 0

while(LineCounter < len(ReadLines)):
    if(ReadLines[LineCounter][0] == "DEF"):
        del ReadLines[LineCounter]
    else:
        LineCounter += 1

CompiledList = []

for LineCount, line in enumerate(ReadLines):
    if(len(line) == 1):
        instruction = opcodes.get(line[0], None)
        
        if(instruction == None):
            print(f"There was an error in compiling\n{LineCount}: {line}")
            exit(1)
        
        CompiledList.append(instruction)
    else:
        instruction = opcodes.get(line[0], None)
        
        if(instruction == None):
            print(f"There was an error in compiling\n{LineCount}: {line}")
            exit(1)
        
        CompiledList.append(instruction)
        if(line[0] == "LDI"):
            CompiledList.extend(From64To8(int(line[1])))
        else:
            CompiledList.extend(From64To8(JumpDict.get(line[1], None)))

CompiledBytes = bytearray(CompiledList)

outfile = input("What file should be written to:\n")

with open(outfile, 'wb') as file:
    file.write(CompiledBytes)
