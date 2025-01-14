; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -verify-machineinstrs -mtriple=x86_64-- | FileCheck %s

define i64 @select_consts_i64(i64 %offset, i32 %x) {
; CHECK-LABEL: select_consts_i64:
; CHECK:       # %bb.0:
; CHECK-NEXT:    leaq 42(%rdi), %rax
; CHECK-NEXT:    testl %esi, %esi
; CHECK-NEXT:    cmovneq %rdi, %rax
; CHECK-NEXT:    retq
  %b = icmp eq i32 %x, 0
  %s = select i1 %b, i64 42, i64 0
  %r = add i64 %s, %offset
  ret i64 %r
}

define i64 @select_consts_big_i64(i64 %offset, i32 %x) {
; CHECK-LABEL: select_consts_big_i64:
; CHECK:       # %bb.0:
; CHECK-NEXT:    movabsq $42000000000, %rax # imm = 0x9C7652400
; CHECK-NEXT:    addq %rdi, %rax
; CHECK-NEXT:    testl %esi, %esi
; CHECK-NEXT:    cmovneq %rdi, %rax
; CHECK-NEXT:    retq
  %b = icmp eq i32 %x, 0
  %s = select i1 %b, i64 42000000000, i64 0
  %r = add i64 %s, %offset
  ret i64 %r
}

define i32 @select_consts_i32(i32 %offset, i64 %x) {
; CHECK-LABEL: select_consts_i32:
; CHECK:       # %bb.0:
; CHECK-NEXT:    # kill: def $edi killed $edi def $rdi
; CHECK-NEXT:    leal 43(%rdi), %eax
; CHECK-NEXT:    cmpq $42, %rsi
; CHECK-NEXT:    cmovgel %edi, %eax
; CHECK-NEXT:    retq
  %b = icmp sgt i64 %x, 41
  %s = select i1 %b, i32 0, i32 43
  %r = add i32 %offset, %s
  ret i32 %r
}

define i16 @select_consts_i16(i16 %offset, i1 %b) {
; CHECK-LABEL: select_consts_i16:
; CHECK:       # %bb.0:
; CHECK-NEXT:    # kill: def $edi killed $edi def $rdi
; CHECK-NEXT:    leal 44(%rdi), %eax
; CHECK-NEXT:    testb $1, %sil
; CHECK-NEXT:    cmovel %edi, %eax
; CHECK-NEXT:    # kill: def $ax killed $ax killed $eax
; CHECK-NEXT:    retq
  %s = select i1 %b, i16 44, i16 0
  %r = add i16 %s, %offset
  ret i16 %r
}

define i8 @select_consts_i8(i8 %offset, i1 %b) {
; CHECK-LABEL: select_consts_i8:
; CHECK:       # %bb.0:
; CHECK-NEXT:    xorl %ecx, %ecx
; CHECK-NEXT:    testb $1, %sil
; CHECK-NEXT:    movl $45, %eax
; CHECK-NEXT:    cmovnel %ecx, %eax
; CHECK-NEXT:    addb %dil, %al
; CHECK-NEXT:    # kill: def $al killed $al killed $eax
; CHECK-NEXT:    retq
  %s = select i1 %b, i8 0, i8 45
  %r = add i8 %offset, %s
  ret i8 %r
}

define i32 @select_consts_use_i32(i32 %offset, i64 %x, i32* %p) {
; CHECK-LABEL: select_consts_use_i32:
; CHECK:       # %bb.0:
; CHECK-NEXT:    xorl %ecx, %ecx
; CHECK-NEXT:    cmpq $42, %rsi
; CHECK-NEXT:    movl $43, %eax
; CHECK-NEXT:    cmovgel %ecx, %eax
; CHECK-NEXT:    movl %eax, (%rdx)
; CHECK-NEXT:    addl %edi, %eax
; CHECK-NEXT:    retq
  %b = icmp sgt i64 %x, 41
  %s = select i1 %b, i32 0, i32 43
  store i32 %s, i32* %p
  %r = add i32 %offset, %s
  ret i32 %r
}

