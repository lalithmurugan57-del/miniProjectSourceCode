// Bank-account program reads a random-access file sequentially,
// updates data in the file, creates new accounts, deletes accounts,
// and exports account information to a formatted text file.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ACCOUNTS 100
#define LAST_NAME_LEN 15
#define FIRST_NAME_LEN 10

struct clientData
{
    unsigned int acctNum;              // account number
    char lastName[LAST_NAME_LEN];       // account last name
    char firstName[FIRST_NAME_LEN];     // account first name
    double balance;                    // account balance
};

// prototypes
unsigned int enterChoice(void);
void exportToTextFile(FILE *readPtr);
void listAccounts(FILE *fPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void initializeRandomAccessFile(FILE *fPtr);
long recordOffset(unsigned int account);
void clearInputBuffer(void);
int promptUnsignedInt(const char *prompt, unsigned int *value);
int promptDouble(const char *prompt, double *value);
FILE *openCreditFile(const char *fileName);

int main(void)
{
    FILE *cfPtr = openCreditFile("credit.dat");
    unsigned int choice;

    if (cfPtr == NULL)
    {
        fprintf(stderr, "Unable to open or create credit.dat\n");
        return EXIT_FAILURE;
    }

    while ((choice = enterChoice()) != 6)
    {
        switch (choice)
        {
            case 1:
                exportToTextFile(cfPtr);
                break;
            case 2:
                updateRecord(cfPtr);
                break;
            case 3:
                newRecord(cfPtr);
                break;
            case 4:
                deleteRecord(cfPtr);
                break;
            case 5:
                listAccounts(cfPtr);
                break;
            default:
                puts("Incorrect choice. Please enter a number from 1 to 6.");
                break;
        }
    }

    fclose(cfPtr);
    puts("Exiting program.");
    return EXIT_SUCCESS;
}

FILE *openCreditFile(const char *fileName)
{
    FILE *fPtr = fopen(fileName, "rb+");

    if (fPtr == NULL)
    {
        fPtr = fopen(fileName, "wb+");
        if (fPtr == NULL)
        {
            return NULL;
        }
        initializeRandomAccessFile(fPtr);
    }

    return fPtr;
}

void initializeRandomAccessFile(FILE *fPtr)
{
    struct clientData blankClient = {0, "", "", 0.0};

    rewind(fPtr);
    for (unsigned int i = 0; i < MAX_ACCOUNTS; ++i)
    {
        fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
    }
    fflush(fPtr);
    rewind(fPtr);
}

unsigned int enterChoice(void)
{
    unsigned int menuChoice;

    puts("\nTransaction Processing System");
    puts("1 - Export accounts to accounts.txt");
    puts("2 - Update an account");
    puts("3 - Create a new account");
    puts("4 - Delete an account");
    puts("5 - List all accounts");
    puts("6 - Exit");

    while (promptUnsignedInt("Enter your choice: ", &menuChoice) != 0 || menuChoice < 1 || menuChoice > 6)
    {
        puts("Invalid choice. Please select a number from 1 to 6.");
    }

    return menuChoice;
}

int promptUnsignedInt(const char *prompt, unsigned int *value)
{
    int result;

    printf("%s", prompt);
    result = scanf("%u", value);
    clearInputBuffer();
    return result == 1 ? 0 : -1;
}

int promptDouble(const char *prompt, double *value)
{
    int result;

    printf("%s", prompt);
    result = scanf("%lf", value);
    clearInputBuffer();
    return result == 1 ? 0 : -1;
}

void clearInputBuffer(void)
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
    {
    }
}

long recordOffset(unsigned int account)
{
    return (long)(account - 1) * (long)sizeof(struct clientData);
}

