#include <iostream>
#include <iomanip>
#include <stdexcept>

#include "stack.h"

enum class Command {
    lit, opr, lod, sto, cal, inc, jpc, jmp,
    max = jmp
};

enum class Operation {
    retrn, neg, add, sub, times, divide, mod, equals, unequals, less, greater,
    less_or_equals, greater_or_equals,
    max = greater_or_equals
};

Command to_cmd(int value) {
    if (value < 0 || value > static_cast<int>(Command::max)) {
        throw std::domain_error { "unknown command" };
    }
    return static_cast<Command>(value);
}

Operation to_op(int value) {
    if (value < 0 || value > static_cast<int>(Operation::max)) {
        throw std::domain_error { "unknown operation" };
    }
    return static_cast<Operation>(value);
}

struct Instruction {
    Command command;
    int level;
    int value;

    Instruction(Command cmd, int val, int lev = 0):
        command { cmd }, level { lev }, value { val}
    { }

    explicit Instruction(Operation op):
        command { Command::opr }, level { 0 }, value { static_cast<int>(op) }
    { }

    explicit Instruction(const int code):
        command { to_cmd((code >> 4) & 0xf) },
        level { code & 0xf },
        value { code >> 8 }
    { }

    operator int() const { // NOLINT(*-explicit-constructor)
        return (static_cast<int>(command) << 4) | level | (value << 8);
    }
};

int &get(Stack &stack, int ref, int idx, int level) {
    for (; level; --level) { ref = stack[ref]; }
    return stack[ref + idx];
}

void interpret(const int *code_begin, const int *code_end, Stack &stack) {
    if (! code_begin) { throw std::range_error { "no code" }; }

    const int *cp = code_begin;
    int ref = 0;

    do {
        if (cp >= code_end) {
            throw std::range_error { "out of code segment" };
        }
        const Instruction inst { *cp++ };
        switch (inst.command) {
            case Command::lit: {
                stack.push(inst.value);
                break;
            }
            case Command::opr: {
                switch (to_op(inst.value)) {
                    case Operation::retrn: {
                        stack.resize(ref + 1 - stack.size());
                        ref = stack.pop();
                        cp = code_begin + stack.pop();
                        break;
                    }
                    case Operation::neg: {
                        stack([&](int a) { return -a; });
                        break;
                    }
                    case Operation::add: {
                        stack([&](int a, int b) { return a + b; });
                        break;
                    }
                    case Operation::sub: {
                        stack([&](int a, int b) { return a - b; });
                        break;
                    }
                    case Operation::times: {
                        stack([&](int a, int b) { return a * b; });
                        break;
                    }
                    case Operation::divide: {
                        stack([&](int a, int b) { return a / b; });
                        break;
                    }
                    case Operation::mod: {
                        stack([&](int a, int b) { return a % b; });
                        break;
                    }
                    case Operation::equals: {
                        stack([&](int a, int b) { return a == b; });
                        break;
                    }
                    case Operation::unequals: {
                        stack([&](int a, int b) { return a != b; });
                        break;
                    }
                    case Operation::less: {
                        stack([&](int a, int b) { return a < b; });
                        break;
                    }
                    case Operation::greater: {
                        stack([&](int a, int b) { return a > b; });
                        break;
                    }
                    case Operation::less_or_equals: {
                        stack([&](int a, int b) { return a <= b; });
                        break;
                    }
                    case Operation::greater_or_equals: {
                        stack([&](int a, int b) { return a >= b; });
                        break;
                    }
                }
                break;
            }
            case Command::lod: {
                stack.push(get(stack, ref, inst.value, inst.level));
                break;
            }
            case Command::sto: {
                get(stack, ref, inst.value, inst.level) = stack.pop();
                break;
            }
            case Command::cal: {
                stack.push(static_cast<int>(cp - code_begin));
                stack.push(ref);
                ref = stack.size() - 1;
                cp = code_begin + inst.value;
                break;
            }
            case Command::inc: {
                stack.resize(inst.value);
                break;
            }
            case Command::jpc: {
                const int cond = stack.pop();
                if (! cond) { break; }
                // fall through
            }
            case Command::jmp: {
                cp = code_begin + inst.value;
                break;
            }
        }

    } while (! stack.empty());
}

int main() {
    int stack_data[100];
    Stack stack(stack_data, stack_data + std::size(stack_data));

    const int code[] = {
        Instruction { Command::lit, 0x002 },
        Instruction { Command::lit, 0x200 },
        Instruction { Command::lit, 0x400 },
        Instruction { Operation::add },
        Instruction { Operation::divide }
    };

    try {
        interpret(code, code + std::size(code), stack);
    } catch (const std::exception &ex) {
        std::cerr << "breaking with: " << ex.what() << "\n";
        std::cerr << "stack:\n";
        for (int i = 0; i < stack.size(); ++i) {
            std::cerr << "\t" << i << "\t0x" <<
                std::hex << std::setw(8) << std::setfill('0') <<
                stack[i] << std::dec << "\n";
        }
    }
    return 0;
}
