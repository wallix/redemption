	.file	"parse_ip.cpp"
	.text
	.align 2
	.p2align 4,,15
	.type	_ZN10LineBuffer9get_spaceEv.constprop.6, @function
_ZN10LineBuffer9get_spaceEv.constprop.6:
.LFB1123:
	.cfi_startproc
	pushl	%edi
	.cfi_def_cfa_offset 8
	.cfi_offset 7, -8
	pushl	%esi
	.cfi_def_cfa_offset 12
	.cfi_offset 6, -12
	movl	%eax, %esi
	pushl	%ebx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	subl	$16, %esp
	.cfi_def_cfa_offset 32
	movl	20500(%eax), %ebx
	movsbl	(%eax,%ebx), %eax
	movl	%eax, (%esp)
	call	isspace
	movl	$-1, %edx
	testl	%eax, %eax
	je	.L2
	movl	20492(%esi), %edi
	addl	$1, %ebx
	xorl	%edx, %edx
	cmpl	%edi, %ebx
	jl	.L7
	jmp	.L2
	.p2align 4,,7
	.p2align 3
.L10:
	addl	$1, %ebx
	cmpl	%edi, %ebx
	je	.L6
.L7:
	movsbl	(%esi,%ebx), %eax
	movl	%eax, (%esp)
	call	isspace
	testl	%eax, %eax
	jne	.L10
.L6:
	xorl	%edx, %edx
.L2:
	movl	%ebx, 20504(%esi)
	addl	$16, %esp
	.cfi_def_cfa_offset 16
	movl	%edx, %eax
	popl	%ebx
	.cfi_def_cfa_offset 12
	.cfi_restore 3
	popl	%esi
	.cfi_def_cfa_offset 8
	.cfi_restore 6
	popl	%edi
	.cfi_def_cfa_offset 4
	.cfi_restore 7
	ret
	.cfi_endproc
.LFE1123:
	.size	_ZN10LineBuffer9get_spaceEv.constprop.6, .-_ZN10LineBuffer9get_spaceEv.constprop.6
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"src="
	.section	.text._ZN10LineBuffer10get_src_ipEv,"axG",@progbits,_ZN10LineBuffer10get_src_ipEv,comdat
	.align 2
	.p2align 4,,15
	.weak	_ZN10LineBuffer10get_src_ipEv
	.type	_ZN10LineBuffer10get_src_ipEv, @function
_ZN10LineBuffer10get_src_ipEv:
.LFB93:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	subl	$28, %esp
	.cfi_def_cfa_offset 48
	movl	48(%esp), %ebx
	movl	20500(%ebx), %edi
	movl	20492(%ebx), %ebp
	cmpl	%ebp, %edi
	jge	.L12
	movzbl	(%ebx,%edi), %eax
	movl	%edi, %edx
	subl	$97, %eax
	cmpb	$25, %al
	jbe	.L15
	jmp	.L12
	.p2align 4,,7
	.p2align 3
.L16:
	movzbl	(%ebx,%edx), %eax
	subl	$97, %eax
	cmpb	$25, %al
	ja	.L14
.L15:
	addl	$1, %edx
	cmpl	%ebp, %edx
	jne	.L16
.L14:
	leal	1(%edx), %ecx
	movl	$-1, %eax
	cmpl	%ebp, %ecx
	movl	%edx, 20504(%ebx)
	jge	.L17
	cmpb	$61, (%ebx,%edx)
	je	.L56
.L17:
	addl	$28, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	popl	%ebx
	.cfi_def_cfa_offset 16
	.cfi_restore 3
	popl	%esi
	.cfi_def_cfa_offset 12
	.cfi_restore 6
	popl	%edi
	.cfi_def_cfa_offset 8
	.cfi_restore 7
	popl	%ebp
	.cfi_def_cfa_offset 4
	.cfi_restore 5
	ret
.L56:
	.cfi_restore_state
	movzbl	(%ebx,%ecx), %edx
	movl	%ecx, 20504(%ebx)
	subl	$48, %edx
	cmpb	$9, %dl
	ja	.L17
	.p2align 4,,7
	.p2align 3
.L52:
	addl	$1, %ecx
	cmpl	%ebp, %ecx
	je	.L19
	movzbl	(%ebx,%ecx), %eax
	subl	$48, %eax
	cmpb	$9, %al
	jbe	.L52
.L19:
	leal	1(%ecx), %edx
	movl	$-1, %eax
	cmpl	%edx, %ebp
	movl	%ecx, 20504(%ebx)
	jle	.L17
	cmpb	$46, (%ebx,%ecx)
	jne	.L17
	movzbl	(%ebx,%edx), %ecx
	movl	%edx, 20504(%ebx)
	subl	$48, %ecx
	cmpb	$9, %cl
	jbe	.L53
	jmp	.L17
	.p2align 4,,7
	.p2align 3
.L25:
	movzbl	(%ebx,%edx), %eax
	subl	$48, %eax
	cmpb	$9, %al
	ja	.L23
.L53:
	addl	$1, %edx
	cmpl	%ebp, %edx
	jne	.L25
.L23:
	leal	1(%edx), %ecx
	movl	$-1, %eax
	cmpl	%ecx, %ebp
	movl	%edx, 20504(%ebx)
	jle	.L17
	cmpb	$46, (%ebx,%edx)
	jne	.L17
	movzbl	(%ebx,%ecx), %edx
	movl	%ecx, 20504(%ebx)
	subl	$48, %edx
	cmpb	$9, %dl
	jbe	.L54
	jmp	.L17
	.p2align 4,,7
	.p2align 3
.L29:
	movzbl	(%ebx,%ecx), %eax
	subl	$48, %eax
	cmpb	$9, %al
	ja	.L27
.L54:
	addl	$1, %ecx
	cmpl	%ebp, %ecx
	jne	.L29
.L27:
	leal	1(%ecx), %esi
	movl	$-1, %eax
	cmpl	%esi, %ebp
	movl	%ecx, 20504(%ebx)
	jle	.L17
	cmpb	$46, (%ebx,%ecx)
	jne	.L17
	movzbl	(%ebx,%esi), %eax
	movl	%esi, 20504(%ebx)
	subl	$48, %eax
	cmpb	$9, %al
	jbe	.L55
	jmp	.L58
	.p2align 4,,7
	.p2align 3
.L33:
	movzbl	(%ebx,%esi), %eax
	subl	$48, %eax
	cmpb	$9, %al
	ja	.L47
.L55:
	addl	$1, %esi
	cmpl	%ebp, %esi
	jne	.L33
	movl	%esi, 20504(%ebx)
	xorl	%ebp, %ebp
.L34:
	movsbl	(%ebx,%esi), %eax
	movl	%eax, (%esp)
	call	isspace
	movl	%eax, %edx
	movl	$-1, %eax
	testl	%edx, %edx
	je	.L17
	cmpl	$-1, %ebp
	je	.L17
	subl	%edi, %esi
	cmpl	$3, %esi
	jle	.L17
	addl	%edi, %ebx
	movl	%ebx, (%esp)
	movl	$4, 8(%esp)
	movl	$.LC0, 4(%esp)
	call	memcmp
	cmpl	$1, %eax
	sbbl	%eax, %eax
	addl	$28, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	notl	%eax
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,7
	.p2align 3
.L47:
	.cfi_restore_state
	xorl	%ebp, %ebp
.L30:
	movl	%esi, 20504(%ebx)
	jmp	.L34
.L12:
	movl	%edi, 20504(%ebx)
	movl	$-1, %eax
	jmp	.L17
.L58:
	movl	$-1, %ebp
	jmp	.L30
	.cfi_endproc
.LFE93:
	.size	_ZN10LineBuffer10get_src_ipEv, .-_ZN10LineBuffer10get_src_ipEv
	.section	.rodata.str1.1
.LC1:
	.string	"dst="
	.section	.text._ZN10LineBuffer10get_dst_ipEv,"axG",@progbits,_ZN10LineBuffer10get_dst_ipEv,comdat
	.align 2
	.p2align 4,,15
	.weak	_ZN10LineBuffer10get_dst_ipEv
	.type	_ZN10LineBuffer10get_dst_ipEv, @function
