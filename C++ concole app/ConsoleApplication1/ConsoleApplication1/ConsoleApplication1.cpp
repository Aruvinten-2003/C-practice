#include <iostream>
#include <string>  
using namespace std;  

class Student {
private:
    string name;
    int age;
    double mark1;
    double mark2;
    double mark3;

public:
    // Constructor
    Student(string n, int a, double m1, double m2, double m3) {
        setName(n);
        setAge(a);
        setMark1(m1);
        setMark2(m2);
        setMark3(m3);
    }

    // Setters
    void setName(string n) {
        name = n;
    }

    void setAge(int a) {
        if (a > 0) {
            age = a;
        }
        else {
            age = 1;
        }
      
    }

    void setMark1(double m1) {
        if (m1 >= 0 && m1 <= 100) {
            mark1 = m1;
        }
        else {
            mark1 = 0;
        }
    }

    void setMark2(double m2) {
        if (m2 >= 0 && m2 <= 100) {
            mark2 = m2;
        }
        else {
            mark2 = 0;
        }
    }

    void setMark3(double m3) {
        if (m3 >= 0 && m3 <= 100) {
            mark3 = m3;
        }
        else {
            mark3 = 0;
        }
    }

    // Getters
    string getName() {
        return name;
    }

    int getAge() {
        return age;
    }

    double getMark1() {
        return mark1;
    }

    double getMark2() {
        return mark2;
    }

    double getMark3() {
        return mark3;
    }

    // Methods
    double calculateTotal() {
        return mark1 + mark2 + mark3;
    }

    double calculateAverage() {
        return calculateTotal() / 3;
    }

    string getGrade() {
        double average = calculateAverage();

        if (average >= 80) {
            return "A";
        }
        else if (average >= 70) {
            return "B";
        }
        else if (average >= 60) {
            return "C";
        }
        else if (average >= 50) {
            return "D";
        }
        else {
            return "F";
        }
    }

    void displayStudent() {
        cout << "\n===== Student Details =====" << endl;
        cout << "Name: " << name << endl;
        cout << "Age: " << age << endl;
        cout << "Mark 1: " << mark1 << endl;
        cout << "Mark 2: " << mark2 << endl;
        cout << "Mark 3: " << mark3 << endl;
        cout << "Total Marks: " << calculateTotal() << endl;
        cout << "Average Marks: " << calculateAverage() << endl;
        cout << "Grade: " << getGrade() << endl;
    }
};

int main() {
    string name;
    int age;
    double mark1, mark2, mark3;
    int choice;

    cout << "===== Student Result Management System =====" << endl;

    cout << "Enter student name: ";
    getline(cin, name);

    cout << "Enter student age: ";
    cin >> age;

    cout << "Enter mark 1: ";
    cin >> mark1;

    cout << "Enter mark 2: ";
    cin >> mark2;

    cout << "Enter mark 3: ";
    cin >> mark3;

    Student student1(name, age, mark1, mark2, mark3);

    do {
        cout << "\n===== MENU =====" << endl;
        cout << "1. Display Student Details" << endl;
        cout << "2. Update Student Name" << endl;
        cout << "3. Update Student Age" << endl;
        cout << "4. Update Marks" << endl;
        cout << "5. Display Grade Only" << endl;
        cout << "6. Exit" << endl;
        cout << "Enter your choice: ";
        cin >> choice;

        if (choice == 1) {
            student1.displayStudent();
        }
        else if (choice == 2) {
            string newName;

            cin.ignore();
            cout << "Enter new name: ";
            getline(cin, newName);

            student1.setName(newName);
            cout << "Name updated successfully." << endl;
        }
        else if (choice == 3) {
            int newAge;

            cout << "Enter new age: ";
            cin >> newAge;

            student1.setAge(newAge);
            cout << "Age updated successfully." << endl;
        }
        else if (choice == 4) {
            double newMark1, newMark2, newMark3;

            cout << "Enter new mark 1: ";
            cin >> newMark1;

            cout << "Enter new mark 2: ";
            cin >> newMark2;

            cout << "Enter new mark 3: ";
            cin >> newMark3;

            student1.setMark1(newMark1);
            student1.setMark2(newMark2);
            student1.setMark3(newMark3);

            cout << "Marks updated successfully." << endl;
        }
        else if (choice == 5) {
            cout << "Student Grade: " << student1.getGrade() << endl;
        }
        else if (choice == 6) {
            cout << "Thank you for using the system." << endl;
        }
        else {
            cout << "Invalid choice. Please try again." << endl;
        }

    } while (choice != 6);

    return 0;
}