void exportToTextFile(FILE *readPtr)
{
    FILE *writePtr = fopen("accounts.txt", "w");
    struct clientData client = {0, "", "", 0.0};

    if (writePtr == NULL)
    {
        puts("Unable to open accounts.txt for writing.");
        return;
    }

    rewind(readPtr);
    fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

    while (fread(&client, sizeof(struct clientData), 1, readPtr) == 1)
    {
        if (client.acctNum != 0)
        {
            fprintf(writePtr, "%-6u%-16s%-11s%10.2f\n",
                    client.acctNum, client.lastName, client.firstName, client.balance);
        }
    }

    fclose(writePtr);
    puts("Export complete: accounts.txt");
}

void listAccounts(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    unsigned int count = 0;

    rewind(fPtr);
    printf("\n%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
    printf("------------------------------------------------\n");

    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1)
    {
        if (client.acctNum != 0)
        {
            printf("%-6u%-16s%-11s%10.2f\n",
                   client.acctNum, client.lastName, client.firstName, client.balance);
            ++count;
        }
    }

    if (count == 0)
    {
        puts("No active accounts found.");
    }
}

void updateRecord(FILE *fPtr)
{
    unsigned int account;
    double transaction;
    struct clientData client = {0, "", "", 0.0};

    if (promptUnsignedInt("Enter account to update (1 - 100): ", &account) != 0 || account < 1 || account > MAX_ACCOUNTS)
    {
        puts("Invalid account number.");
        return;
    }

    fseek(fPtr, recordOffset(account), SEEK_SET);
    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1 || client.acctNum == 0)
    {
        printf("Account #%u has no information.\n", account);
        return;
    }

    printf("Current record:\n%-6u%-16s%-11s%10.2f\n",
           client.acctNum, client.lastName, client.firstName, client.balance);

    if (promptDouble("Enter charge (+) or payment (-): ", &transaction) != 0)
    {
        puts("Invalid transaction amount.");
        return;
    }

    client.balance += transaction;

    fseek(fPtr, recordOffset(account), SEEK_SET);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);
    fflush(fPtr);

    printf("Updated record:\n%-6u%-16s%-11s%10.2f\n",
           client.acctNum, client.lastName, client.firstName, client.balance);
}

void deleteRecord(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    struct clientData blankClient = {0, "", "", 0.0};
    unsigned int accountNum;

    if (promptUnsignedInt("Enter account number to delete (1 - 100): ", &accountNum) != 0 || accountNum < 1 || accountNum > MAX_ACCOUNTS)
    {
        puts("Invalid account number.");
        return;
    }

    fseek(fPtr, recordOffset(accountNum), SEEK_SET);
    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1 || client.acctNum == 0)
    {
        printf("Account #%u does not exist.\n", accountNum);
        return;
    }

    fseek(fPtr, recordOffset(accountNum), SEEK_SET);
    fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
    fflush(fPtr);

    printf("Account #%u deleted.\n", accountNum);
}

void newRecord(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    unsigned int accountNum;

    if (promptUnsignedInt("Enter new account number (1 - 100): ", &accountNum) != 0 || accountNum < 1 || accountNum > MAX_ACCOUNTS)
    {
        puts("Invalid account number.");
        return;
    }

    fseek(fPtr, recordOffset(accountNum), SEEK_SET);
    if (fread(&client, sizeof(struct clientData), 1, fPtr) == 1 && client.acctNum != 0)
    {
        printf("Account #%u already contains information.\n", client.acctNum);
        return;
    }

    printf("Enter lastname, firstname, balance\n? ");
    if (scanf("%14s %9s %lf", client.lastName, client.firstName, &client.balance) != 3)
    {
        clearInputBuffer();
        puts("Invalid input. Record creation aborted.");
        return;
    }
    clearInputBuffer();

    client.acctNum = accountNum;
    fseek(fPtr, recordOffset(accountNum), SEEK_SET);
    fwrite(&client, sizeof(struct clientData), 1, fPtr);
    fflush(fPtr);

    printf("Account #%u created.\n", client.acctNum);
}
