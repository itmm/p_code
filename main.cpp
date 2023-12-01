#include <iostream>
#include <iomanip>
#include <cassert>
#include <stdexcept>

#include "stack.h"

enum Command {
    lit, opr, lod, sto, cal, inc, jpc, jmp
};

enum Operation {
    retrn, neg, add, sub, times, divide, mod, equals, unequals, less, greater,
    less_or_equals, greater_or_equals,
    max_operation = greater_or_equals

};

enum Operation to_op(int value = 0) {
    if (value < 0 || value > max_operation) { throw std::domain_error { "unknown operation" }; }
    return static_cast<enum Operation>(value);
}

struct Instruction {
    enum Command command;
    int level;
    int value;

    Instruction(enum Command cmd, int val, int lev = 0): command { cmd }, level { lev }, value { val} { }
    explicit Instruction(enum Operation op): command { opr }, level { 0 }, value { static_cast<int>(op) } { }

    explicit Instruction(const int code):
        command { static_cast<enum Command>((code >> 4) & 0xf) },
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
    assert(code_begin && code_begin <= code_end);

    const int *cp = code_begin;
    int ref = 0;

    do {
        if (cp >= code_end) { throw std::range_error { "out of code segment" }; }
        const struct Instruction inst { *cp++ };
        switch (inst.command) {
            case lit: {
                stack.push(inst.value);
                break;
            }
            case opr: {
                switch (to_op(inst.value)) {
                    case retrn: {
                        stack.resize(ref + 1 - stack.size());
                        ref = stack.pop();
                        cp = code_begin + stack.pop();
                        break;
                    }
                    case neg: { stack([&](int a) { return -a; }); break; }
                    case add: { stack([&](int a, int b) { return a + b; }); break; }
                    case sub: { stack([&](int a, int b) { return a - b; }); break; }
                    case times: { stack([&](int a, int b) { return a * b; }); break; }
                    case divide: { stack([&](int a, int b) { return a / b; }); break; }
                    case mod: { stack([&](int a, int b) { return a % b; }); break; }
                    case equals: { stack([&](int a, int b) { return a == b; }); break; }
                    case unequals: { stack([&](int a, int b) { return a != b; }); break; }
                    case less: { stack([&](int a, int b) { return a < b; }); break; }
                    case greater: { stack([&](int a, int b) { return a > b; }); break; }
                    case less_or_equals: { stack([&](int a, int b) { return a <= b; }); break; }
                    case greater_or_equals: { stack([&](int a, int b) { return a >= b; }); break; }
                }
                break;
            }
            case lod: {
                stack.push(get(stack, ref, inst.value, inst.level));
                break;
            }
            case sto: {
                get(stack, ref, inst.value, inst.level) = stack.pop();
                break;
            }
            case cal: {
                stack.push(static_cast<int>(cp - code_begin));
                stack.push(ref);
                ref = stack.size() - 1;
                cp = code_begin + inst.value;
                break;
            }
            case inc: {
                stack.resize(inst.value);
                break;
            }
            case jpc: {
                const int cond = stack.pop();
                if (! cond) { break; }
                // fall through
            }
            case jmp: {
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
        Instruction { lit, 0x002 },
        Instruction { lit, 0x200 },
        Instruction { lit, 0x400 },
        Instruction { add },
        Instruction { divide }
    };

    try {
        interpret(code, code + std::size(code), stack);
    } catch (const std::exception &ex) {
        std::cerr << "breaking with: " << ex.what() << "\n";
        std::cerr << "stack:\n";
        for (int i = 0; i < stack.size(); ++i) {
            std::cerr << "\t" << i << "\t0x" << std::hex << std::setw(8) << std::setfill('0') << stack[i] << std::dec << "\n";
        }
    }
    return 0;
}