; Special-case LEA hacks are done before we try to push the add into a CMOV.

define i32 @select_40_43_i32(i32 %offset, i64 %x) {
; CHECK-LABEL: select_40_43_i32:
; CHECK:       # %bb.0:
; CHECK-NEXT:    # kill: def $edi killed $edi def $rdi
; CHECK-NEXT:    xorl %eax, %eax
; CHECK-NEXT:    cmpq $42, %rsi
; CHECK-NEXT:    setl %al
; CHECK-NEXT:    leal (%rax,%rax,2), %eax
; CHECK-NEXT:    leal 40(%rdi,%rax), %eax
; CHECK-NEXT:    retq
  %b = icmp sgt i64 %x, 41
  %s = select i1 %b, i32 40, i32 43
  %r = add i32 %offset, %s
  ret i32 %r
}

define i32 @select_0_1_i32(i32 %offset, i64 %x) {
; CHECK-LABEL: select_0_1_i32:
; CHECK:       # %bb.0:
; CHECK-NEXT:    movl %edi, %eax
; CHECK-NEXT:    cmpq $42, %rsi
; CHECK-NEXT:    adcl $0, %eax
; CHECK-NEXT:    retq
  %b = icmp ugt i64 %x, 41
  %s = select i1 %b, i32 0, i32 1
  %r = add i32 %offset, %s
  ret i32 %r
}

define i32 @select_1_0_i32(i32 %offset, i64 %x) {
; CHECK-LABEL: select_1_0_i32:
; CHECK:       # %bb.0:
; CHECK-NEXT:    movl %edi, %eax
; CHECK-NEXT:    cmpq $42, %rsi
; CHECK-NEXT:    sbbl $-1, %eax
; CHECK-NEXT:    retq
  %b = icmp ugt i64 %x, 41
  %s = select i1 %b, i32 1, i32 0
  %r = add i32 %offset, %s
  ret i32 %r
}

define i64 @select_max32_2_i64(i64 %offset, i64 %x) {
; CHECK-LABEL: select_max32_2_i64:
; CHECK:       # %bb.0:
; CHECK-NEXT:    leaq 2(%rdi), %rax
; CHECK-NEXT:    leaq 2147483647(%rdi), %rcx
; CHECK-NEXT:    cmpq $41, %rsi
; CHECK-NEXT:    cmovneq %rcx, %rax
; CHECK-NEXT:    retq
  %b = icmp ne i64 %x, 41
  %s = select i1 %b, i64 2147483647, i64 2
  %r = add i64 %offset, %s
  ret i64 %r
}

define i64 @select_42_min32_i64(i64 %offset, i1 %b) {
; CHECK-LABEL: select_42_min32_i64:
; CHECK:       # %bb.0:
; CHECK-NEXT:    testb $1, %sil
; CHECK-NEXT:    movl $42, %ecx
; CHECK-NEXT:    movl $2147483648, %eax # imm = 0x80000000
; CHECK-NEXT:    cmovneq %rcx, %rax
; CHECK-NEXT:    addq %rdi, %rax
; CHECK-NEXT:    retq
  %s = select i1 %b, i64 42, i64 2147483648
  %r = add i64 %offset, %s
  ret i64 %r
}

define i64 @select_big_42_i64(i64 %offset, i64 %x) {
; CHECK-LABEL: select_big_42_i64:
; CHECK:       # %bb.0:
; CHECK-NEXT:    cmpq $41, %rsi
; CHECK-NEXT:    movl $2147483649, %ecx # imm = 0x80000001
; CHECK-NEXT:    movl $42, %eax
; CHECK-NEXT:    cmovneq %rcx, %rax
; CHECK-NEXT:    addq %rdi, %rax
; CHECK-NEXT:    retq
  %b = icmp ne i64 %x, 41
  %s = select i1 %b, i64 2147483649, i64 42
  %r = add i64 %s, %offset
  ret i64 %r
}