_ZN10LineBuffer10get_dst_ipEv:
.LFB94:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	subl	$28, %esp
	.cfi_def_cfa_offset 48
	movl	48(%esp), %ebx
	movl	20500(%ebx), %edi
	movl	20492(%ebx), %ebp
	cmpl	%ebp, %edi
	jge	.L60
	movzbl	(%ebx,%edi), %eax
	movl	%edi, %edx
	subl	$97, %eax
	cmpb	$25, %al
	jbe	.L63
	jmp	.L60
	.p2align 4,,7
	.p2align 3
.L64:
	movzbl	(%ebx,%edx), %eax
	subl	$97, %eax
	cmpb	$25, %al
	ja	.L62
.L63:
	addl	$1, %edx
	cmpl	%ebp, %edx
	jne	.L64
.L62:
	leal	1(%edx), %ecx
	movl	$-1, %eax
	cmpl	%ebp, %ecx
	movl	%edx, 20504(%ebx)
	jge	.L65
	cmpb	$61, (%ebx,%edx)
	je	.L104
.L65:
	addl	$28, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	popl	%ebx
	.cfi_def_cfa_offset 16
	.cfi_restore 3
	popl	%esi
	.cfi_def_cfa_offset 12
	.cfi_restore 6
	popl	%edi
	.cfi_def_cfa_offset 8
	.cfi_restore 7
	popl	%ebp
	.cfi_def_cfa_offset 4
	.cfi_restore 5
	ret
.L104:
	.cfi_restore_state
	movzbl	(%ebx,%ecx), %edx
	movl	%ecx, 20504(%ebx)
	subl	$48, %edx
	cmpb	$9, %dl
	ja	.L65
	.p2align 4,,7
	.p2align 3
.L100:
	addl	$1, %ecx
	cmpl	%ebp, %ecx
	je	.L67
	movzbl	(%ebx,%ecx), %eax
	subl	$48, %eax
	cmpb	$9, %al
	jbe	.L100
.L67:
	leal	1(%ecx), %edx
	movl	$-1, %eax
	cmpl	%edx, %ebp
	movl	%ecx, 20504(%ebx)
	jle	.L65
	cmpb	$46, (%ebx,%ecx)
	jne	.L65
	movzbl	(%ebx,%edx), %ecx
	movl	%edx, 20504(%ebx)
	subl	$48, %ecx
	cmpb	$9, %cl
	jbe	.L101
	jmp	.L65
	.p2align 4,,7
	.p2align 3
.L73:
	movzbl	(%ebx,%edx), %eax
	subl	$48, %eax
	cmpb	$9, %al
	ja	.L71
.L101:
	addl	$1, %edx
	cmpl	%ebp, %edx
	jne	.L73
.L71:
	leal	1(%edx), %ecx
	movl	$-1, %eax
	cmpl	%ecx, %ebp
	movl	%edx, 20504(%ebx)
	jle	.L65
	cmpb	$46, (%ebx,%edx)
	jne	.L65
	movzbl	(%ebx,%ecx), %edx
	movl	%ecx, 20504(%ebx)
	subl	$48, %edx
	cmpb	$9, %dl
	jbe	.L102
	jmp	.L65
	.p2align 4,,7
	.p2align 3
.L77:
	movzbl	(%ebx,%ecx), %eax
	subl	$48, %eax
	cmpb	$9, %al
	ja	.L75
.L102:
	addl	$1, %ecx
	cmpl	%ebp, %ecx
	jne	.L77
.L75:
	leal	1(%ecx), %esi
	movl	$-1, %eax
	cmpl	%esi, %ebp
	movl	%ecx, 20504(%ebx)
	jle	.L65
	cmpb	$46, (%ebx,%ecx)
	jne	.L65
	movzbl	(%ebx,%esi), %eax
	movl	%esi, 20504(%ebx)
	subl	$48, %eax
	cmpb	$9, %al
	jbe	.L103
	jmp	.L106
	.p2align 4,,7
	.p2align 3
.L81:
	movzbl	(%ebx,%esi), %eax
	subl	$48, %eax
	cmpb	$9, %al
	ja	.L95
.L103:
	addl	$1, %esi
	cmpl	%ebp, %esi
	jne	.L81
	movl	%esi, 20504(%ebx)
	xorl	%ebp, %ebp
.L82:
	movsbl	(%ebx,%esi), %eax
	movl	%eax, (%esp)
	call	isspace
	movl	%eax, %edx
	movl	$-1, %eax
	testl	%edx, %edx
	je	.L65
	cmpl	$-1, %ebp
	je	.L65
	subl	%edi, %esi
	cmpl	$3, %esi
	jle	.L65
	addl	%edi, %ebx
	movl	%ebx, (%esp)
	movl	$4, 8(%esp)
	movl	$.LC1, 4(%esp)
	call	memcmp
	cmpl	$1, %eax
	sbbl	%eax, %eax
	addl	$28, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	notl	%eax
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,7
	.p2align 3
.L95:
	.cfi_restore_state
	xorl	%ebp, %ebp
.L78:
	movl	%esi, 20504(%ebx)
	jmp	.L82
.L60:
	movl	%edi, 20504(%ebx)
	movl	$-1, %eax
	jmp	.L65
.L106:
	movl	$-1, %ebp
	jmp	.L78
	.cfi_endproc
.LFE94:
	.size	_ZN10LineBuffer10get_dst_ipEv, .-_ZN10LineBuffer10get_dst_ipEv
	.section	.text._ZN10LineBuffer11get_var_numEv,"axG",@progbits,_ZN10LineBuffer11get_var_numEv,comdat
	.align 2
	.p2align 4,,15
	.weak	_ZN10LineBuffer11get_var_numEv
	.type	_ZN10LineBuffer11get_var_numEv, @function
_ZN10LineBuffer11get_var_numEv:
.LFB95:
	.cfi_startproc
	pushl	%esi
	.cfi_def_cfa_offset 8
	.cfi_offset 6, -8
	pushl	%ebx
	.cfi_def_cfa_offset 12
	.cfi_offset 3, -12
	subl	$20, %esp
	.cfi_def_cfa_offset 32
	movl	32(%esp), %ebx
	movl	20500(%ebx), %edx
	movl	20492(%ebx), %esi
	cmpl	%esi, %edx
	jge	.L108
	movzbl	(%ebx,%edx), %eax
	subl	$97, %eax
	cmpb	$25, %al
	jbe	.L125
	jmp	.L108
	.p2align 4,,7
	.p2align 3
.L112:
	movzbl	(%ebx,%edx), %eax
	subl	$97, %eax
	cmpb	$25, %al
	ja	.L110
.L125:
	addl	$1, %edx
	cmpl	%esi, %edx
	jne	.L112
.L110:
	leal	1(%edx), %ecx
	movl	$-1, %eax
	cmpl	%esi, %ecx
	movl	%edx, 20504(%ebx)
	jge	.L113
	cmpb	$61, (%ebx,%edx)
	je	.L127
.L113:
	addl	$20, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 12
	popl	%ebx
	.cfi_def_cfa_offset 8
	.cfi_restore 3
	popl	%esi
	.cfi_def_cfa_offset 4
	.cfi_restore 6
	ret
	.p2align 4,,7
	.p2align 3
.L127:
	.cfi_restore_state
	movzbl	(%ebx,%ecx), %edx
	movl	%ecx, 20504(%ebx)
	subl	$48, %edx
	cmpb	$9, %dl
	ja	.L113
	.p2align 4,,7
	.p2align 3
.L123:
	addl	$1, %ecx
	cmpl	%esi, %ecx
	je	.L128
	movzbl	(%ebx,%ecx), %eax
	subl	$48, %eax
	cmpb	$9, %al
	jbe	.L123
	cmpl	%esi, %ecx
	movl	%ecx, 20504(%ebx)
	jge	.L129
	movsbl	(%ebx,%ecx), %eax
	movl	%eax, (%esp)
	call	isspace
	cmpl	$1, %eax
	sbbl	%eax, %eax
	addl	$20, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 12
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 8
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,7
	.p2align 3
.L128:
	.cfi_restore_state
	movl	%ecx, 20504(%ebx)
	addl	$20, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 12
	movl	$-1, %eax
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 8
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 4
	ret
.L108:
	.cfi_restore_state
	movl	%edx, 20504(%ebx)
	movl	$-1, %eax
	jmp	.L113
.L129:
	movl	$-1, %eax
	jmp	.L113
	.cfi_endproc
.LFE95:
	.size	_ZN10LineBuffer11get_var_numEv, .-_ZN10LineBuffer11get_var_numEv
	.section	.rodata.str1.1
