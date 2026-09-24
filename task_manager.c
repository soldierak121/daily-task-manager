/*
 * Daily Task Manager
 * Data Structures PBL Project (BCS-301)
 * Language: C
 *
 * Data structures (all written from scratch):
 *   1. Singly linked list (with tail pointer) - stores all tasks
 *   2. Min-heap                               - finds the most urgent tasks
 *   3. Stack (linked)                         - undo last delete
 *
 * Tasks are saved to tasks.txt and loaded on the next run.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define FILE_NAME "tasks.txt"
#define MAX_TITLE 99
#define MAX_LINE  256

/* ================= Task (node of the linked list) ================= */

typedef struct Task {
    int id;
    char title[MAX_TITLE + 1];
    int priority;              /* 1 = High, 2 = Medium, 3 = Low */
    char dueDate[11];          /* format YYYY-MM-DD */
    int completed;             /* 0 = pending, 1 = completed */
    struct Task *next;
} Task;

Task *head = NULL;   /* start of the linked list */
Task *tail = NULL;   /* tail pointer: makes append O(1) */


/* ================= Undo stack (linked) ================= */

typedef struct StackNode {
    Task data;
    struct StackNode *next;
} StackNode;

StackNode *undoTop = NULL;


/* ================= Helper functions ================= */

const char *priorityName(int p) {
    if (p == 1) return "High";
    if (p == 2) return "Medium";
    return "Low";
}

void todayDate(char *out) {
    time_t now = time(NULL);
    struct tm *lt = localtime(&now);
    strftime(out, 11, "%Y-%m-%d", lt);
}

/* Checks the format YYYY-MM-DD and that the date really exists */
int isValidDate(const char *s) {
    if (strlen(s) != 10 || s[4] != '-' || s[7] != '-')
        return 0;
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) continue;
        if (!isdigit((unsigned char)s[i])) return 0;
    }
    int y = atoi(s);
    int m = atoi(s + 5);
    int d = atoi(s + 8);

    if (y < 2000 || y > 2100 || m < 1 || m > 12 || d < 1)
        return 0;

    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
    int maxDay = days[m - 1];
    if (m == 2 && leap) maxDay = 29;
    return d <= maxDay;
}

/* Removes leading/trailing whitespace and the trailing newline in place */
void trim(char *s) {
    s[strcspn(s, "\n")] = '\0';
    int start = 0;
    while (isspace((unsigned char)s[start])) start++;
    if (start > 0) memmove(s, s + start, strlen(s + start) + 1);
    int len = (int)strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) s[--len] = '\0';
}

void readLine(const char *prompt, char *buf, int size) {
    printf("%s", prompt);
    if (fgets(buf, size, stdin) == NULL) {
        printf("\n");
        exit(0);
    }
    trim(buf);
}

/* Keeps asking until the user types a whole number */
int readInt(const char *prompt) {
    char buf[50];
    int v;
    char extra;
    while (1) {
        readLine(prompt, buf, sizeof(buf));
        if (sscanf(buf, "%d %c", &v, &extra) == 1)
            return v;
        printf("Please enter a valid number.\n");
    }
}

void printTask(const Task *t, const char *today) {
    printf("\nID: %d", t->id);
    printf("\nTask: %s", t->title);
    printf("\nPriority: %s", priorityName(t->priority));
    printf("\nDue Date: %s", t->dueDate);
    printf("\nStatus: %s", t->completed ? "Completed" : "Pending");
    if (!t->completed) {
        if (strcmp(t->dueDate, today) < 0)      printf("  (OVERDUE)");
        else if (strcmp(t->dueDate, today) == 0) printf("  (DUE TODAY)");
    }
    printf("\n");
}


/* ================= 1. Singly linked list ================= */

/* Search by ID. O(n) */
Task *findTask(int id) {
    Task *cur = head;
    while (cur != NULL) {
        if (cur->id == id) return cur;
        cur = cur->next;
    }
    return NULL;
}

/* Insert at the end. O(1) with the tail pointer */
void appendTask(Task t) {
    Task *node = (Task *)malloc(sizeof(Task));
    if (node == NULL) {
        printf("Memory allocation failed!\n");
        return;
    }
    *node = t;
    node->next = NULL;
    if (head == NULL) {
        head = tail = node;
    } else {
        tail->next = node;
        tail = node;
    }
}

/* Delete by ID; a copy of the deleted task is written into *removed. O(n) */
int removeTask(int id, Task *removed) {
    Task *cur = head;
    Task *prev = NULL;
    while (cur != NULL && cur->id != id) {
        prev = cur;
        cur = cur->next;
    }
    if (cur == NULL) return 0;

    *removed = *cur;
    removed->next = NULL;

    if (prev == NULL) head = cur->next;      /* deleting the first node */
    else prev->next = cur->next;             /* bypass the node */
    if (cur == tail) tail = prev;            /* deleting the last node */

    free(cur);
    return 1;
}

