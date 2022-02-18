/* On my honor, I have neither given nor received unauthorized aid on this assignment */
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <deque>

#define START_ADDRESS 260
#define BREAK "00011000000000000000000000000000"

int get32BitSignedInteger(std::string binaryString)
{
    int x = stol(binaryString, 0, 2);
    return x;
}

int get16BitSignedInteger(std::string binaryString)
{
    short x = stoi(binaryString, 0, 2);
    return x;
}

class Instruction
{
    static const std::string categoryMap[24];

public:
    std::string binaryCode;
    std::string assemblyCode;
    int opcode;
    int src1;
    int src2;
    int dest;
    bool isBranch;
    bool isSrc1Register;
    bool isSrc2Register;
    bool isDestRegister;
    int cycle;

public:
    Instruction(std::string s)
    {
        binaryCode = s;
        opcode = stoi(s.substr(0, 6), 0, 2);
        assemblyCode = "[" + categoryMap[opcode];
        isBranch = false;
        isSrc1Register = false;
        isSrc2Register = false;
        isDestRegister = false;
        switch (opcode)
        {
        case 0: //Jump
            dest = stol(s.substr(6, 26), 0, 2);
            assemblyCode += " #" + std::to_string(dest * 4);
            isBranch = true;
            break;
        case 1: //BEQ
        case 2: //BNE
            src1 = stoi(s.substr(6, 5), 0, 2);
            src2 = stoi(s.substr(11, 5), 0, 2);
            dest = get16BitSignedInteger(s.substr(16, 16));
            assemblyCode += " R" + std::to_string(src1) + ", R" + std::to_string(src2) +
                            ", #" + std::to_string(dest * 4);
            isBranch = true;
            isSrc1Register = true;
            isSrc2Register = true;
            break;
        case 3: //BGTZ
            src1 = stoi(s.substr(6, 5), 0, 2);
            dest = get16BitSignedInteger(s.substr(16, 16));
            assemblyCode += " R" + std::to_string(src1) + ", #" +
                            std::to_string(dest * 4);
            isBranch = true;
            isSrc1Register = true;
            break;
        case 4: //SW
        case 5: //LW
            src1 = stoi(s.substr(6, 5), 0, 2);
            dest = stol(s.substr(11, 5), 0, 2);
            src2 = get16BitSignedInteger(s.substr(16, 16));
            assemblyCode += " R" + std::to_string(dest) + ", " + std::to_string(src2) +
                            "(R" + std::to_string(src1) + ")";
            isSrc1Register = true;
            isDestRegister = true;
            break;
        case 6: //BREAK
            assemblyCode += "";
            break;
        case 8:  //ADD
        case 9:  //SUB
        case 10: //AND
        case 11: //OR
        case 14: //SRL
            dest = stoi(s.substr(6, 5), 0, 2);
            src1 = stoi(s.substr(11, 5), 0, 2);
            src2 = stoi(s.substr(16, 5), 0, 2);
            assemblyCode += " R" + std::to_string(dest) + ", R" +
                            std::to_string(src1) + ", R" + std::to_string(src2);
            isSrc1Register = true;
            isSrc2Register = true;
            isDestRegister = true;
            break;
        case 12: //SRA
        case 13: //MUL
            dest = stoi(s.substr(6, 5), 0, 2);
            src1 = stoi(s.substr(11, 5), 0, 2);
            src2 = stoi(s.substr(16, 5), 0, 2);
            assemblyCode += " R" + std::to_string(dest) + ", R" +
                            std::to_string(src1) + ", #" + std::to_string(src2);
            isSrc1Register = true;
            isDestRegister = true;
            break;

        case 16: //ADDI
            dest = stoi(s.substr(6, 5), 0, 2);
            src1 = stoi(s.substr(11, 5), 0, 2);
            src2 = get16BitSignedInteger(s.substr(16, 16));
            assemblyCode += " R" + std::to_string(dest) + ", R" +
                            std::to_string(src1) + ", #" + std::to_string(src2);
            isSrc1Register = true;
            isDestRegister = true;
            break;
        case 17: //ANDI
        case 18: //ORI
            dest = stoi(s.substr(6, 5), 0, 2);
            src1 = stoi(s.substr(11, 5), 0, 2);
            src2 = stol(s.substr(16, 16), 0, 2);
            assemblyCode += " R" + std::to_string(dest) + ", R" +
                            std::to_string(src1) + ", #" + std::to_string(src2);
            isSrc1Register = true;
            isDestRegister = true;
            break;
        }

        assemblyCode += "]\n";
    }
};

