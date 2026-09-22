/*
 * Daily Task Manager
 * Data Structures PBL Project (BCS-301)
 *
 * Data structures (all written from scratch, no STL containers):
 *   1. Singly linked list (with tail pointer) - stores all tasks
 *   2. Min-heap                               - finds the most urgent tasks
 *   3. Stack (linked)                         - undo last delete
 *
 * Tasks are saved to tasks.txt and loaded on the next run.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <ctime>

using namespace std;

const char *FILE_NAME = "tasks.txt";
const int MAX_TITLE = 99;


/* ================= Task (node of the linked list) ================= */

struct Task {
    int id;
    string title;
    int priority;       // 1 = High, 2 = Medium, 3 = Low
    string dueDate;     // format YYYY-MM-DD
    bool completed;
    Task *next;
};


/* ================= Helper functions ================= */

string priorityName(int p) {
    if (p == 1) return "High";
    if (p == 2) return "Medium";
    return "Low";
}

string todayDate() {
    time_t now = time(NULL);
    tm *lt = localtime(&now);
    char buf[16];
    strftime(buf, sizeof(buf), "%Y-%m-%d", lt);
    return string(buf);
}

/* Checks the format YYYY-MM-DD and that the date really exists */
bool isValidDate(const string &s) {
    if (s.size() != 10 || s[4] != '-' || s[7] != '-')
        return false;
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) continue;
        if (!isdigit((unsigned char)s[i])) return false;
    }
    int y = atoi(s.substr(0, 4).c_str());
    int m = atoi(s.substr(5, 2).c_str());
    int d = atoi(s.substr(8, 2).c_str());

    if (y < 2000 || y > 2100 || m < 1 || m > 12 || d < 1)
        return false;

    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
    int maxDay = days[m - 1];
    if (m == 2 && leap) maxDay = 29;
    return d <= maxDay;
}