define i64 @select_n42_big_i64(i64 %offset, i64 %x) {
; CHECK-LABEL: select_n42_big_i64:
; CHECK:       # %bb.0:
; CHECK-NEXT:    cmpq $41, %rsi
; CHECK-NEXT:    movq $-42, %rcx
; CHECK-NEXT:    movl $2147483649, %eax # imm = 0x80000001
; CHECK-NEXT:    cmovneq %rcx, %rax
; CHECK-NEXT:    addq %rdi, %rax
; CHECK-NEXT:    retq
  %b = icmp ne i64 %x, 41
  %s = select i1 %b, i64 -42, i64 2147483649
  %r = add i64 %s, %offset
  ret i64 %r
}

define i64 @select_big_bigger_i64(i64 %offset, i64 %x) {
; CHECK-LABEL: select_big_bigger_i64:
; CHECK:       # %bb.0:
; CHECK-NEXT:    cmpq $41, %rsi
; CHECK-NEXT:    movl $2147483649, %ecx # imm = 0x80000001
; CHECK-NEXT:    movabsq $42000000000, %rax # imm = 0x9C7652400
; CHECK-NEXT:    cmovneq %rcx, %rax
; CHECK-NEXT:    addq %rdi, %rax
; CHECK-NEXT:    retq
  %b = icmp ne i64 %x, 41
  %s = select i1 %b, i64 2147483649, i64 42000000000
  %r = add i64 %s, %offset
  ret i64 %r
}

define i32 @select_20_43_i32(i32 %offset, i64 %x) {
; CHECK-LABEL: select_20_43_i32:
; CHECK:       # %bb.0:
; CHECK-NEXT:    # kill: def $edi killed $edi def $rdi
; CHECK-NEXT:    leal 43(%rdi), %ecx
; CHECK-NEXT:    leal 20(%rdi), %eax
; CHECK-NEXT:    cmpq $42, %rsi
; CHECK-NEXT:    cmovll %ecx, %eax
; CHECK-NEXT:    retq
  %b = icmp sgt i64 %x, 41
  %s = select i1 %b, i32 20, i32 43
  %r = add i32 %offset, %s
  ret i32 %r
}

define i16 @select_n2_17_i16(i16 %offset, i1 %b) {
; CHECK-LABEL: select_n2_17_i16:
; CHECK:       # %bb.0:
; CHECK-NEXT:    # kill: def $edi killed $edi def $rdi
; CHECK-NEXT:    leal 17(%rdi), %ecx
; CHECK-NEXT:    leal 65534(%rdi), %eax
; CHECK-NEXT:    testb $1, %sil
; CHECK-NEXT:    cmovel %ecx, %eax
; CHECK-NEXT:    # kill: def $ax killed $ax killed $eax
; CHECK-NEXT:    retq
  %s = select i1 %b, i16 -2, i16 17
  %r = add i16 %offset, %s
  ret i16 %r
}

%class.btAxis = type { %struct.btBroadphaseProxy.base, [3 x i16], [3 x i16], %struct.btBroadphaseProxy* }
%struct.btBroadphaseProxy.base = type <{ i8*, i16, i16, [4 x i8], i8*, i32, [4 x float], [4 x float] }>
%struct.btBroadphaseProxy = type <{ i8*, i16, i16, [4 x i8], i8*, i32, [4 x float], [4 x float], [4 x i8] }>

define i16* @bullet(i1 %b, %class.btAxis* readnone %ptr, i64 %idx) {
; CHECK-LABEL: bullet:
; CHECK:       # %bb.0:
; CHECK-NEXT:    leaq (%rdx,%rdx,4), %rax
; CHECK-NEXT:    shlq $4, %rax
; CHECK-NEXT:    leaq 60(%rsi,%rax), %rcx
; CHECK-NEXT:    leaq 66(%rsi,%rax), %rax
; CHECK-NEXT:    testb $1, %dil
; CHECK-NEXT:    cmovneq %rcx, %rax
; CHECK-NEXT:    retq
  %gep2 = getelementptr inbounds %class.btAxis, %class.btAxis* %ptr, i64 %idx, i32 2, i64 0
  %gep1 = getelementptr inbounds %class.btAxis, %class.btAxis* %ptr, i64 %idx, i32 1, i64 0
  %sel = select i1 %b, i16* %gep1, i16* %gep2
  ret i16* %sel
}