class InstBuffer
{
public:
    std::deque<Instruction> buf;
    int size;

public:
    InstBuffer(int s)
    {
        size = s;
    }

    bool isEmpty()
    {
        return buf.empty();
    }

    bool isFull()
    {
        return buf.size() >= size;
    }

    void push(Instruction inst)
    {
        buf.push_back(inst);
    }

    void remove(int i)
    {
        buf.erase(buf.begin() + i);
    }

    Instruction pop()
    {
        Instruction inst = buf.front();
        buf.pop_front();
        return inst;
    }

    std::string getBufferString()
    {
        std::string returnVal = "";
        if (size == 1)
        {
            if (buf.size() == 1)
            {
                returnVal += " " + buf[0].assemblyCode;
            }
            else
            {
                returnVal += "\n";
            }
        }
        else
        {

            int i = 0;
            for (; i < buf.size(); i++)
            {
                returnVal += "\tEntry " + std::to_string(i) + ": " + buf[i].assemblyCode;
            }
            for (; i < size; i++)
            {
                returnVal += "\tEntry " + std::to_string(i) + ":\n";
            }
        }
        return returnVal;
    }
};

struct Result
{
    int destination;
    int value;
    int cycle;
};

struct Register
{
    bool isReading;
    bool isWriting;
    int value;
};

class ResultBuffer
{
public:
    std::deque<Result> buf;
    int size;

public:
    ResultBuffer(int s)
    {
        size = s;
    }

    bool isEmpty()
    {
        return buf.empty();
    }

    bool isFull()
    {
        return buf.size() == size;
    }

    void push(Result rslt)
    {
        buf.push_back(rslt);
    }

    Result pop()
    {
        Result result = buf.front();
        buf.pop_front();
        return result;
    }

    std::string getBufferString()
    {
        std::string returnVal = "";
        if (size == 1)
        {
            if (buf.size() == 1)
            {
                returnVal += " [" + std::to_string(buf[0].value) + ", R" + std::to_string(buf[0].destination) + "]\n";
            }
            else
            {
                returnVal += "\n";
            }
        }
        return returnVal;
    }
};

const std::string Instruction::categoryMap[24] = {"J", "BEQ", "BNE", "BGTZ", "SW", "LW", "BREAK", "NA",
                                                  "ADD", "SUB", "AND", "OR", "SRL", "SRA", "MUL", "NA",
                                                  "ADDI", "ANDI", "ORI", "NA", "NA", "NA", "NA", "NA"};

std::vector<std::string> readFile(const char *const filename)
{
    std::ifstream inFile;
    inFile.open(filename);
    std::vector<std::string> inputText;
    if (inFile.is_open())
    {
        while (!inFile.eof())
        {
            std::string temp;
            inFile >> temp;
            if (temp != "\0")
                inputText.push_back(temp);
        }
        inFile.close(); // Close input file
    }

    return inputText;
}

