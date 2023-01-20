# reviewer
Rough draft of some code to show how CHERI can help to prevent a user from exercising write privileges they shouldn't have.

A. In a non-CHERI environment:
1. Programmatically deny write privileges when read-only privileges apply.
2. Use a heap overflow to try to corrupt the read-only data. 
    - reviewer_read_only.c
    - reviewer_weak_read_only.c

B. In a CHERI environment:
1. Programmatically deny write privileges when read-only privileges apply.
2. Use a heap overflow to try to corrupt the read-only data.
    - reviewer_weak_read_only.c

OR

1. Use capabilities to set a data structure to read-only. 
2. Try to write to the read-only data structure, and crash. 
3. The heap overflow is unreachable.
    - reviewer_read_only.c

OR

1. Use capabilities to set a data structure to read-only. 
2. Use a heap overflow to try to corrupt the read-only data.
    - reviewer_cheri_read_only_then_heap_overflow.c