define i16* @bullet_alt1(i1 %b, i16* readnone %ptr, i64 %idx) {
; CHECK-LABEL: bullet_alt1:
; CHECK:       # %bb.0:
; CHECK-NEXT:    leaq 60(%rsi), %rax
; CHECK-NEXT:    leaq 66(%rsi), %rcx
; CHECK-NEXT:    testb $1, %dil
; CHECK-NEXT:    cmovneq %rax, %rcx
; CHECK-NEXT:    leaq (%rdx,%rdx,4), %rax
; CHECK-NEXT:    shlq $4, %rax
; CHECK-NEXT:    addq %rcx, %rax
; CHECK-NEXT:    retq
  %idx40 = mul i64 %idx, 40
  %gep2 = getelementptr inbounds i16, i16* %ptr, i64 33
  %gep1 = getelementptr inbounds i16, i16* %ptr, i64 30
  %sel = select i1 %b, i16* %gep1, i16* %gep2
  %gep3 = getelementptr inbounds i16, i16* %sel, i64 %idx40
  ret i16* %gep3
}

define void @bullet_load_store(i32 %x, i64 %y, %class.btAxis* %p) {
; CHECK-LABEL: bullet_load_store:
; CHECK:       # %bb.0:
; CHECK-NEXT:    leaq (%rsi,%rsi,4), %rax
; CHECK-NEXT:    shlq $4, %rax
; CHECK-NEXT:    leaq 66(%rdx), %rcx
; CHECK-NEXT:    addq $60, %rdx
; CHECK-NEXT:    testb $1, %dil
; CHECK-NEXT:    cmovneq %rcx, %rdx
; CHECK-NEXT:    decw (%rdx,%rax)
; CHECK-NEXT:    retq
  %and = and i32 %x, 1
  %b = icmp eq i32 %and, 0
  %gep2 = getelementptr inbounds %class.btAxis, %class.btAxis* %p, i64 %y, i32 2, i64 0
  %gep1 = getelementptr inbounds %class.btAxis, %class.btAxis* %p, i64 %y, i32 1, i64 0
  %sel = select i1 %b, i16* %gep1, i16* %gep2
  %ld = load i16, i16* %sel, align 4
  %dec = add i16 %ld, -1
  store i16 %dec, i16* %sel, align 4
  ret void
}

define void @complex_lea_alt1(i1 %b, i16* readnone %ptr, i64 %idx) {
; CHECK-LABEL: complex_lea_alt1:
; CHECK:       # %bb.0:
; CHECK-NEXT:    leaq 60(%rdx), %rax
; CHECK-NEXT:    addq $66, %rdx
; CHECK-NEXT:    testb $1, %dil
; CHECK-NEXT:    cmovneq %rax, %rdx
; CHECK-NEXT:    decw (%rdx,%rsi)
; CHECK-NEXT:    retq
  %i = ptrtoint i16* %ptr to i64
  %sum = add i64 %idx, %i
  %base = inttoptr i64 %sum to i16*
  %gep2 = getelementptr inbounds i16, i16* %base, i64 33
  %gep1 = getelementptr inbounds i16, i16* %base, i64 30
  %sel = select i1 %b, i16* %gep1, i16* %gep2
  %ld = load i16, i16* %sel, align 4
  %dec = add i16 %ld, -1
  store i16 %dec, i16* %sel, align 4
  ret void
}