.LC2:
	.string	"sport="
	.section	.text._ZN10LineBuffer9get_sportEv,"axG",@progbits,_ZN10LineBuffer9get_sportEv,comdat
	.align 2
	.p2align 4,,15
	.weak	_ZN10LineBuffer9get_sportEv
	.type	_ZN10LineBuffer9get_sportEv, @function
_ZN10LineBuffer9get_sportEv:
.LFB96:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	subl	$28, %esp
	.cfi_def_cfa_offset 48
	movl	48(%esp), %esi
	movl	20500(%esi), %edi
	movl	20492(%esi), %ecx
	cmpl	%ecx, %edi
	jge	.L131
	movzbl	(%esi,%edi), %eax
	movl	%edi, %edx
	subl	$97, %eax
	cmpb	$25, %al
	jbe	.L134
	jmp	.L131
	.p2align 4,,7
	.p2align 3
.L135:
	movzbl	(%esi,%edx), %eax
	subl	$97, %eax
	cmpb	$25, %al
	ja	.L133
.L134:
	addl	$1, %edx
	cmpl	%ecx, %edx
	jne	.L135
.L133:
	leal	1(%edx), %ebx
	movl	$-1, %ebp
	cmpl	%ecx, %ebx
	movl	%edx, 20504(%esi)
	jge	.L136
	cmpb	$61, (%esi,%edx)
	je	.L155
.L136:
	addl	$28, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	movl	%ebp, %eax
	popl	%ebx
	.cfi_def_cfa_offset 16
	.cfi_restore 3
	popl	%esi
	.cfi_def_cfa_offset 12
	.cfi_restore 6
	popl	%edi
	.cfi_def_cfa_offset 8
	.cfi_restore 7
	popl	%ebp
	.cfi_def_cfa_offset 4
	.cfi_restore 5
	ret
	.p2align 4,,7
	.p2align 3
.L155:
	.cfi_restore_state
	movzbl	(%esi,%ebx), %eax
	movl	%ebx, 20504(%esi)
	subl	$48, %eax
	cmpb	$9, %al
	ja	.L136
	.p2align 4,,7
	.p2align 3
.L150:
	addl	$1, %ebx
	cmpl	%ecx, %ebx
	je	.L156
	movzbl	(%esi,%ebx), %eax
	subl	$48, %eax
	cmpb	$9, %al
	jbe	.L150
	cmpl	%ecx, %ebx
	movl	%ebx, 20504(%esi)
	jge	.L157
	movsbl	(%esi,%ebx), %eax
	movl	$-1, %ebp
	movl	%eax, (%esp)
	call	isspace
	testl	%eax, %eax
	je	.L136
	subl	%edi, %ebx
	cmpl	$5, %ebx
	jle	.L136
	addl	%edi, %esi
	movl	%esi, (%esp)
	movl	$6, 8(%esp)
	movl	$.LC2, 4(%esp)
	call	memcmp
	cmpl	$1, %eax
	sbbl	%ebp, %ebp
	addl	$28, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	notl	%ebp
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	movl	%ebp, %eax
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,7
	.p2align 3
.L156:
	.cfi_restore_state
	movl	%ebx, 20504(%esi)
	movl	$-1, %ebp
	addl	$28, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	movl	%ebp, %eax
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
.L131:
	.cfi_restore_state
	movl	%edi, 20504(%esi)
	movl	$-1, %ebp
	jmp	.L136
.L157:
	movl	$-1, %ebp
	jmp	.L136
	.cfi_endproc
.LFE96:
	.size	_ZN10LineBuffer9get_sportEv, .-_ZN10LineBuffer9get_sportEv
	.section	.rodata.str1.1
.LC3:
	.string	"dport="
	.section	.text._ZN10LineBuffer9get_dportEv,"axG",@progbits,_ZN10LineBuffer9get_dportEv,comdat
	.align 2
	.p2align 4,,15
	.weak	_ZN10LineBuffer9get_dportEv
	.type	_ZN10LineBuffer9get_dportEv, @function
_ZN10LineBuffer9get_dportEv:
.LFB97:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	subl	$28, %esp
	.cfi_def_cfa_offset 48
	movl	48(%esp), %esi
	movl	20500(%esi), %edi
	movl	20492(%esi), %ecx
	cmpl	%ecx, %edi
	jge	.L159
	movzbl	(%esi,%edi), %eax
	movl	%edi, %edx
	subl	$97, %eax
	cmpb	$25, %al
	jbe	.L162
	jmp	.L159
	.p2align 4,,7
	.p2align 3
.L163:
	movzbl	(%esi,%edx), %eax
	subl	$97, %eax
	cmpb	$25, %al
	ja	.L161
.L162:
	addl	$1, %edx
	cmpl	%ecx, %edx
	jne	.L163
.L161:
	leal	1(%edx), %ebx
	movl	$-1, %ebp
	cmpl	%ecx, %ebx
	movl	%edx, 20504(%esi)
	jge	.L164
	cmpb	$61, (%esi,%edx)
	je	.L183
.L164:
	addl	$28, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	movl	%ebp, %eax
	popl	%ebx
	.cfi_def_cfa_offset 16
	.cfi_restore 3
	popl	%esi
	.cfi_def_cfa_offset 12
	.cfi_restore 6
	popl	%edi
	.cfi_def_cfa_offset 8
	.cfi_restore 7
	popl	%ebp
	.cfi_def_cfa_offset 4
	.cfi_restore 5
	ret
	.p2align 4,,7
	.p2align 3
.L183:
	.cfi_restore_state
	movzbl	(%esi,%ebx), %eax
	movl	%ebx, 20504(%esi)
	subl	$48, %eax
	cmpb	$9, %al
	ja	.L164
	.p2align 4,,7
	.p2align 3
.L178:
	addl	$1, %ebx
	cmpl	%ecx, %ebx
	je	.L184
	movzbl	(%esi,%ebx), %eax
	subl	$48, %eax
	cmpb	$9, %al
	jbe	.L178
	cmpl	%ecx, %ebx
	movl	%ebx, 20504(%esi)
	jge	.L185
	movsbl	(%esi,%ebx), %eax
	movl	$-1, %ebp
	movl	%eax, (%esp)
	call	isspace
	testl	%eax, %eax
	je	.L164
	subl	%edi, %ebx
	cmpl	$5, %ebx
	jle	.L164
	addl	%edi, %esi
	movl	%esi, (%esp)
	movl	$6, 8(%esp)
	movl	$.LC3, 4(%esp)
	call	memcmp
	cmpl	$1, %eax
	sbbl	%ebp, %ebp
	addl	$28, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	notl	%ebp
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	movl	%ebp, %eax
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,7
	.p2align 3
.L184:
	.cfi_restore_state
	movl	%ebx, 20504(%esi)
	movl	$-1, %ebp
	addl	$28, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	movl	%ebp, %eax
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
.L159:
	.cfi_restore_state
	movl	%edi, 20504(%esi)
	movl	$-1, %ebp
	jmp	.L164
.L185:
	movl	$-1, %ebp
	jmp	.L164
	.cfi_endproc
.LFE97:
	.size	_ZN10LineBuffer9get_dportEv, .-_ZN10LineBuffer9get_dportEv
	.section	.rodata.str1.1
.LC4:
	.string	"packets="
	.section	.text._ZN10LineBuffer11get_packetsEv,"axG",@progbits,_ZN10LineBuffer11get_packetsEv,comdat
	.align 2
	.p2align 4,,15
	.weak	_ZN10LineBuffer11get_packetsEv
	.type	_ZN10LineBuffer11get_packetsEv, @function
_ZN10LineBuffer11get_packetsEv:
.LFB98:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	subl	$28, %esp
	.cfi_def_cfa_offset 48
	movl	48(%esp), %esi
	movl	20500(%esi), %edi
	movl	20492(%esi), %ecx
	cmpl	%ecx, %edi
	jge	.L187
	movzbl	(%esi,%edi), %eax
	movl	%edi, %edx
	subl	$97, %eax
	cmpb	$25, %al
	jbe	.L190
	jmp	.L187
	.p2align 4,,7
	.p2align 3
.L191:
	movzbl	(%esi,%edx), %eax
	subl	$97, %eax
	cmpb	$25, %al
	ja	.L189
.L190:
	addl	$1, %edx
	cmpl	%ecx, %edx
	jne	.L191
