#include <iostream>
#include <string>
using namespace std;

// User login information
// VULNERABILITY: Password is hardcoded in the program.
string storedUsername = "admin";
string password = "zPLR";

// Client names
string name1 = "Bob Jones";
string name2 = "Sarah Davis";
string name3 = "Amy Friendly";
string name4 = "Johnny Smith";
string name5 = "Carol Spears";

// Client service selections
int num1 = 1;
int num2 = 2;
int num3 = 1;
int num4 = 1;
int num5 = 2;

// Program variables
int answer;
int changechoice;
int newservice;

// Checks user login information
int CheckUserPermissionAccess()
{
    string enteredUsername;
    string userPassword;

    cout << "Enter your username: ";
    cin >> enteredUsername;

    cout << "Enter your password: ";
    cin >> userPassword;

    // FIXED: The program now checks both username and password.
    if (enteredUsername == storedUsername && userPassword.compare(password) == 0)
    {
        return 1;
    }

    return 2;
}

// Displays client information
void DisplayInfo()
{
    cout << "  Client's Name    Service Selected (1 = Brokerage, 2 = Retirement)" << endl;

    cout << name1 << " selected option " << num1 << endl;
    cout << name2 << " selected option " << num2 << endl;
    cout << name3 << " selected option " << num3 << endl;
    cout << name4 << " selected option " << num4 << endl;
    cout << name5 << " selected option " << num5 << endl;
}

// Changes a client's service selection
void ChangeCustomerChoice()
{
    cout << "Enter the number of the client that you wish to change" << endl;
    cin >> changechoice;

    // FIXED: Client number must be between 1 and 5.
    if (changechoice < 1 || changechoice > 5)
    {
        cout << "Invalid client number." << endl;
        return;
    }

    cout << "Please enter the client's new service choice (1 = Brokerage, 2 = Retirement)" << endl;
    cin >> newservice;

    // FIXED: Service choice must be either 1 or 2.
    if (newservice != 1 && newservice != 2)
    {
        cout << "Invalid service choice." << endl;
        return;
    }

    if (changechoice == 1)
        num1 = newservice;
    else if (changechoice == 2)
        num2 = newservice;
    else if (changechoice == 3)
        num3 = newservice;
    else if (changechoice == 4)
        num4 = newservice;
    else if (changechoice == 5)
        num5 = newservice;
}

// Main program
int main()
{
    cout << "Created by Janai Williams" << endl;
    cout << "Hello! Welcome to our Investment Company" << endl;

    int loginAttempts = 0;

    do
    {
        answer = CheckUserPermissionAccess();
        loginAttempts++;

        if (answer != 1)
        {
            cout << "Invalid Password. Please try again" << endl;
        }

        // FIXED: Login attempts are limited to 3.
        if (loginAttempts >= 3 && answer != 1)
        {
            cout << "Too many failed login attempts. Program ending." << endl;
            return 0;
        }

    } while (answer != 1);

    do
    {
        cout << "What would you like to do?" << endl;
        cout << "DISPLAY the client list (enter 1)" << endl;
        cout << "CHANGE a client's choice (enter 2)" << endl;
        cout << "Exit the program.. (enter 3)" << endl;

        cin >> answer;

        // FIXED: Menu choice must be 1, 2, or 3.
        if (answer < 1 || answer > 3)
        {
            cout << "Invalid menu option." << endl;
            continue;
        }

        cout << "You chose " << answer << endl;

        if (answer == 1)
        {
            DisplayInfo();
        }
        else if (answer == 2)
        {
            ChangeCustomerChoice();
        }

    } while (answer != 3);

    return 0;
}