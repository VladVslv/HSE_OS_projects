Программа состоит из двух единиц компиляции: f_main.c и s_main.c. Причем программа f_main.c должна запускаться первой.

# Условные обозначения
Cоздание дочернего процесса process: "--- process"  
Cоздание именного канала pipe: "- pipe"  
Вывод содержимого текстового файла в канал: "pipe <- file"   
Вывод содержимого  канала в текстовый файл: "pipe -> file"  
Нахождении исходной последовательности из содержимого канала pipe#1 и запись результата в pipe#2: "pipe#1 ->>> pipe#2"    
Действия процесса process: "process: {...}"   
Ожидание открытия доступа к pipe для чтения : "wait pipe"

# Схема решаемой задачи  
f_main: { - pipe_f_s_sep.fifo ; - pipe_s_f_sep.fifo ; pipe_f_s_sep.fifo <- input ; wait pipe_s_f_sep.fifo ; pipe_s_f_sep.fifo -> output}  
s_main: { wait pipe_f_s_sep.fifo ; pipe_f_s_sep.fifo ->>> pipe_s_f_sep.fifo }  
 