/* File line format: id|priority|dueDate|completed|title */
void saveTasks(void) {
    FILE *file = fopen(FILE_NAME, "w");
    if (file == NULL) {
        printf("Error saving tasks!\n");
        return;
    }
    for (Task *cur = head; cur != NULL; cur = cur->next) {
        fprintf(file, "%d|%d|%s|%d|%s\n",
                cur->id, cur->priority, cur->dueDate, cur->completed, cur->title);
    }
    fclose(file);
}

void loadTasks(void) {
    FILE *file = fopen(FILE_NAME, "r");
    if (file == NULL) return;              /* first run: no file yet */

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';

        char *p1 = strchr(line, '|');
        char *p2 = p1 ? strchr(p1 + 1, '|') : NULL;
        char *p3 = p2 ? strchr(p2 + 1, '|') : NULL;
        char *p4 = p3 ? strchr(p3 + 1, '|') : NULL;
        if (p4 == NULL) continue;          /* skip bad or old-format lines */

        *p1 = '\0'; *p2 = '\0'; *p3 = '\0'; *p4 = '\0';

        Task t;
        t.id        = atoi(line);
        t.priority  = atoi(p1 + 1);
        strncpy(t.dueDate, p2 + 1, sizeof(t.dueDate) - 1);
        t.dueDate[sizeof(t.dueDate) - 1] = '\0';
        t.completed = atoi(p3 + 1);
        strncpy(t.title, p4 + 1, sizeof(t.title) - 1);
        t.title[sizeof(t.title) - 1] = '\0';
        t.next = NULL;

        if (t.id <= 0 || t.priority < 1 || t.priority > 3 ||
            !isValidDate(t.dueDate) || strlen(t.title) == 0 || findTask(t.id) != NULL)
            continue;

        appendTask(t);
    }
    fclose(file);
}


/* ================= 2. Min-heap (most urgent task at the top) ================= */
/* Urgency order: lower priority number first, then earlier due date, then smaller ID */

typedef struct {
    Task **arr;
    int size;
    int capacity;
} MinHeap;

int moreUrgent(const Task *a, const Task *b) {
    if (a->priority != b->priority) return a->priority < b->priority;
    int cmp = strcmp(a->dueDate, b->dueDate);
    if (cmp != 0) return cmp < 0;
    return a->id < b->id;
}

void heapSwap(MinHeap *h, int i, int j) {
    Task *tmp = h->arr[i];
    h->arr[i] = h->arr[j];
    h->arr[j] = tmp;
}

void siftUp(MinHeap *h, int i) {
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (moreUrgent(h->arr[i], h->arr[parent])) {
            heapSwap(h, i, parent);
            i = parent;
        } else break;
    }
}

void siftDown(MinHeap *h, int i) {
    while (1) {
        int left = 2 * i + 1, right = 2 * i + 2, smallest = i;
        if (left < h->size && moreUrgent(h->arr[left], h->arr[smallest]))   smallest = left;
        if (right < h->size && moreUrgent(h->arr[right], h->arr[smallest])) smallest = right;
        if (smallest == i) break;
        heapSwap(h, i, smallest);
        i = smallest;
    }
}

MinHeap *createHeap(int capacity) {
    MinHeap *h = (MinHeap *)malloc(sizeof(MinHeap));
    h->arr = (Task **)malloc(sizeof(Task *) * (capacity > 0 ? capacity : 1));
    h->size = 0;
    h->capacity = capacity > 0 ? capacity : 1;
    return h;
}

void freeHeap(MinHeap *h) {
    free(h->arr);
    free(h);
}

/* O(log n) */
void heapInsert(MinHeap *h, Task *t) {
    if (h->size >= h->capacity) return;
    h->arr[h->size] = t;
    siftUp(h, h->size);
    h->size++;
}

/* Remove and return the most urgent task. O(log n) */
Task *heapExtractMin(MinHeap *h) {
    if (h->size == 0) return NULL;
    Task *top = h->arr[0];
    h->size--;
    h->arr[0] = h->arr[h->size];
    siftDown(h, 0);
    return top;
}


/* ================= 3. Undo stack ================= */

/* O(1) */
void stackPush(Task t) {
    StackNode *n = (StackNode *)malloc(sizeof(StackNode));
    n->data = t;
    n->data.next = NULL;
    n->next = undoTop;
    undoTop = n;
}

/* Look at the top without removing it. O(1) */
int stackPeek(Task *out) {
    if (undoTop == NULL) return 0;
    *out = undoTop->data;
    return 1;
}

/* Remove the top. O(1) */
int stackPop(void) {
    if (undoTop == NULL) return 0;
    StackNode *n = undoTop;
    undoTop = undoTop->next;
    free(n);
    return 1;
}


/* ================= Menu operations ================= */

