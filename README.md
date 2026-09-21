# Daily Task Manager

A console-based daily task manager written in C, built as a Data Structures mini project (AKTU B.Tech CSE, 2nd year, 3rd semester).

## Features
- Add a task with a unique ID
- View all tasks with their status (Pending / Completed)
- Mark a task as completed
- Delete a task
- Tasks are saved to `tasks.txt` and loaded automatically on the next run

## Data Structure Used
A **singly linked list**. Each task is a node holding the ID, task text, completion status, and a pointer to the next node. The list is loaded from the file at startup and saved after every change.

| Operation | Time Complexity |
|---|---|
| Search by ID | O(n) |
| Add at end | O(n) |
| Delete by ID | O(n) |

## How to Run
```
gcc task_manager.c -o task_manager
./task_manager
```

## Future Scope
- Priority and due date for each task
- Min-heap to show the most urgent task first
- Stack for undo-last-delete