.L189:
	leal	1(%edx), %ebx
	movl	$-1, %ebp
	cmpl	%ecx, %ebx
	movl	%edx, 20504(%esi)
	jge	.L192
	cmpb	$61, (%esi,%edx)
	je	.L211
.L192:
	addl	$28, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	movl	%ebp, %eax
	popl	%ebx
	.cfi_def_cfa_offset 16
	.cfi_restore 3
	popl	%esi
	.cfi_def_cfa_offset 12
	.cfi_restore 6
	popl	%edi
	.cfi_def_cfa_offset 8
	.cfi_restore 7
	popl	%ebp
	.cfi_def_cfa_offset 4
	.cfi_restore 5
	ret
	.p2align 4,,7
	.p2align 3
.L211:
	.cfi_restore_state
	movzbl	(%esi,%ebx), %eax
	movl	%ebx, 20504(%esi)
	subl	$48, %eax
	cmpb	$9, %al
	ja	.L192
	.p2align 4,,7
	.p2align 3
.L206:
	addl	$1, %ebx
	cmpl	%ecx, %ebx
	je	.L212
	movzbl	(%esi,%ebx), %eax
	subl	$48, %eax
	cmpb	$9, %al
	jbe	.L206
	cmpl	%ecx, %ebx
	movl	%ebx, 20504(%esi)
	jge	.L213
	movsbl	(%esi,%ebx), %eax
	movl	$-1, %ebp
	movl	%eax, (%esp)
	call	isspace
	testl	%eax, %eax
	je	.L192
	subl	%edi, %ebx
	cmpl	$7, %ebx
	jle	.L192
	addl	%edi, %esi
	movl	%esi, (%esp)
	movl	$8, 8(%esp)
	movl	$.LC4, 4(%esp)
	call	memcmp
	cmpl	$1, %eax
	sbbl	%ebp, %ebp
	addl	$28, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	notl	%ebp
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	movl	%ebp, %eax
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,7
	.p2align 3
.L212:
	.cfi_restore_state
	movl	%ebx, 20504(%esi)
	movl	$-1, %ebp
	addl	$28, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	movl	%ebp, %eax
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
.L187:
	.cfi_restore_state
	movl	%edi, 20504(%esi)
	movl	$-1, %ebp
	jmp	.L192
.L213:
	movl	$-1, %ebp
	jmp	.L192
	.cfi_endproc
.LFE98:
	.size	_ZN10LineBuffer11get_packetsEv, .-_ZN10LineBuffer11get_packetsEv
	.section	.rodata.str1.1
.LC5:
	.string	"bytes="
	.section	.text._ZN10LineBuffer9get_bytesEv,"axG",@progbits,_ZN10LineBuffer9get_bytesEv,comdat
	.align 2
	.p2align 4,,15
	.weak	_ZN10LineBuffer9get_bytesEv
	.type	_ZN10LineBuffer9get_bytesEv, @function
_ZN10LineBuffer9get_bytesEv:
.LFB99:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	subl	$28, %esp
	.cfi_def_cfa_offset 48
	movl	48(%esp), %esi
	movl	20500(%esi), %edi
	movl	20492(%esi), %ecx
	cmpl	%ecx, %edi
	jge	.L215
	movzbl	(%esi,%edi), %eax
	movl	%edi, %edx
	subl	$97, %eax
	cmpb	$25, %al
	jbe	.L218
	jmp	.L215
	.p2align 4,,7
	.p2align 3
.L219:
	movzbl	(%esi,%edx), %eax
	subl	$97, %eax
	cmpb	$25, %al
	ja	.L217
.L218:
	addl	$1, %edx
	cmpl	%ecx, %edx
	jne	.L219
.L217:
	leal	1(%edx), %ebx
	movl	$-1, %ebp
	cmpl	%ecx, %ebx
	movl	%edx, 20504(%esi)
	jge	.L220
	cmpb	$61, (%esi,%edx)
	je	.L239
.L220:
	addl	$28, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	movl	%ebp, %eax
	popl	%ebx
	.cfi_def_cfa_offset 16
	.cfi_restore 3
	popl	%esi
	.cfi_def_cfa_offset 12
	.cfi_restore 6
	popl	%edi
	.cfi_def_cfa_offset 8
	.cfi_restore 7
	popl	%ebp
	.cfi_def_cfa_offset 4
	.cfi_restore 5
	ret
	.p2align 4,,7
	.p2align 3
.L239:
	.cfi_restore_state
	movzbl	(%esi,%ebx), %eax
	movl	%ebx, 20504(%esi)
	subl	$48, %eax
	cmpb	$9, %al
	ja	.L220
	.p2align 4,,7
	.p2align 3
.L234:
	addl	$1, %ebx
	cmpl	%ecx, %ebx
	je	.L240
	movzbl	(%esi,%ebx), %eax
	subl	$48, %eax
	cmpb	$9, %al
	jbe	.L234
	cmpl	%ecx, %ebx
	movl	%ebx, 20504(%esi)
	jge	.L241
	movsbl	(%esi,%ebx), %eax
	movl	$-1, %ebp
	movl	%eax, (%esp)
	call	isspace
	testl	%eax, %eax
	je	.L220
	subl	%edi, %ebx
	cmpl	$5, %ebx
	jle	.L220
	addl	%edi, %esi
	movl	%esi, (%esp)
	movl	$6, 8(%esp)
	movl	$.LC5, 4(%esp)
	call	memcmp
	cmpl	$1, %eax
	sbbl	%ebp, %ebp
	addl	$28, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	notl	%ebp
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	movl	%ebp, %eax
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
	.p2align 4,,7
	.p2align 3
.L240:
	.cfi_restore_state
	movl	%ebx, 20504(%esi)
	movl	$-1, %ebp
	addl	$28, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 16
	movl	%ebp, %eax
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 12
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 8
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa_offset 4
	ret
.L215:
	.cfi_restore_state
	movl	%edi, 20504(%esi)
	movl	$-1, %ebp
	jmp	.L220
.L241:
	movl	$-1, %ebp
	jmp	.L220
	.cfi_endproc
.LFE99:
	.size	_ZN10LineBuffer9get_bytesEv, .-_ZN10LineBuffer9get_bytesEv
	.section	.rodata.str1.1
.LC6:
	.string	"%u"
.LC7:
	.string	"Line: %.*s"
.LC8:
	.string	"%d\n"
.LC9:
	.string	"tcp"
.LC10:
	.string	"ESTABLISHED"
.LC11:
	.string	"mark="
.LC12:
	.string	"secmark="
.LC13:
	.string	"use="
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align 4
.LC14:
	.string	"No enough space to store transparent ip target address"
	.section	.rodata.str1.1
.LC15:
	.string	"Match found: %s"
	.text
	.p2align 4,,15
	.type	_ZL18parse_ip_conntrackiPKcS0_iiPcij.isra.3.constprop.4, @function
_ZL18parse_ip_conntrackiPKcS0_iiPcij.isra.3.constprop.4:
.LFB1125:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	pushl	%edi
	.cfi_def_cfa_offset 12
	.cfi_offset 7, -12
	pushl	%esi
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushl	%ebx
	.cfi_def_cfa_offset 20
	.cfi_offset 3, -20
	subl	$20684, %esp
	.cfi_def_cfa_offset 20704
	movl	%edx, 44(%esp)
	movl	20704(%esp), %edx
	leal	84(%esp), %ebx
	movl	%eax, 20568(%esp)
	leal	20656(%esp), %eax
	movl	%ecx, 48(%esp)
	movl	$0, 20564(%esp)
	movl	%gs:20, %ecx
	movl	%ecx, 20668(%esp)
	xorl	%ecx, %ecx
	movl	%edx, 52(%esp)
	movl	$0, 20572(%esp)
	movl	$0, 20576(%esp)
	movl	$1, 20580(%esp)
	movl	$0, 20584(%esp)
	movl	$0, 20588(%esp)
	movl	$22, 16(%esp)
	movl	$.LC6, 12(%esp)
	movl	$6, 8(%esp)
	movl	$1, 4(%esp)
	movl	%eax, (%esp)
	call	__sprintf_chk
	leal	20662(%esp), %edx
	movl	%edx, (%esp)
	movl	$36699, 16(%esp)
	movl	$.LC6, 12(%esp)
	movl	$6, 8(%esp)
	movl	$1, 4(%esp)
	movl	%eax, 56(%esp)
	call	__sprintf_chk
	movl	44(%esp), %edx
	movl	%edx, (%esp)
	movl	%eax, 60(%esp)
	call	strlen
	movl	48(%esp), %ecx
	movl	%ecx, (%esp)
	movl	%eax, 64(%esp)
	call	strlen
	movl	20572(%esp), %edx
	movl	20564(%esp), %esi
	cmpl	%esi, %edx
	movl	%eax, 68(%esp)
	movl	%edx, %eax
	jge	.L243
	cmpb	$10, 84(%esp,%edx)
	leal	84(%esp), %ebx
	je	.L244
	movl	%edx, %eax
	leal	84(%esp), %ebx
	jmp	.L246
	.p2align 4,,7
	.p2align 3