void addTask(void) {
    int id = readInt("\nEnter Task ID: ");
    if (id <= 0) {
        printf("Task ID must be a positive number!\n");
        return;
    }
    if (findTask(id) != NULL) {
        printf("A task with this ID already exists!\n");
        return;
    }

    Task t;
    t.id = id;
    t.completed = 0;
    t.next = NULL;

    readLine("Enter Task: ", t.title, sizeof(t.title));
    if (strlen(t.title) == 0) {
        printf("Task cannot be empty!\n");
        return;
    }

    while (1) {
        t.priority = readInt("Enter Priority (1 = High, 2 = Medium, 3 = Low): ");
        if (t.priority >= 1 && t.priority <= 3) break;
        printf("Priority must be 1, 2 or 3.\n");
    }

    while (1) {
        readLine("Enter Due Date (YYYY-MM-DD): ", t.dueDate, sizeof(t.dueDate));
        if (isValidDate(t.dueDate)) break;
        printf("Invalid date. Example: 2026-10-05\n");
    }

    appendTask(t);
    saveTasks();
    printf("Task added successfully!\n");
}

void viewTasks(void) {
    if (head == NULL) {
        printf("\nNo tasks found.\n");
        return;
    }
    char today[11];
    todayDate(today);
    printf("\n========== DAILY TASK LIST ==========\n");
    for (Task *cur = head; cur != NULL; cur = cur->next)
        printTask(cur, today);
}

void completeTask(void) {
    int id = readInt("\nEnter Task ID to mark as completed: ");
    Task *t = findTask(id);
    if (t == NULL) {
        printf("Task ID not found!\n");
        return;
    }
    t->completed = 1;
    saveTasks();
    printf("Task marked as completed!\n");
}

void deleteTask(void) {
    int id = readInt("\nEnter Task ID to delete: ");
    Task removed;
    if (!removeTask(id, &removed)) {
        printf("Task ID not found!\n");
        return;
    }
    stackPush(removed);         /* remember it so it can be undone */
    saveTasks();
    printf("Task deleted! (Use option 5 to undo.)\n");
}

void undoDelete(void) {
    Task t;
    if (!stackPeek(&t)) {
        printf("\nNothing to undo.\n");
        return;
    }
    if (findTask(t.id) != NULL) {
        printf("\nCannot restore: Task ID %d is now used by another task.\n", t.id);
        return;
    }
    stackPop();
    appendTask(t);
    saveTasks();
    printf("\nRestored task ID %d: %s\n", t.id, t.title);
}

/* Build a min-heap of all pending tasks and take out the top 3 */
void showUrgentTasks(void) {
    int pending = 0;
    for (Task *cur = head; cur != NULL; cur = cur->next)
        if (!cur->completed) pending++;

    if (pending == 0) {
        printf("\nNo pending tasks. Great job!\n");
        return;
    }

    MinHeap *h = createHeap(pending);
    for (Task *cur = head; cur != NULL; cur = cur->next)
        if (!cur->completed) heapInsert(h, cur);

    char today[11];
    todayDate(today);
    printf("\n========== MOST URGENT TASKS ==========\n");
    for (int rank = 1; rank <= 3 && h->size > 0; rank++) {
        Task *t = heapExtractMin(h);
        printf("\n#%d", rank);
        printTask(t, today);
    }
    freeHeap(h);
}

/* Pending tasks that are due today or already overdue */
void showTodayTasks(void) {
    char today[11];
    todayDate(today);
    int any = 0;

    printf("\n========== TODAY'S TASKS (%s) ==========\n", today);
    for (Task *cur = head; cur != NULL; cur = cur->next) {
        if (!cur->completed && strcmp(cur->dueDate, today) <= 0) {
            printTask(cur, today);
            any = 1;
        }
    }
    if (!any)
        printf("\nNothing due today. Enjoy your day!\n");
}

void freeAll(void) {
    Task *cur = head;
    while (cur != NULL) {
        Task *nxt = cur->next;
        free(cur);
        cur = nxt;
    }
    while (stackPop()) { }
}


int main(void) {
    loadTasks();

    while (1) {
        printf("\n\n========== DAILY TASK MANAGER ==========\n");
        printf("1. Add Task\n");
        printf("2. View All Tasks\n");
        printf("3. Mark Task as Completed\n");
        printf("4. Delete Task\n");
        printf("5. Undo Last Delete\n");
        printf("6. Show Most Urgent Tasks\n");
        printf("7. Show Today's Tasks\n");
        printf("8. Exit\n\n");

        int choice = readInt("Enter your choice: ");

        switch (choice) {
            case 1: addTask();         break;
            case 2: viewTasks();       break;
            case 3: completeTask();    break;
            case 4: deleteTask();      break;
            case 5: undoDelete();      break;
            case 6: showUrgentTasks(); break;
            case 7: showTodayTasks();  break;
            case 8:
                saveTasks();
                freeAll();
                printf("\nThank you for using Task Manager!\n");
                return 0;
            default:
                printf("\nInvalid choice! Try again.\n");
        }
    }
}
