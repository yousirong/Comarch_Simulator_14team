# as_ex03_ifelse.s
 
        .text
        .globl main
main:
        addi $sp, $sp, -4	# push return address
        sw $ra, 0($sp)		# push return address
        li $a0, 1			# argument setting
        li $a1, 2 			# argument setting
        jal leaf_fct		# call leaf_fct()
        add $s0, $v0, $zero	# save return value to $s0
        lw $ra, 0($sp)		# pop return address
        addi $sp, $sp, 4	# pop return address
        jr $ra				# return from main()

leaf_fct:
        add  $v0, $a0, $a1	# calculate and set return value
        jr   $ra			# return from leaf_fct()
