# mini-discrete-math — 离散数学 (C 语言实现)

> 参考 MIT 6.042J (Mathematics for Computer Science) + 6.045J
> 计算机所有算法、数据库、网络、编译原理的底层逻辑根基，全部手写 C 实现。

## 模块与 MIT 课程对照

| 模块 | 对应 MIT 6.042J 章节 | CS 应用 |
|------|---------------------|---------|
| 01-逻辑与证明 | Unit 1: Proofs, Induction | 程序正确性验证、归纳证明 |
| 02-数论 | Unit 2: Number Theory | RSA 加密、哈希、模运算 |
| 03-集合与关系 | Unit 3: Sets & Relations | Python set、SQL 关系模型 |
| 04-图论 | Unit 4: Graph Theory | 网络路由、社交图谱、调度 |
| 05-树结构 | Unit 5: Trees | B-Tree 索引、编译器 AST、Huffman |
| 06-组合数学 | Unit 6: Counting | 算法复杂度、密码学、概率 |
| 07-布尔代数 | 延伸 | 数字电路、位运算、SQL WHERE |

## 目录结构

```
mini-discrete-math/
├── README.md
├── 01-proofs-logic/        logic.h / logic.c / main.c
├── 02-number-theory/       number.h / number.c / main.c
├── 03-sets-relations/      set.h / set.c / relation.h / relation.c / main.c
├── 04-graph-theory/        graph.h / graph.c / main.c
├── 05-trees/               tree.h / tree.c / main.c
├── 06-counting/            combo.h / combo.c / main.c
├── 07-boolean-algebra/     boolalg.h / boolalg.c / main.c
```

## 编译运行

```bash
cd mini-discrete-math

gcc -o bin/logic   01-proofs-logic/logic.c   01-proofs-logic/main.c -lm && bin/logic
gcc -o bin/number  02-number-theory/number.c 02-number-theory/main.c -lm && bin/number
gcc -o bin/set     03-sets-relations/set.c 03-sets-relations/relation.c 03-sets-relations/main.c -lm && bin/set
gcc -o bin/graph   04-graph-theory/graph.c   04-graph-theory/main.c -lm && bin/graph
gcc -o bin/tree    05-trees/tree.c           05-trees/main.c -lm && bin/tree
gcc -o bin/combo   06-counting/combo.c       06-counting/main.c -lm && bin/combo
gcc -o bin/boolalg 07-boolean-algebra/boolalg.c 07-boolean-algebra/main.c -lm && bin/boolalg
```