.L248:
	cmpb	$10, (%ebx,%eax)
	je	.L244
.L246:
	addl	$1, %eax
	cmpl	%esi, %eax
	jne	.L248
.L243:
	cmpl	$20480, %esi
	je	.L407
.L249:
	movl	$20480, %eax
	subl	%esi, %eax
	addl	%ebx, %esi
	movl	%eax, 8(%esp)
	movl	20568(%esp), %eax
	movl	%esi, 4(%esp)
	movl	%eax, (%esp)
	call	read
	movl	%eax, %esi
	orl	$-1, %eax
	testl	%esi, %esi
	js	.L250
	movl	20572(%esp), %edx
	addl	20564(%esp), %esi
	cmpl	%edx, %esi
	movl	%edx, %ecx
	movl	%esi, 20564(%esp)
	je	.L250
	jle	.L251
	cmpb	$10, 84(%esp,%edx)
	jne	.L382
	.p2align 4,,6
	jmp	.L252
	.p2align 4,,7
	.p2align 3
.L255:
	cmpb	$10, (%ebx,%ecx)
	je	.L252
.L382:
	addl	$1, %ecx
	cmpl	%esi, %ecx
	.p2align 4,,2
	jne	.L255
.L251:
	movl	%esi, 20576(%esp)
	movl	$0, 20580(%esp)
	jmp	.L247
.L335:
	movl	%ebx, %eax
	movl	%edi, 20584(%esp)
	call	_ZN10LineBuffer9get_spaceEv.constprop.6
	testl	%eax, %eax
	js	.L408
	cmpl	$15, %esi
	jg	.L409
	movl	52(%esp), %edx
	leal	20592(%esp), %ecx
	movl	%ecx, 4(%esp)
	movl	%esi, 8(%esp)
	movl	%edx, (%esp)
	call	memcpy
	movl	52(%esp), %edx
	movb	$0, (%edx,%esi)
	movl	stderr, %eax
	movl	%edx, 12(%esp)
	movl	$.LC15, 8(%esp)
	movl	$1, 4(%esp)
	movl	%eax, (%esp)
	call	__fprintf_chk
	xorl	%eax, %eax
	.p2align 4,,7
	.p2align 3
.L250:
	movl	20668(%esp), %ecx
	xorl	%gs:20, %ecx
	jne	.L410
	addl	$20684, %esp
	.cfi_remember_state
	.cfi_def_cfa_offset 20
	popl	%ebx
	.cfi_def_cfa_offset 16
	.cfi_restore 3
	popl	%esi
	.cfi_def_cfa_offset 12
	.cfi_restore 6
	popl	%edi
	.cfi_def_cfa_offset 8
	.cfi_restore 7
	popl	%ebp
	.cfi_def_cfa_offset 4
	.cfi_restore 5
	ret
.L244:
	.cfi_restore_state
	addl	$1, %eax
	movl	%eax, 20576(%esp)
	movl	$1, 20580(%esp)
.L247:
	leal	20592(%esp), %eax
	movl	$16, %ecx
	movl	%eax, 36(%esp)
	movl	36(%esp), %edi
	xorl	%eax, %eax
	rep stosl
	movl	64(%esp), %ecx
	movl	20576(%esp), %eax
	addl	$4, %ecx
	movl	%ecx, 72(%esp)
	movl	68(%esp), %ecx
	addl	$4, %ecx
	movl	%ecx, 76(%esp)
	.p2align 4,,7
	.p2align 3
.L396:
	subl	%edx, %eax
	movl	%eax, 12(%esp)
	movl	stderr, %eax
	leal	(%ebx,%edx), %ecx
	movl	%ecx, 16(%esp)
	movl	$.LC7, 8(%esp)
	movl	$1, 4(%esp)
	movl	%eax, (%esp)
	call	__fprintf_chk
	movl	20572(%esp), %ebp
	movl	20576(%esp), %edi
	movl	%ebp, 20584(%esp)
	movl	%ebp, %esi
	cmpl	%edi, %ebp
	jge	.L256
	.p2align 4,,7
	.p2align 3
.L259:
	movzbl	(%ebx,%esi), %eax
	leal	-97(%eax), %edx
	cmpb	$25, %dl
	jbe	.L257
	leal	-65(%eax), %edx
	cmpb	$25, %dl
	jbe	.L257
	leal	-48(%eax), %edx
	cmpb	$9, %dl
	ja	.L411
.L257:
	addl	$1, %esi
	cmpl	%edi, %esi
	jne	.L259
.L256:
	movl	%esi, 20588(%esp)
	movl	$518, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	.p2align 4,,7
	.p2align 3
.L260:
	movl	20576(%esp), %edx
	movl	20564(%esp), %esi
	movl	%edx, 20572(%esp)
	cmpl	%esi, %edx
	jge	.L337
	cmpb	$10, 84(%esp,%edx)
	movl	%edx, %eax
	jne	.L340
	jmp	.L338
	.p2align 4,,7
	.p2align 3
.L342:
	cmpb	$10, (%ebx,%eax)
	je	.L338
.L340:
	addl	$1, %eax
	cmpl	%esi, %eax
	.p2align 4,,2
	jne	.L342
.L337:
	cmpl	$20480, %esi
	je	.L412
.L344:
	movl	$20480, %eax
	subl	%esi, %eax
	addl	%ebx, %esi
	movl	%eax, 8(%esp)
	movl	20568(%esp), %eax
	movl	%esi, 4(%esp)
	movl	%eax, (%esp)
	call	read
	testl	%eax, %eax
	js	.L376
	movl	20572(%esp), %edx
	addl	20564(%esp), %eax
	cmpl	%edx, %eax
	movl	%eax, 20564(%esp)
	je	.L376
	jle	.L345
	cmpb	$10, 84(%esp,%edx)
	movl	%edx, %ecx
	jne	.L348
	.p2align 4,,4
	jmp	.L346
	.p2align 4,,7
	.p2align 3
.L349:
	cmpb	$10, (%ebx,%ecx)
	je	.L346
.L348:
	addl	$1, %ecx
	cmpl	%eax, %ecx
	.p2align 4,,2
	jne	.L349
.L345:
	movl	%eax, 20576(%esp)
	movl	$0, 20580(%esp)
	jmp	.L396
	.p2align 4,,7
	.p2align 3
.L338:
	addl	$1, %eax
	movl	%eax, 20576(%esp)
	movl	$1, 20580(%esp)
	jmp	.L396
	.p2align 4,,7
	.p2align 3
.L411:
	cmpb	$45, %al
	je	.L257
	movsbl	%al, %eax
	movl	%eax, (%esp)
	movl	%eax, 40(%esp)
	call	isspace
	testl	%eax, %eax
	je	.L256
	movl	%esi, %eax
	subl	%ebp, %eax
	cmpl	$3, %eax
	movl	%esi, 20588(%esp)
	jne	.L350
	addl	%ebx, %ebp
	movl	$3, 8(%esp)
	movl	$.LC9, 4(%esp)
	movl	%ebp, (%esp)
	call	memcmp
	testl	%eax, %eax
	je	.L261
.L350:
	movl	$521, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
	.p2align 4,,7
	.p2align 3
.L376:
	movl	$-1, %eax
	jmp	.L250
	.p2align 4,,7
	.p2align 3
.L346:
	leal	1(%ecx), %eax
	movl	%eax, 20576(%esp)
	movl	$1, 20580(%esp)
	jmp	.L396
.L261:
	movl	40(%esp), %edx
	movl	%esi, 20584(%esp)
	movl	%edx, (%esp)
	call	isspace
	testl	%eax, %eax
	je	.L262
	addl	$1, %esi
	cmpl	%esi, %edi
	jg	.L381
	.p2align 4,,4
	jmp	.L263
	.p2align 4,,7
	.p2align 3
