# as_ex03_ifelse.s
 
        .text
        .globl main
main:
        li $s0, 1				# i = 1
        li $s1, 1				# j = 1
        li $s2, 0
        li $s3, 4

		# i == j case
        bne $s0, $s1, else1
        add $s2, $zero, $s3		# f = g		
        j exit1
else1:  sub $s2, $zero $s3		# f = -g
exit1:
		# i != j case
        add $s0, $zero, $zero # $s0 = 0, i = 0

        bne $s0, $s1, else2
        add $s2, $zero, $s3
        j exit2
else2:  sub $s2, $zero $s3
exit2:

        jr $ra