define void @complex_lea_alt2(i1 %b, i16* readnone %ptr, i64 %idx) {
; CHECK-LABEL: complex_lea_alt2:
; CHECK:       # %bb.0:
; CHECK-NEXT:    leaq 60(%rsi), %rax
; CHECK-NEXT:    addq $66, %rsi
; CHECK-NEXT:    testb $1, %dil
; CHECK-NEXT:    cmovneq %rax, %rsi
; CHECK-NEXT:    decw (%rsi,%rdx)
; CHECK-NEXT:    retq
  %i = ptrtoint i16* %ptr to i64
  %sum = add i64 %i, %idx
  %base = inttoptr i64 %sum to i16*
  %gep2 = getelementptr inbounds i16, i16* %base, i64 33
  %gep1 = getelementptr inbounds i16, i16* %base, i64 30
  %sel = select i1 %b, i16* %gep1, i16* %gep2
  %ld = load i16, i16* %sel, align 4
  %dec = add i16 %ld, -1
  store i16 %dec, i16* %sel, align 4
  ret void
}

define void @complex_lea_alt3(i1 %b, i16* readnone %ptr, i64 %idx) {
; CHECK-LABEL: complex_lea_alt3:
; CHECK:       # %bb.0:
; CHECK-NEXT:    leaq 60(%rsi), %rax
; CHECK-NEXT:    addq $66, %rsi
; CHECK-NEXT:    testb $1, %dil
; CHECK-NEXT:    cmovneq %rax, %rsi
; CHECK-NEXT:    decw (%rsi,%rdx)
; CHECK-NEXT:    retq
  %i = ptrtoint i16* %ptr to i64
  %i66 = add i64 %i, 66
  %i60 = add i64 %i, 60
  %o66 = add i64 %i66, %idx
  %o60 = add i64 %i60, %idx
  %p66 = inttoptr i64 %o66 to i16*
  %p60 = inttoptr i64 %o60 to i16*
  %sel = select i1 %b, i16* %p60, i16* %p66
  %ld = load i16, i16* %sel, align 4
  %dec = add i16 %ld, -1
  store i16 %dec, i16* %sel, align 4
  ret void
}

define void @complex_lea_alt4(i1 %b, i16* readnone %ptr, i64 %idx) {
; CHECK-LABEL: complex_lea_alt4:
; CHECK:       # %bb.0:
; CHECK-NEXT:    leaq 60(%rsi), %rax
; CHECK-NEXT:    addq $66, %rsi
; CHECK-NEXT:    testb $1, %dil
; CHECK-NEXT:    cmovneq %rax, %rsi
; CHECK-NEXT:    decw (%rdx,%rsi)
; CHECK-NEXT:    retq
  %i = ptrtoint i16* %ptr to i64
  %i66 = add i64 %i, 66
  %i60 = add i64 %i, 60
  %o66 = add i64 %idx, %i66
  %o60 = add i64 %idx, %i60
  %p66 = inttoptr i64 %o66 to i16*
  %p60 = inttoptr i64 %o60 to i16*
  %sel = select i1 %b, i16* %p60, i16* %p66
  %ld = load i16, i16* %sel, align 4
  %dec = add i16 %ld, -1
  store i16 %dec, i16* %sel, align 4
  ret void
}

define void @complex_lea_alt5(i1 %b, i16* readnone %ptr, i64 %idx) {
; CHECK-LABEL: complex_lea_alt5:
; CHECK:       # %bb.0:
; CHECK-NEXT:    leaq 60(%rdx), %rax
; CHECK-NEXT:    addq $66, %rdx
; CHECK-NEXT:    testb $1, %dil
; CHECK-NEXT:    cmovneq %rax, %rdx
; CHECK-NEXT:    decw (%rdx,%rsi)
; CHECK-NEXT:    retq
  %i = ptrtoint i16* %ptr to i64
  %i66 = add i64 %idx, 66
  %i60 = add i64 %idx, 60
  %o66 = add i64 %i66, %i
  %o60 = add i64 %i60, %i
  %p66 = inttoptr i64 %o66 to i16*
  %p60 = inttoptr i64 %o60 to i16*
  %sel = select i1 %b, i16* %p60, i16* %p66
  %ld = load i16, i16* %sel, align 4
  %dec = add i16 %ld, -1
  store i16 %dec, i16* %sel, align 4
  ret void
}

