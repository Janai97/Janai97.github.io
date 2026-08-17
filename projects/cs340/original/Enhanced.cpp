#include <iostream>
#include <string>
#include <vector>

using namespace std;

// Struct to group client information together efficiently
struct Client {
    string name;
    int serviceChoice; // 1 = Brokerage, 2 = Retirement
};

// System credentials
const string STORED_USERNAME = "admin";
const string STORED_PASSWORD = "zPLR";

// Checks user login information securely with attempt tracking
int CheckUserPermissionAccess()
{
    string enteredUsername;
    string userPassword;

    cout << "Enter your username: ";
    cin >> enteredUsername;

    cout << "Enter your password: ";
    cin >> userPassword;

    // Validate credentials
    if (enteredUsername == STORED_USERNAME && userPassword == STORED_PASSWORD)
    {
        return 1; // Success
    }

    return 2; // Failure
}

// Displays client information using a clean tabular layout with separators
void DisplayInfo(const vector<Client>& clients)
{
    cout << "\n--------------------------------------------------------" << endl;
    cout << " No. | Client's Name       | Service Selected (1=Brokerage, 2=Retirement)" << endl;
    cout << "--------------------------------------------------------" << endl;

    for (size_t i = 0; i < clients.size(); ++i)
    {
        // Formatting output with separators and fixed spacing for neat alignment
        cout << " " << i + 1 << "   | " << clients[i].name;

        // Pad spacing dynamically based on name length to keep the pipe aligned
        int namePadding = 20 - static_cast<int>(clients[i].name.length());
        for (int p = 0; p < namePadding; ++p) cout << " ";

        cout << "| Option " << clients[i].serviceChoice << endl;
    }
    cout << "--------------------------------------------------------\n" << endl;
}

// Changes a client's service selection using index-based lookup instead of if/else chains
void ChangeCustomerChoice(vector<Client>& clients)
{
    int changechoice;
    int newservice;

    cout << "Enter the number of the client that you wish to change (1 to " << clients.size() << "): ";
    cin >> changechoice;

    // Input validation for client index
    if (cin.fail() || changechoice < 1 || changechoice > static_cast<int>(clients.size()))
    {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Invalid client number selection.\n" << endl;
        return;
    }

    cout << "Please enter the client's new service choice (1 = Brokerage, 2 = Retirement): ";
    cin >> newservice;

    // Input validation for service choice
    if (cin.fail() || (newservice != 1 && newservice != 2))
    {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Invalid service choice entered.\n" << endl;
        return;
    }

    // Direct O(1) index access avoiding redundant conditional chains
    clients[changechoice - 1].serviceChoice = newservice;
    cout << "Client service updated successfully!\n" << endl;
}

// Main program flow
int main()
{
    cout << "Created by Janai Williams" << endl;
    cout << "Hello! Welcome to our Investment Company Client Manager\n" << endl;

    // Load initial client data into a structured vector format
    vector<Client> clients = {
        {"Bob Jones", 1},
        {"Sarah Davis", 2},
        {"Amy Friendly", 1},
        {"Johnny Smith", 1},
        {"Carol Spears", 2}
    };

    int loginAttempts = 0;
    int authResult = 2;

    // Authentication Loop with security attempt restrictions
    do
    {
        authResult = CheckUserPermissionAccess();
        loginAttempts++;

        if (authResult != 1)
        {
            cout << "Invalid credentials. Please try again." << endl;
        }

        if (loginAttempts >= 3 && authResult != 1)
        {
            cout << "Too many failed login attempts. Security lockout engaged. Program ending." << endl;
            return 0;
        }

    } while (authResult != 1);

    cout << "\nAccess Granted. Welcome, Administrator.\n" << endl;

    int menuChoice = 0;

    // Main Menu Loop
    do
    {
        cout << "What would you like to do?" << endl;
        cout << "1. DISPLAY the client list" << endl;
        cout << "2. CHANGE a client's choice" << endl;
        cout << "3. Exit the program" << endl;
        cout << "Enter selection: ";

        cin >> menuChoice;

        // Handle non-integer input errors
        if (cin.fail())
        {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid input type. Please enter a number.\n" << endl;
            continue;
        }

        if (menuChoice < 1 || menuChoice > 3)
        {
            cout << "Invalid menu option. Choose between 1 and 3.\n" << endl;
            continue;
        }

        if (menuChoice == 1)
        {
            DisplayInfo(clients);
        }
        else if (menuChoice == 2)
        {
            ChangeCustomerChoice(clients);
        }
        else if (menuChoice == 3)
        {
            cout << "Exiting program. Have a great day!" << endl;
        }

    } while (menuChoice != 3);

    return 0;
}