.data
a: .space 16
.text
main:
Label_0:
addi $29,$zero,0
li $30,3
sw $30,a($29)
addi $29,$zero,0
lw $3,a($29)
addi $30,$zero,0
lw $29,a($30)
addi $30,$zero,1
add $4,$29,$30
addi $29,$zero,4
sw $4,a($29)
addi $30,$zero,4
lw $29,a($30)
addi $30,$zero,7
add $6,$29,$30
addi $29,$zero,8
sw $6,a($29)
addi $30,$zero,8
lw $29,a($30)
addi $30,$zero,2
add $8,$29,$30
addi $30,$zero,8
lw $29,a($30)
addi $30,$zero,2
mul $9,$29,$30
move $10,$9
addi $11,$zero,2
addi $30,$zero,4
lw $29,a($30)
add $30,$zero,$10
add $12,$29,$30
addi $30,$zero,0
lw $29,a($30)
add $30,$zero,$12
slt $31,$30,$29
bne $31,$zero,Label_1
addi $13,$zero,0
j Label_2
Label_1:
addi $13,$zero,1
addi $30,$zero,4
lw $29,a($30)
add $30,$zero,$10
mul $14,$29,$30
add $29,$zero,$14
addi $30,$zero,1
add $15,$29,$30
addi $30,$zero,0
lw $29,a($30)
add $30,$zero,$15
add $16,$29,$30
move $17,$16
j Label_3
Label_2:
addi $29,$zero,0
lw $17,a($29)
Label_3:
add $29,$zero,$11
addi $30,$zero,100
slt $31,$29,$30
bne $31,$zero,Label_4
sub $31,$29,$30
beq $31,$zero,Label_4
addi $18,$zero,0
j Label_5
Label_4:
addi $18,$zero,1
add $29,$zero,$11
addi $30,$zero,2
mul $19,$29,$30
move $11,$19
j Label_3
Label_5:
add $29,$zero,$11
add $30,$zero,$17
add $20,$29,$30
move $21,$20
addi $29,$zero,12
sw $21,a($29)
addi $29,$zero,12
lw $3,a($29)