define void @complex_lea_alt6(i1 %b, i16* readnone %ptr, i64 %idx) {
; CHECK-LABEL: complex_lea_alt6:
; CHECK:       # %bb.0:
; CHECK-NEXT:    leaq 60(%rdx), %rax
; CHECK-NEXT:    addq $66, %rdx
; CHECK-NEXT:    testb $1, %dil
; CHECK-NEXT:    cmovneq %rax, %rdx
; CHECK-NEXT:    decw (%rsi,%rdx)
; CHECK-NEXT:    retq
  %i = ptrtoint i16* %ptr to i64
  %i66 = add i64 %idx, 66
  %i60 = add i64 %idx, 60
  %o66 = add i64 %i, %i66
  %o60 = add i64 %i, %i60
  %p66 = inttoptr i64 %o66 to i16*
  %p60 = inttoptr i64 %o60 to i16*
  %sel = select i1 %b, i16* %p60, i16* %p66
  %ld = load i16, i16* %sel, align 4
  %dec = add i16 %ld, -1
  store i16 %dec, i16* %sel, align 4
  ret void
}

define void @complex_lea_alt7(i1 %b, i16* readnone %ptr, i64 %idx) {
; CHECK-LABEL: complex_lea_alt7:
; CHECK:       # %bb.0:
; CHECK-NEXT:    leaq 60(%rdx), %rax
; CHECK-NEXT:    addq $66, %rdx
; CHECK-NEXT:    testb $1, %dil
; CHECK-NEXT:    cmovneq %rax, %rdx
; CHECK-NEXT:    decw (%rdx,%rsi)
; CHECK-NEXT:    retq
  %i = ptrtoint i16* %ptr to i64
  %o = add i64 %idx, %i
  %o66 = add i64 %o, 66
  %o60 = add i64 %o, 60
  %p66 = inttoptr i64 %o66 to i16*
  %p60 = inttoptr i64 %o60 to i16*
  %sel = select i1 %b, i16* %p60, i16* %p66
  %ld = load i16, i16* %sel, align 4
  %dec = add i16 %ld, -1
  store i16 %dec, i16* %sel, align 4
  ret void
}

define void @complex_lea_alt8(i1 %b, i16* readnone %ptr, i64 %idx) {
; CHECK-LABEL: complex_lea_alt8:
; CHECK:       # %bb.0:
; CHECK-NEXT:    leaq 60(%rsi), %rax
; CHECK-NEXT:    addq $66, %rsi
; CHECK-NEXT:    testb $1, %dil
; CHECK-NEXT:    cmovneq %rax, %rsi
; CHECK-NEXT:    decw (%rsi,%rdx)
; CHECK-NEXT:    retq
  %i = ptrtoint i16* %ptr to i64
  %o = add i64 %i, %idx
  %o66 = add i64 %o, 66
  %o60 = add i64 %o, 60
  %p66 = inttoptr i64 %o66 to i16*
  %p60 = inttoptr i64 %o60 to i16*
  %sel = select i1 %b, i16* %p60, i16* %p66
  %ld = load i16, i16* %sel, align 4
  %dec = add i16 %ld, -1
  store i16 %dec, i16* %sel, align 4
  ret void
}

define i32 @loadfold_select_const_arms(i32* %x, i1 %y) {
; CHECK-LABEL: loadfold_select_const_arms:
; CHECK:       # %bb.0:
; CHECK-NEXT:    movl (%rdi), %eax
; CHECK-NEXT:    leal -10(%rax), %ecx
; CHECK-NEXT:    addl $10, %eax
; CHECK-NEXT:    testb $1, %sil
; CHECK-NEXT:    cmovel %ecx, %eax
; CHECK-NEXT:    # kill: def $eax killed $eax killed $rax
; CHECK-NEXT:    retq
  %cond = select i1 %y, i32 10, i32 -10
  %t0 = load i32, i32* %x, align 4
  %add = add nsw i32 %t0, %cond
  ret i32 %add
}