string trim(const string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

string readLine(const string &prompt) {
    string s;
    cout << prompt;
    if (!getline(cin, s)) {         // input ended (Ctrl+D)
        cout << "\n";
        exit(0);
    }
    return s;
}

/* Keeps asking until the user types a whole number */
int readInt(const string &prompt) {
    while (true) {
        string s = readLine(prompt);
        int v;
        char extra;
        if (sscanf(s.c_str(), "%d %c", &v, &extra) == 1)
            return v;
        cout << "Please enter a valid number.\n";
    }
}

void printTask(const Task *t, const string &today) {
    cout << "\nID: " << t->id;
    cout << "\nTask: " << t->title;
    cout << "\nPriority: " << priorityName(t->priority);
    cout << "\nDue Date: " << t->dueDate;
    cout << "\nStatus: " << (t->completed ? "Completed" : "Pending");
    if (!t->completed) {
        if (t->dueDate < today)       cout << "  (OVERDUE)";
        else if (t->dueDate == today) cout << "  (DUE TODAY)";
    }
    cout << "\n";
}


/* ================= 1. Singly linked list ================= */

class TaskList {
private:
    Task *head;
    Task *tail;     // tail pointer makes insertion at the end O(1)

public:
    TaskList() : head(NULL), tail(NULL) {}

    ~TaskList() {
        Task *cur = head;
        while (cur != NULL) {
            Task *nxt = cur->next;
            delete cur;
            cur = nxt;
        }
    }

    Task *first() const { return head; }

    /* Search by ID. O(n) */
    Task *find(int id) const {
        Task *cur = head;
        while (cur != NULL) {
            if (cur->id == id) return cur;
            cur = cur->next;
        }
        return NULL;
    }

    int count() const {
        int n = 0;
        for (Task *cur = head; cur != NULL; cur = cur->next) n++;
        return n;
    }

    /* Insert at the end. O(1) */
    void append(const Task &t) {
        Task *node = new Task(t);
        node->next = NULL;
        if (head == NULL) {
            head = tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
    }

    /* Delete by ID; a copy of the deleted task is returned in 'removed'. O(n) */
    bool remove(int id, Task &removed) {
        Task *cur = head;
        Task *prev = NULL;
        while (cur != NULL && cur->id != id) {
            prev = cur;
            cur = cur->next;
        }
        if (cur == NULL) return false;

        removed = *cur;
        removed.next = NULL;

        if (prev == NULL) head = cur->next;     // deleting the first node
        else prev->next = cur->next;            // bypass the node
        if (cur == tail) tail = prev;           // deleting the last node

        delete cur;
        return true;
    }

    /* File line format: id|priority|dueDate|completed|title */
    void save() const {
        ofstream out(FILE_NAME);
        if (!out) {
            cout << "Error saving tasks!\n";
            return;
        }
        for (Task *cur = head; cur != NULL; cur = cur->next) {
            out << cur->id << '|' << cur->priority << '|' << cur->dueDate
                << '|' << (cur->completed ? 1 : 0) << '|' << cur->title << '\n';
        }
    }

    void load() {
        ifstream in(FILE_NAME);
        if (!in) return;                    // first run: no file yet

        string line;
        while (getline(in, line)) {
            size_t p1 = line.find('|');
            size_t p2 = (p1 == string::npos) ? p1 : line.find('|', p1 + 1);
            size_t p3 = (p2 == string::npos) ? p2 : line.find('|', p2 + 1);
            size_t p4 = (p3 == string::npos) ? p3 : line.find('|', p3 + 1);
            if (p4 == string::npos) continue;           // skip bad or old-format lines

            Task t;
            t.id        = atoi(line.substr(0, p1).c_str());
            t.priority  = atoi(line.substr(p1 + 1, p2 - p1 - 1).c_str());
            t.dueDate   = line.substr(p2 + 1, p3 - p2 - 1);
            t.completed = atoi(line.substr(p3 + 1, p4 - p3 - 1).c_str()) == 1;
            t.title     = line.substr(p4 + 1);              // title may contain '|'
            t.next      = NULL;

            if (t.id <= 0 || t.priority < 1 || t.priority > 3 ||
                !isValidDate(t.dueDate) || t.title.empty() || find(t.id) != NULL)
                continue;

            append(t);
        }
    }
};


/* ================= 2. Min-heap (most urgent task at the top) ================= */
/* Urgency order: lower priority number first, then earlier due date, then smaller ID */

class MinHeap {
private:
    Task **arr;
    int size;
    int capacity;

    static bool moreUrgent(const Task *a, const Task *b) {
        if (a->priority != b->priority) return a->priority < b->priority;
        if (a->dueDate != b->dueDate)   return a->dueDate < b->dueDate;
        return a->id < b->id;
    }

    void swapNodes(int i, int j) {
        Task *tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }

    void siftUp(int i) {
        while (i > 0) {
            int parent = (i - 1) / 2;
            if (moreUrgent(arr[i], arr[parent])) {
                swapNodes(i, parent);
                i = parent;
            } else {
                break;
            }
        }
    }

    void siftDown(int i) {
        while (true) {
            int left = 2 * i + 1;
            int right = 2 * i + 2;
            int smallest = i;
            if (left < size && moreUrgent(arr[left], arr[smallest]))   smallest = left;
            if (right < size && moreUrgent(arr[right], arr[smallest])) smallest = right;
            if (smallest == i) break;
            swapNodes(i, smallest);
            i = smallest;
        }
    }

public:
    MinHeap(int cap) : size(0), capacity(cap > 0 ? cap : 1) {
        arr = new Task *[capacity];
    }

    ~MinHeap() { delete[] arr; }

    bool empty() const { return size == 0; }

    /* O(log n) */
    void insert(Task *t) {
        if (size >= capacity) return;
        arr[size] = t;
        siftUp(size);
        size++;
    }

    /* Remove and return the most urgent task. O(log n) */
    Task *extractMin() {
        if (size == 0) return NULL;
        Task *top = arr[0];
        size--;
        arr[0] = arr[size];
        siftDown(0);
        return top;
    }
};


/* ================= 3. Stack (undo last delete) ================= */

struct StackNode {
    Task data;
    StackNode *next;
};

class UndoStack {
private:
    StackNode *top;

public:
    UndoStack() : top(NULL) {}

    ~UndoStack() {
        while (top != NULL) {
            StackNode *nxt = top->next;
            delete top;
            top = nxt;
        }
    }

    /* O(1) */
    void push(const Task &t) {
        StackNode *n = new StackNode;
        n->data = t;
        n->data.next = NULL;
        n->next = top;
        top = n;
    }

    /* Look at the top without removing it. O(1) */
    bool peek(Task &out) const {
        if (top == NULL) return false;
        out = top->data;
        return true;
    }

    /* Remove the top. O(1) */
    bool pop() {
        if (top == NULL) return false;
        StackNode *n = top;
        top = top->next;
        delete n;
        return true;
    }
};


/* ================= Menu operations ================= */

void addTask(TaskList &list) {
    int id = readInt("\nEnter Task ID: ");
    if (id <= 0) {
        cout << "Task ID must be a positive number!\n";
        return;
    }
    if (list.find(id) != NULL) {
        cout << "A task with this ID already exists!\n";
        return;
    }

    Task t;
    t.id = id;
    t.next = NULL;
    t.completed = false;

    t.title = trim(readLine("Enter Task: "));
    if (t.title.empty()) {
        cout << "Task cannot be empty!\n";
        return;
    }
    if ((int)t.title.size() > MAX_TITLE) {
        cout << "Task is too long (maximum " << MAX_TITLE << " characters)!\n";
        return;
    }

    while (true) {
        t.priority = readInt("Enter Priority (1 = High, 2 = Medium, 3 = Low): ");
        if (t.priority >= 1 && t.priority <= 3) break;
        cout << "Priority must be 1, 2 or 3.\n";
    }

    while (true) {
        t.dueDate = trim(readLine("Enter Due Date (YYYY-MM-DD): "));
        if (isValidDate(t.dueDate)) break;
        cout << "Invalid date. Example: 2026-10-05\n";
    }

    list.append(t);
    list.save();
    cout << "Task added successfully!\n";
}

void viewTasks(const TaskList &list) {
    if (list.first() == NULL) {
        cout << "\nNo tasks found.\n";
        return;
    }
    string today = todayDate();
    cout << "\n========== DAILY TASK LIST ==========\n";
    for (Task *cur = list.first(); cur != NULL; cur = cur->next)
        printTask(cur, today);
}

void completeTask(TaskList &list) {
    int id = readInt("\nEnter Task ID to mark as completed: ");
    Task *t = list.find(id);
    if (t == NULL) {
        cout << "Task ID not found!\n";
        return;
    }
    t->completed = true;
    list.save();
    cout << "Task marked as completed!\n";
}

void deleteTask(TaskList &list, UndoStack &undo) {
    int id = readInt("\nEnter Task ID to delete: ");
    Task removed = Task();
    if (!list.remove(id, removed)) {
        cout << "Task ID not found!\n";
        return;
    }
    undo.push(removed);         // remember it so it can be undone
    list.save();
    cout << "Task deleted! (Use option 5 to undo.)\n";
}

void undoDelete(TaskList &list, UndoStack &undo) {
    Task t = Task();
    if (!undo.peek(t)) {
        cout << "\nNothing to undo.\n";
        return;
    }
    if (list.find(t.id) != NULL) {
        cout << "\nCannot restore: Task ID " << t.id
             << " is now used by another task.\n";
        return;
    }
    undo.pop();
    list.append(t);
    list.save();
    cout << "\nRestored task ID " << t.id << ": " << t.title << "\n";
}

/* Build a min-heap of all pending tasks and take out the top 3 */
void showUrgentTasks(const TaskList &list) {
    int pending = 0;
    for (Task *cur = list.first(); cur != NULL; cur = cur->next)
        if (!cur->completed) pending++;

    if (pending == 0) {
        cout << "\nNo pending tasks. Great job!\n";
        return;
    }

    MinHeap heap(pending);
    for (Task *cur = list.first(); cur != NULL; cur = cur->next)
        if (!cur->completed) heap.insert(cur);

    string today = todayDate();
    cout << "\n========== MOST URGENT TASKS ==========\n";
    for (int rank = 1; rank <= 3 && !heap.empty(); rank++) {
        Task *t = heap.extractMin();
        cout << "\n#" << rank;
        printTask(t, today);
    }
}

/* Pending tasks that are due today or already overdue */
void showTodayTasks(const TaskList &list) {
    string today = todayDate();
    bool any = false;

    cout << "\n========== TODAY'S TASKS (" << today << ") ==========\n";
    for (Task *cur = list.first(); cur != NULL; cur = cur->next) {
        if (!cur->completed && cur->dueDate <= today) {
            printTask(cur, today);
            any = true;
        }
    }
    if (!any)
        cout << "\nNothing due today. Enjoy your day!\n";
}


int main() {
    TaskList list;
    UndoStack undo;
    list.load();

    while (true) {
        cout << "\n\n========== DAILY TASK MANAGER ==========\n";
        cout << "1. Add Task\n";
        cout << "2. View All Tasks\n";
        cout << "3. Mark Task as Completed\n";
        cout << "4. Delete Task\n";
        cout << "5. Undo Last Delete\n";
        cout << "6. Show Most Urgent Tasks\n";
        cout << "7. Show Today's Tasks\n";
        cout << "8. Exit\n\n";

        int choice = readInt("Enter your choice: ");

        switch (choice) {
            case 1: addTask(list);            break;
            case 2: viewTasks(list);          break;
            case 3: completeTask(list);       break;
            case 4: deleteTask(list, undo);   break;
            case 5: undoDelete(list, undo);   break;
            case 6: showUrgentTasks(list);    break;
            case 7: showTodayTasks(list);     break;
            case 8:
                list.save();
                cout << "\nThank you for using Task Manager!\n";
                return 0;
            default:
                cout << "\nInvalid choice! Try again.\n";
        }
    }
}
