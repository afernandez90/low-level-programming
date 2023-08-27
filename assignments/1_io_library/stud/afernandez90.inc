section .text

; Arguments:
;   rdi: exit code.
; Results:
;   Does not return.
exit:
    mov rax, 60
    syscall

; Arguments:
;   rdi: address of null-terminated string.
; Results:
;   rax: length of given string.
string_length:
    xor rax, rax
.loop:
    cmp byte [rdi+rax], 0
    je .done
    inc rax
    jmp .loop
.done:
    ret

; Arguments:
;   rdi: address of null-terminated string.
; Results:
;   Prints the given string to stdout.
print_string:
    push rdi
    call string_length
    pop rsi
    mov rdx, rax
    mov rax, 1
    mov rdi, 1
    syscall
    ret

; Arguments:
;   rdi: character value.
; Results:
;   Prints the given character to stdout.
print_char:
    push rdi
    mov rdi, rsp
    call print_string
    pop rdi
    ret

; Arguments:
;   None.
; Results:
;   Prints the given character to stdout.
print_newline:
    mov rdi, 10
    jmp print_char

; Arguments:
;   rdi: A negative integer value.
; Results:
;   Prints the given integer to stdout.
print_negative_int:
    push rdi
    mov rdi, '-'
    call print_char
    pop rdi
    neg rdi
    jmp print_uint

; Code-efficient version as per Question 22.
; Arguments:
;   rdi: An integer value.
; Results:
;   Prints the given integer to stdout.
print_int:
    test rdi, rdi
    js print_negative_int

; Arguments:
;   rdi: An unsigned integer value.
; Results:
;   Prints the given unsigned integer to stdout.
print_uint:
    mov rax, rdi
    mov rsi, rsp
    push 0
    mov rcx, 10
.loop:
    xor rdx, rdx
    div rcx
    or rdx, 0x30 ; convert to ASCII
    dec rsp
    mov [rsp], dl
    cmp rax, 0
    jnz .loop
.done:
    mov rdi, rsp
    push rsi
    call print_string
    pop rsp
    ret

; Arguments:
;   None.
; Results:
;   rax: A single character from stdin or 0 if stream terminated.
read_char:
    xor rax, rax
    xor rdi, rdi
    push 0
    mov rsi, rsp
    mov rdx, 1
    syscall
    pop rax
    ret

; Reads a word from stdin into the given buffer and null-terminates it. 
; Skips any opening spaces, tabs or line breaks.
;
; Arguments:
;   rdi: Starting address of a buffer.
;   rsi: Buffer size.
; Results:
;   rax: Address of given buffer, or 0 if word didn't fit.
;   rdx: Number of characters in the read word.
read_word:
    push r12 ; buffer
    push r13 ; size
    push r14 ; current char
    push r15 ; current index
    mov r12, rdi
    mov r13, rsi
.skip_whitespaces:
    call read_char
    mov r14, rax
    mov rdi, r14
    call is_whitespace
    test rax, rax
    jnz .skip_whitespaces
    xor r15, r15
.loop:
    test r14, r14
    jz .finalise
    mov [r12+r15], r14b ; store next character
    inc r15
    cmp r15, r13 ; check if the word doesn't fit
    je .not_enough_space
    call read_char
    mov r14, rax
    mov rdi, r14
    call is_whitespace ; check if the word has ended
    test rax, rax
    jz .loop
.finalise:
    mov byte[r12+r15], 0
    mov rax, r12
.done:
    mov rdx, r15
    pop r15
    pop r14
    pop r13
    pop r12
    ret
.not_enough_space:
    xor rax, rax
    jmp .done

; Whether rdi is a whitespace, tab or line break.
; Results:
;   rax: 1 if true, 0 if false.
is_whitespace:
    cmp rdi, 0x20 ; space
    je .yes
    cmp rdi, 0x9 ; tab
    je .yes
    cmp rdi, 0x0a ; line break
    jne .no
.yes:
    mov rax, 1
    ret
.no:
    xor rax, rax
    ret

; Reads a negative integer from a given buffer.
;
; Arguments:
;   rdi: Address of a null-terminated string containing a negative unsigned integer.
; Results:
;   rax: Parsed number.
;   rdx: Number of characters in buffer (including sign characters).
parse_negative_int:
    inc rdi
    call parse_int
    neg rax
    inc rdx
    ret

; Reads an integer from a given buffer.
;
; Arguments:
;   rdi: Address of a null-terminated string containing an integer.
; Results:
;   rax: Parsed number.
;   rdx: Number of characters in buffer (including sign characters).
parse_int:
    cmp byte [rdi], '-'
    je parse_negative_int

; Reads an unsiged integer from a given buffer.
;
; Arguments:
;   rdi: Address of a null-terminated string containing an unsigned integer.
; Results:
;   rax: Parsed number.
;   rdx: Number of digits.
parse_uint:
    xor rax, rax
    xor r10, r10 ; digit counter
    mov r11, 10  ; divisor
.loop:
    mov cl, [rdi+r10]
    test cl, cl
    jz .done
    and cl, 0xCF ; subtract ASCII offset
    cmp cl, 9 ; check if it's a digit. If not, end here.
    ja .done
    inc r10
    mul r11
    add al, cl
    jmp .loop
.done:
    mov rdx, r10
    ret

; Arguments:
;   rdi: Address of a null-terminated string.
;   rsi: Address of a null-terminated string.
; Results:
;   rax: 1 if both strings are equal, 0 otherwise.
string_equals:
    mov al, [rdi]
    cmp al, [rsi]
    jne .not_equal
    inc rdi
    inc rsi
    test al, al
    jnz string_equals
    mov rax, 1
    ret
.not_equal:
    xor rax, rax    
    ret

; Arguments:
;   rdi: Address of a null-terminated string (input).
;   rsi: Address of a buffer (output).
;   rdx: Size of output buffer.
; Results:
;   rax: Number of bytes copied.
string_copy:
    test rdx, rdx
    jz .not_enough_space
    xor rax, rax
.loop:
    mov cl, [rdi+rax]
    mov [rsi+rax], cl
    inc rax
    cmp rax, rdx
    je .not_enough_space
    test cl, cl
    jnz .loop
    mov rax, rsi
    ret
.not_enough_space:
    xor rax, rax
    ret
