# mini-computation-theory — 计算理论 (C 语言实现)

> 参考 MIT 6.045J Automata, Computability, and Complexity
> 判断"什么问题能被程序解决，什么不能"，所有编译原理、形式语言、算法复杂度理论的根基。

## 模块与课程对照

| 模块 | MIT 6.045J 章节 | CS 应用 |
|------|----------------|---------|
| DFA / NFA | Unit 1: Finite Automata | 词法分析、协议状态机、输入验证 |
| Regex Engine | Thompson NFA → DFA | 正则匹配引擎 (grep 原理) |
| Context-Free Grammar | Unit 2: CFG, PDA | 编译器语法分析 (Yacc/Bison) |
| Pushdown Automaton | CFG ↔ PDA 等价 | 括号匹配、XML 验证 |
| Turing Machine | Unit 3: Computability | 通用计算模型、停机问题 |
| SAT Solver | Unit 4: Complexity, NP | 约束求解、形式验证 |

## 目录结构

```
02-mini-computation-theory/
├── README.md
├── Makefile
├── include/
│   ├── dfa.h            DFA: 确定性有限自动机
│   ├── nfa.h            NFA: 非确定性有限自动机
│   ├── regex.h          Regex: 正则引擎 (Thompson 构造)
│   ├── grammar.h        CFG: 上下文无关文法
│   ├── pda.h            PDA: 下推自动机
│   ├── turing_machine.h TM:  图灵机模拟器
│   └── sat.h            SAT: DPLL 可满足性求解器
├── src/                 实现文件
├── examples/            演示程序
└── bin/                 编译输出
```

## 编译运行

```bash
make all
make test
```

## 核心能力

| 权力 | 说明 |
|------|------|
| **可计算性权** | 判断什么问题能不能被程序解决 |
| **复杂度权** | 判断什么问题理论上能不能高效解决 (P vs NP) |
| **语言权** | 连接正则、上下文无关、编译器、协议解析 |
| **规约权** | 把难问题映射到另一个难问题 |

## 已实现的图灵机

- `a^n b^n` 识别器：用图灵机识别 `{aⁿbⁿ | n≥1}`（非正则，但上下文无关）
- `w#w^R` 识别器：判断回文串（非上下文无关，图灵可判定）
- 二进制递增：计算 `x + 1`（图灵可计算函数示例）

## SAT 求解器

实现 DPLL 算法，包括：
- 单位传播 (Unit Propagation)
- 纯文字消除 (Pure Literal Elimination)
- 递归分支搜索
