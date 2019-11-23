void not_called() {
    printf("Enjoy your shell!\n");
    system("/bin/bash");
}

void vulnerable_function(char* string) {
    char buffer[100];
    strcpy(buffer, string);
}

int main(int argc, char** argv) {
    vulnerable_function(argv[1]);
    return 0;
}

/*
$ gdb -q a.out
Reading symbols from /home/ppp/a.out...(no debugging symbols found)...done.
(gdb) disas vulnerable_function 
Dump of assembler code for function vulnerable_function:
   0x08048464 <+0>:  push   %ebp
   0x08048465 <+1>:  mov    %esp,%ebp
   0x08048467 <+3>:  sub    $0x88,%esp
   0x0804846d <+9>:  mov    0x8(%ebp),%eax
   0x08048470 <+12>: mov    %eax,0x4(%esp)
   0x08048474 <+16>: lea    -0x6c(%ebp),%eax
   0x08048477 <+19>: mov    %eax,(%esp)
   0x0804847a <+22>: call   0x8048340 <strcpy@plt>
   0x0804847f <+27>: leave  
   0x08048480 <+28>: ret   
End of assembler dump.
(gdb) print not_called
$1 = {<text variable, no debug info>} 0x8048444 <not_called>
*/