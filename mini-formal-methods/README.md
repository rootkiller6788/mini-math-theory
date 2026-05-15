# mini-formal-methods — 形式化方法 (C 语言实现)

> 参考 Software Foundations (UPenn) + CMU Formal Methods
> 用 C 实现设计契约、不变式、符号执行、有界模型检查——把"程序正确性"变成可运行的检查。

## 模块

| 模块 | 功能 | SF / CMU 对标 |
|------|------|-------------|
| `contracts` | REQUIRE/ENSURE/INVARIANT 宏 | Hoare Logic {P}C{Q} |
| `assertions` | 扩展断言框架 | 单元测试基础设施 |
| `invariants` | 循环不变式运行时检查 | Loop Invariant Proof |
| `spec` | 形式化规约 (pre + post + inv) | Functional Specification |
| `verification` | 符号执行 + 有界模型检查 | Symbolic Execution, BMC |

## 目录

```
mini-formal-methods/
├── README.md
├── Makefile
├── docs/        course-alignment, SW-foundations, verified-systems
├── include/     5 个头文件
├── src/         5 个实现
├── examples/    3 个验证示例 (stack, queue, sort)
├── demos/       3 个系统 demo (stack, scheduler, policy-engine)
├── tests/
└── benches/
```

## 编译运行

```bash
make all    # 编译并运行所有示例
```

## 核心概念

| 概念 | 我们的实现 | 工业对应 |
|------|-----------|---------|
| 前置条件 | `REQUIRE(cond, msg)` | `assert` in code |
| 后置条件 | `ENSURE(cond, msg)` | Unit test assertions |
| 类不变式 | `INVARIANT(cond, msg)` | `assert` in destructor |
| 循环不变式 | `loop_inv_maintain(&inv, state)` | Dafny `invariant` clause |
| 符号执行 | `sym_exec_explore()` | KLEE, SMT solvers |
| 有界模型检查 | `bmc_check()` | CBMC, NuSMV |
