; ========================================================================
;
; (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
;
; This software is provided 'as-is', without any express or implied
; warranty. In no event will the authors be held liable for any damages
; arising from the use of this software.
;
; Please see https://computerenhance.com for further information
;
; ========================================================================

; ========================================================================
; LISTING 40
; ========================================================================

bits 16

; Signed displacements
mov ax, [bx + di - 37]
mov [si - 300], cx
mov dx, [bx + 32767]
mov dx, [bx - 32767]

; Explicit sizes
mov [bp + di], byte 7
mov [di + 901], word 347

; Direct address
mov bp, [bp + 5]
mov bp, [5]
mov bx, [bx + 3458]
mov bx, [3458]

; Memory-to-accumulator test
mov ax, [16]
mov ax, [2555]
mov al, [12]
mov ah, [4]

; Accumulator-to-memory test
mov [2554], ax
mov [15], ax
mov [30], ah
mov [24], al
mov [10], ax
