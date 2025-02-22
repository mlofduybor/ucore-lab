set confirm off
set architecture riscv:rv64
target remote 127.0.0.1:15234
symbol-file build/kernel
display/12i $pc-8
set riscv use-compressed-breakpoints yes
break *0x1000


define c
        continue
        refresh
end

define n
        next
        refresh
end

define r
        run
        refresh
end

define s
        step
        refresh
end

define si
				step
				refresh
end

define ni
				step
				refresh
end
