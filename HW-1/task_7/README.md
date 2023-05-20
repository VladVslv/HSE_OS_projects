Программа состоит из одной единицы компиляции - main.c

# Условные обозначения
Cоздание дочернего процесса process: "--- process"  
Cоздание именного канала pipe: "- pipe"  
Вывод содержимого текстового файла в канал: "pipe <- file"   
Вывод содержимого  канала в текстовый файл: "pipe -> file"  
Нахождении исходной последовательности из содержимого канала pipe#1 и запись результата в pipe#2: "pipe#1 ->>> pipe#2"    
Действия процесса process: "process: {...}"   
Ожидание открытия доступа к pipe для чтения : "wait pipe"

# Схема решаемой задачи  
first_process: { - pipe_f_s.fifo ; - pipe_s_f.fifo ;  --- second_process ; pipe_f_s.fifo <- input ; wait pipe_s_f.fifo ; pipe_s_f.fifo -> output}  
second_process: { wait pipe_f_s.fifo ; pipe_f_s.fifo ->>> pipe_s_f.fifo }  
 