define void @rmw_add(i32* %x, i1 %y, i32 %z, i32 %w) {
; CHECK-LABEL: rmw_add:
; CHECK:       # %bb.0:
; CHECK-NEXT:    testb $1, %sil
; CHECK-NEXT:    cmovel %ecx, %edx
; CHECK-NEXT:    addl %edx, (%rdi)
; CHECK-NEXT:    retq
  %cond = select i1 %y, i32 %z, i32 %w
  %t0 = load i32, i32* %x, align 4
  %add = add nsw i32 %t0, %cond
  store i32 %add, i32* %x, align 4
  ret void
}

define void @rmw_add_select_const_arm(i32* %x, i1 %y, i32 %z) {
; CHECK-LABEL: rmw_add_select_const_arm:
; CHECK:       # %bb.0:
; CHECK-NEXT:    testb $1, %sil
; CHECK-NEXT:    movl $-10, %eax
; CHECK-NEXT:    cmovnel %edx, %eax
; CHECK-NEXT:    addl %eax, (%rdi)
; CHECK-NEXT:    retq
  %cond = select i1 %y, i32 %z, i32 -10
  %t0 = load i32, i32* %x, align 4
  %add = add nsw i32 %t0, %cond
  store i32 %add, i32* %x, align 4
  ret void
}

define void @rmw_select_const_arms(i32* %x, i1 %y) {
; CHECK-LABEL: rmw_select_const_arms:
; CHECK:       # %bb.0:
; CHECK-NEXT:    movl (%rdi), %eax
; CHECK-NEXT:    leal -10(%rax), %ecx
; CHECK-NEXT:    addl $10, %eax
; CHECK-NEXT:    testb $1, %sil
; CHECK-NEXT:    cmovel %ecx, %eax
; CHECK-NEXT:    movl %eax, (%rdi)
; CHECK-NEXT:    retq
  %cond = select i1 %y, i32 10, i32 -10
  %t0 = load i32, i32* %x, align 4
  %add = add nsw i32 %t0, %cond
  store i32 %add, i32* %x, align 4
  ret void
}

define i32 @rmw_select_const_arms_extra_load_use(i32* %x, i1 %y) {
; CHECK-LABEL: rmw_select_const_arms_extra_load_use:
; CHECK:       # %bb.0:
; CHECK-NEXT:    movl (%rdi), %eax
; CHECK-NEXT:    leal -10(%rax), %ecx
; CHECK-NEXT:    leal 10(%rax), %edx
; CHECK-NEXT:    testb $1, %sil
; CHECK-NEXT:    cmovel %ecx, %edx
; CHECK-NEXT:    movl %edx, (%rdi)
; CHECK-NEXT:    # kill: def $eax killed $eax killed $rax
; CHECK-NEXT:    retq
  %cond = select i1 %y, i32 10, i32 -10
  %t0 = load i32, i32* %x, align 4
  %add = add nsw i32 %t0, %cond
  store i32 %add, i32* %x, align 4
  ret i32 %t0
}

define i32 @rmw_select_const_arms_extra_add_use(i32* %x, i1 %y) {
; CHECK-LABEL: rmw_select_const_arms_extra_add_use:
; CHECK:       # %bb.0:
; CHECK-NEXT:    movl (%rdi), %eax
; CHECK-NEXT:    leal -10(%rax), %ecx
; CHECK-NEXT:    addl $10, %eax
; CHECK-NEXT:    testb $1, %sil
; CHECK-NEXT:    cmovel %ecx, %eax
; CHECK-NEXT:    movl %eax, (%rdi)
; CHECK-NEXT:    # kill: def $eax killed $eax killed $rax
; CHECK-NEXT:    retq
  %cond = select i1 %y, i32 10, i32 -10
  %t0 = load i32, i32* %x, align 4
  %add = add nsw i32 %t0, %cond
  store i32 %add, i32* %x, align 4
  ret i32 %add
}
