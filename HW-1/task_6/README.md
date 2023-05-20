Программа состоит из одной единицы компиляции - main.c

# Условные обозначения
Cоздание дочернего процесса process: "--- process"  
Cоздание канала pipe: "- pipe"  
Вывод содержимого текстового файла в канал: "pipe <- file"   
Вывод содержимого  канала в текстовый файл: "pipe -> file"  
Нахождении исходной последовательности из содержимого канала pipe#1 и запись результата в pipe#2: "pipe#1 ->>> pipe#2"    
Действия процесса process: "process: {...}"  
Ожидание открытия доступа к pipe для чтения : "wait pipe"  
Ожидание окончания всех дочерних процессов: "wait" 

# Схема решаемой задачи  
first_process: { - pipe_fs ;- pipe_sf ; --- second_process ; pipe_fs <- input ; wait ; pipe_sf -> output}  
second_process: { wait pipe_fs ; pipe_fs ->>> pipe_sf }  