.L413:
	addl	$1, %esi
	cmpl	%edi, %esi
	je	.L263
.L381:
	movsbl	(%ebx,%esi), %eax
	movl	%eax, (%esp)
	call	isspace
	testl	%eax, %eax
	jne	.L413
	cmpl	%esi, %edi
	movl	%esi, 20588(%esp)
	movl	%esi, 20584(%esp)
	jle	.L352
	movzbl	84(%esp,%esi), %eax
	movl	%esi, %ebp
	subl	$48, %eax
	cmpb	$9, %al
	jbe	.L390
	jmp	.L447
	.p2align 4,,7
	.p2align 3
.L268:
	movzbl	(%ebx,%ebp), %eax
	subl	$48, %eax
	cmpb	$9, %al
	ja	.L369
.L390:
	addl	$1, %ebp
	cmpl	%edi, %ebp
	jne	.L268
	movl	%ebp, 20588(%esp)
.L271:
	movl	%ebp, %eax
	subl	%esi, %eax
	cmpl	$1, %eax
	jne	.L272
	cmpb	$54, (%ebx,%esi)
	je	.L273
.L272:
	movl	$530, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L412:
	subl	%edx, %esi
	addl	%ebx, %edx
	movl	$20508, 12(%esp)
	movl	%esi, 8(%esp)
	movl	%edx, 4(%esp)
	movl	%ebx, (%esp)
	call	__memmove_chk
	movl	%esi, 20564(%esp)
	movl	$0, 20572(%esp)
	jmp	.L344
.L263:
	movl	%esi, 20584(%esp)
.L352:
	cmpl	%esi, %edi
	movl	%esi, 20588(%esp)
	je	.L270
	movl	%esi, %ebp
	orl	$-1, %edi
	jmp	.L354
	.p2align 4,,7
	.p2align 3
.L369:
	xorl	%edi, %edi
.L266:
	movl	%ebp, 20588(%esp)
.L354:
	movsbl	84(%esp,%ebp), %eax
	movl	%eax, (%esp)
	call	isspace
	testl	%eax, %eax
	jne	.L415
.L270:
	movl	$527, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L415:
	addl	$1, %edi
	jne	.L271
	.p2align 4,,8
	jmp	.L270
.L273:
	movl	%ebx, %eax
	movl	%ebp, 20584(%esp)
	call	_ZN10LineBuffer9get_spaceEv.constprop.6
	testl	%eax, %eax
	js	.L416
	movl	20588(%esp), %esi
	movl	20576(%esp), %edx
	movl	%esi, 20584(%esp)
	cmpl	%esi, %edx
	jle	.L275
	movzbl	84(%esp,%esi), %eax
	subl	$48, %eax
	cmpb	$9, %al
	jbe	.L393
	jmp	.L448
	.p2align 4,,7
	.p2align 3
.L279:
	movzbl	(%ebx,%esi), %eax
	subl	$48, %eax
	cmpb	$9, %al
	ja	.L371
.L393:
	addl	$1, %esi
	cmpl	%edx, %esi
	jne	.L279
	movl	%esi, 20588(%esp)
.L282:
	movl	%ebx, %eax
	movl	%esi, 20584(%esp)
	call	_ZN10LineBuffer9get_spaceEv.constprop.6
	testl	%eax, %eax
	js	.L418
	movl	20588(%esp), %edi
	movl	20576(%esp), %edx
	movl	%edi, 20584(%esp)
	cmpl	%edx, %edi
	jge	.L284
	movzbl	84(%esp,%edi), %eax
	movl	%edi, %esi
	leal	-65(%eax), %ecx
	cmpb	$25, %cl
	jbe	.L287
	jmp	.L285
	.p2align 4,,7
	.p2align 3
.L289:
	movzbl	(%ebx,%esi), %eax
	leal	-65(%eax), %ecx
	cmpb	$25, %cl
	ja	.L285
.L287:
	addl	$1, %esi
	cmpl	%edx, %esi
	jne	.L289
.L406:
	movl	%esi, %edi
.L284:
	movl	%edi, 20588(%esp)
	movl	$543, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L407:
	subl	%edx, %esi
	addl	%ebx, %edx
	movl	$20508, 12(%esp)
	movl	%esi, 8(%esp)
	movl	%edx, 4(%esp)
	movl	%ebx, (%esp)
	call	__memmove_chk
	movl	%esi, 20564(%esp)
	movl	$0, 20572(%esp)
	jmp	.L249
.L416:
	movl	$532, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L252:
	addl	$1, %ecx
	movl	%ecx, 20576(%esp)
	movl	$1, 20580(%esp)
	jmp	.L247
.L262:
	movl	$523, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L447:
	orl	$-1, %edi
	jmp	.L266
.L371:
	xorl	%edi, %edi
.L276:
	movl	%esi, 20588(%esp)
.L355:
	movsbl	84(%esp,%esi), %eax
	movl	%eax, (%esp)
	call	isspace
	testl	%eax, %eax
	je	.L281
	addl	$1, %edi
	jne	.L282
.L281:
	movl	$536, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L285:
	movsbl	%al, %eax
	movl	%eax, (%esp)
	call	isspace
	testl	%eax, %eax
	je	.L406
	movl	%esi, %eax
	subl	%edi, %eax
	cmpl	$11, %eax
	movl	%esi, 20588(%esp)
	je	.L419
.L356:
	movl	$546, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L419:
	addl	%ebx, %edi
	movl	$11, 8(%esp)
	movl	$.LC10, 4(%esp)
	movl	%edi, (%esp)
	call	memcmp
	testl	%eax, %eax
	jne	.L356
	movl	%ebx, %eax
	movl	%esi, 20584(%esp)
	call	_ZN10LineBuffer9get_spaceEv.constprop.6
	testl	%eax, %eax
	js	.L420
	movl	20588(%esp), %eax
	movl	%ebx, (%esp)
	movl	%eax, 20584(%esp)
	call	_ZN10LineBuffer10get_src_ipEv
	testl	%eax, %eax
	js	.L421
	movl	20588(%esp), %eax
	movl	%eax, 20584(%esp)
	movl	%ebx, %eax
	call	_ZN10LineBuffer9get_spaceEv.constprop.6
	testl	%eax, %eax
	js	.L422
	movl	20588(%esp), %eax
	movl	%ebx, (%esp)
	movl	%eax, 20584(%esp)
	call	_ZN10LineBuffer10get_dst_ipEv
	testl	%eax, %eax
	js	.L423
	movl	20584(%esp), %eax
	movl	20588(%esp), %esi
	subl	%eax, %esi
	cmpl	$68, %esi
	jg	.L376
	leal	20592(%esp), %ecx
	subl	$4, %esi
	leal	4(%ebx,%eax), %eax
	movl	%ecx, (%esp)
	movl	$64, 12(%esp)
	movl	%esi, 8(%esp)
	movl	%eax, 4(%esp)
	call	__memcpy_chk
	movl	20588(%esp), %eax
	movl	%eax, 20584(%esp)
	movl	%ebx, %eax
	call	_ZN10LineBuffer9get_spaceEv.constprop.6
	testl	%eax, %eax
	js	.L424
	movl	20588(%esp), %eax
	movl	%ebx, (%esp)
	movl	%eax, 20584(%esp)
	call	_ZN10LineBuffer9get_sportEv
	testl	%eax, %eax
	js	.L425
	movl	20588(%esp), %eax
	movl	%eax, 20584(%esp)
	movl	%ebx, %eax
	call	_ZN10LineBuffer9get_spaceEv.constprop.6
	testl	%eax, %eax
	js	.L426
	movl	20588(%esp), %eax
	movl	%ebx, (%esp)
	movl	%eax, 20584(%esp)
	call	_ZN10LineBuffer9get_dportEv
	testl	%eax, %eax
	js	.L427
	movl	20588(%esp), %eax
	movl	%eax, 20584(%esp)
	movl	%ebx, %eax
	call	_ZN10LineBuffer9get_spaceEv.constprop.6
	testl	%eax, %eax
	js	.L428
	movl	20588(%esp), %eax
	movl	%ebx, (%esp)
	movl	%eax, 20584(%esp)
	call	_ZN10LineBuffer11get_packetsEv
	testl	%eax, %eax
	jne	.L300
	movl	20588(%esp), %eax
	movl	%eax, 20584(%esp)
	movl	%ebx, %eax
	call	_ZN10LineBuffer9get_spaceEv.constprop.6
	testl	%eax, %eax
	js	.L429
	movl	20588(%esp), %eax
	movl	%eax, 20584(%esp)
