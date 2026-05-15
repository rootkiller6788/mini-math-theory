# Mini Math Theory（迷你数学理论）

**从零开始、零依赖的 C 语言实现**，涵盖计算机科学所需的大学级数学理论。每个模块对应 MIT（及其他顶尖大学）的一门或多门课程，将教科书中的公式转化为可运行的 C 代码，实现理论与实践的桥接。

## 模块总览

| 模块 | 主题 | 参考课程 |
|--------|--------|-------------|
| [mini-algo-ds](mini-algo-ds/) | 数据结构、排序、图算法、动态规划、贪心 | MIT 6.006, MIT 6.046J |
| [mini-computation-theory](mini-computation-theory/) | DFA/NFA、正则引擎、CFG/PDA、图灵机、SAT 求解器 | MIT 6.045J, Sipser |
| [mini-discrete-math](mini-discrete-math/) | 逻辑、数论、集合与关系、图论、组合数学、布尔代数 | MIT 6.042J |
| [mini-formal-methods](mini-formal-methods/) | Hoare 逻辑合约、不变量、符号执行、有界模型检测 | UPenn Software Foundations, CMU |
| [mini-information-theory](mini-information-theory/) | 熵、互信息、编码理论、压缩、纠错码 | MIT 6.441, Cover & Thomas |
| [mini-linear-algebra](mini-linear-algebra/) | 向量、矩阵、分解（LU/QR/Cholesky）、特征值、SVD、稀疏运算 | MIT 18.06, MIT 18.065, Stanford CS229 |
| [mini-prob-stats](mini-prob-stats/) | 概率、分布、贝叶斯推断、假设检验、回归、蒙特卡洛 | MIT 18.05, Stanford CS229 |
| [mini-system-theory](mini-system-theory/) | 状态空间模型、稳定性、PID 控制、系统动力学、排队论、权衡空间分析 | MIT 16.842, MIT 6.241J |

## 设计理念

- **零外部依赖** — 纯 C（C99/C11），仅使用 `libc` 和 `libm`
- **模块自包含** — 每个目录自带 `Makefile`、`include/`、`src/`、`examples/`、`demos/`、`tests/`
- **理论到代码的映射** — 每个模块包含 `docs/` 目录，内有课程对齐说明
- **实用演示程序** — 调度器、路径搜索、压缩器、推荐系统、控制器等

## 构建方式

每个模块相互独立。进入模块目录后运行：

```bash
cd mini-algo-ds
make all    # 构建全部
make test   # 运行测试
```

需要 **GCC** 和 **GNU Make**。

## 项目结构

```
mini-math-theory/
├── mini-algo-ds/              # 算法与数据结构
├── mini-computation-theory/   # 计算理论
├── mini-discrete-math/        # 离散数学
├── mini-formal-methods/       # 形式化方法与验证
├── mini-information-theory/   # 信息论
├── mini-linear-algebra/       # 线性代数与矩阵方法
├── mini-prob-stats/           # 概率论与数理统计
└── mini-system-theory/        # 系统工程与控制理论
```

## 许可证

MIT
