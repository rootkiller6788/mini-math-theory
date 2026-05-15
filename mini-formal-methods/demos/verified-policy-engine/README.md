# Verified Policy Engine

Access-control policy evaluator with formal verification:

1. Policies are Boolean expressions over attributes (role, time, clearance)
2. **Symbolic execution** verifies no policy grants access to unauthorized users
3. **Contracts**: every policy evaluation has explicit pre/post conditions

```c
SymExecutor se;
sym_exec_init(&se);
sym_exec_add_var(&se, 0, 3);  // role: 0=guest, 1=user, 2=admin, 3=root
sym_exec_add_var(&se, 0, 1);  // emergency flag

// Property: guest never gets admin access
sym_exec_explore(&se, no_guest_admin, NULL);
```

This mirrors how AWS IAM / Zanzibar policy engines are formally reasoned about.