.L300:
	movl	%ebx, (%esp)
	call	_ZN10LineBuffer9get_bytesEv
	testl	%eax, %eax
	jne	.L302
	movl	20588(%esp), %eax
	movl	%eax, 20584(%esp)
	movl	%ebx, %eax
	call	_ZN10LineBuffer9get_spaceEv.constprop.6
	testl	%eax, %eax
	js	.L430
	movl	20588(%esp), %eax
	movl	%eax, 20584(%esp)
.L302:
	movl	%ebx, (%esp)
	call	_ZN10LineBuffer10get_src_ipEv
	testl	%eax, %eax
	js	.L431
	movl	20588(%esp), %edi
	movl	20584(%esp), %eax
	movl	%edi, %edx
	subl	%eax, %edx
	cmpl	72(%esp), %edx
	jne	.L305
	movl	64(%esp), %edx
	leal	4(%ebx,%eax), %eax
	movl	%eax, (%esp)
	movl	%edx, 8(%esp)
	movl	44(%esp), %edx
	movl	%edx, 4(%esp)
	call	memcmp
	testl	%eax, %eax
	je	.L306
.L305:
	movl	$597, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L418:
	movl	$539, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L275:
	movl	%esi, 20588(%esp)
	je	.L281
	orl	$-1, %edi
	jmp	.L355
.L448:
	orl	$-1, %edi
	jmp	.L276
.L410:
	.p2align 4,,5
	call	__stack_chk_fail
.L431:
	movl	$595, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L428:
	movl	$577, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L427:
	movl	$575, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L430:
	movl	$590, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L306:
	movl	%ebx, %eax
	movl	%edi, 20584(%esp)
	call	_ZN10LineBuffer9get_spaceEv.constprop.6
	testl	%eax, %eax
	js	.L432
	movl	20588(%esp), %eax
	movl	%ebx, (%esp)
	movl	%eax, 20584(%esp)
	call	_ZN10LineBuffer10get_dst_ipEv
	testl	%eax, %eax
	js	.L433
	movl	20588(%esp), %edi
	movl	20584(%esp), %eax
	movl	%edi, %edx
	subl	%eax, %edx
	cmpl	76(%esp), %edx
	jne	.L309
	movl	68(%esp), %ecx
	leal	4(%ebx,%eax), %eax
	movl	%eax, (%esp)
	movl	%ecx, 8(%esp)
	movl	48(%esp), %ecx
	movl	%ecx, 4(%esp)
	call	memcmp
	testl	%eax, %eax
	je	.L310
.L309:
	movl	$606, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L433:
	movl	$604, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L432:
	movl	$599, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L310:
	movl	%ebx, %eax
	movl	%edi, 20584(%esp)
	call	_ZN10LineBuffer9get_spaceEv.constprop.6
	testl	%eax, %eax
	js	.L434
	movl	20588(%esp), %eax
	movl	%ebx, (%esp)
	movl	%eax, 20584(%esp)
	call	_ZN10LineBuffer9get_sportEv
	testl	%eax, %eax
	js	.L435
	movl	20588(%esp), %edi
	movl	20584(%esp), %eax
	movl	56(%esp), %edx
	movl	%edi, %ecx
	subl	%eax, %ecx
	addl	$6, %edx
	cmpl	%edx, %ecx
	jne	.L313
	movl	56(%esp), %edx
	leal	20656(%esp), %ecx
	leal	6(%ebx,%eax), %eax
	movl	%ecx, 4(%esp)
	movl	%eax, (%esp)
	movl	%edx, 8(%esp)
	call	memcmp
	testl	%eax, %eax
	je	.L314
.L313:
	movl	$614, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L435:
	movl	$612, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L434:
	movl	$608, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L314:
	movl	%ebx, %eax
	movl	%edi, 20584(%esp)
	call	_ZN10LineBuffer9get_spaceEv.constprop.6
	testl	%eax, %eax
	js	.L436
	movl	20588(%esp), %eax
	movl	%ebx, (%esp)
	movl	%eax, 20584(%esp)
	call	_ZN10LineBuffer9get_dportEv
	testl	%eax, %eax
	js	.L437
	movl	20588(%esp), %edi
	movl	20584(%esp), %eax
	movl	60(%esp), %edx
	movl	%edi, %ecx
	subl	%eax, %ecx
	addl	$6, %edx
	cmpl	%edx, %ecx
	jne	.L317
	movl	60(%esp), %edx
	leal	20662(%esp), %ecx
	leal	6(%ebx,%eax), %eax
	movl	%ecx, 4(%esp)
	movl	%eax, (%esp)
	movl	%edx, 8(%esp)
	call	memcmp
	testl	%eax, %eax
	je	.L318
.L317:
	movl	$622, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L437:
	movl	$620, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L436:
	movl	$616, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L318:
	movl	%ebx, %eax
	movl	%edi, 20584(%esp)
	call	_ZN10LineBuffer9get_spaceEv.constprop.6
	testl	%eax, %eax
	js	.L438
	movl	20588(%esp), %eax
	movl	%ebx, (%esp)
	movl	%eax, 20584(%esp)
	call	_ZN10LineBuffer11get_packetsEv
	testl	%eax, %eax
	jne	.L320
	movl	20588(%esp), %eax
	movl	%eax, 20584(%esp)
	movl	%ebx, %eax
	call	_ZN10LineBuffer9get_spaceEv.constprop.6
	testl	%eax, %eax
	js	.L439
	movl	20588(%esp), %eax
	movl	%eax, 20584(%esp)
.L320:
	movl	%ebx, (%esp)
	call	_ZN10LineBuffer9get_bytesEv
	testl	%eax, %eax
	je	.L440
.L322:
	movl	20584(%esp), %eax
	movl	20576(%esp), %edx
	cmpl	%edx, %eax
	jge	.L324
	cmpb	$91, 84(%esp,%eax)
	je	.L441
.L324:
	movl	$642, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L440:
	movl	20588(%esp), %eax
	movl	%eax, 20584(%esp)
	movl	%ebx, %eax
	call	_ZN10LineBuffer9get_spaceEv.constprop.6
	testl	%eax, %eax
	js	.L442
	movl	20588(%esp), %eax
	movl	%eax, 20584(%esp)
	jmp	.L322
.L441:
	addl	$1, %eax
	orl	$-1, %ecx
	movl	%edx, 40(%esp)
.L325:
	cmpl	40(%esp), %eax
	je	.L326
	movzbl	(%ebx,%eax), %edi
	subl	$65, %edi
	movl	%edi, %edx
	cmpb	$25, %dl
	ja	.L326
	addl	$1, %eax
	xorl	%ecx, %ecx
	jmp	.L325
.L442:
	movl	$637, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L326:
	addl	$1, %ecx
	movl	40(%esp), %edx
	je	.L324
	cmpl	%eax, %edx
	.p2align 4,,2
	jle	.L324
	cmpb	$93, 84(%esp,%eax)
	jne	.L324
	addl	$1, %eax
	movl	%eax, 20588(%esp)
	movl	%eax, 20584(%esp)
	movl	%ebx, %eax
	call	_ZN10LineBuffer9get_spaceEv.constprop.6
	testl	%eax, %eax
	js	.L443
	movl	20588(%esp), %eax
	movl	%ebx, (%esp)
	movl	%eax, 20584(%esp)
	call	_ZN10LineBuffer11get_var_numEv
	testl	%eax, %eax
	js	.L330
	movl	20588(%esp), %edi
	movl	20584(%esp), %eax
	movl	%edi, %edx
	subl	%eax, %edx
	cmpl	$4, %edx
	jle	.L330
	addl	%ebx, %eax
	movl	$5, 8(%esp)
	movl	$.LC11, 4(%esp)
	movl	%eax, (%esp)
	call	memcmp
	testl	%eax, %eax
	je	.L331
.L330:
	movl	$648, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L443:
	movl	$644, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L331:
	movl	%ebx, %eax
	movl	%edi, 20584(%esp)
	call	_ZN10LineBuffer9get_spaceEv.constprop.6
	testl	%eax, %eax
	js	.L444
	movl	20588(%esp), %eax
	movl	%ebx, (%esp)
	movl	%eax, 20584(%esp)
	call	_ZN10LineBuffer11get_var_numEv
	testl	%eax, %eax
	js	.L332
	movl	20588(%esp), %edi
	movl	20584(%esp), %eax
	movl	%edi, %edx
	subl	%eax, %edx
	cmpl	$7, %edx
	jle	.L332
	addl	%ebx, %eax
	movl	$8, 8(%esp)
	movl	$.LC12, 4(%esp)
	movl	%eax, (%esp)
	call	memcmp
	testl	%eax, %eax
	je	.L445
