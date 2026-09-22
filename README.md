

Readme · MD
Daily Task Manager
A console-based daily task manager written in C++, built as a Data Structures mini project (AKTU B.Tech CSE, 2nd year, 3rd semester, BCS-301).

Features
Add a task with a unique ID, priority (High / Medium / Low), and due date
View all tasks, with overdue and due-today tasks flagged automatically
Show the most urgent pending tasks, ranked by priority and due date
Show today's tasks (due today or overdue)
Mark a task as completed
Delete a task, with a one-step Undo Last Delete
Tasks are saved to tasks.txt and loaded automatically on the next run
Data Structures Used
Three data structures, each written from scratch (no STL containers):

Data Structure	Used For	Key Operations
Singly linked list (with tail pointer)	Storing all tasks	Insert O(1), search/delete O(n)
Min-heap	Finding the most urgent tasks	Insert and extract in O(log n)
Stack (linked)	Undoing the last delete	Push and pop in O(1)
Tasks are loaded from the file into the linked list at startup, and the list is saved back to the file after every change.

How to Run
g++ task_manager.cpp -o task_manager
./task_manager
Project Report
A full PBL report covering the problem statement, methodology, implementation, and test results is prepared separately and submitted to the course faculty.

Future Scope
Multi-level undo history
Recurring/daily-repeating tasks using a queue
A graphical or mobile interface
Reminder notifications