int main(int argc, char *argv[])
{
    if (argc <= 1)
    {
        printf("Please specify the filename for the input byte code");
        return 1;
    }

    const char *const inputFileName = argv[1];
    std::vector<std::string> inputText = readFile(inputFileName);
    std::vector<Instruction> instrList;
    std::vector<int> dataList;

    //Disassembler
    int pc = 0;
    int address = START_ADDRESS;
    while (true)
    {
        Instruction instr(inputText[pc]);
        instrList.push_back(instr);
        if (inputText[pc] == BREAK)
        {
            pc++;
            address += 4;
            break;
        }
        pc++;
        address += 4;
    }

    int dataStartAddress = address;
    for (int i = pc; i < inputText.size(); i++)
    {
        int x = get32BitSignedInteger(inputText[i]);
        dataList.push_back(x);
        address += 4;
    }

    //simulator
    std::ofstream simulatorFile;
    simulatorFile.open("simulation.txt");
    Register registerArray[32];
    InstBuffer waiting(1);
    InstBuffer executed(1);
    InstBuffer buf1(8);
    InstBuffer buf2(2);
    InstBuffer buf3(2);
    InstBuffer buf4(2);
    InstBuffer buf5(1);
    ResultBuffer buf6(1);
    InstBuffer buf7(1);
    ResultBuffer buf8(1);
    InstBuffer buf9(1);
    ResultBuffer buf10(1);

    for (int i = 0; i < 32; i++)
    {
        registerArray[i].value = 0;
        registerArray[i].isReading = false;
        registerArray[i].isWriting = false;
    }
    int cycle = 0;
    int nextPC = 0;
    bool breakInst = false;
    while (!breakInst)
    {
        cycle++;
        if (!executed.isEmpty())
            executed.pop();

        // Fetch Unit
        if (waiting.isEmpty() && !buf1.isFull())
        {

            for (int k = 0; k < 4; k++)
            {
                pc = nextPC;
                nextPC++;
                if (instrList[pc].isBranch)
                {
                    // TODO
                    waiting.push(instrList[pc]);
                    break;
                }
                else if (instrList[pc].opcode == 6)
                {
                    executed.push(instrList[pc]);
                    breakInst = true;
                    break;
                }
                else
                {
                    instrList[pc].cycle = cycle;
                    buf1.push(instrList[pc]);
                }

                if (buf1.isFull())
                    break;
            }
        }

        if (!waiting.isEmpty())
        {
            Instruction instr = waiting.buf.front();
            bool hazard = false;
            if (instr.isSrc1Register && registerArray[instr.src1].isWriting ||
                instr.isSrc2Register && registerArray[instr.src2].isWriting ||
                instr.isDestRegister && registerArray[instr.dest].isWriting)
            {
                hazard = true;
            }

            for (int i = 0; i < buf1.buf.size(); i++)
            {
                if (buf1.buf[i].isDestRegister &&
                    (instr.isSrc1Register && buf1.buf[i].dest == instr.src1 ||
                     instr.isSrc2Register && buf1.buf[i].dest == instr.src2))
                {
                    hazard = true;
                }
            }

            if (!hazard)
            {

                executed.push(waiting.pop());
                switch (instr.opcode)
                {
                case 0:
                    nextPC = instr.dest - START_ADDRESS / 4;
                    break;

                case 1:

                    if (registerArray[instr.src1].value == registerArray[instr.src2].value)
                        nextPC += instr.dest;
                    break;

                case 2:
                    if (registerArray[instr.src1].value != registerArray[instr.src2].value)
                        nextPC += instr.dest;
                    break;
                case 3:
                    if (registerArray[instr.src1].value > 0)
                        nextPC += instr.dest;
                    break;

                default:
                    break;
                }
            }
        }

        // Issue Unit
        bool isStoreInQueue = false;
        std::vector<int> toRemoveList;
        for (int i = 0; i < buf1.buf.size(); i++)
        {

            Instruction instr = buf1.buf[i];
            if (cycle == instr.cycle)
            {
                continue;
            }
            bool hazard = false;
            if (instr.isSrc1Register && registerArray[instr.src1].isWriting ||
                instr.isSrc2Register && registerArray[instr.src2].isWriting ||
                instr.isDestRegister && registerArray[instr.dest].isWriting)
            {
                hazard = true;
            }

            for (int j = 0; j < i; j++)
            {
                if (buf1.buf[j].isDestRegister &&
                    (instr.isSrc1Register && buf1.buf[j].dest == instr.src1 ||
                     instr.isSrc2Register && buf1.buf[j].dest == instr.src2 ||
                     instr.isDestRegister && buf1.buf[j].dest == instr.dest))
                {
                    hazard = true;
                }
                else if (buf1.buf[j].isSrc1Register && instr.isDestRegister && buf1.buf[j].src1 == instr.dest ||
                         buf1.buf[j].isSrc2Register && instr.isDestRegister && buf1.buf[j].src2 == instr.dest)
                {
                    hazard = true;
                }
            }
            switch (instr.opcode)
            {
            case 4:

                if (!buf2.isFull() && !hazard && !isStoreInQueue)
                {
                    registerArray[instr.dest].isWriting = true;
                    registerArray[instr.src1].isReading = true;
                    instr.cycle = cycle;
                    buf2.push(instr);
                    toRemoveList.push_back(i);
                    isStoreInQueue = false;
                }
                else
                {
                    isStoreInQueue = true;
                }
                break;
            case 5:
                if (!buf2.isFull() && !hazard && !isStoreInQueue)
                {
                    registerArray[instr.dest].isWriting = true;
                    registerArray[instr.src1].isReading = true;
                    instr.cycle = cycle;
                    buf2.push(instr);
                    toRemoveList.push_back(i);
                }
                break;

            case 8:
                if (!buf3.isFull() && !hazard)
                {
                    registerArray[instr.dest].isWriting = true;
                    registerArray[instr.src1].isReading = true;
                    registerArray[instr.src2].isReading = true;
                    instr.cycle = cycle;
                    buf3.push(instr);
                    toRemoveList.push_back(i);
                }
                break;
            case 9:
                if (!buf3.isFull() && !hazard)
                {
                    registerArray[instr.dest].isWriting = true;
                    registerArray[instr.src1].isReading = true;
                    registerArray[instr.src2].isReading = true;
                    instr.cycle = cycle;
                    buf3.push(instr);
                    toRemoveList.push_back(i);
                }
                break;
            case 10:
                if (!buf3.isFull() && !hazard)
                {
                    registerArray[instr.dest].isWriting = true;
                    registerArray[instr.src1].isReading = true;
                    registerArray[instr.src2].isReading = true;
                    instr.cycle = cycle;
                    buf3.push(instr);
                    toRemoveList.push_back(i);
                }
                break;
            case 11:
                if (!buf3.isFull() && !hazard)
                {
                    registerArray[instr.dest].isWriting = true;
                    registerArray[instr.src1].isReading = true;
                    registerArray[instr.src2].isReading = true;
                    instr.cycle = cycle;
                    buf3.push(instr);
                    toRemoveList.push_back(i);
                }
                break;
            case 12:
                if (!buf3.isFull() && !hazard)
                {
                    registerArray[instr.dest].isWriting = true;
                    registerArray[instr.src1].isReading = true;
                    instr.cycle = cycle;
                    buf3.push(instr);
                    toRemoveList.push_back(i);
                }
                break;
            case 13:
                if (!buf3.isFull() && !hazard)
                {
                    registerArray[instr.dest].isWriting = true;
                    registerArray[instr.src1].isReading = true;
                    instr.cycle = cycle;
                    buf3.push(instr);
                    toRemoveList.push_back(i);
                }
                break;

            case 14:
                if (!buf4.isFull() && !hazard)
                {
                    registerArray[instr.dest].isWriting = true;
                    registerArray[instr.src1].isReading = true;
                    registerArray[instr.src2].isReading = true;
                    instr.cycle = cycle;
                    buf4.push(instr);
                    toRemoveList.push_back(i);
                }
                break;

            case 16:
                if (!buf3.isFull() && !hazard)
                {
                    registerArray[instr.dest].isWriting = true;
                    registerArray[instr.src1].isReading = true;
                    instr.cycle = cycle;
                    buf3.push(instr);
                    toRemoveList.push_back(i);
                }
                break;
            case 17:
                if (!buf3.isFull() && !hazard)
                {
                    registerArray[instr.dest].isWriting = true;
                    registerArray[instr.src1].isReading = true;
                    instr.cycle = cycle;
                    buf3.push(instr);
                    toRemoveList.push_back(i);
                }
                break;
            case 18:
                if (!buf3.isFull() && !hazard)
                {
                    registerArray[instr.dest].isWriting = true;
                    registerArray[instr.src1].isReading = true;
                    instr.cycle = cycle;
                    buf3.push(instr);
                    toRemoveList.push_back(i);
                }
                break;
            }
        }

        for (int i = toRemoveList.size() - 1; i >= 0; i--)
        {
            buf1.remove(toRemoveList[i]);
        }

        // ALU1
        if (!buf2.isEmpty() && buf2.buf[0].cycle != cycle)
        {
            Instruction instr = buf2.pop();
            instr.cycle = cycle;
            buf5.push(instr);
        }
        // MEM
        if (!buf5.isEmpty() && buf5.buf[0].cycle != cycle)
        {
            int dataIndex;

            Instruction loadStoreInstr = buf5.pop();
            if (loadStoreInstr.opcode == 5)
            {
                Result result;
                dataIndex = (registerArray[loadStoreInstr.src1].value + loadStoreInstr.src2 - dataStartAddress) / 4;
                result.value = dataList[dataIndex];
                result.destination = loadStoreInstr.dest;
                result.cycle = cycle;
                buf8.push(result);
            }
            else if (loadStoreInstr.opcode == 4)
            {
                dataIndex = (registerArray[loadStoreInstr.src1].value + loadStoreInstr.src2 - dataStartAddress) / 4;
                dataList[dataIndex] = registerArray[loadStoreInstr.dest].value;
                registerArray[loadStoreInstr.dest].isWriting = false;
            }
        }

        // ALU 2
        if (!buf3.isEmpty() && buf3.buf[0].cycle != cycle)
        {

            Instruction alu2Instr = buf3.pop();
            Result result;
            result.destination = alu2Instr.dest;
            switch (alu2Instr.opcode)
            {
            case 8: //ADD
                result.value = registerArray[alu2Instr.src1].value + registerArray[alu2Instr.src2].value;
                break;
            case 9: //SUB
                result.value = registerArray[alu2Instr.src1].value - registerArray[alu2Instr.src2].value;
                break;
            case 10: //AND
                result.value = (unsigned int)registerArray[alu2Instr.src1].value & (unsigned int)registerArray[alu2Instr.src2].value;
                break;
            case 11: //OR
                result.value = (unsigned int)registerArray[alu2Instr.src1].value | (unsigned int)registerArray[alu2Instr.src2].value;
                break;
            case 12: //SRL
                result.value = (unsigned int)registerArray[alu2Instr.src1].value >> alu2Instr.src2;
                break;
            case 13: //SRA
                result.value = registerArray[alu2Instr.src1].value >> alu2Instr.src2;
                break;
            case 16: //ADDI
                result.value = registerArray[alu2Instr.src1].value + alu2Instr.src2;
                break;
            case 17: //ANDI
                result.value = registerArray[alu2Instr.src1].value & alu2Instr.src2;
                break;
            case 18: //ORI
                result.value = registerArray[alu2Instr.src1].value | alu2Instr.src2;
                break;
            }
            result.cycle = cycle;
            buf6.push(result);
        }

        // MUL1
        if (!buf4.isEmpty() && buf4.buf[0].cycle != cycle)
        {
            Instruction instr = buf4.pop();
            instr.cycle = cycle;
            buf7.push(instr);
        }

        // MUL2
        if (!buf7.isEmpty() && buf7.buf[0].cycle != cycle)
        {
            Instruction instr = buf7.pop();
            instr.cycle = cycle;
            buf9.push(instr);
        }

        //  MUL3
        if (!buf9.isEmpty() && buf9.buf[0].cycle != cycle)
        {
            Instruction mulInstr = buf9.pop();
            Result result;

            result.value = registerArray[mulInstr.src1].value * registerArray[mulInstr.src2].value;
            result.destination = mulInstr.dest;
            result.cycle = cycle;
            buf10.push(result);
        }

        // WB
        if (!buf6.isEmpty() && buf6.buf[0].cycle != cycle)
        {
            Result result = buf6.pop();
            registerArray[result.destination].value = result.value;
            registerArray[result.destination].isWriting = false;
        }
        if (!buf8.isEmpty() && buf8.buf[0].cycle != cycle)
        {
            Result result = buf8.pop();
            registerArray[result.destination].value = result.value;
            registerArray[result.destination].isWriting = false;
        }
        if (!buf10.isEmpty() && buf10.buf[0].cycle != cycle)
        {
            Result result = buf10.pop();
            registerArray[result.destination].value = result.value;
            registerArray[result.destination].isWriting = false;
        }

        simulatorFile << "--------------------\nCycle " << cycle << ":\n\n";
        simulatorFile << "IF:\n\tWaiting:" << waiting.getBufferString() << "\tExecuted:" << executed.getBufferString();
        simulatorFile << "Buf1:\n"
                      << buf1.getBufferString();
        simulatorFile << "Buf2:\n"
                      << buf2.getBufferString();
        simulatorFile << "Buf3:\n"
                      << buf3.getBufferString();
        simulatorFile << "Buf4:\n"
                      << buf4.getBufferString();
        simulatorFile << "Buf5:"
                      << buf5.getBufferString();
        simulatorFile << "Buf6:"
                      << buf6.getBufferString();
        simulatorFile << "Buf7:"
                      << buf7.getBufferString();
        simulatorFile << "Buf8:"
                      << buf8.getBufferString();
        simulatorFile << "Buf9:"
                      << buf9.getBufferString();
        simulatorFile << "Buf10:"
                      << buf10.getBufferString();
        simulatorFile << "\nRegisters";
        for (int i = 0; i < 32; i++)
        {
            if (i % 8 == 0)
            {
                std::string st = std::to_string(i);
                if (st.length() < 2)
                {
                    st = "0" + st;
                }
                simulatorFile << "\nR" << st << ":";
            }
            simulatorFile << "\t" << std::to_string(registerArray[i].value);
        }
        simulatorFile << "\n\nData";
        for (int i = 0; i < dataList.size(); i++)
        {
            if (i % 8 == 0)
            {
                simulatorFile << "\n"
                              << std::to_string(dataStartAddress + i * 4) << ":";
            }
            simulatorFile << "\t" << std::to_string(dataList[i]);
        }

        if (!breakInst)
            simulatorFile << "\n";
    }
    simulatorFile.close();

    return 0;
}