.L332:
	movl	%ebx, (%esp)
	call	_ZN10LineBuffer11get_var_numEv
	testl	%eax, %eax
	js	.L334
	movl	20588(%esp), %edi
	movl	20584(%esp), %eax
	movl	%edi, %edx
	subl	%eax, %edx
	cmpl	$3, %edx
	jle	.L334
	addl	%ebx, %eax
	movl	$4, 8(%esp)
	movl	$.LC13, 4(%esp)
	movl	%eax, (%esp)
	call	memcmp
	testl	%eax, %eax
	je	.L335
.L334:
	movl	$661, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L444:
	movl	$650, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L445:
	movl	%ebx, %eax
	movl	%edi, 20584(%esp)
	call	_ZN10LineBuffer9get_spaceEv.constprop.6
	testl	%eax, %eax
	js	.L446
	movl	20588(%esp), %eax
	movl	%eax, 20584(%esp)
	jmp	.L332
.L438:
	movl	$624, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L409:
	movl	stderr, %eax
	movl	$54, 8(%esp)
	movl	$1, 4(%esp)
	movl	$.LC14, (%esp)
	movl	%eax, 12(%esp)
	call	fwrite
	orl	$-1, %eax
	jmp	.L250
.L408:
	movl	$663, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L439:
	movl	$630, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L446:
	movl	$656, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L426:
	movl	$571, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L425:
	movl	$569, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L424:
	movl	$565, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L423:
	movl	$558, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L422:
	movl	$554, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L421:
	movl	$552, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L420:
	movl	$548, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
.L429:
	movl	$583, 8(%esp)
	movl	$.LC8, 4(%esp)
	movl	$1, (%esp)
	call	__printf_chk
	jmp	.L260
	.cfi_endproc
.LFE1125:
	.size	_ZL18parse_ip_conntrackiPKcS0_iiPcij.isra.3.constprop.4, .-_ZL18parse_ip_conntrackiPKcS0_iiPcij.isra.3.constprop.4
	.section	.rodata.str1.1
.LC16:
	.string	"/tmp/ip_conntrack"
	.section	.rodata.str1.4
	.align 4
.LC17:
	.string	"Failed to get transparent proxy target from ip_conntrack\n"
	.section	.text.startup,"ax",@progbits
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
.LFB1112:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	andl	$-16, %esp
	subl	$64, %esp
	movl	%gs:20, %eax
	movl	%eax, 60(%esp)
	xorl	%eax, %eax
	movl	$825110577, 36(%esp)
	movl	$859057712, 40(%esp)
	movl	$3354926, 44(%esp)
	movl	$825110577, 48(%esp)
	movl	$926166576, 52(%esp)
	movl	$3355182, 56(%esp)
	movl	$773857312, 20(%esp)
	movl	$773857312, 24(%esp)
	movl	$773857312, 28(%esp)
	movl	$2105376, 32(%esp)
	movl	$0, 4(%esp)
	movl	$.LC16, (%esp)
	.cfi_offset 3, -12
	call	open
	leal	36(%esp), %ecx
	leal	48(%esp), %edx
	movl	%eax, %ebx
	leal	20(%esp), %eax
	movl	%eax, (%esp)
	movl	%ebx, %eax
	call	_ZL18parse_ip_conntrackiPKcS0_iiPcij.isra.3.constprop.4
	testl	%eax, %eax
	je	.L450
	movl	$.LC17, 4(%esp)
	movl	$_ZSt4cerr, (%esp)
	call	_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc
.L450:
	movl	%ebx, (%esp)
	call	close
	xorl	%eax, %eax
	movl	60(%esp), %edx
	xorl	%gs:20, %edx
	jne	.L452
	movl	-4(%ebp), %ebx
	leave
	.cfi_remember_state
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	.cfi_restore 3
	ret
.L452:
	.cfi_restore_state
	call	__stack_chk_fail
	.cfi_endproc
.LFE1112:
	.size	main, .-main
	.p2align 4,,15
	.type	_GLOBAL__sub_I_main, @function
_GLOBAL__sub_I_main:
.LFB1117:
	.cfi_startproc
	subl	$28, %esp
	.cfi_def_cfa_offset 32
	movl	$_ZStL8__ioinit, (%esp)
	call	_ZNSt8ios_base4InitC1Ev
	movl	$__dso_handle, 8(%esp)
	movl	$_ZStL8__ioinit, 4(%esp)
	movl	$_ZNSt8ios_base4InitD1Ev, (%esp)
	call	__cxa_atexit
	addl	$28, %esp
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
.LFE1117:
	.size	_GLOBAL__sub_I_main, .-_GLOBAL__sub_I_main
	.section	.ctors,"aw",@progbits
	.align 4
	.long	_GLOBAL__sub_I_main
	.local	_ZStL8__ioinit
	.comm	_ZStL8__ioinit,1,1
	.weakref	_ZL20__gthrw_pthread_oncePiPFvvE,pthread_once
	.weakref	_ZL27__gthrw_pthread_getspecificj,pthread_getspecific
	.weakref	_ZL27__gthrw_pthread_setspecificjPKv,pthread_setspecific
	.weakref	_ZL22__gthrw_pthread_createPmPK14pthread_attr_tPFPvS3_ES3_,pthread_create
	.weakref	_ZL20__gthrw_pthread_joinmPPv,pthread_join
	.weakref	_ZL21__gthrw_pthread_equalmm,pthread_equal
	.weakref	_ZL20__gthrw_pthread_selfv,pthread_self
	.weakref	_ZL22__gthrw_pthread_detachm,pthread_detach
	.weakref	_ZL22__gthrw_pthread_cancelm,pthread_cancel
	.weakref	_ZL19__gthrw_sched_yieldv,sched_yield
	.weakref	_ZL26__gthrw_pthread_mutex_lockP15pthread_mutex_t,pthread_mutex_lock
	.weakref	_ZL29__gthrw_pthread_mutex_trylockP15pthread_mutex_t,pthread_mutex_trylock
	.weakref	_ZL31__gthrw_pthread_mutex_timedlockP15pthread_mutex_tPK8timespec,pthread_mutex_timedlock
	.weakref	_ZL28__gthrw_pthread_mutex_unlockP15pthread_mutex_t,pthread_mutex_unlock
	.weakref	_ZL26__gthrw_pthread_mutex_initP15pthread_mutex_tPK19pthread_mutexattr_t,pthread_mutex_init
	.weakref	_ZL29__gthrw_pthread_mutex_destroyP15pthread_mutex_t,pthread_mutex_destroy
	.weakref	_ZL30__gthrw_pthread_cond_broadcastP14pthread_cond_t,pthread_cond_broadcast
	.weakref	_ZL27__gthrw_pthread_cond_signalP14pthread_cond_t,pthread_cond_signal
	.weakref	_ZL25__gthrw_pthread_cond_waitP14pthread_cond_tP15pthread_mutex_t,pthread_cond_wait
	.weakref	_ZL30__gthrw_pthread_cond_timedwaitP14pthread_cond_tP15pthread_mutex_tPK8timespec,pthread_cond_timedwait
	.weakref	_ZL28__gthrw_pthread_cond_destroyP14pthread_cond_t,pthread_cond_destroy
	.weakref	_ZL26__gthrw_pthread_key_createPjPFvPvE,pthread_key_create
	.weakref	_ZL26__gthrw_pthread_key_deletej,pthread_key_delete
	.weakref	_ZL30__gthrw_pthread_mutexattr_initP19pthread_mutexattr_t,pthread_mutexattr_init
	.weakref	_ZL33__gthrw_pthread_mutexattr_settypeP19pthread_mutexattr_ti,pthread_mutexattr_settype
	.weakref	_ZL33__gthrw_pthread_mutexattr_destroyP19pthread_mutexattr_t,pthread_mutexattr_destroy
	.ident	"GCC: (Ubuntu/Linaro 4.6.2-10ubuntu1~11.10.1) 4.6.2"
	.section	.note.GNU-stack,"",@progbits
