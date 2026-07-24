#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    int id;
    char name[32];
    float salary;
} Employee;

void create_file(const char *path) {
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) { perror("open"); exit(1); }

    Employee e1 = {101, "Alice", 50000};
    Employee e2 = {102, "Bob", 60000};
    Employee e3 = {103, "Charlie", 55000};

    write(fd, &e1, sizeof(Employee));
    write(fd, &e2, sizeof(Employee));
    write(fd, &e3, sizeof(Employee));

    close(fd);
    printf("Created file with 3 employee records\n");
}

void update_record(const char *path, int id, float new_salary) {
    int fd = open(path, O_RDWR);
    if (fd < 0) { perror("open"); exit(1); }

    Employee emp;
    int found = 0;
    while (read(fd, &emp, sizeof(Employee)) == sizeof(Employee)) {
        if (emp.id == id) {
            off_t pos = lseek(fd, -sizeof(Employee), SEEK_CUR);
            emp.salary = new_salary;
            write(fd, &emp, sizeof(Employee));
            found = 1;
            printf("Updated employee %d salary to %.2f\n", id, new_salary);
            break;
        }
    }
    if (!found) printf("Employee %d not found\n", id);
    close(fd);
}

void read_record(const char *path, int index) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) { perror("open"); exit(1); }

    Employee emp;
    lseek(fd, index * sizeof(Employee), SEEK_SET);
    if (read(fd, &emp, sizeof(Employee)) == sizeof(Employee))
        printf("Record %d: ID=%d Name=%s Salary=%.2f\n", index, emp.id, emp.name, emp.salary);
    else
        printf("Record %d not found\n", index);

    close(fd);
}

void read_all(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) { perror("open"); exit(1); }

    Employee emp;
    int i = 0;
    while (read(fd, &emp, sizeof(Employee)) == sizeof(Employee)) {
        printf("Record %d: ID=%d Name=%s Salary=%.2f\n", i++, emp.id, emp.name, emp.salary);
    }
    close(fd);
}

int main() {
    const char *file = "employees.dat";

    create_file(file);
    read_all(file);

    printf("\n--- Updating Bob's salary ---\n");
    update_record(file, 102, 75000);

    printf("\n--- Reading all records after update ---\n");
    read_all(file);

    printf("\n--- Reading record at index 2 directly ---\n");
    read_record(file, 2);

    remove(file);
    return 0;
}