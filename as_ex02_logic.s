# as_ex02_logic.s
 
        .text
        .globl main
main:
        li $s0, 0x000055AA
        li $s1, 0x0000AA55
        li $s2, 0xAA550000

        sll $t0, $s1, 4		# $t0 = $s1 << 4
        srl $t1, $s2, 4		# $t1 = $s2 >> 4
        sra $t2, $s2, 4
        and $t3, $s0, $s1	# $t3 = $s0 & $s1
        or  $t4, $s0, $s1 	# $t4 = $s0 | $s1
        xor $t5, $s0, $s1	# $t5 = $s0 ^ $s1
        nor $t6, $s0, $s1	# $t6 = ~($s0 | $s1)

        jr $ra
