#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "tasks.txt"

/* Each task is a node in a singly linked list */
struct Task {
    int id;
    char task[100];
    int completed;
    struct Task *next;
};

struct Task *head = NULL;   /* start of the list */


/* ---------- helpers ---------- */

/* Safe integer input: keeps asking until the user types a number */
int readInt(const char *prompt) {
    char buf[50];
    int val;
    while (1) {
        printf("%s", prompt);
        if (fgets(buf, sizeof(buf), stdin) == NULL)
            exit(0);
        if (sscanf(buf, "%d", &val) == 1)
            return val;
        printf("Please enter a valid number.\n");
    }
}

/* Search the list for a task by ID. O(n) */
struct Task *findTask(int id) {
    struct Task *cur = head;
    while (cur != NULL) {
        if (cur->id == id)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

/* Insert a node at the end of the list. O(n) */
void appendTask(struct Task *node) {
    node->next = NULL;
    if (head == NULL) {
        head = node;
        return;
    }
    struct Task *cur = head;
    while (cur->next != NULL)
        cur = cur->next;
    cur->next = node;
}

void freeList() {
    struct Task *cur = head;
    while (cur != NULL) {
        struct Task *next = cur->next;
        free(cur);
        cur = next;
    }
    head = NULL;
}


/* ---------- file handling ---------- */

/* Write the whole list to the file */
void saveTasks() {
    FILE *file = fopen(FILE_NAME, "w");
    if (file == NULL) {
        printf("Error saving tasks!\n");
        return;
    }
    struct Task *cur = head;
    while (cur != NULL) {
        fprintf(file, "%d|%s|%d\n", cur->id, cur->task, cur->completed);
        cur = cur->next;
    }
    fclose(file);
}

/* Read the file into the list. Line format: id|task|completed */
void loadTasks() {
    FILE *file = fopen(FILE_NAME, "r");
    if (file == NULL)
        return;                     /* first run: no file yet */

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';

        char *first = strchr(line, '|');    /* after the id        */
        char *last  = strrchr(line, '|');   /* before the status   */
        if (first == NULL || first == last)
            continue;                       /* skip bad lines */

        *first = '\0';
        *last  = '\0';

        struct Task *node = (struct Task *)malloc(sizeof(struct Task));
        if (node == NULL)
            break;

        node->id = atoi(line);
        strncpy(node->task, first + 1, sizeof(node->task) - 1);
        node->task[sizeof(node->task) - 1] = '\0';
        node->completed = atoi(last + 1);

        appendTask(node);
    }
    fclose(file);
}


/* ---------- menu operations ---------- */

void addTask() {
    int id = readInt("\nEnter Task ID: ");

    if (findTask(id) != NULL) {
        printf("A task with this ID already exists!\n");
        return;
    }

    struct Task *node = (struct Task *)malloc(sizeof(struct Task));
    if (node == NULL) {
        printf("Memory allocation failed!\n");
        return;
    }

    node->id = id;
    printf("Enter Task: ");
    fgets(node->task, sizeof(node->task), stdin);
    node->task[strcspn(node->task, "\n")] = '\0';

    if (strlen(node->task) == 0) {
        printf("Task cannot be empty!\n");
        free(node);
        return;
    }

    node->completed = 0;
    appendTask(node);
    saveTasks();
    printf("Task added successfully!\n");
}

void viewTasks() {
    if (head == NULL) {
        printf("\nNo tasks found.\n");
        return;
    }

    printf("\n========== DAILY TASK LIST ==========\n");
    struct Task *cur = head;
    while (cur != NULL) {
        printf("\nID: %d", cur->id);
        printf("\nTask: %s", cur->task);
        printf("\nStatus: %s\n", cur->completed ? "Completed" : "Pending");
        cur = cur->next;
    }
}

void completeTask() {
    int id = readInt("\nEnter Task ID to mark as completed: ");
    struct Task *t = findTask(id);

    if (t == NULL) {
        printf("Task ID not found!\n");
        return;
    }
    t->completed = 1;
    saveTasks();
    printf("Task marked as completed!\n");
}

void deleteTask() {
    int id = readInt("\nEnter Task ID to delete: ");

    struct Task *cur = head;
    struct Task *prev = NULL;

    while (cur != NULL && cur->id != id) {
        prev = cur;
        cur = cur->next;
    }

    if (cur == NULL) {
        printf("Task ID not found!\n");
        return;
    }

    if (prev == NULL)
        head = cur->next;       /* deleting the first node */
    else
        prev->next = cur->next; /* bypass the node */

    free(cur);
    saveTasks();
    printf("Task deleted!\n");
}


int main() {
    int choice;

    loadTasks();

    while (1) {
        printf("\n\n========== DAILY TASK MANAGER ==========\n");
        printf("1. Add Task\n");
        printf("2. View Tasks\n");
        printf("3. Mark Task as Completed\n");
        printf("4. Delete Task\n");
        printf("5. Exit\n\n");

        choice = readInt("Enter your choice: ");

        switch (choice) {
            case 1: addTask();      break;
            case 2: viewTasks();    break;
            case 3: completeTask(); break;
            case 4: deleteTask();   break;
            case 5:
                saveTasks();
                freeList();
                printf("\nThank you for using Task Manager!\n");
                return 0;
            default:
                printf("\nInvalid choice! Try again.\n");
        }
    }
}
