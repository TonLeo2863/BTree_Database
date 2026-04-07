#include <iostream>
// Nhớ bỏ comment các file .h khi bắt đầu code nhé!
// #include "BTree.h"
// #include "Record.h"
// #include "FileHandler.h"
// #include "UserInterface.h"

using namespace std;

int main() {
    int choice;
    do {
        cout << "      MENU TEST DU AN QUAN LY LINH KIEN\n";
        cout << "1. Test chuc nang B-Tree (Thanh vien 1 & 2)\n";
        cout << "2. Test chuc nang Database/Record (Thanh vien 3)\n";
        cout << "3. Test chuc nang File I/O (Thanh vien 4)\n";
        cout << "4. Test chuc nang UI Console (Thanh vien 5)\n";
        cout << "0. Thoat\n";
        cout << "Chon module de test: ";
        cin >> choice;

        switch (choice) {
        case 1: {
            cout << "\n--- Dang chay Test B-Tree ---\n";
            // Thành viên 1 & 2 viết code test của mình ở đây
            // Ví dụ:
            // BTree myTree;
            // myTree.insert(101, ...);
            break;
        }
        case 2: {
            cout << "\n--- Dang chay Test Database ---\n";
            // Thành viên 3 viết code test cấu trúc HardwareRecord ở đây
            break;
        }
        case 3: {
            cout << "\n--- Dang chay Test File I/O ---\n";
            // Thành viên 4 viết code test dọc/ghi file .csv ở đây
            break;
        }
        case 4: {
            cout << "\n--- Dang chay Test UI Console ---\n";
            // Thành viên 5 viết code test giao diện ở đây
            break;
        }
        case 0:
            cout << "\nThoat chuong trinh test...\n";
            break;
        default:
            cout << "\nLua chon khong hop le. Vui long chon lai!\n";
        }
    } while (choice != 0);

    return 0;
}