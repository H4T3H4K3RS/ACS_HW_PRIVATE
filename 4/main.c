#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>

// Буфер 5000 - по условию
const int buf_size = 5000;

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Command have to be: main <input> <output>\n");
        exit(0);
    }
    int input_file_d;
    int output_file_d;
    int fork_result;
    int pipe_1[2];
    int pipe_2[2];
    char buffer[buf_size];
    ssize_t read_bytes;
    ssize_t written_bytes;

    // Создаем неименнованные каналы
    if (pipe(pipe_1) < 0) {
        printf("pipe_1: Can\'t open pipe\n");
        exit(-1);
    }
    if (pipe(pipe_2) < 0) {
        printf("pipe_2: Can\'t open pipe\n");
        exit(-1);
    }

    // Создаем дочерние процессы
    fork_result = fork();
    if (fork_result < 0) {
        printf("fork: Can\'t fork child\n");
        exit(-1);
    } else if (fork_result > 0) {
        // Закрываем ненужные неименнованные каналы
        close(pipe_1[0]);
        close(pipe_2[0]);
        close(pipe_2[1]);
        // Читаем файл
        input_file_d = open(argv[1], O_RDONLY);
        if (input_file_d < 0) {
            printf("process_1: Can\'t open file\n");
            exit(-1);
        }
        read_bytes = read(input_file_d, buffer, buf_size);
        if (read_bytes > 0) {
            // Отправляем 2 пайпу
            written_bytes = write(pipe_1[1], buffer, read_bytes);
        }
        printf("process_1: Complete\n");
        close(input_file_d);
        close(pipe_1[1]);
        printf("process_1: Exit\n");
    } else {
        fork_result = fork();
        if (fork_result < 0) {
            printf("fork: Can\'t fork child\n");
            exit(-1);
        } else if (fork_result > 0) {
            // Закрываем ненужные неименнованные каналы
            close(pipe_1[1]);
            close(pipe_2[0]);
            // Получаем данные от первого пайпа
            read_bytes = read(pipe_1[0], buffer, buf_size);

            // Алгоритм
            int n = 4;
            char ans[n];
            int current, l, r;
            // Алгоритм
            for (int i = read_bytes - 1; i >= n; --i)
            {
                int flag = 1;
                ans[i] = buffer[i];
                for (int j = i - 1; j <= i - n; j--) {
                    if (buffer[j] > buffer[j + 1]) {
                        flag = 0;
                        break;
                    }
                    ans[j] = buffer[j];
                }
                if (flag) {
                    l = i - n;
                    r = i;
                    break;
                }

            }
            // Отправляем данные
            written_bytes = write(pipe_2[1], ans, read_bytes);
            if (read_bytes > 0) {
                // Отправляем данные 3 пайпу
                written_bytes = write(pipe_2[0], ans, read_bytes);
            }
            printf("process_2: Complete\n");
            close(pipe_2[1]);
            close(pipe_1[0]);
            printf("process_2: Exit\n");
        } else {
            // Закрываем ненужные неименнованные каналы
            close(pipe_1[0]);
            close(pipe_1[1]);
            close(pipe_2[1]);
            output_file_d = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
            if (output_file_d < 0) {
                printf("process_3: Can\'t create file\n");
                exit(-1);
            }
            // Получаем данные от 2 пайпа
            read_bytes = read(pipe_2[0], buffer, buf_size);
            // Записываем данные в файл
            written_bytes = write(output_file_d, buffer, read_bytes);
            printf("process_3: Complete\n");
            close(output_file_d);
            close(pipe_2[0]);
            printf("process_3: Exit\n");
        }
    }
    return 0;
}